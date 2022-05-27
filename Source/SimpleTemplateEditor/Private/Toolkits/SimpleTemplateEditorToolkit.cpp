// Copyright Playspace S.L. 2017

#include "SimpleTemplateEditorToolkit.h"

#include "Editor.h"
#include "EditorReimportHandler.h"
#include "EditorStyleSet.h"
#include "SimpleTemplate.h"
#include "UObject/NameTypes.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Framework/Application/SlateApplication.h"
#include "DesktopPlatformModule.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "EditorDirectories.h"

#define LOCTEXT_NAMESPACE "FSimpleTemplateEditorToolkit"

DEFINE_LOG_CATEGORY_STATIC(LogSimpleTemplateEditor, Log, All);


/* Local constants
 *****************************************************************************/

namespace SimpleTemplateEditor
{
	static const FName AppIdentifier("SimpleTemplateEditorApp");
	static const FName TabId("TemplateEditor");
	static const FName OutputTabId("TemplateCompileOutput");
}


/* FSimpleTemplateEditorToolkit structors
 *****************************************************************************/

FSimpleTemplateEditorToolkit::FSimpleTemplateEditorToolkit(const TSharedRef<ISlateStyle>& InStyle)
	: SimpleTemplate(nullptr)
	, Style(InStyle)
{ }


FSimpleTemplateEditorToolkit::~FSimpleTemplateEditorToolkit()
{
	FReimportManager::Instance()->OnPreReimport().RemoveAll(this);
	FReimportManager::Instance()->OnPostReimport().RemoveAll(this);

	GEditor->UnregisterForUndo(this);
}


/* FSimpleTemplateEditorToolkit interface
 *****************************************************************************/

void FSimpleTemplateEditorToolkit::Initialize(USimpleTemplate* InSimpleTemplate, const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost)
{
	BindCommands();

	SimpleTemplate = InSimpleTemplate;

	// Support undo/redo
	SimpleTemplate->SetFlags(RF_Transactional);
	GEditor->RegisterForUndo(this);

	// create tab layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_SimpleTemplateEditor")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
						->SetOrientation(Orient_Vertical)
						->SetSizeCoefficient(0.66f)

						// Toolbar
						->Split
						(
							FTabManager::NewStack()
								->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.1f)
								
						)

						// Compile error view
						->Split
						(
							FTabManager::NewStack()
							->AddTab(SimpleTemplateEditor::OutputTabId, ETabState::ClosedTab)
							->SetHideTabWell(true)
							->SetSizeCoefficient(0.1f)
						)
						
						// Template editor
						->Split
						(
							FTabManager::NewStack()
								->AddTab(SimpleTemplateEditor::TabId, ETabState::OpenedTab)
								->SetHideTabWell(false)
								->SetSizeCoefficient(0.8f)
						)
				)
		);

	FAssetEditorToolkit::InitAssetEditor(
		InMode,
		InToolkitHost,
		SimpleTemplateEditor::AppIdentifier,
		Layout,
		true /*bCreateDefaultStandaloneMenu*/,
		true /*bCreateDefaultToolbar*/,
		InSimpleTemplate
	);

	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();
}


/* FAssetEditorToolkit interface
 *****************************************************************************/

FString FSimpleTemplateEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("https://github.com/PlayspaceDev/SimpleTemplateEngine"));
}


void FSimpleTemplateEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_SimpleTemplateEditor", "Simple Template Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(SimpleTemplateEditor::TabId, FOnSpawnTab::CreateSP(this, &FSimpleTemplateEditorToolkit::HandleTabManagerSpawnTab, SimpleTemplateEditor::TabId))
		.SetDisplayName(LOCTEXT("TemplateEditorTabName", "Template Editor"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(SimpleTemplateEditor::OutputTabId, FOnSpawnTab::CreateSP(this, &FSimpleTemplateEditorToolkit::HandleTabManagerSpawnTab, SimpleTemplateEditor::OutputTabId))
		.SetDisplayName(LOCTEXT("CompileOutputTabName", "Compile Output"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}


void FSimpleTemplateEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(SimpleTemplateEditor::TabId);
}


/* IToolkit interface
 *****************************************************************************/

FText FSimpleTemplateEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Simple Template Editor");
}


FName FSimpleTemplateEditorToolkit::GetToolkitFName() const
{
	return FName("SimpleTemplateEditor");
}

FText FSimpleTemplateEditorToolkit::GetToolkitName() const
{
	if (SimpleTemplate != nullptr)
	{
		const bool bDirtyState = SimpleTemplate->GetOutermost()->IsDirty();

		FFormatNamedArguments Args;
		Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());

		Args.Add(TEXT("TemplateName"), FText::FromString(SimpleTemplate->GetName()));
		return FText::Format(LOCTEXT("EditorTitle", "{TemplateName}{DirtyState}"), Args);
	}
	return FText::GetEmpty();
}


FLinearColor FSimpleTemplateEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}


FString FSimpleTemplateEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "SimpleTemplate ").ToString();
}


/* FGCObject interface
 *****************************************************************************/

void FSimpleTemplateEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(SimpleTemplate);
}


/* FEditorUndoClient interface
*****************************************************************************/

void FSimpleTemplateEditorToolkit::PostUndo(bool bSuccess)
{ }


void FSimpleTemplateEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}


/* FSimpleTemplateEditorToolkit callbacks
 *****************************************************************************/

TSharedRef<SDockTab> FSimpleTemplateEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
	if (TabIdentifier == SimpleTemplateEditor::TabId)
	{
		SAssignNew(TemplateEditor, SSimpleTemplateEditor, SimpleTemplate, Style);

		return SNew(SDockTab)
			.TabRole(ETabRole::PanelTab)
			[
				TemplateEditor.ToSharedRef()
			];
	}
	else if (TabIdentifier == SimpleTemplateEditor::OutputTabId)
	{
		FString ErrorText;

		if (SimpleTemplate->LastErrors.Num() > 0)
		{
			ErrorText.Append(LOCTEXT("CompileFailed", "Failed to compile template!").ToString());
			ErrorText += TEXT("\n");
			for (auto& CompileError : SimpleTemplate->LastErrors)
			{
				ErrorText.Append(CompileError);
				ErrorText += TEXT("\n");
			}
		}

		SAssignNew(TemplateOutput, STextBlock)
			.Text(ErrorText.Len() > 0 ? FText::FromString(ErrorText) : LOCTEXT("CompileHint", "Compile template"))
			.ToolTipText(LOCTEXT("OutputTabToolTip", "Compile result, double click to highlight error in editor."))
			.OnDoubleClicked(this, &FSimpleTemplateEditorToolkit::GoToError);

		return SNew(SDockTab)
			.TabRole(ETabRole::PanelTab)
			[
				TemplateOutput.ToSharedRef()
			];
	}

	// Just an empty tab
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			TabWidget.ToSharedRef()
		];
}

/* Commands and Menu extensions
*****************************************************************************/

void FSimpleTemplateEditorToolkit::BindCommands()
{
	const FSimpleTemplateEditorCommands& Commands = FSimpleTemplateEditorCommands::Get();

	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();

	UICommandList->MapAction(Commands.Compile,
		FExecuteAction::CreateSP(this, &FSimpleTemplateEditorToolkit::ActionCompile));
	UICommandList->MapAction(Commands.Export,
		FExecuteAction::CreateSP(this, &FSimpleTemplateEditorToolkit::ActionExport));
	UICommandList->MapAction(Commands.Import,
		FExecuteAction::CreateSP(this, &FSimpleTemplateEditorToolkit::ActionImport));
}

void FSimpleTemplateEditorToolkit::ExtendMenu()
{
}

void FSimpleTemplateEditorToolkit::FillTemplateToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("Command");
	{
		const FSimpleTemplateEditorCommands& Commands = FSimpleTemplateEditorCommands::Get();
		ToolbarBuilder.AddToolBarButton(Commands.Compile,
			NAME_None,
			TAttribute<FText>(),
			TAttribute<FText>(this, &FSimpleTemplateEditorToolkit::GetStatusTooltip),
			TAttribute<FSlateIcon>(this, &FSimpleTemplateEditorToolkit::GetStatusImage),
			FName(TEXT("Compile")));
		ToolbarBuilder.AddToolBarButton(Commands.Export);
		ToolbarBuilder.AddToolBarButton(Commands.Import);
	}
	ToolbarBuilder.EndSection();
}

void FSimpleTemplateEditorToolkit::ExtendToolbar()
{

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FSimpleTemplateEditorToolkit::FillTemplateToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
}

FSlateIcon FSimpleTemplateEditorToolkit::GetStatusImage() const
{
	ETemplateStatus Status = SimpleTemplate->Status;
	switch (Status)
	{
	case ETemplateStatus::TS_Error:
		return FSlateIcon(FSimpleTemplateStyle::GetStyleSetName(), "SimpleTemplateEditor.Compile.Error");
	case ETemplateStatus::TS_UpToDate:
		return FSlateIcon(FSimpleTemplateStyle::GetStyleSetName(), "SimpleTemplateEditor.Compile");
	}
	return FSlateIcon(FSimpleTemplateStyle::GetStyleSetName(), "SimpleTemplateEditor.Compile.Dirty");
}

FText FSimpleTemplateEditorToolkit::GetStatusTooltip() const
{
	ETemplateStatus Status = SimpleTemplate->Status;
	switch (Status)
	{
	case ETemplateStatus::TS_Unknown:
		return LOCTEXT("Recompile_Status", "Unknown status; should recompile");
	case ETemplateStatus::TS_BeingCreated:
		return LOCTEXT("Created_Status", "You have to compile the template for the first time");
	case ETemplateStatus::TS_Dirty:
		return LOCTEXT("Dirty_Status", "Dirty; needs to be recompiled");
	case ETemplateStatus::TS_Error:
		return LOCTEXT("CompileError_Status", "There was an error during compilation, see the log for details");
	}
	return LOCTEXT("GoodToGo_Status", "Good to go");
}

void FSimpleTemplateEditorToolkit::ActionCompile()
{
	if (SimpleTemplate->Status == ETemplateStatus::TS_BeingCreated)
	{
		SaveAsset_Execute();
	}

	if (!SimpleTemplate->Compile())
	{
		FString ErrorText;
		ErrorText.Append(LOCTEXT("CompileFailed", "Failed to compile template!").ToString());
		ErrorText += TEXT("\n");
		for (auto& CompileError : SimpleTemplate->LastErrors)
		{
			ErrorText.Append(CompileError);
			ErrorText += TEXT("\n");
		}
		if (TemplateOutput.IsValid())
		{
			TemplateOutput->SetText(ErrorText);
		}
	}
	else
	{
		if (TemplateOutput.IsValid())
		{
			TemplateOutput->SetText(LOCTEXT("CompileSuccessful", "Template compiled successfully"));
		}
		SaveAsset_Execute();
	}
}

void FSimpleTemplateEditorToolkit::ActionExport()
{
	if (SimpleTemplate->Status == ETemplateStatus::TS_BeingCreated)
	{
		SaveAsset_Execute();
	}

	TArray<FString> SaveFilenames;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bSaved = false;
	if (DesktopPlatform != nullptr)
	{
		bSaved = DesktopPlatform->SaveFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("ExportDialogTitle", "Export template to file").ToString(),
			*(FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_EXPORT)),
			TEXT(""),
			TEXT("Simple Template File|*.stf"),
			EFileDialogFlags::None,
			SaveFilenames);
	}

	if (bSaved)
	{
		FString FileName = SaveFilenames[0];
		FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, FPaths::GetPath(FileName)); // Save path as default for next time.
		FFileHelper::SaveStringToFile(SimpleTemplate->Template.ToString(), *FileName, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

void FSimpleTemplateEditorToolkit::ActionImport()
{
	if (SimpleTemplate->Status == ETemplateStatus::TS_BeingCreated)
	{
		SaveAsset_Execute();
	}

	TArray<FString> OpenFilenames;
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bOpened = false;
	if (DesktopPlatform != nullptr)
	{
		const FString DefaultBrowsePath = FString::Printf(TEXT("%slogs/"), *FPaths::ProjectSavedDir());

		bOpened = DesktopPlatform->OpenFileDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
			LOCTEXT("OpenProjectBrowseTitle", "Open Project").ToString(),
			FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_OPEN),
			TEXT(""),
			TEXT("Simple Template File|*.stf"),
			EFileDialogFlags::None,
			OpenFilenames
		);
	}

	if (bOpened)
	{
		FString FileName = OpenFilenames[0];
		FString FileContent;
		if (FFileHelper::LoadFileToString(FileContent, *FileName))
		{
			SimpleTemplate->Template = FText::FromString(FileContent);
			SimpleTemplate->Status = ETemplateStatus::TS_Dirty;
			SimpleTemplate->PostEditChange();
			SimpleTemplate->MarkPackageDirty();
		}		
	}
}

FReply FSimpleTemplateEditorToolkit::GoToError()
{
	if (SimpleTemplate->Status == ETemplateStatus::TS_BeingCreated)
	{
		SaveAsset_Execute();
	}
	TemplateEditor->GoTo(SimpleTemplate->LineNumber, SimpleTemplate->CharacterNumber);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
