// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class SimpleTemplateEngine : ModuleRules
	{
		public SimpleTemplateEngine(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
                    "Json"
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"Runtime/SimpleTemplateEngine/Private",
				});
		}
	}
}
