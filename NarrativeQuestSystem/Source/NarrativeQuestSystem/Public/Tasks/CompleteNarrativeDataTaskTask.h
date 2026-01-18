// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "CompleteNarrativeDataTaskTask.generated.h"

/**
 * Task that completes when a specific narrative data task is completed
 * This allows quests to track generic gameplay events without needing custom task classes
 */
UCLASS(DisplayName = "Complete Narrative Data Task")
class NARRATIVEQUESTSYSTEM_API UCompleteNarrativeDataTaskTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UCompleteNarrativeDataTaskTask();

	/** The data task to check for (e.g., "PickupItem", "DefeatEnemy") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName DataTask;

	/** The argument for the data task (e.g., "Apple", "Goblin") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FString Argument;

	/** If true, check for tasks completed before this quest started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bRetroactive;

	virtual void BeginTask_Implementation() override;
	virtual void TaskTick_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	/** The count when the task started (for retroactive tracking) */
	int32 StartingCount;
};
