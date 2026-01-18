#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "QuestDelegates.h"
#include "QuestComponent.generated.h"

class UQuest;
class UQuestState;

// Quest event delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestListUpdated, UQuestComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyKilled, AActor*, Enemy, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectInteracted, AActor*, Object, AActor*, Interactor);

USTRUCT(BlueprintType)
struct FQuestDataTask
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Data Task")
	FName TaskName;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Data Task")
	FString Argument;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Data Task")
	int32 Count;

	FQuestDataTask()
		: TaskName(NAME_None)
		, Argument(TEXT(""))
		, Count(0)
	{
	}

	FQuestDataTask(FName InName, const FString& InArg)
		: TaskName(InName)
		, Argument(InArg)
		, Count(0)
	{
	}
};

USTRUCT(BlueprintType)
struct FEnemyKillData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TSubclassOf<AActor> EnemyClass;

	UPROPERTY(SaveGame)
	FName EnemyTag;

	UPROPERTY(SaveGame)
	int32 KillCount;

	FEnemyKillData()
		: EnemyClass(nullptr)
		, EnemyTag(NAME_None)
		, KillCount(0)
	{
	}

	bool Matches(TSubclassOf<AActor> InClass, FName InTag) const
	{
		if (!InClass && InTag.IsNone())
		{
			return !EnemyClass && EnemyTag.IsNone();
		}

		bool ClassMatches = !InClass || (EnemyClass == InClass);
		bool TagMatches = InTag.IsNone() || (EnemyTag == InTag);

		return ClassMatches && TagMatches;
	}
};

USTRUCT(BlueprintType)
struct FObjectInteractionData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TSubclassOf<AActor> ObjectClass;

	UPROPERTY(SaveGame)
	FName ObjectTag;

	UPROPERTY(SaveGame)
	TWeakObjectPtr<AActor> SpecificActor;

	UPROPERTY(SaveGame)
	int32 InteractionCount;

	FObjectInteractionData()
		: ObjectClass(nullptr)
		, ObjectTag(NAME_None)
		, SpecificActor(nullptr)
		, InteractionCount(0)
	{
	}

	bool Matches(TSubclassOf<AActor> InClass, FName InTag, AActor* InActor) const
	{
		if (InActor)
		{
			return SpecificActor == InActor;
		}

		bool ClassMatches = !InClass || (ObjectClass == InClass);
		bool TagMatches = InTag.IsNone() || (ObjectTag == InTag);

		return ClassMatches && TagMatches && !SpecificActor.IsValid();
	}
};

/**
 * Branch save data for a single branch
 */
USTRUCT(BlueprintType)
struct FQuestBranchSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	bool bIsActive;

	UPROPERTY(SaveGame)
	bool bIsComplete;

	UPROPERTY(SaveGame)
	TArray<int32> TaskProgress;

	UPROPERTY(SaveGame)
	TArray<int32> TaskActiveStates;  // Using int32 instead of bool for Unreal compatibility

	UPROPERTY(SaveGame)
	TArray<int32> TaskCompleteStates;  // Using int32 instead of bool for Unreal compatibility

	FQuestBranchSaveData()
		: bIsActive(false)
		, bIsComplete(false)
	{
	}
};

/**
 * Save data for a single quest
 */
USTRUCT(BlueprintType)
struct FQuestInstanceSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FString QuestPath;

	UPROPERTY(SaveGame)
	bool bIsInProgress;

	UPROPERTY(SaveGame)
	bool bHasSucceeded;

	UPROPERTY(SaveGame)
	bool bHasFailed;

	UPROPERTY(SaveGame)
	int32 CurrentStateIndex;

	UPROPERTY(SaveGame)
	TArray<FQuestBranchSaveData> Branches;

	FQuestInstanceSaveData()
		: bIsInProgress(false)
		, bHasSucceeded(false)
		, bHasFailed(false)
		, CurrentStateIndex(0)
	{
	}
};

/**
 * Complete save data structure for the Quest Component
 * Everything you need to save/load in one struct!
 */
USTRUCT(BlueprintType)
struct QUESTSYSTEM_API FQuestComponentSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quest Save")
	TArray<FQuestDataTask> CompletedDataTasks;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quest Save")
	TArray<FEnemyKillData> EnemyKills;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quest Save")
	TArray<FObjectInteractionData> ObjectInteractions;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quest Save")
	TArray<FQuestInstanceSaveData> ActiveQuests;
};

UCLASS(BlueprintType, ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class QUESTSYSTEM_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;

	// ========================================
	// SIMPLE SAVE/LOAD INTERFACE
	// ========================================

	/**
	 * Get ALL quest data in one struct - ready to save!
	 * Use this to get everything you need to save
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	FQuestComponentSaveData GetSaveData() const;

	/**
	 * Load ALL quest data from one struct
	 * Use this to restore everything from a save
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void LoadSaveData(const FQuestComponentSaveData& SaveData);

	/**
	 * Clear all quest data (use before loading or for new game)
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void ClearAllQuestData();

	// ========================================
	// QUEST DATA
	// ========================================

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UQuest>> ActiveQuests;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<FQuestDataTask> CompletedDataTasks;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<FEnemyKillData> EnemyKills;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<FObjectInteractionData> ObjectInteractions;

	// ========================================
	// EVENTS
	// ========================================

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

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnEnemyKilled OnEnemyKilled;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnObjectInteracted OnObjectInteracted;

	// ========================================
	// QUEST MANAGEMENT
	// ========================================

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (DisplayName = "Begin Quest (Asset)"))
	UQuest* BeginQuest(UQuest* QuestAsset);

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (DisplayName = "Begin Quest (Class)"))
	UQuest* BeginQuestByClass(TSubclassOf<UQuest> QuestClass);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ForgetQuest(UQuest* Quest);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RestartQuest(UQuest* Quest);

	UFUNCTION(BlueprintPure, Category = "Quest", meta = (DisplayName = "Is Quest Active (Asset)"))
	bool IsQuestActive(UQuest* QuestAsset) const;

	UFUNCTION(BlueprintPure, Category = "Quest", meta = (DisplayName = "Is Quest Active (Class)"))
	bool IsQuestActiveByClass(TSubclassOf<UQuest> QuestClass) const;

	UFUNCTION(BlueprintPure, Category = "Quest", meta = (DisplayName = "Get Quest (Asset)"))
	UQuest* GetQuest(UQuest* QuestAsset) const;

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

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteDataTask(FName TaskName, const FString& Argument, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasCompletedDataTask(FName TaskName, const FString& Argument) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetDataTaskCount(FName TaskName, const FString& Argument) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearDataTasks();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RecordEnemyKill(AActor* Enemy, AActor* Killer);

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetEnemyKillCount(TSubclassOf<AActor> EnemyClass, FName EnemyTag) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearEnemyKills();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RecordObjectInteraction(AActor* Object, AActor* Interactor);

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetObjectInteractionCount(TSubclassOf<AActor> ObjectClass, FName ObjectTag, AActor* SpecificActor) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearObjectInteractions();

protected:
	UFUNCTION()
	void HandleQuestStarted(UQuest* Quest);

	UFUNCTION()
	void HandleQuestSucceeded(UQuest* Quest);

	UFUNCTION()
	void HandleQuestFailed(UQuest* Quest);

	UFUNCTION()
	void HandleQuestStateChanged(UQuest* Quest, UQuestState* NewState);
};