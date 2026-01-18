#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"  // ADD THIS LINE
#include "QuestDelegates.h"
#include "QuestComponent.generated.h"

class UQuest;
class UQuestComponent;

// Quest event delegates are declared in QuestDelegates.h (shared with Quest.h)
// We only declare FOnQuestListUpdated here as it's specific to QuestComponent
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
		// If both filters are empty, match everything
		if (!InClass && InTag.IsNone())
		{
			return !EnemyClass && EnemyTag.IsNone();
		}

		// Match class if specified
		bool ClassMatches = !InClass || (EnemyClass == InClass);

		// Match tag if specified
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
		// If looking for specific actor
		if (InActor)
		{
			return SpecificActor == InActor;
		}

		// Match class if specified
		bool ClassMatches = !InClass || (ObjectClass == InClass);

		// Match tag if specified
		bool TagMatches = InTag.IsNone() || (ObjectTag == InTag);

		return ClassMatches && TagMatches && !SpecificActor.IsValid();
	}
};

UCLASS(BlueprintType, ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class QUESTSYSTEM_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestComponent();

	/** Component Tick */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Begin Play */
	virtual void BeginPlay() override;

	/** Active Quests */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UQuest>> ActiveQuests;

	/** Completed Data Tasks */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<FQuestDataTask> CompletedDataTasks;

	/** Enemy Kill Tracking */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<FEnemyKillData> EnemyKills;

	/** Object Interaction Tracking */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	TArray<FObjectInteractionData> ObjectInteractions;

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

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnEnemyKilled OnEnemyKilled;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnObjectInteracted OnObjectInteracted;

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
	void CompleteDataTask(FName TaskName, const FString& Argument, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasCompletedDataTask(FName TaskName, const FString& Argument) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetDataTaskCount(FName TaskName, const FString& Argument) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearDataTasks();

	/** Enemy Kill Tracking */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RecordEnemyKill(AActor* Enemy, AActor* Killer);

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetEnemyKillCount(TSubclassOf<AActor> EnemyClass, FName EnemyTag) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearEnemyKills();

	/** Object Interaction Tracking */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RecordObjectInteraction(AActor* Object, AActor* Interactor);

	UFUNCTION(BlueprintPure, Category = "Quest")
	int32 GetObjectInteractionCount(TSubclassOf<AActor> ObjectClass, FName ObjectTag, AActor* SpecificActor) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ClearObjectInteractions();

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