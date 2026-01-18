

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestBranch.generated.h"

class UQuest;
class UQuestState;
class UQuestTask;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class QUESTSYSTEM_API UQuestBranch : public UObject
{
	GENERATED_BODY()

public:
	UQuestBranch();

	// Branch ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Branch")
	FName ID;

	// Branch Description 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Branch")
	FText Description;

	// Tasks that must be completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Branch")
	TArray<TObjectPtr<UQuestTask>> QuestTasks;

	//Hide this branch from UI 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Branch")
	bool bHidden;

	// Is this branch currently active? 
	UPROPERTY(BlueprintReadOnly, Category = "Branch")
	bool bIsActive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Branch")
	TObjectPtr<UQuestState> FromState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Branch")
	TObjectPtr<UQuestState> ToState;

	// Owning Quest 
	UPROPERTY(BlueprintReadOnly, Category = "Branch")
	TObjectPtr<UQuest> OwningQuest;

	// Initialize this branch 
	void Initialize(UQuest* InQuest, UQuestState* InFromState, UQuestState* InToState);

	// Activate this branch (start checking tasks) 
	void ActivateBranch(class UQuestComponent* QuestComp);

	// Deactivate this branch 
	void DeactivateBranch();

	// Check if all tasks are complete 
	UFUNCTION(BlueprintPure, Category = "Branch")
	bool AreAllTasksComplete() const;

	//Check task progress (called every tick or when tasks update)
	void CheckTaskProgress();

	// Get completion percentage
	UFUNCTION(BlueprintPure, Category = "Branch")
	float GetCompletionPercentage() const;

	// Is this branch complete?
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Branch")
	bool bIsComplete;

protected:
	// Cached Quest Component for checking task completion
	UPROPERTY()
	TObjectPtr<class UQuestComponent> CachedQuestComp;
};