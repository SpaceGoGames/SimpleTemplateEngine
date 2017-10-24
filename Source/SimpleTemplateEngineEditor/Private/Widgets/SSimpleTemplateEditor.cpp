// Copyright Playspace S.L. 2017

#include "SSimpleTemplateEditor.h"

#include "Fonts/SlateFontInfo.h"
#include "Internationalization/Text.h"
#include "UObject/Class.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"


#define LOCTEXT_NAMESPACE "SSimpleTemplateEditor"


/* SSimpleTemplateEditor interface
 *****************************************************************************/

SSimpleTemplateEditor::~SSimpleTemplateEditor()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}


void SSimpleTemplateEditor::Construct(const FArguments& InArgs, USimpleTemplate* InSimpleTemplate, const TSharedRef<ISlateStyle>& InStyle)
{
	SimpleTemplate = InSimpleTemplate;
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(EditableTextBox, SMultiLineEditableTextBox)
					.OnTextChanged(this, &SSimpleTemplateEditor::HandleEditableTextBoxTextChanged)
					.OnTextCommitted(this, &SSimpleTemplateEditor::HandleEditableTextBoxTextCommitted)
					.Text(SimpleTemplate->Template)
			]
	];

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(this, &SSimpleTemplateEditor::HandleAssetPropertyChanged);
}


void SSimpleTemplateEditor::HandleEditableTextBoxTextChanged(const FText& NewText)
{
	SimpleTemplate->MarkPackageDirty();
}


void SSimpleTemplateEditor::HandleEditableTextBoxTextCommitted(const FText& Comment, ETextCommit::Type CommitType)
{
	SimpleTemplate->Template = EditableTextBox->GetText();
}


void SSimpleTemplateEditor::HandleAssetPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (Object == SimpleTemplate)
	{
		EditableTextBox->SetText(SimpleTemplate->Template);
	}
}


#undef LOCTEXT_NAMESPACE
