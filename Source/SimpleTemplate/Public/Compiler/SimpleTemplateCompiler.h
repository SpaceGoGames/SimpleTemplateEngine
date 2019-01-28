// Copyright Playspace S.L. 2017

#pragma once

#include "CoreMinimal.h"
#include "ISimpleTemplate.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonTypes.h"
#include "Serialization/BufferReader.h"
#include "Serialization/MemoryWriter.h"
#include "Interfaces/SimpleTemplateDataProvider.h"

#include "SimpleTemplateCompiler.generated.h"

// Static tokens
static FString TPL_START_TOKEN(TEXT("{"));
static FString TPL_END_TOKEN(TEXT("}"));
static FString TPL_START_IF_TOKEN(TEXT("if"));
static FString TPL_END_IF_TOKEN(TEXT("endif"));
static FString TPL_START_FOR_TOKEN(TEXT("for"));
static FString TPL_END_FOR_TOKEN(TEXT("endfor"));

// The template serialization version
// 1: Initial version
// 2: If token changed it's bool values from uint32 with pack : 1 to a real bool
static uint32 TPL_VERSION = 2;

class SIMPLETEMPLATE_API FTemplateCompilerContent
{
public:
	// The dynamic scope
	TSharedPtr<FJsonObject> DynamicScope;

	// The lexical scope
	TArray<TSharedPtr<FJsonObject>> LexicalScope;
};

class TTemplateCompilerHelper
{
public:
	static void PushScope(FTemplateCompilerContent& Context)
	{
		Context.LexicalScope.Push(MakeShareable(new FJsonObject()));
	}

	static void PopScope(FTemplateCompilerContent& Context)
	{
		Context.LexicalScope.Pop();
	}

	static TSharedPtr<FJsonValue> GetValue(FTemplateCompilerContent& Context, const FString& Key)
	{
		// Get it always from the lexical scope first
		for (int32 i = Context.LexicalScope.Num()-1; i >= 0; i--)
		{
			if (Context.LexicalScope[i].IsValid() && Context.LexicalScope[i]->Values.Num() > 0)
			{
				TSharedPtr<FJsonValue> Value = GetValue(Key, Context.LexicalScope[i]);
				if (Value.IsValid())
				{
					return Value;
				}
			}
		}
		
		// Dynamic scope is our last guess
		return GetValue(Key, Context.DynamicScope);
	}

	static void SetValue(FTemplateCompilerContent& Context, const FString& Key, TSharedPtr<FJsonValue>& Value)
	{
		TSharedPtr<FJsonObject> Scope = Context.LexicalScope.Last();
		if (Scope.IsValid())
		{
			Scope->SetField(Key, Value);
		}
	}

private:
	static TSharedPtr<FJsonValue> GetValue(const FString& Key, TSharedPtr<FJsonObject> Data)
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

	virtual FString Build()
	{
		return FString();
	}
	
	virtual void Serialize(FArchive& Ar) {}

	// Interpret the token 
	virtual void Interpret(FTemplateCompilerContent& Context, FArchive& WriteStream, TSharedPtr<FJsonObject> Data) {}

	// Some tokens are nested
	virtual void AddBranch(TArray<TSharedPtr<FToken>>& children) {}
};

typedef TSharedPtr<FToken> FTokenPtr;

USTRUCT(Blueprintable)
struct SIMPLETEMPLATE_API FTokenArray
{
	GENERATED_USTRUCT_BODY()

public:

	void Serialize(FArchive& Ar);

public:
	TArray<FTokenPtr> Items;
};

class SIMPLETEMPLATE_API FTokenText : public FToken
{
public:
	FTokenText() : FToken() {}

    FTokenText(const FString& InText)
        : FToken()
		, Text(InText)
	{}

    ETokenType GetType() const override
    {
        return ETokenType::Text;
    }

	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Text;
	}

	virtual void Interpret(FTemplateCompilerContent& Context, FArchive& WriteStream, TSharedPtr<FJsonObject> Data) override
	{
		WriteStream.Serialize((void*)*Text, Text.Len() * sizeof(TCHAR));
	}

public:
	FString Text;
};

class SIMPLETEMPLATE_API FTokenVar : public FToken
{
public:
	FTokenVar() : FToken() {}

	FTokenVar(const FString& InKey)
		: FToken()
		, Key(InKey)
	{}

	virtual FString Build() override
	{
		return FString();
	}

	ETokenType GetType() const override
	{
		return ETokenType::Var;
	}

	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Key;
	}

	virtual void Interpret(FTemplateCompilerContent& Context, FArchive& WriteStream, TSharedPtr<FJsonObject> Data) override
	{
		auto value = TTemplateCompilerHelper::GetValue(Context, Key);
		FString valueStr;
		if (value.IsValid() && value->TryGetString(valueStr))
		{
			WriteStream.Serialize((void*)*valueStr, valueStr.Len() * sizeof(TCHAR));
		}
	}

public:
	FString Key;
};

class SIMPLETEMPLATE_API FTokenNested : public FToken
{
public:
	FTokenNested() : FToken() {}

	FTokenNested(const FString& InExpression)
		: FToken()
		, Expression(InExpression)
	{}

	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Expression;
		Children.Serialize(Ar);
	}

	// Some tokens are nested
	virtual void AddBranch(TArray<TSharedPtr<FToken>>& children)
	{
		Children.Items = children;
	}

public:
	FString Expression;
	FTokenArray Children;
};

class SIMPLETEMPLATE_API FTokenFor : public FTokenNested
{
public:
	FTokenFor() : FTokenNested() {}

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
		Value = ForValues[1];
		List = ForValues[3];
		return FString();
	}

	virtual void Interpret(FTemplateCompilerContent& Context, FArchive& WriteStream, TSharedPtr<FJsonObject> Data) override
	{
		TTemplateCompilerHelper::PushScope(Context);
		auto listDataPtr = TTemplateCompilerHelper::GetValue(Context, List);
		const TArray<TSharedPtr<FJsonValue>>* list;
		if (listDataPtr.IsValid() && listDataPtr->TryGetArray(list))
		{
			for (int i = 0; i < list->Num(); i++)
			{
				// Add loop data
				// loop.index
				TSharedPtr<FJsonObject> loopData = MakeShareable(new FJsonObject());
				loopData->SetNumberField("index", i);
				TSharedPtr<FJsonValue> LoopValue = MakeShareable(new FJsonValueObject(loopData));

				TTemplateCompilerHelper::SetValue(Context, "loop", LoopValue);

				//Data->SetField();

				// Set item
				auto item = (*list)[i];
				TTemplateCompilerHelper::SetValue(Context, Value, item);
				//Data->SetField(Value, item);

				// Now propagate
				for (auto child : Children.Items)
				{
					child->Interpret(Context, WriteStream, Data);
				}
			}
		}
		TTemplateCompilerHelper::PopScope(Context);
	}

    ETokenType GetType() const override
    {
        return ETokenType::For;
    }

	virtual void Serialize(FArchive& Ar) override
	{
		FTokenNested::Serialize(Ar);
		Ar << List;
		Ar << Value;
	}

public:
	FString List;
	FString Value;
};

class SIMPLETEMPLATE_API FTokenIf : public FTokenNested
{
public:
	FTokenIf() : FTokenNested() {}

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
		if (IfValues[1].Equals("not"))
		{
			bSign = false;
			Key = IfValues[2];
		}
		// if var
		else if (IfValues.Num() == 2)
		{
			Key = IfValues[1];
			if (IfValues[1].StartsWith("!"))
			{
				bSign = false;
				Key.RemoveFromStart("!");
			}
			else
			{
				bSign = true;
				Key = IfValues[1];
			}
			
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

	virtual void Interpret(FTemplateCompilerContent& Context, FArchive& WriteStream, TSharedPtr<FJsonObject> Data) override
	{
		TTemplateCompilerHelper::PushScope(Context);
		if (IsTrue(Context, Data))
		{
			for(auto child : Children.Items)
			{
				child->Interpret(Context, WriteStream, Data);
			}
		}
		TTemplateCompilerHelper::PopScope(Context);
	}

    ETokenType GetType() const override
    {
        return ETokenType::If;
    }

	virtual void Serialize(FArchive& Ar) override
	{
		FTokenNested::Serialize(Ar);
		Ar << bSign;
		Ar << bIgnoreCase;
		Ar << Key;
		Ar << Value;
	}

private:
	bool IsTrue(FTemplateCompilerContent& Context, TSharedPtr<FJsonObject> Data)
	{
		// Only key provided
		if (Value.IsEmpty())
		{
			bool boolValue = false;
			auto keyDataPtr = TTemplateCompilerHelper::GetValue(Context, Key);
			if (keyDataPtr.IsValid())
			{
				// Only check against the actual singn in case we have a bool, all other types
				// are TRUE if they exists and FALSE otherwise
				if (keyDataPtr->TryGetBool(boolValue))
				{
					return boolValue == bSign;
				}
				return bSign;
			}
			// Just check the sign
			return !bSign;
		}

		// Find l-value and r-value
		auto lValuePtr = TTemplateCompilerHelper::GetValue(Context, Key);
		auto rValuePtr = TTemplateCompilerHelper::GetValue(Context, Value);
		if (!rValuePtr.IsValid())
		{
			rValuePtr = MakeShareable(new FJsonValueString(Value.TrimQuotes()));
		}

		// Compare
		FString lValue;
		FString rValue;
		return lValuePtr.IsValid() && rValuePtr.IsValid() && lValuePtr->TryGetString(lValue) && rValuePtr->TryGetString(rValue) && (lValue.Equals(rValue, bIgnoreCase ? ESearchCase::IgnoreCase : ESearchCase::CaseSensitive) == bSign);
	}

public:
	bool bSign;
	bool bIgnoreCase;
	FString Key;
	FString Value;
};

class SIMPLETEMPLATE_API FTokenEnd : public FToken
{
public:
	FTokenEnd() : FToken() {}

	FTokenEnd(const FString& InExpression)
		: FToken()
		, Expression(InExpression)
	{}

	virtual void Serialize(FArchive& Ar) override
	{
		Ar << Expression;
	}

public:
	FString Expression;
};

class SIMPLETEMPLATE_API FTokenEndIf : public FTokenEnd
{
public:
	FTokenEndIf() : FTokenEnd() {}

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

	ETokenType GetType() const override
	{
		return ETokenType::EndIf;
	}
};

class SIMPLETEMPLATE_API FTokenEndFor : public FTokenEnd
{
public:
	FTokenEndFor() : FTokenEnd() {}

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
    static TSharedRef< TTemplateTokenizer<CharType>> Create(FArchive* const Stream)
    {
        return MakeShareable(new TTemplateTokenizer<CharType>(Stream));
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
				token->AddBranch(children.Items);
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
					Buffer.TrimStartInline();
					if (!AddToken(new FTokenVar(Buffer)))
					{
						return false;
					}
					Buffer = "";
				}
				else
				{
					SetError(FString::Printf(TEXT("Missing '}' after a var token"), *TPL_END_TOKEN));
					return false;
				}
			}
			else if (IsControlToken(Char))
			{
				if (NextEndToken(Buffer))
				{
					Buffer.TrimStartInline();
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
						Buffer.TrimEndInline();

						// Check end token to match ParseState
						if (ParseState.Num() > 0)
						{
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
						else
						{
							SetError(FString::Printf(TEXT("Unexpected token '%s'."), *Buffer));
							return false;
						}
					}
					Buffer = "";
				}
				else
				{
					SetError(FString::Printf(TEXT("Missing '%s' after control token"), *TPL_END_TOKEN));
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
			// Check for escape and append the next read
			if (IsEscapeToken(Char))
			{
				if (!ReadNext(Char))
				{
					break;
				}
			}
			else
			{
				if (IsTokenStart(Char))
				{
					return true;
				}
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
		return Char == CharType('\0');
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

	bool IsEscapeToken(const CharType& Char)
	{
		return Char == CharType('\\');
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

	// TODO: Add error handling

	bool Interpret(FArchive& WriteStream, TSharedPtr<FJsonObject> Data)
	{
		FTemplateCompilerContent Context;
		Context.DynamicScope = Data;

		TTemplateCompilerHelper::PushScope(Context);
		for (auto token : TokenTree.Items)
		{
			token->Interpret(Context, WriteStream, Data);
		}
		TTemplateCompilerHelper::PopScope(Context);
		return true;
	}

	bool Interpret(FString& OutString, TSharedPtr<FJsonObject> Data)
	{
		TArray<uint8> Bytes;
		auto WriteStream = new FMemoryWriter(Bytes);

		if (Interpret(*WriteStream, Data))
		{

			FString Out;
			for (int32 i = 0; i < Bytes.Num(); i += sizeof(TCHAR))
			{
				TCHAR* Char = static_cast<TCHAR*>(static_cast<void*>(&Bytes[i]));
				Out += *Char;
			}
			OutString = Out;
			WriteStream->Close();
			delete WriteStream;
			return true;
		}
		return false;
	}

	bool Interpret(FArchive& WriteStream, TScriptInterface<ISimpleTemplateDataProvider> DataProvider)
	{
		return DataProvider != nullptr ? Interpret(WriteStream, DataProvider->GetData()) : false;
	}

	bool Interpret(FString& OutString, TScriptInterface<ISimpleTemplateDataProvider> DataProvider)
	{
		return DataProvider != nullptr ? Interpret(OutString, DataProvider->GetData()) : false;
	}

protected:
	TTemplateInterpreter(FTokenArray& InTokenTree)
		: TokenTree(InTokenTree)
	{
	}

protected:
	FTokenArray TokenTree;
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