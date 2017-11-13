// Copyright Playspace S.L. 2017

#include "SimpleTemplate.h"

DEFINE_LOG_CATEGORY(LogSTE);

void USimpleTemplate::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// Serialize token array
	if (Ar.IsLoading())
	{
		uint32 TemplateVersion;
		Ar << TemplateVersion;
		if (TemplateVersion <= TPL_VERSION)
		{
			UE_LOG(LogSTE, Error, TEXT("Serialized template version is incompatible with your current version!"));
		}
		check(TemplateVersion <= TPL_VERSION);
		Tokens.Serialize(Ar);
	}
	else if (Ar.IsSaving())
	{
		Ar << TPL_VERSION;
		Tokens.Serialize(Ar);
	}
}

#if WITH_EDITOR

void USimpleTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = (PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None);
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USimpleTemplate, Template))
	{
		Status = ETemplateStatus::TS_Dirty;
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool USimpleTemplate::Compile()
{
	LineNumber = 0;
	CharacterNumber = 0;
	LastErrors.Empty();
	auto compiler = TTemplateCompilerFactory<TCHAR>::Create(Template.ToString());
	if (compiler->Compile())
	{
		Tokens = compiler->GetTokenTree();
		Status = ETemplateStatus::TS_UpToDate;
		PostEditChange();
		MarkPackageDirty();
		return true;
	}
	LineNumber = compiler->GetLineNumber();
	CharacterNumber = compiler->GetCharNumber();
	LastErrors.Add(compiler->GetLastError());
	Status = ETemplateStatus::TS_Error;
	PostEditChange();
	MarkPackageDirty();
	return false;
}

#endif