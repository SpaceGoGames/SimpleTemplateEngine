// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SimpleTemplateEngineEditor : ModuleRules
{
	public SimpleTemplateEngineEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"MainFrame",
//				"WorkspaceMenuStructure",
			});

		PrivateIncludePaths.AddRange(
			new string[] {
				"SimpleTemplateEngineEditor/Private",
				"SimpleTemplateEngineEditor/Private/AssetTools",
				"SimpleTemplateEngineEditor/Private/Factories",
				"SimpleTemplateEngineEditor/Private/Shared",
				"SimpleTemplateEngineEditor/Private/Styles",
				"SimpleTemplateEngineEditor/Private/Toolkits",
				"SimpleTemplateEngineEditor/Private/Widgets",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"ContentBrowser",
				"Core",
				"CoreUObject",
				"DesktopWidgets",
				"EditorStyle",
				"Engine",
				"InputCore",
				"Projects",
				"Slate",
				"SlateCore",
				"SimpleTemplateEngine",
				"UnrealEd",
			});

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"UnrealEd",
//				"WorkspaceMenuStructure",
			});
	}
}
