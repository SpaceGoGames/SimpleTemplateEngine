// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SimpleTemplateEditorSettings.h"

#include "Misc/Paths.h"


USimpleTemplateEditorSettings::USimpleTemplateEditorSettings()
	: BackgroundColor(FLinearColor::White)
	, ForegroundColor(FLinearColor::Black)
	, Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/DroidSansMono.ttf"), 10))
	, Margin(4.0f)
{ }
