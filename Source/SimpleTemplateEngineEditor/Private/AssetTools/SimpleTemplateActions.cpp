// Copyright Playspace S.L. 2017

#include "SimpleTemplateActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/SlateStyle.h"
#include "SimpleTemplate.h"

#include "SimpleTemplateEditorToolkit.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions"


/* FTextAssetActions constructors
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
		LOCTEXT("SimpleTemplate_CompileToolTip", "Compile all selected SimpleTemplate asset(s)"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([=]{
				for (auto& SimpleTemplate : SimpleTemplates)
				{
					if (SimpleTemplate.IsValid() && !SimpleTemplate->Template.IsEmpty())
					{
						SimpleTemplate->Stuff();
						SimpleTemplate->PostEditChange();
						SimpleTemplate->MarkPackageDirty();
					}
				}
			}),
			FCanExecuteAction::CreateLambda([=] {
				for (auto& SimpleTemplate : SimpleTemplates)
				{
					if (SimpleTemplate.IsValid() && !SimpleTemplate->Template.IsEmpty())
					{
						return true;
					}
				}
				return false;
			})
		)
	);
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
