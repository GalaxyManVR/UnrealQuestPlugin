// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestDelegates.h"
#include "NarrativeComponent.generated.h"

class UQuest;
class UQuestState;

// Quest event delegates are declared in QuestDelegates.h (shared with Quest.h)
// We only declare FOnQuestListUpdated here as it's specific to NarrativeComponent
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestListUpdated, UNarrativeComponent*, Component);

USTRUCT(BlueprintType)
struct FNarrativeDataTask
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Data Task")
	FName TaskName;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Data Task")
	FString Argument;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Data Task")
	int32 Count;

	FNarrativeDataTask()
		: TaskName(NAME_None)
		, Argument(TEXT(""))
		, Count(0)
	{
	}

	FNarrativeDataTask(FName InName, const FString& InArg)
		: TaskName(InName)
		, Argument(InArg)
		, Count(0)
	{
	}
};

UCLASS(BlueprintType, ClassGroup = (Narrative), meta = (BlueprintSpawnableComponent))
class NARRATIVEQUESTSYSTEM_API UNarrativeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNarrativeComponent();

	/** Component Tick */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Begin Play */
	virtual void BeginPlay() override;

	/** Active Quests */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UQuest>> ActiveQuests;

	/** Completed Data Tasks */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<FNarrativeDataTask> CompletedDataTasks;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestListUpdated OnQuestListUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStarted OnQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestSucceeded OnQuestSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged;

	/** Quest Management */
	// Primary API: Accept UQuest* (asset reference) - This is the recommended way
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (DisplayName = "Begin Quest (Asset)"))
	UQuest* BeginQuest(UQuest* QuestAsset);

	// Legacy API: Also support TSubclassOf for backwards compatibility
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (DisplayName = "Begin Quest (Class)"))
	UQuest* BeginQuestByClass(TSubclassOf<UQuest> QuestClass);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ForgetQuest(UQuest* Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RestartQuest(UQuest* Quest);

	/** Quest Queries */
	// Primary API: Accept UQuest* (asset reference)
	UFUNCTION(BlueprintPure, Category = "Quest", meta = (DisplayName = "Is Quest Active (Asset)"))
	bool IsQuestActive(UQuest* QuestAsset) const;

	// Legacy API: Also support TSubclassOf for backwards compatibility
	UFUNCTION(BlueprintPure, Category = "Quest", meta = (DisplayName = "Is Quest Active (Class)"))
	bool IsQuestActiveByClass(TSubclassOf<UQuest> QuestClass) const;

	// Primary API: Accept UQuest* (asset reference)
	UFUNCTION(BlueprintPure, Category = "Quest", meta = (DisplayName = "Get Quest (Asset)"))
	UQuest* GetQuest(UQuest* QuestAsset) const;

	// Legacy API: Also support TSubclassOf for backwards compatibility
	UFUNCTION(BlueprintPure, Category = "Quest", meta = (DisplayName = "Get Quest (Class)"))
	UQuest* GetQuestByClass(TSubclassOf<UQuest> QuestClass) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<UQuest*> GetAllQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<UQuest*> GetInProgressQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<UQuest*> GetSucceededQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<UQuest*> GetFailedQuests() const;

	/** Data Tasks */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteNarrativeDataTask(FName TaskName, const FString& Argument, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasCompletedDataTask(FName TaskName, const FString& Argument) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetDataTaskCount(FName TaskName, const FString& Argument) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearDataTasks();

protected:
	/** Event forwarding handlers */
	UFUNCTION()
	void HandleQuestStarted(UQuest* Quest);

	UFUNCTION()
	void HandleQuestSucceeded(UQuest* Quest);

	UFUNCTION()
	void HandleQuestFailed(UQuest* Quest);

	UFUNCTION()
	void HandleQuestStateChanged(UQuest* Quest, UQuestState* NewState);
};