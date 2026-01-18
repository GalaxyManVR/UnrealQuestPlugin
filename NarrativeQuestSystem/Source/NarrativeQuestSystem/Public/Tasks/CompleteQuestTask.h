// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "CompleteQuestTask.generated.h"

class UQuest;

/**
 * Task that completes when another quest is completed
 */
UCLASS(DisplayName = "Complete Quest")
class NARRATIVEQUESTSYSTEM_API UCompleteQuestTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UCompleteQuestTask();

	/** The quest that needs to be completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TSubclassOf<UQuest> QuestToComplete;

	virtual void BeginTask_Implementation() override;
	virtual void EndTask_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	/** Callback for when the target quest succeeds */
	UFUNCTION()
	void OnTargetQuestSucceeded(UQuest* Quest);

	/** The quest instance we're tracking */
	UPROPERTY()
	TObjectPtr<UQuest> TrackedQuest;
};
