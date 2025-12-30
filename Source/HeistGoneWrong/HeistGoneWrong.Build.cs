// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeistGoneWrong : ModuleRules
{
	public HeistGoneWrong(ReadOnlyTargetRules Target) : base(Target)
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
			"HeistGoneWrong",
			"HeistGoneWrong/Variant_Platforming",
			"HeistGoneWrong/Variant_Platforming/Animation",
			"HeistGoneWrong/Variant_Combat",
			"HeistGoneWrong/Variant_Combat/AI",
			"HeistGoneWrong/Variant_Combat/Animation",
			"HeistGoneWrong/Variant_Combat/Gameplay",
			"HeistGoneWrong/Variant_Combat/Interfaces",
			"HeistGoneWrong/Variant_Combat/UI",
			"HeistGoneWrong/Variant_SideScrolling",
			"HeistGoneWrong/Variant_SideScrolling/AI",
			"HeistGoneWrong/Variant_SideScrolling/Gameplay",
			"HeistGoneWrong/Variant_SideScrolling/Interfaces",
			"HeistGoneWrong/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
