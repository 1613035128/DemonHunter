// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DemonHunter : ModuleRules
{
	public DemonHunter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"DemonHunter",
			"DemonHunter/Variant_Platforming",
			"DemonHunter/Variant_Platforming/Animation",
			"DemonHunter/Variant_Combat",
			"DemonHunter/Variant_Combat/AI",
			"DemonHunter/Variant_Combat/Animation",
			"DemonHunter/Variant_Combat/Gameplay",
			"DemonHunter/Variant_Combat/Interfaces",
			"DemonHunter/Variant_Combat/UI",
			"DemonHunter/Variant_SideScrolling",
			"DemonHunter/Variant_SideScrolling/AI",
			"DemonHunter/Variant_SideScrolling/Gameplay",
			"DemonHunter/Variant_SideScrolling/Interfaces",
			"DemonHunter/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
