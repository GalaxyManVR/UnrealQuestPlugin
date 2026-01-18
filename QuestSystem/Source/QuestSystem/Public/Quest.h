
#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestDelegates.h"
#include <functional>
#include "Quest.generated.h"

class UQuestState;
class UQuestBranch;
class UQuestComponent;
class UEdGraph;

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
	NotStarted,
	InProgress,
	Succeeded,
	Failed
};

UCLASS(Blueprintable, BlueprintType)
class QUESTSYSTEM_API UQuest : public UObject
{
	GENERATED_BODY()

public:
	UQuest();

	/** Quest Information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (MultiLine = true))
	FText QuestDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bTracked;

	/** Quest State */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	EQuestStatus QuestStatus;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TObjectPtr<UQuestState> CurrentState;

	// Save/Load compatibility flags
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", SaveGame)
	TArray<UQuestState*> QuestStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", SaveGame)
	TArray<UQuestBranch*> QuestBranches;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	bool bIsInProgress;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	bool bHasSucceeded;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Quest")
	bool bHasFailed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<TObjectPtr<UQuestState>> InheritableStates;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	UQuestState* QuestStartState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	UQuestState* QuestSuccessState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	UQuestState* QuestFailureState;

	/** Owner */
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TObjectPtr<UQuestComponent> OwningComp;

	/** Editor Graph */
	UPROPERTY()
	TObjectPtr<UEdGraph> QuestGraph;

	/** Events */
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStarted OnQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestSucceeded OnQuestSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged;

	/** Lifecycle Functions */
	void Initialize(UQuestComponent* InOwningComp);
#if WITH_EDITOR
	/** Compile the quest from editor graph nodes into runtime branches */
	void CompileQuest();
#endif

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void BeginQuest();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RestartQuest();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ForgetQuest();

	void MoveToState(UQuestState* NewState);
	void OnBranchCompleted(UQuestBranch* Branch);

	/** Query Functions */
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestStarted() const { return QuestStatus != EQuestStatus::NotStarted; }

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestInProgress() const { return QuestStatus == EQuestStatus::InProgress; }

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasQuestSucceeded() const { return QuestStatus == EQuestStatus::Succeeded; }

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool HasQuestFailed() const { return QuestStatus == EQuestStatus::Failed; }

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestAtState(FName StateID) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<UQuestBranch*> GetActiveBranches() const;

	/** Hook for editor to trigger compilation before save */
	void SetPreSaveListener(std::function<void()> InListener)
	{
		PreSaveListener = InListener;
	}

	/** Override to trigger compilation before saving */

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
#endif

protected:
	UPROPERTY()
	TArray<TObjectPtr<UQuestBranch>> ActiveBranches;

	/** Activate all branches from a given state */
	void ActivateBranchesFromState(UQuestState* State);

	/** Create default states for new quest */
	void CreateDefaultStates();

private:
	/** Editor callback for compilation - not serialized */

	std::function<void()> PreSaveListener = nullptr;

	void UpdateStatusFlags()
	{
		bIsInProgress = (QuestStatus == EQuestStatus::InProgress);
		bHasSucceeded = (QuestStatus == EQuestStatus::Succeeded);
		bHasFailed = (QuestStatus == EQuestStatus::Failed);
	}
};