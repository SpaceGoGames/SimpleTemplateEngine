// Copyright Playspace S.L. 2017

#include "SimpleTemplateFactory.h"

#include "Containers/UnrealString.h"
#include "SimpleTemplate.h"
#include "Misc/FileHelper.h"


/* USimpleTemplateFactory structors
 *****************************************************************************/

USimpleTemplateFactory::USimpleTemplateFactory( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("txt;")) + NSLOCTEXT("USimpleTemplateFactory", "FormatTxt", "Text File").ToString());
	SupportedClass = USimpleTemplate::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
}

UObject* USimpleTemplateFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	USimpleTemplate* SimpleTemplate = nullptr;
	FString TextString;

	if (FFileHelper::LoadFileToString(TextString, *Filename))
	{
		SimpleTemplate = NewObject<USimpleTemplate>(InParent, InClass, InName, Flags);
		SimpleTemplate->Template = FText::FromString(TextString);
	}

	bOutOperationCanceled = false;

	return SimpleTemplate;
}
