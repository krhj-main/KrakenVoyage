// KrakenVoyage.Build.cs

using UnrealBuildTool;

public class KrakenVoyage : ModuleRules
{
	public KrakenVoyage(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"NetCore",
			"OnlineSubsystem",        // ★ Steam 세션 관리
			"OnlineSubsystemUtils"    // ★ 세션 유틸리티
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
