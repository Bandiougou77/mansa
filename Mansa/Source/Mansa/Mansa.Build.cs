// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Mansa : ModuleRules
{
	public Mansa(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Mansa",
			"Mansa/Data",
			"Mansa/Movement",
			"Mansa/Escalade",
			"Mansa/Parcours",
			"Mansa/Input",
			"Mansa/Camera",
			"Mansa/Variant_Combat",
			"Mansa/Variant_Combat/AI",
			"Mansa/Variant_Combat/Animation",
			"Mansa/Variant_Combat/Gameplay",
			"Mansa/Variant_Combat/Interfaces",
			"Mansa/Variant_Combat/UI",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
