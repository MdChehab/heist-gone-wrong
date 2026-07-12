// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Heist_Gone_Wrong : ModuleRules
{
	public Heist_Gone_Wrong(ReadOnlyTargetRules Target) : base(Target)
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
			"Heist_Gone_Wrong",
			"Heist_Gone_Wrong/Variant_Platforming",
			"Heist_Gone_Wrong/Variant_Platforming/Animation",
			"Heist_Gone_Wrong/Variant_Combat",
			"Heist_Gone_Wrong/Variant_Combat/AI",
			"Heist_Gone_Wrong/Variant_Combat/Animation",
			"Heist_Gone_Wrong/Variant_Combat/Gameplay",
			"Heist_Gone_Wrong/Variant_Combat/Interfaces",
			"Heist_Gone_Wrong/Variant_Combat/UI",
			"Heist_Gone_Wrong/Variant_SideScrolling",
			"Heist_Gone_Wrong/Variant_SideScrolling/AI",
			"Heist_Gone_Wrong/Variant_SideScrolling/Gameplay",
			"Heist_Gone_Wrong/Variant_SideScrolling/Interfaces",
			"Heist_Gone_Wrong/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
