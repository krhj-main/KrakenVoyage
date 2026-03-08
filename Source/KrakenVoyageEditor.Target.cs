// KrakenVoyageEditor.Target.cs

using UnrealBuildTool;
using System.Collections.Generic;

public class KrakenVoyageEditorTarget : TargetRules
{
	public KrakenVoyageEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("KrakenVoyage");
	}
}
