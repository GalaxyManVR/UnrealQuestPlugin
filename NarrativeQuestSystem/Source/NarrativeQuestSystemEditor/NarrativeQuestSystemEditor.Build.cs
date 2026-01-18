// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NarrativeQuestSystemEditor : ModuleRules
{
    public NarrativeQuestSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
        "NarrativeQuestSystemEditor/Public"
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
        "NarrativeQuestSystemEditor/Private"
            }
        );



        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "NarrativeQuestSystem"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",
                "AssetTools",
                "Slate",
                "SlateCore",
                "GraphEditor",
                "PropertyEditor",
                "AppFramework",  // Updated from EditorStyle for UE5
                "Kismet",
                "KismetCompiler",
                "ApplicationCore",
                "ToolMenus",
                "Projects",
                "InputCore",
                "EditorWidgets",  // Added for custom editor widgets
                "ToolWidgets"     // Added for additional widget support
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}