// Copyright Playspace S.L. 2017

#include "SSimpleTemplateEditor.h"

#include "Fonts/SlateFontInfo.h"
#include "Internationalization/Text.h"
#include "SimpleTemplate.h"
#include "UObject/Class.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Input/Reply.h"

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

	HorizontalScrollbar =
		SNew(SScrollBar)
		.Orientation(Orient_Horizontal)
		.Thickness(FVector2D(10.0f, 10.0f));

	VerticalScrollbar =
		SNew(SScrollBar)
		.Orientation(Orient_Vertical)
		.Thickness(FVector2D(10.0f, 10.0f));

	auto Settings = GetDefault<USimpleTemplateEditorSettings>();

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FSimpleTemplateStyle::Get()->GetBrush("TextEditor.Border"))
		[
			SNew(SGridPanel)
			.FillColumn(0, 1.0f)
			.FillRow(0, 1.0f)
			+SGridPanel::Slot(0, 0)
			[
				SAssignNew(EditableTextBox, SSimpleTemplateEditableText)
				.Text(SimpleTemplate->Template)
				.HScrollBar(HorizontalScrollbar)
				.VScrollBar(VerticalScrollbar)
				.OnTextChanged(this, &SSimpleTemplateEditor::HandleEditableTextBoxTextChanged)
			]
			+SGridPanel::Slot(1, 0)
			[
				VerticalScrollbar.ToSharedRef()
			]
			+SGridPanel::Slot(0, 1)
			[
				HorizontalScrollbar.ToSharedRef()
			]
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
