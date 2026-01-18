// QuestTaskTypes.h
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "QuestTaskTypes.generated.h"

/**
 * Simple data task - completes when a data task is finished
 * This is the most common task type and works entirely through the data task system
 */
UCLASS(DisplayName = "Data Task", meta = (ShortTooltip = "Completes when a specific data task is completed"))
class NARRATIVEQUESTSYSTEM_API UQuestTask_DataTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UQuestTask_DataTask();

	// All functionality is handled by base class
	// Just set TaskName, TaskArgument, and RequiredQuantity in the editor
};

/**
 * Task that completes based on a custom Blueprint condition
 * Override "Check Task Condition" in Blueprint to implement custom logic
 */
UCLASS(Blueprintable, DisplayName = "Blueprint Condition Task")
class NARRATIVEQUESTSYSTEM_API UQuestTask_BlueprintCondition : public UQuestTask
{
	GENERATED_BODY()

public:
	UQuestTask_BlueprintCondition();

	/** How often to check the condition (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float CheckInterval;

	/** Override this in Blueprint to check your custom condition */
	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	bool CheckTaskCondition();
	virtual bool CheckTaskCondition_Implementation();

protected:
	virtual void OnTaskBegin_Implementation() override;
	virtual void TaskTick_Implementation() override;

private:
	float TimeUntilNextCheck;
};

/**
 * Task that requires the player to be in a specific location
 */
UCLASS(DisplayName = "Location Task", meta = (ShortTooltip = "Completes when player reaches a location"))
class NARRATIVEQUESTSYSTEM_API UQuestTask_Location : public UQuestTask
{
	GENERATED_BODY()

public:
	UQuestTask_Location();

	/** Target location to reach */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FVector TargetLocation;

	/** How close the player needs to be */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float AcceptanceRadius;

	/** Check every X seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float CheckInterval;

protected:
	virtual void OnTaskBegin_Implementation() override;
	virtual void TaskTick_Implementation() override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

private:
	float TimeUntilNextCheck;
};

/**
 * Task that automatically completes after a delay
 */
UCLASS(DisplayName = "Wait Task", meta = (ShortTooltip = "Completes after a time delay"))
class NARRATIVEQUESTSYSTEM_API UQuestTask_Wait : public UQuestTask
{
	GENERATED_BODY()

public:
	UQuestTask_Wait();

	/** How long to wait in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float WaitTime;

protected:
	virtual void OnTaskBegin_Implementation() override;
	virtual void TaskTick_Implementation() override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

private:
	float TimeElapsed;
};

/**
 * Manual completion task - must be completed via Blueprint or C++
 * Useful for custom quest logic
 */
UCLASS(DisplayName = "Manual Task", meta = (ShortTooltip = "Must be manually completed via Blueprint"))
class NARRATIVEQUESTSYSTEM_API UQuestTask_Manual : public UQuestTask
{
	GENERATED_BODY()

public:
	UQuestTask_Manual();

	/** Complete this task manually */
	UFUNCTION(BlueprintCallable, Category = "Task")
	void CompleteTask();

	/** Set specific progress */
	UFUNCTION(BlueprintCallable, Category = "Task")
	void SetTaskProgress(int32 Progress);
};