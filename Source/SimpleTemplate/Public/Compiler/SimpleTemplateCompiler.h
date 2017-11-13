// Copyright Playspace S.L. 2017

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonTypes.h"
#include "Serialization/BufferReader.h"
#include "Serialization/MemoryWriter.h"

// Static tokens
static FString TPL_START_TOKEN(TEXT("{"));
static FString TPL_START_IF_TOKEN(TEXT("if"));
static FString TPL_END_IF_TOKEN(TEXT("endif"));
static FString TPL_START_FOR_TOKEN(TEXT("for"));
static FString TPL_END_FOR_TOKEN(TEXT("endfor"));

// The template serialization version
static uint32 TPL_VERSION = 1;

class TTemplateCompilerHelper
{
public:
	static TSharedPtr<FJsonValue> GetValue(FString& Key, TSharedPtr<FJsonObject> Data)
	{
		if (Data.IsValid() && !Key.IsEmpty())
		{
			TArray<FString> KeyList;
			Key.ParseIntoArray(KeyList, TEXT("."), true);

			TSharedPtr<FJsonValue> CurrentValue = nullptr;
			TSharedPtr<FJsonObject> CurrentObject = Data;
			for (auto i = 0; i < KeyList.Num(); i++)
			{
				auto CurrentKey = KeyList[i];

				// Get field
				CurrentValue = CurrentObject->TryGetField(CurrentKey);
				if (!CurrentValue.IsValid())
				{
					return nullptr;
				}

				// Prepare for next field
				if (i < KeyList.Num() - 1)
				{
					// Get next
					CurrentObject = CurrentValue->AsObject();
					if (!CurrentObject.IsValid())
					{
						return nullptr;
					}
				}
			}
			return CurrentValue;
		}
		return nullptr;
	}
};

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
	FToken() {}

    virtual ~FToken() {}

	virtual ETokenType GetType() const
	{
		return ETokenType::None;
	}

#if WITH_EDITOR
	virtual FString Build()
	{
		return FString();
	}
#endif
	
	virtual void Serialize(FArchive& Ar) {}

	// Interpret the token 
	virtual void Interpret(FArchive& WriteStream, TSharedPtr<FJsonObject> Data) {}

	// Some tokens are nested
	virtual void SetChildren(TArray<TSharedPtr<FToken>>& children) {}
	virtual TArray<TSharedPtr<FToken>>* GetChildren() { return nullptr; }
};

typedef TSharedPtr<FToken> FTokenPtr;

class SIMPLETEMPLATE_API FTokenArray : public FToken
{
public:
	FTokenArray() {}
	~FTokenArray() {}

	void Serialize(FArchive& Ar);

public:
	TArray<FTokenPtr> Items;
};

class SIMPLETEMPLATE_API FTokenText : public FToken
{
public:
	FTokenText() : FToken() {}

#if WITH_EDITOR
    FTokenText(const FString& InText)
        : FToken()
		, Text(InText)
	{}
#endif

    ETokenType GetType() const override
    {
        return ETokenType::Text;
    }

	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Text;
	}

	virtual void Interpret(FArchive& WriteStream, TSharedPtr<FJsonObject> Data) override
	{
		WriteStream << Text;
	}

public:
	FString Text;
};

class SIMPLETEMPLATE_API FTokenVar : public FToken
{
public:
	FTokenVar() : FToken() {}

#if WITH_EDITOR
	FTokenVar(const FString& InKey)
		: FToken()
		, Key(InKey)
	{}

	virtual FString Build() override
	{
		return FString();
	}
#endif

	ETokenType GetType() const override
	{
		return ETokenType::Var;
	}

	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Key;
	}

	virtual void Interpret(FArchive& WriteStream, TSharedPtr<FJsonObject> Data) override
	{
		auto value = TTemplateCompilerHelper::GetValue(Key, Data);
		FString valueStr;
		if (value.IsValid() && value->TryGetString(valueStr))
		{
			WriteStream << valueStr;
		}
	}

public:
	FString Key;
};

class SIMPLETEMPLATE_API FTokenNested : public FToken
{
public:
	FTokenNested() : FToken() {}

#if WITH_EDITOR
	FTokenNested(const FString& InExpression)
		: FToken()
		, Expression(InExpression)
	{}
#endif

	virtual void Serialize(FArchive& Ar) override
	{
		Children.Serialize(Ar);
	}

	// Some tokens are nested
	virtual void SetChildren(TArray<TSharedPtr<FToken>>& children)
	{
		Children.Items = children;
	}

	virtual TArray<TSharedPtr<FToken>>* GetChildren()
	{
		return &Children.Items;
	}

public:
#if WITH_EDITOR
	FString Expression;
#endif
	FTokenArray Children;
};

class SIMPLETEMPLATE_API FTokenFor : public FTokenNested
{
public:
	FTokenFor() : FTokenNested() {}

#if WITH_EDITOR
	FTokenFor(const FString& Expresion)
		: FTokenNested(Expresion)
	{
	}

	virtual FString Build() override
	{
		TArray<FString> ForValues;
		Expression.ParseIntoArray(ForValues, TEXT(" "));
		// Expresion must be: for key in value
		if (ForValues.Num() != 4 || !ForValues[2].Equals("in"))
		{
			return FString::Printf(TEXT("'for' token must in form of: for key in value. '%s' found instead"), *Expression);
		}
		Key = ForValues[1];
		Value = ForValues[3];
		return FString();
	}
#endif

    ETokenType GetType() const override
    {
        return ETokenType::For;
    }

	virtual void Serialize(FArchive& Ar) override
	{
		FTokenNested::Serialize(Ar);
		Ar << Key;
		Ar << Value;
	}

public:
	FString Key;
	FString Value;
};

class SIMPLETEMPLATE_API FTokenIf : public FTokenNested
{
public:
	FTokenIf() : FTokenNested() {}

#if WITH_EDITOR
	FTokenIf(const FString& Expresion)
		: FTokenNested(Expresion) {}


	virtual FString Build() override
	{
		TArray<FString> IfValues;
		Expression.ParseIntoArray(IfValues, TEXT(" "));

		// Parse sign

		if (IfValues.Num() < 2)
		{
			return FString::Printf(TEXT("'if' token is not a boolean operation. '{%s}' found instead."), *Expression);
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
#endif

	virtual void Interpret(FArchive& WriteStream, TSharedPtr<FJsonObject> Data) override
	{
		if (IsTrue(Data))
		{
			for(auto child : Children.Items)
			{
				child->Interpret(WriteStream, Data);
			}
		}
	}

    ETokenType GetType() const override
    {
        return ETokenType::If;
    }

	virtual void Serialize(FArchive& Ar) override
	{
		FTokenNested::Serialize(Ar);
		bool bAux;
		Ar << bAux;
		bSign = bAux;
		Ar << bAux;
		bIgnoreCase = bAux;
		Ar << Key;
		Ar << Value;
	}

private:
	bool IsTrue(TSharedPtr<FJsonObject> Data)
	{
		// Only key provided
		if (Value.IsEmpty())
		{
			bool boolValue = false;
			auto keyDataPtr = TTemplateCompilerHelper::GetValue(Key, Data);
			return keyDataPtr.IsValid() && keyDataPtr->TryGetBool(boolValue) && (boolValue == bSign);
		}

		// Find l-value and r-value
		auto lValuePtr = TTemplateCompilerHelper::GetValue(Key, Data);
		auto rValuePtr = TTemplateCompilerHelper::GetValue(Value, Data);
		if (!rValuePtr.IsValid())
		{
			rValuePtr = MakeShareable(new FJsonValueString(Value.TrimQuotes()));
		}

		// Compare
		FString lValue;
		FString rValue;
		return lValuePtr->TryGetString(lValue) && rValuePtr->TryGetString(rValue) && (lValue.Equals(rValue, bIgnoreCase ? ESearchCase::IgnoreCase : ESearchCase::CaseSensitive) == bSign);
	}

public:
	uint32 bSign : 1;
	uint32 bIgnoreCase : 1;
	FString Key;
	FString Value;
};

class SIMPLETEMPLATE_API FTokenEnd : public FToken
{
public:
	FTokenEnd() : FToken() {}

#if WITH_EDITOR
	FTokenEnd(const FString& InExpression)
		: FToken()
		, Expression(InExpression)
	{}
#endif

public:
#if WITH_EDITOR
	FString Expression;
#endif
};

class SIMPLETEMPLATE_API FTokenEndIf : public FTokenEnd
{
public:
	FTokenEndIf() : FTokenEnd() {}

#if WITH_EDITOR
	FTokenEndIf(const FString& Expresion)
		: FTokenEnd(Expresion) {}


	virtual FString Build() override
	{
		if (!Expression.Equals(TPL_END_IF_TOKEN, ESearchCase::IgnoreCase))
		{
			return FString::Printf(TEXT("'%s' token expected. '%s' found instead."), *TPL_END_IF_TOKEN, *Expression);
		}
		return FString();
	}
#endif

	ETokenType GetType() const override
	{
		return ETokenType::EndIf;
	}
};

class SIMPLETEMPLATE_API FTokenEndFor : public FTokenEnd
{
public:
	FTokenEndFor() : FTokenEnd() {}

#if WITH_EDITOR
	FTokenEndFor(const FString& Expresion)
		: FTokenEnd(Expresion) {}

	virtual FString Build() override
	{
		if (!Expression.Equals(TPL_END_FOR_TOKEN, ESearchCase::IgnoreCase))
		{
			return FString::Printf(TEXT("'%s' token expected. '%s' found instead."), *TPL_END_FOR_TOKEN, *Expression);
		}
		return FString();
	}
#endif

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

	FTokenArray GetTokenTree()
	{
		return Tree;
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
		bHasTokens = Tokenize();
		if (bHasTokens)
		{
			Parse(Tokens, Tree, ETokenType::None);
		}
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
	FTokenArray Tree;
	TArray<ETokenType> ParseState;

	// Line/Char for error tracking
	FString ErrorMessage;
    uint32 LineNumber;
    uint32 CharNumber;

	// Parser state
	uint32 bHasTokens : 1;

private:

	// Parse the plain token list into a tree
	void Parse(FTokenArray& tokens, FTokenArray& tree, ETokenType mark)
	{
		while (Tokens.Items.Num() > 0)
		{
			auto token = Tokens.Items[0];
			Tokens.Items.RemoveAt(0);
			if (token->GetType() == ETokenType::For || token->GetType() == ETokenType::If)
			{
				FTokenArray children;
				Parse(tokens, children, token->GetType() == ETokenType::For ? ETokenType::EndFor : ETokenType::EndIf);
				token->SetChildren(children.Items);
			}
			else if (token->GetType() == mark)
			{
				return;
			}
			tree.Items.Add(token);
		}
	}

	// Tokenize the input stream
	bool Tokenize()
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

		if (ParseState.Num() > 0)
		{
			ETokenType expectedToken = ParseState.Pop();
			switch (expectedToken)
			{
			case ETokenType::EndIf:
				SetError(FString::Printf(TEXT("Missing end token '%s' at EOF"), *TPL_END_IF_TOKEN));
				break;
			case ETokenType::EndFor:
				SetError(FString::Printf(TEXT("Missing end token '%s' at EOF"), *TPL_END_FOR_TOKEN));
				break;
			default:
				SetError(FString::Printf(TEXT("Missing unexpected end token")));
				return false;
				break;
			}
			return false;
		}
		return true;
	}

	void SetError(const FString& Error)
	{
		ErrorMessage = FString::Printf(TEXT("[ERROR] Line: %u Ch: %u: %s"), LineNumber, CharNumber, *Error);
		UE_LOG(LogSTE, Error, TEXT("%s"), *ErrorMessage);
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
			Tokens.Items.Add(MakeShareable(token));
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

//
// Factory for easy access
//

class SIMPLETEMPLATE_API TTemplateInterpreter
{
public:

	static TSharedRef< TTemplateInterpreter > Create(FTokenArray& TokenTree)
	{
		return MakeShareable(new TTemplateInterpreter(TokenTree));
	}

	void Interpret(FArchive& WriteStream, TSharedPtr<FJsonObject> Data)
	{
		for (auto token : TokenTree.Items)
		{
			token->Interpret(WriteStream, Data);
		}
	}

	void Interpret(FString& OutString, TSharedPtr<FJsonObject> Data)
	{
		TArray<uint8> Bytes;
		auto WriteStream = new FMemoryWriter(Bytes);

		Interpret(*WriteStream, Data);

		FString Out;
		for (int32 i = 0; i < Bytes.Num(); i += sizeof(TCHAR))
		{
			TCHAR* Char = static_cast<TCHAR*>(static_cast<void*>(&Bytes[i]));
			Out += *Char;
		}
		OutString = Out;
		WriteStream->Close();
		delete WriteStream;
	}

protected:
	TTemplateInterpreter(FTokenArray& InTokenTree)
		: TokenTree(InTokenTree)
	{
	}

protected:
	FTokenArray const TokenTree;
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