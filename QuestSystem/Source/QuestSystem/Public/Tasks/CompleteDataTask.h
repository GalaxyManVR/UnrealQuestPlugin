// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "CompleteDataTask.generated.h"

/**
 * Task that completes when a specific narrative data task is completed
 * This allows quests to track generic gameplay events without needing custom task classes
 */
UCLASS(DisplayName = "Complete Data Task")
class QUESTSYSTEM_API UCompleteDataTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UCompleteDataTask();

	// The data task to check for (e.g., "PickupItem", "DefeatEnemy")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName DataTask;

	// The argument for the data task (e.g., "Apple", "Goblin")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FString Argument;

	// If true, check for tasks completed before this quest started
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bRetroactive;

	virtual void BeginTask_Implementation() override;
	virtual void TaskTick_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	// The count when the task started (for non-retroactive tracking)
	UPROPERTY()
	int32 StartingCount;
};