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
			"EnhancedInput",    // Enhanced Input System
			"UMG",              // UI (UserWidget, HUD)
			"Slate",            // UI 프레임워크
			"SlateCore",        // UI 프레임워크 코어
			"NetCore"           // 네트워크 기본
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// === Phase 4에서 활성화 (Steam 연동 시) ===
		// PublicDependencyModuleNames.Add("OnlineSubsystem");
		// PublicDependencyModuleNames.Add("OnlineSubsystemSteam");

		// === Phase 5에서 활성화 (보이스챗 시) ===
		// PrivateDependencyModuleNames.Add("VoiceChat");
	}
}
