// Copyright Playspace S.L. 2017

#include "SimpleTemplateActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/SlateStyle.h"

#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

#include "SimpleTemplateEditorToolkit.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions"


/* FSimpleTemplateActions constructors
 *****************************************************************************/

FSimpleTemplateActions::FSimpleTemplateActions(const TSharedRef<ISlateStyle>& InStyle)
	: Style(InStyle)
{ }


/* FAssetTypeActions_Base overrides
 *****************************************************************************/

bool FSimpleTemplateActions::CanFilter()
{
	return true;
}


void FSimpleTemplateActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

	auto SimpleTemplates = GetTypedWeakObjectPtrs<USimpleTemplate>(InObjects);


	MenuBuilder.AddMenuEntry(
		LOCTEXT("SimpleTemplate_Compile", "Compile"),
		LOCTEXT("SimpleTemplate_CompileToolTip", "Compile selected SimpleTemplate asset(s)."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FSimpleTemplateActions::CompileSelected, SimpleTemplates), FCanExecuteAction()));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("SimpleTemplate_Export", "Export"),
		LOCTEXT("SimpleTemplate_ExportToolTip", "Export selected SimpleTemplate asset(s)."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FSimpleTemplateActions::ExportTemplates, SimpleTemplates), FCanExecuteAction()));
}

void FSimpleTemplateActions::CompileSelected(TArray<TWeakObjectPtr<USimpleTemplate>> SimpleTemplates)
{
	for (auto& SimpleTemplate : SimpleTemplates)
	{
		if (SimpleTemplate.IsValid() && !SimpleTemplate->Template.IsEmpty())
		{
			SimpleTemplate->Compile();
			SimpleTemplate->PostEditChange();
			SimpleTemplate->MarkPackageDirty();
		}
	}
}

void FSimpleTemplateActions::ExportTemplates(TArray<TWeakObjectPtr<USimpleTemplate>> SimpleTemplates)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform != nullptr)
	{
		FString OutputDirectory = FPaths::GameDir();
		FString FolderPath;
		const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("FolderDialogTitle", "Choose a directory to export the templates").ToString(),
			OutputDirectory,
			FolderPath
		);

		if (bFolderSelected)
		{
			if (!FolderPath.EndsWith(TEXT("/")))
			{
				FolderPath += TEXT("/");
			}

			OutputDirectory = FolderPath;

			for (auto& SimpleTemplate: SimpleTemplates)
			{
				if (SimpleTemplate.IsValid())
				{
					TArray<FStringFormatArg> Args;
					Args.Add(SimpleTemplate->GetName());
					FString FileName = FString::Format(TEXT("{0}.stf"), Args);
					FFileHelper::SaveStringToFile(SimpleTemplate->Template.ToString(), *(OutputDirectory / FileName), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
				}
			}
		}
	}
}

uint32 FSimpleTemplateActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}


FText FSimpleTemplateActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SimpleTemplate", "Simple Template");
}


UClass* FSimpleTemplateActions::GetSupportedClass() const
{
	return USimpleTemplate::StaticClass();
}


FColor FSimpleTemplateActions::GetTypeColor() const
{
	return FColor::White;
}


bool FSimpleTemplateActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}


void FSimpleTemplateActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto SimpleTemplate = Cast<USimpleTemplate>(*ObjIt);

		if (SimpleTemplate != nullptr)
		{
			TSharedRef<FSimpleTemplateEditorToolkit> EditorToolkit = MakeShareable(new FSimpleTemplateEditorToolkit(Style));
			EditorToolkit->Initialize(SimpleTemplate, Mode, EditWithinLevelEditor);
		}
	}
}


#undef LOCTEXT_NAMESPACE
