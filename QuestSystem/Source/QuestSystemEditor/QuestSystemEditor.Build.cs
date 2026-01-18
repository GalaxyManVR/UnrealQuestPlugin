// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class QuestSystemEditor : ModuleRules
{
    public QuestSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
        "QuestSystemEditor/Public"
            }
        );

        PrivateIncludePaths.AddRange(
            new string[] {
        "QuestSystemEditor/Private"
            }
        );



        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "QuestSystem"
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