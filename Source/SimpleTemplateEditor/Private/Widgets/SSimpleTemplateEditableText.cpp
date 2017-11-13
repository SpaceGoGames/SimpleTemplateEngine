// Copyright Playspace S.L. 2017

#include "SSimpleTemplateEditableText.h"


void SSimpleTemplateEditableText::Construct( const FArguments& InArgs )
{
	SMultiLineEditableText::Construct(
		SMultiLineEditableText::FArguments()
		.Text(InArgs._Text)
		.AutoWrapText(false)
		.Margin(0)
		.HScrollBar(InArgs._HScrollBar)
		.VScrollBar(InArgs._VScrollBar)
		.OnTextChanged(InArgs._OnTextChanged)
	);
}

FReply SSimpleTemplateEditableText::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& InCharacterEvent)
{
    FReply Reply = FReply::Unhandled();

    const TCHAR Character = InCharacterEvent.GetCharacter();
    if(Character == TEXT('\t'))
    {
        if (!IsTextReadOnly())
        {
            FString String;
            String.AppendChar(Character);
            InsertTextAtCursor(String);
            Reply = FReply::Handled();
        }
        else
        {
            Reply = FReply::Unhandled();
        }
    }
    else
    {
        Reply = SMultiLineEditableText::OnKeyChar( MyGeometry, InCharacterEvent );
    }

    return Reply;
}
