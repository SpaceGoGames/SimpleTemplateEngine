// Copyright Playspace S.L. 2017

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SimpleTemplateEditorStyle.h"

class FSimpleTemplateEditorCommands : public TCommands<FSimpleTemplateEditorCommands>
{
public:
	FSimpleTemplateEditorCommands()
		: TCommands<FSimpleTemplateEditorCommands>(
			TEXT("SimpleTemplateEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "SimpleTemplateEditor", "SimpleTemplate Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FSimpleTemplateEditorStyle::GetName()
			)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;
	// End of TCommand<> interface

public:
	TSharedPtr<FUICommandInfo> Compile;
	TSharedPtr<FUICommandInfo> Import;
	TSharedPtr<FUICommandInfo> Export;
};
