// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SimpleTemplateEditorToolkit.h"

#include "Editor.h"
#include "EditorReimportHandler.h"
#include "EditorStyleSet.h"
#include "SSimpleTemplateEditor.h"
#include "UObject/NameTypes.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FSimpleTemplateEditorToolkit"


/* Local constants
 *****************************************************************************/

namespace SimpleTemplateEditor
{
	static const FName AppIdentifier("SimpleTemplateEditorApp");
	static const FName TabId("SimpleTemplateEditor");
}


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
						->Split
						(
							FTabManager::NewStack()
								->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.1f)
								
						)
						->Split
						(
							FTabManager::NewStack()
								->AddTab(SimpleTemplateEditor::TabId, ETabState::OpenedTab)
								->SetHideTabWell(true)
								->SetSizeCoefficient(0.9f)
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
		SimpleTemplate
	);

	RegenerateMenusAndToolbars();
}


/* FAssetEditorToolkit interface
 *****************************************************************************/

FString FSimpleTemplateEditorToolkit::GetDocumentationLink() const
{
	return FString(TEXT("https://www.playspace.com"));
}


void FSimpleTemplateEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_SimpleTemplateditor", "Simple Template Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(SimpleTemplateEditor::TabId, FOnSpawnTab::CreateSP(this, &FSimpleTemplateEditorToolkit::HandleTabManagerSpawnTab, SimpleTemplateEditor::TabId))
		.SetDisplayName(LOCTEXT("SimpleTemplateTabName", "Simple Templater"))
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


/* SimpleTemplateEditorToolkit callbacks
 *****************************************************************************/

TSharedRef<SDockTab> FSimpleTemplateEditorToolkit::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier)
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

	if (TabIdentifier == SimpleTemplateEditor::TabId)
	{
		TabWidget = SNew(SSimpleTemplateEditor, SimpleTemplate, Style);
	}

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			TabWidget.ToSharedRef()
		];
}


#undef LOCTEXT_NAMESPACE
