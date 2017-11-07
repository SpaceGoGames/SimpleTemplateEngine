// Copyright Playspace S.L. 2017

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonTypes.h"
#include "Serialization/BufferReader.h"

// Static tokens
static FString TPL_START_TOKEN(TEXT("{"));
static FString TPL_START_IF_TOKEN(TEXT("if"));
static FString TPL_END_IF_TOKEN(TEXT("endif"));
static FString TPL_START_FOR_TOKEN(TEXT("for"));
static FString TPL_END_FOR_TOKEN(TEXT("endfor"));

// The template serialization version
static uint32 TPL_VERSION = 1;

//
// Tokens
//

/** Tokens our template engine can parse */
UENUM()
enum class ETokenType : uint8
{
    None,
    Text,
    Var,
    If,
    For,
    EndIf,
    EndFor
};

class SIMPLETEMPLATE_API FToken
{
public:
	FToken(const FString& InExpression)
		: Expression(InExpression)
	{ }

    virtual ~FToken() {}

	virtual ETokenType GetType() const
	{
		return ETokenType::None;
	}
	virtual FString Build()
	{
		return FString();
	}

	// UObject interface
	virtual void Serialize(FArchive& Ar)
	{
		if (Ar.IsSaving())
		{
			ETokenType serializedType = GetType();
			Ar << serializedType;
			Ar << Expression;
		}
	}

protected:
	FString Expression;
};

typedef TSharedPtr<FToken> FTokenPtr;
typedef TArray<FTokenPtr> FTokenArray;

class SIMPLETEMPLATE_API FTokenText : public FToken
{
public:
    FTokenText(const FString& InText)
        : FToken(InText) {}

    ETokenType GetType() const override
    {
        return ETokenType::Text;
    }
};

class SIMPLETEMPLATE_API FTokenVar : public FToken
{
public:
    FTokenVar(const FString& InKey)
        : FToken(InKey) {}

	virtual FString Build() override
	{
		return FString();
	}

    ETokenType GetType() const override
    {
        return ETokenType::Var;
    }
};


class SIMPLETEMPLATE_API FTokenNested : public FToken
{

public:
	FTokenNested(const FString& InExpression)
		: FToken(InExpression) {}

public:
    //FString Expression;
	//FTokenArray Children;
};

class SIMPLETEMPLATE_API FTokenFor : public FTokenNested
{
public:
	FTokenFor(const FString& Expresion)
		: FTokenNested(Expresion)
	{
	}

	virtual FString Build() override
	{
		TArray<FString> ForValues;
		Expression.ParseIntoArray(ForValues, TEXT(" "));
		if (ForValues.Num() != 4)
		{
			return FString::Printf(TEXT("'for' token must in form of: for key in value. '%s' found instead"), *Expression);
		}

		Key = ForValues[1];
		Value = ForValues[3];
		return FString();
	}

    ETokenType GetType() const override
    {
        return ETokenType::For;
    }

public:
	FString Key;
	FString Value;
};

class SIMPLETEMPLATE_API FTokenIf : public FTokenNested
{
public:
	FTokenIf(const FString& Expresion)
		: FTokenNested(Expresion) {}

	virtual FString Build() override
	{
		TArray<FString> IfValues;
		Expression.ParseIntoArray(IfValues, TEXT(" "));

		// Parse sign

		if (IfValues.Num() < 2)
		{
			return FString::Printf(TEXT("'if' token is not a boolean operation^. '{%s}' found instead."), *Expression);
		}

		// if not var
		if (IfValues[1].Equals("not") || IfValues[1].Equals("!"))
		{
			bSign = false;
			Key = IfValues[2];
		}
		// if var
		else if (IfValues.Num() == 2)
		{
			Key = IfValues[1];
		}
		// if var == value | if var != value
		else
		{
			if (IfValues[2].Equals("==") | IfValues[2].Equals("!=") || IfValues[2].Equals("~="))
			{
				bSign = IfValues[2].Equals("==") || IfValues[2].Equals("~=");
				bIgnoreCase = IfValues[2].Equals("~=");
			}
			else
			{
				return FString::Printf(TEXT("'if' boolean operation can either be '==' or '!='. '%s' found instead."), *Expression);
			}
			
			Key = IfValues[1];
			Value = IfValues[3];
		}
		return FString();
	}

    ETokenType GetType() const override
    {
        return ETokenType::If;
    }

	bool IsTrue()
	{
		return false;
	}

public:
	uint32 bSign : 1;
	uint32 bIgnoreCase : 1;
	FString Key;
	FString Value;
};

class SIMPLETEMPLATE_API FTokenEndIf : public FTokenNested
{
public:
	FTokenEndIf(const FString& Expresion)
		: FTokenNested(Expresion) {}

	virtual FString Build() override
	{
		if (!Expression.Equals(TPL_END_IF_TOKEN, ESearchCase::IgnoreCase))
		{
			return FString::Printf(TEXT("'%s' token expected. '%s' found instead."), *TPL_END_IF_TOKEN, *Expression);
		}
		return FString();
	}

	ETokenType GetType() const override
	{
		return ETokenType::EndIf;
	}
};

class SIMPLETEMPLATE_API FTokenEndFor : public FTokenNested
{
public:
	FTokenEndFor(const FString& Expresion)
		: FTokenNested(Expresion) {}

	virtual FString Build() override
	{
		if (!Expression.Equals(TPL_END_FOR_TOKEN, ESearchCase::IgnoreCase))
		{
			return FString::Printf(TEXT("'%s' token expected. '%s' found instead."), *TPL_END_FOR_TOKEN, *Expression);
		}
		return FString();
	}

	ETokenType GetType() const override
	{
		return ETokenType::EndFor;
	}
};

//
// The template parser
//
template <class CharType = TCHAR>
class SIMPLETEMPLATE_API TTemplateTokenizer
{
public:
    static TSharedRef< TTemplateTokenizer<CharType>> Create()
    {
        return MakeShareable(new TTemplateTokenizer<CharType>());
    }

public:
    virtual ~TTemplateTokenizer() {}

	FTokenArray GetTokens()
	{
		return Tokens;
	}

	FString GetLastError()
	{
		return ErrorMessage;
	}

	uint32 GetLineNumber()
	{
		return LineNumber;
	}

	uint32 GetCharNumber()
	{
		return CharNumber;
	}

	bool Compile()
	{
		if (bHasTokens)
		{
			return true;
		}
		bHasTokens = CompileWorker();
		return bHasTokens;
	}

protected:

    /** Hidden default constructor. */
	TTemplateTokenizer()
		: ReadStream(nullptr)
		, ErrorMessage()
		, LineNumber(0)
		, CharNumber(0)
		, bHasTokens(false)
	{ }

	TTemplateTokenizer(FArchive* InStream)
		: ReadStream(InStream)
		, ErrorMessage()
		, LineNumber(0)
		, CharNumber(0)
		, bHasTokens(false)
	{ }

protected:

	// Current Stream
	FArchive* ReadStream;
    FTokenArray Tokens;
	TArray<ETokenType> ParseState;

	// Line/Char for error tracking
	FString ErrorMessage;
    uint32 LineNumber;
    uint32 CharNumber;

	// Parser state
	uint32 bHasTokens : 1;

private:

	bool CompileWorker()
	{
		if (bHasTokens)
		{
			return true;
		}

		ClearError();
		if (ReadStream == nullptr)
		{
			SetError(TEXT("Null Stream"));
			return false;
		}

		FString Buffer = "";
		while (!ReadStream->AtEnd())
		{
			// Find start token
			if (!NextStartToken(Buffer))
			{
				if (!AddToken(new FTokenText(Buffer)))
				{
					return false;
				}
				break;
			}

			// Create text token for the left part
			if (!Buffer.IsEmpty())
			{
				if (!AddToken(new FTokenText(Buffer)))
				{
					return false;
				}
			}

			// Reset buffer
			Buffer = "";

			// Read token
			CharType Char;
			if (!ReadNext(Char))
			{
				break;
			}

			if (IsVarToken(Char))
			{
				if (NextEndToken(Buffer))
				{
					Buffer.Trim();
					if (!AddToken(new FTokenVar(Buffer)))
					{
						return false;
					}
					Buffer = "";
				}
				else
				{
					SetError(TEXT("Missing end token after a var token"));
					return false;
				}
			}
			else if (IsControlToken(Char))
			{
				if (NextEndToken(Buffer))
				{
					Buffer.Trim();
					if (Buffer.StartsWith(TPL_START_FOR_TOKEN))
					{
						if (!AddToken(new FTokenFor(Buffer)))
						{
							return false;
						}
						ParseState.Push(ETokenType::EndFor);
					}
					else if (Buffer.StartsWith(TPL_START_IF_TOKEN))
					{
						if (!AddToken(new FTokenIf(Buffer)))
						{
							return false;
						}
						ParseState.Push(ETokenType::EndIf);
					}
					else
					{
						Buffer.TrimTrailing();

						// Check end token to match ParseState
						ETokenType expectedToken = ParseState.Pop();
						switch (expectedToken)
						{
						case ETokenType::EndIf:
							if (!Buffer.Equals(TPL_END_IF_TOKEN, ESearchCase::IgnoreCase))
							{
								SetError(FString::Printf(TEXT("'%s' expected. '%s' found instead."), *TPL_END_IF_TOKEN, *Buffer));
								return false;
							}
							// Add token
							if (!AddToken(new FTokenEndIf(Buffer)))
							{
								return false;
							}
							break;
						case ETokenType::EndFor:
							if (!Buffer.Equals(TPL_END_FOR_TOKEN, ESearchCase::IgnoreCase))
							{
								SetError(FString::Printf(TEXT("'%s' expected. '%s' found instead."), *TPL_END_FOR_TOKEN, *Buffer));
								return false;
							}
							// Add token
							if (!AddToken(new FTokenEndFor(Buffer)))
							{
								return false;
							}
							break;
						default:
							SetError(FString::Printf(TEXT("Unexpected token '%s'."), *Buffer));
							return false;
							break;
						}
					}
					Buffer = "";
				}
				else
				{
					SetError(TEXT("Missing '}'"));
					return false;
				}
			}
			else
			{
				if (!AddToken(new FTokenText(TPL_START_TOKEN)))
				{
					return false;
				}
			}
		}
		return true;
	}

	bool ParseTemplateWorker(FArchive* WriteStream, TSharedPtr<FJsonValue> Data)
	{
		ClearError();
		if (!bHasTokens)
		{
			SetError(TEXT("A template must be tokenized before it can be parsed. Please call Tokenize() atleast once."));
			return false;
		}
		// Do not use state-full data here!

		return true;
	}

	void SetError(const FString& Error)
	{
		ErrorMessage = FString::Printf(TEXT("[ERROR] Line: %u Ch: %u: %s"), LineNumber, CharNumber, *Error);
	}

	void ClearError()
	{
		ErrorMessage = TEXT("");
	}

	bool AddToken(FToken* token)
	{
		FString buildError = token->Build();
		if (buildError.IsEmpty())
		{
			Tokens.Add(MakeShareable(token));
			return true;
		}
		SetError(buildError);
		return false;
	}

	bool ReadNext(CharType& Char)
	{
		ReadStream->Serialize(&Char, sizeof(CharType));
		++CharNumber;
		bool readNext = !IsEOF(Char);
		if (readNext && IsLineBreak(Char))
		{
			++LineNumber;
			CharNumber = 0;
		}
		return readNext;
	}

	bool NextStartToken(FString& Text)
	{
		while (!ReadStream->AtEnd())
		{
			CharType Char;
			if (!ReadNext(Char))
			{
				break;
			}
			if (IsTokenStart(Char))
			{
				return true;
			}
			Text.AppendChar(Char);
		}
		return false;
	}

	bool NextEndToken(FString& Text)
	{
		while (!ReadStream->AtEnd())
		{
			CharType Char;
			if (!ReadNext(Char))
			{
				break;
			}
			if (IsTokenEnd(Char))
			{
				return true;
			}
			if (!IsControlToken(Char))
			{
				Text.AppendChar(Char);
			}
		}
		return false;
	}

	//void SkipWhiteSpaces()
	//{
	//	while (!ReadStream->AtEnd())
	//	{
	//		CharType Char;
	//		if (!ReadNext(Char))
	//		{
	//			break;
	//		}
	//		if (!IsWhitespace(Char))
	//		{
	//			Stream->Seek(Stream->Tell() - sizeof(CharType));
	//			--CharacterNumber;
	//			break;
	//		}
	//	}
	//}

	bool IsLineBreak(const CharType& Char)
	{
		return Char == CharType('\n');
	}
	
	bool IsEOF(const CharType& Char)
	{
		return Char == CharType('0');
	}

	bool IsTokenStart(const CharType& Char)
	{
		return Char == CharType('{');
	}

	bool IsTokenEnd(const CharType& Char)
	{
		return Char == CharType('}');
	}

	bool IsVarToken(const CharType& Char)
	{
		return Char == CharType('$');
	}

	bool IsControlToken(const CharType& Char)
	{
		return Char == CharType('%');
	}

};

class SIMPLETEMPLATE_API FStringTemplateParser
	: public TTemplateTokenizer<TCHAR>
{
public:

	static TSharedRef<FStringTemplateParser> Create(const FString& JsonString)
	{
		return MakeShareable(new FStringTemplateParser(JsonString));
	}

public:

	virtual ~FStringTemplateParser()
	{
		if (Reader != nullptr)
		{
			check(Reader->Close());
			delete Reader;
		}
	}

protected:
	FStringTemplateParser(const FString& JsonString)
		: TTemplateTokenizer<TCHAR>()
	{
		Reader = new FBufferReader((void*)*JsonString, JsonString.Len() * sizeof(TCHAR), false);
		check(Reader);
		ReadStream = Reader;
	}

protected:
	FBufferReader* Reader;
};


template <class CharType = TCHAR>
class SIMPLETEMPLATE_API TTemplateCompilerFactory
{
public:

    // Add string streams plus BP objects, then we can create a BP lib to parse stuff
	static TSharedRef<TTemplateTokenizer<CharType>> Create(const FString& JsonString)
	{
		return FStringTemplateParser::Create(JsonString);
	}

    static TSharedRef<TTemplateTokenizer<CharType>> Create(FArchive* const Stream)
    {
        return TTemplateTokenizer<CharType>::Create(Stream);
    }
};

