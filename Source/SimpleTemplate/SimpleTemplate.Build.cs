// Copyright Playspace S.L. 2017

namespace UnrealBuildTool.Rules
{
	public class SimpleTemplate : ModuleRules
	{
		public SimpleTemplate(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
                    "Engine",
                    "Json",
                    "JsonUtilities"
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"Runtime/SimpleTemplate/Private",
				});
		}
	}
}
