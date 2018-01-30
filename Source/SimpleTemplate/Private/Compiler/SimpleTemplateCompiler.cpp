// Copyright Playspace S.L. 2017

#include "Compiler/SimpleTemplateCompiler.h"

void FTokenArray::Serialize(FArchive& Ar)
{
	int32 NumTokens = Items.Num();
	Ar << NumTokens;
	if (Ar.IsLoading())
	{
		while (NumTokens > 0)
		{
			--NumTokens;

			ETokenType TokenType;
			Ar << TokenType;

			FToken* token = nullptr;
			switch (TokenType)
			{
			case ETokenType::Text:
				token = new FTokenText();
				break;
			case ETokenType::Var:
				token = new FTokenVar();
				break;
			case ETokenType::If:
				token = new FTokenIf();
				break;
			case ETokenType::For:
				token = new FTokenFor();
				break;
			case ETokenType::EndIf:
				token = new FTokenEndIf();
				break;
			case ETokenType::EndFor:
				token = new FTokenEndFor();
				break;
			}

			token->Serialize(Ar);
			Items.Add(MakeShareable(token));
		}
	}
	else
	{
		for (auto Token : Items)
		{
			// Always add the type first
			ETokenType serializedType = Token->GetType();
			Ar << serializedType;
			Token->Serialize(Ar);
		}
	}
}
