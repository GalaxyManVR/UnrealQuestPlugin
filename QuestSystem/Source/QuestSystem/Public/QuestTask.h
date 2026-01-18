#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestTask.generated.h"

class UQuestBranch;
class UQuest;
class UQuestComponent;

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class QUESTSYSTEM_API UQuestTask : public UObject
{
	GENERATED_BODY()

public:
	UQuestTask();

	/** Task Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FText TaskDescription;

	/** Optional description override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FText DescriptionOverride;

	/** Is this task optional? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bOptional;

	/** Current progress (0 to RequiredQuantity) */
	UPROPERTY(BlueprintReadOnly, Category = "Task")
	int32 CurrentProgress;

	/** How many needed to complete this task */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	int32 RequiredQuantity;

	/** How often this task ticks (0 = no ticking) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float TickInterval;

	/** Task Name (for data task completion) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName TaskName;

	/** Task Argument (for data task completion) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FString TaskArgument;

	/** References */
	UPROPERTY(BlueprintReadOnly, Category = "Task")
	TObjectPtr<UQuestBranch> OwningBranch;

	UPROPERTY(BlueprintReadOnly, Category = "Task")
	TObjectPtr<UQuest> OwningQuest;

	/** Cached quest component */
	UPROPERTY(BlueprintReadOnly, Category = "Task")
	TObjectPtr<UQuestComponent> OwningComp;

	/** Task Lifecycle - Base implementations */
	// FIXED: Added bInitializeProgress parameter with default value
	virtual void BeginTask(UQuestComponent* QuestComp, bool bInitializeProgress = true);
	virtual void EndTask();
	virtual void UpdateTaskProgress(UQuestComponent* QuestComp);
	virtual void TickTask();

	/** Virtual functions for child classes to override - called by the base implementations above */
	virtual void BeginTask_Implementation();
	virtual void EndTask_Implementation();

	/** Blueprint Native Events - Override these in Blueprint! */
	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnTaskBegin();
	virtual void OnTaskBegin_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnTaskEnd();
	virtual void OnTaskEnd_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void TaskTick();
	virtual void TaskTick_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnTaskProgressChanged(int32 NewProgress);
	virtual void OnTaskProgressChanged_Implementation(int32 NewProgress);

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Task")
	FText GetTaskDescription() const;
	virtual FText GetTaskDescription_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Task")
	FString GetTaskNodeDescription() const;
	virtual FString GetTaskNodeDescription_Implementation() const;

	/** Check if task is complete */
	UFUNCTION(BlueprintPure, Category = "Task")
	bool IsComplete() const;

	/** Set progress manually */
	UFUNCTION(BlueprintCallable, Category = "Task")
	void SetProgress(int32 NewProgress);

	/** Add progress incrementally */
	UFUNCTION(BlueprintCallable, Category = "Task")
	void AddProgress(int32 Amount);

	/** Get progress as percentage (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Task")
	float GetProgressPercentage() const;

	/** Check if this task is currently active */
	UFUNCTION(BlueprintPure, Category = "Task")
	bool IsTaskActive() const { return bIsActive; }

	// ADDED: Getter methods for save/load system
	UFUNCTION(BlueprintPure, Category = "Task")
	int32 GetCurrentProgress() const { return CurrentProgress; }

	UFUNCTION(BlueprintPure, Category = "Task")
	int32 GetRequiredQuantity() const { return RequiredQuantity; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Task")
	bool bIsActive;

	float TimeSinceLastTick;
};