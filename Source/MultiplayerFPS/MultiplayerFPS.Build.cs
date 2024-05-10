using UnrealBuildTool;

public class MultiplayerFPS : ModuleRules {
	public MultiplayerFPS(ReadOnlyTargetRules Target) : base(Target) {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "UMG" });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		OptimizeCode = CodeOptimization.Never;
	}
}
