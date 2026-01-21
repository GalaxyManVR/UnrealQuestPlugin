// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n


#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "QuestSettings.generated.h"

//Settings for the Quest System
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Quest System"))
class QUESTSYSTEM_API UQuestSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UQuestSettings();

	// Maximum number of active quests a player can have at once (0 = unlimited) 
	UPROPERTY(config, EditAnywhere, Category = "Quest Limits", meta = (ClampMin = "0"))
	int32 MaxActiveQuests;

	//Whether to automatically save quest progress
	UPROPERTY(config, EditAnywhere, Category = "Quest System")
	bool bAutoSaveQuestProgress;

	//Whether to show debug information for quests in development builds 
	UPROPERTY(config, EditAnywhere, Category = "Debug")
	bool bShowQuestDebugInfo;

	//Default color for quest markers/waypoints 
	UPROPERTY(config, EditAnywhere, Category = "UI")
	FLinearColor DefaultQuestMarkerColor;

	// Default color for optional objective markers 
	UPROPERTY(config, EditAnywhere, Category = "UI")
	FLinearColor OptionalObjectiveColor;

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override;
#if WITH_EDITOR
	virtual FText GetSectionText() const override;
#endif
	// End of UDeveloperSettings interface
};
