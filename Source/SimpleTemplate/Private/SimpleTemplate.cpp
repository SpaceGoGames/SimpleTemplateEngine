// Copyright Playspace S.L. 2017

#include "SimpleTemplate.h"

void USimpleTemplate::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// Serialize token array
	if (Ar.IsLoading())
	{
		uint32 TemplateVersion;
		Ar << TemplateVersion;
		//UE_LOG(LogSTE, Error, TEXT("Serialized template version is incompatible with your current verion!"));
		check(TemplateVersion <= TPL_VERSION);

		int32 NumTokens;
		Ar << NumTokens;

		while (NumTokens > 0)
		{
			--NumTokens;
			ETokenType TokenType;
			FString TokenExpression;
			Ar << TokenType;
			Ar << TokenExpression;

			switch (TokenType)
			{
			case ETokenType::Text:
				AddToken(new FTokenText(TokenExpression), true);
				break;
			case ETokenType::Var:
				AddToken(new FTokenVar(TokenExpression), true);
				break;
			case ETokenType::If:
				AddToken(new FTokenIf(TokenExpression), true);
				break;
			case ETokenType::For:
				AddToken(new FTokenFor(TokenExpression), true);
				break;
			case ETokenType::EndIf:
				AddToken(new FTokenEndIf(TokenExpression), true);
				break;
			case ETokenType::EndFor:
				AddToken(new FTokenEndFor(TokenExpression), true);
				break;
			default:
				break;
			}
		}
	}
	else if (Ar.IsSaving())
	{
		Ar << TPL_VERSION;
		int32 NumTokens = Tokens.Num();
		Ar << NumTokens;
		for (auto Token : Tokens)
		{
			Token->Serialize(Ar);
		}
	}
}

#if WITH_EDITOR
bool USimpleTemplate::Compile()
{
	LastErrors.Empty();
	auto compiler = TTemplateCompilerFactory<TCHAR>::Create(Template.ToString());
	if (compiler->Compile())
	{
		Tokens.Empty();
		FTokenArray compiledTokens = compiler->GetTokens();
		for (auto Token: compiledTokens)
		{
			Tokens.Add(Token);
		}
		Status = ETemplateStatus::TS_UpToDate;
		PostEditChange();
		MarkPackageDirty();
		return true;
	}
	LastErrors.Add(compiler->GetLastError());
	Status = ETemplateStatus::TS_Error;
	PostEditChange();
	MarkPackageDirty();
	return false;
}
#endif