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
                    "Json"
				});

			PrivateIncludePaths.AddRange(
				new string[] {
					"Runtime/SimpleTemplate/Private",
				});
		}
	}
}
