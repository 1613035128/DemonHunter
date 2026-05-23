using UnrealBuildTool;

public class DemonHunterEditor : ModuleRules
{
	public DemonHunterEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[]  {"Core", "CoreUObject", "Engine", "DemonHunter", "Slate", "SlateCore"});
	}
}