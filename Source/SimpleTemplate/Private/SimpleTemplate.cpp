// Copyright Playspace S.L. 2017

#include "SimpleTemplate.h"

void USimpleTemplate::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// Serialize token array
	if (Ar.IsLoading())
	{
		int64 EndOffset = 0;
		Ar << EndOffset;

		uint32 TemplateVersion;
		Ar << TemplateVersion;

		if (TemplateVersion != TPL_VERSION)
		{
			UE_LOG(LogSTE, Error, TEXT("Serialized template version is incompatible with your current version! Need to recompile!"));
			Status = ETemplateStatus::TS_Dirty;

			// Skip over the data
			Ar.Seek(EndOffset);
		}
		else
		{
			Tokens.Serialize(Ar);
		}
	}
	else if (Ar.IsSaving())
	{
		int64 SkipOffset = Ar.Tell();
		// We have to serialize the placeholder value to count the real end offset
		Ar << SkipOffset;

		// Write the template version first
		Ar << TPL_VERSION;
		Tokens.Serialize(Ar);

		// Set back to inject the offset to the end of the serialization, this way we can skip the data alltogether
		int64 EndOffset = Ar.Tell();
		Ar.Seek(SkipOffset);
		Ar << EndOffset;
		Ar.Seek(EndOffset);
	}
}

FString USimpleTemplate::Interpret(TSharedPtr<FJsonObject> Data)
{
	if (IsUpToDate())
	{
		auto interpreter = TTemplateInterpreter::Create(Tokens);
		FString OutString;
		if (interpreter->Interpret(OutString, Data))
		{
			return OutString;
		}
	}
	return FString();
}

FString USimpleTemplate::Interpret(TScriptInterface<ISimpleTemplateDataProvider> DataProvider)
{
	if (IsUpToDate())
	{
		auto interpreter = TTemplateInterpreter::Create(Tokens);
		FString OutString;
		if (interpreter->Interpret(OutString, DataProvider))
		{
			return OutString;
		}
	}
	return FString();
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