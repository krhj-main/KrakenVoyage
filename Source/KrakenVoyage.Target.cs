// KrakenVoyage.Target.cs

using UnrealBuildTool;
using System.Collections.Generic;

public class KrakenVoyageTarget : TargetRules
{
	public KrakenVoyageTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("KrakenVoyage");

		// ★ Steam Shipping 빌드용 정의
		ProjectDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=4529530");
		ProjectDefinitions.Add("UE_PROJECT_STEAMGAMEDIR=KrakenVoyage");
		ProjectDefinitions.Add("UE_PROJECT_STEAMGAMEDESC=KrakenVoyage");
	}
}
