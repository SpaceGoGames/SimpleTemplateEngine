// Copyright Playspace S.L. 2017

#include "SSimpleTemplateEditor.h"

#include "Fonts/SlateFontInfo.h"
#include "Internationalization/Text.h"
#include "SimpleTemplate.h"
#include "UObject/Class.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#include "SimpleTemplateEditorSettings.h"


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

	auto Settings = GetDefault<USimpleTemplateEditorSettings>();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(EditableTextBox, SMultiLineEditableTextBox)
					.BackgroundColor((Settings != nullptr) ? Settings->BackgroundColor : FLinearColor::White)
					.Font((Settings != nullptr) ? Settings->Font : FSlateFontInfo())
					.ForegroundColor((Settings != nullptr) ? Settings->ForegroundColor : FLinearColor::Black)
					.Margin((Settings != nullptr) ? Settings->Margin : 4.0f)
					.OnTextChanged(this, &SSimpleTemplateEditor::HandleEditableTextBoxTextChanged)
					.OnTextCommitted(this, &SSimpleTemplateEditor::HandleEditableTextBoxTextCommitted)
					.Text(SimpleTemplate->Template)
			]
	];

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(this, &SSimpleTemplateEditor::HandleSimpleTemplatePropertyChanged);
}


void SSimpleTemplateEditor::GoTo(int32 LineNumber, int32 ColumnNumber)
{
	FTextLocation Location(LineNumber, ColumnNumber);
	EditableTextBox->GoTo(Location);
	EditableTextBox->ScrollTo(Location);
	FSlateApplication::Get().SetKeyboardFocus(EditableTextBox.ToSharedRef());
}


/* SSimpleTemplateEditor callbacks
 *****************************************************************************/

void SSimpleTemplateEditor::HandleEditableTextBoxTextChanged(const FText& NewText)
{
	FText newText = EditableTextBox->GetText();
	if (!newText.EqualTo(SimpleTemplate->Template))
	{
		SimpleTemplate->Template = newText;
		SimpleTemplate->Status = ETemplateStatus::TS_Dirty;
		SimpleTemplate->MarkPackageDirty();
	}
}

	

void SSimpleTemplateEditor::HandleEditableTextBoxTextCommitted(const FText& Comment, ETextCommit::Type CommitType)
{
	FText newText = EditableTextBox->GetText();
	if (!newText.EqualTo(SimpleTemplate->Template))
	{
		SimpleTemplate->Template = newText;
		SimpleTemplate->Status = ETemplateStatus::TS_Dirty;
		SimpleTemplate->MarkPackageDirty();
	}
}


void SSimpleTemplateEditor::HandleSimpleTemplatePropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (Object == SimpleTemplate)
	{
		EditableTextBox->SetText(SimpleTemplate->Template);
	}
}


#undef LOCTEXT_NAMESPACE
