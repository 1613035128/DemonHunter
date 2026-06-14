using UnrealBuildTool;

public class DemonHunterEditor : ModuleRules
{
	public DemonHunterEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[]  {"Core", "CoreUObject", "Engine", "DemonHunter", "Slate", "SlateCore"});
		PublicDefinitions.Add("UE_USE_LITE_ENSURES=0");
	}
}