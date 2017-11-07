// Copyright Playspace S.L. 2017

#include "Styles/SimpleTemplateEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "EditorStyleSet.h"
#include "Interfaces/IPluginManager.h"
#include "SlateOptMacros.h"


#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FSimpleTemplateStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(StyleSet->RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define TTF_CORE_FONT(RelativePath, ...) FSlateFontInfo(StyleSet->RootToCoreContentDir(RelativePath, TEXT(".ttf") ), __VA_ARGS__)

FString FSimpleTemplateStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("SimpleTemplate"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr< FSlateStyleSet > FSimpleTemplateStyle::StyleSet = nullptr;
TSharedPtr< class ISlateStyle > FSimpleTemplateStyle::Get() { return StyleSet; }

FName FSimpleTemplateStyle::GetStyleSetName()
{
	static FName SimpleTemplateStyleName(TEXT("SimpleTemplateStyle"));
	return SimpleTemplateStyleName;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FSimpleTemplateStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon16x16(16.0f, 16.0f);	
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	// Class icons
	{
		StyleSet->Set("ClassIcon.SimpleTemplate", new IMAGE_PLUGIN_BRUSH("Icons/SimpleTemplate_16x", Icon16x16));
		StyleSet->Set("ClassThumbnail.SimpleTemplate", new IMAGE_PLUGIN_BRUSH("Icons/SimpleTemplate_64x", Icon64x64));
	}

	// Template editor
	{
		StyleSet->Set("SimpleTemplateEditor.Compile", new IMAGE_PLUGIN_BRUSH("Icons/icon_compile_40x", Icon40x40));
		StyleSet->Set("SimpleTemplateEditor.Compile.Dirty", new IMAGE_PLUGIN_BRUSH("Icons/icon_compile_dirty_40x", Icon40x40));
		StyleSet->Set("SimpleTemplateEditor.Compile.Error", new IMAGE_PLUGIN_BRUSH("Icons/icon_compile_error_40x", Icon40x40));
		StyleSet->Set("SimpleTemplateEditor.Export", new IMAGE_PLUGIN_BRUSH("Icons/icon_export_40x", Icon40x40));
		StyleSet->Set("SimpleTemplateEditor.Import", new IMAGE_PLUGIN_BRUSH("Icons/icon_import_40x", Icon40x40));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


#undef IMAGE_PLUGIN_BRUSH
#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef TTF_FONT
#undef TTF_CORE_FONT

void FSimpleTemplateStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
	}
}
