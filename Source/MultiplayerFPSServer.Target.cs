using UnrealBuildTool;
using System.Collections.Generic;

public class MultiplayerFPSServerTarget : TargetRules {
	public MultiplayerFPSServerTarget(TargetInfo Target) : base(Target) {
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.Add("MultiplayerFPS");
	}
}
