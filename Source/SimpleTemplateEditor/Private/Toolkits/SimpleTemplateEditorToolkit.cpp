// Copyright Playspace S.L. 2017

#include "SimpleTemplateEditorToolkit.h"

#include "Editor.h"
#include "EditorReimportHandler.h"
#include "EditorStyleSet.h"
#include "SSimpleTemplateEditor.h"
#include "SimpleTemplate.h"
#include "UObject/NameTypes.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

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
						
						// Template editor
						->Split
						(
							FTabManager::NewStack()
								->AddTab(SimpleTemplateEditor::TabId, ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.8f)
						)

						// Compile error view
						->Split
						(
							FTabManager::NewStack()
							->AddTab(SimpleTemplateEditor::OutputTabId, ETabState::OpenedTab)
							->SetHideTabWell(true)
							->SetSizeCoefficient(0.1f)
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
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	if (TabIdentifier == SimpleTemplateEditor::TabId)
	{
		TabWidget = SNew(SSimpleTemplateEditor, SimpleTemplate, Style);
	}
	else if (TabIdentifier == SimpleTemplateEditor::OutputTabId)
	{
		TabWidget = SNew(STextBlock)
			.Text(LOCTEXT("NotImplemented", "Compile tab not implemented"));
	}

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
}

void FSimpleTemplateEditorToolkit::ExtendMenu()
{
}

void FSimpleTemplateEditorToolkit::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Command");
			{
				ToolbarBuilder.AddToolBarButton(FSimpleTemplateEditorCommands::Get().Compile);
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);

	//IPaper2DEditorModule* Paper2DEditorModule = &FModuleManager::LoadModuleChecked<IPaper2DEditorModule>("Paper2DEditor");
	//AddToolbarExtender(Paper2DEditorModule->GetFlipbookEditorToolBarExtensibilityManager()->GetAllExtenders());
}

void FSimpleTemplateEditorToolkit::ActionCompile()
{
	SimpleTemplate->Compile();
}

#undef LOCTEXT_NAMESPACE
