// Copyright Playspace S.L. 2017

#pragma once

#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "SSimpleTemplateEditableText.h"

class FText;
class ISlateStyle;
class USimpleTemplate;


/**
 * Implements the USimpleTemplate asset editor widget.
 */
class SSimpleTemplateEditor
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SSimpleTemplateEditor) { }
	SLATE_END_ARGS()

public:

	/** Virtual destructor. */
	virtual ~SSimpleTemplateEditor();

	/**
	 * Construct this widget
	 *
	 * @param InArgs The declaration data for this widget.
	 * @param InSimpleTemplate The USimpleTemplate asset to edit.
	 * @param InStyleSet The style set to use.
	 */
	void Construct(const FArguments& InArgs, USimpleTemplate* InSimpleTemplate, const TSharedRef<ISlateStyle>& InStyle);

	void GoTo(int32 LineNumber, int32 CharacterNumber);

private:

	/** Callback for text changes in the editable text box. */
	void HandleEditableTextBoxTextChanged(const FText& NewText);

	/** Callback for committed text in the editable text box. */
	void HandleEditableTextBoxTextCommitted(const FText& Comment, ETextCommit::Type CommitType);

	/** Callback for property changes in the Simple Template. */
	void HandleSimpleTemplatePropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent);

private:

	/** Holds the editable text box widget. */
	TSharedPtr<SSimpleTemplateEditableText> EditableTextBox;

	/** Pointer to the Simple Template that is being edited. */
	USimpleTemplate* SimpleTemplate;

	TSharedPtr<SScrollBar> HorizontalScrollbar;
	TSharedPtr<SScrollBar> VerticalScrollbar;
};
