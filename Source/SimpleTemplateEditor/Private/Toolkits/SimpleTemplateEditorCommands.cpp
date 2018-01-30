// Copyright Playspace S.L. 2017

#include "SimpleTemplateEditorCommands.h"

#define LOCTEXT_NAMESPACE "SimpleTemplateEditorCommands"

void FSimpleTemplateEditorCommands::RegisterCommands()
{
	UI_COMMAND(Compile, "Compile", "Compile a template", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Import, "Import", "Import a template from a .stf file", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Export, "Export", "Export a template to a .stf file.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE