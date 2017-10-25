// Copyright Playspace S.L. 2017

using UnrealBuildTool;

public class SimpleTemplateEditor : ModuleRules
{
	public SimpleTemplateEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"SimpleTemplateEditor/Private",
				"SimpleTemplateEditor/Private/AssetTools",
				"SimpleTemplateEditor/Private/Factories",
				"SimpleTemplateEditor/Private/Shared",
				"SimpleTemplateEditor/Private/Styles",
				"SimpleTemplateEditor/Private/Toolkits",
				"SimpleTemplateEditor/Private/Widgets",
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
				"SimpleTemplate",
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
