

#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "FinishDialogueTask.generated.h"

/**
 * Task that completes when a specific dialogue finishes
 * Useful for quests like "Go talk to NPC"
 */
UCLASS(DisplayName = "Finish Dialogue")
class QUESTSYSTEM_API UFinishDialogueTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UFinishDialogueTask();

	/** The dialogue that needs to be completed (could be a UObject reference to your dialogue system) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TSoftObjectPtr<UObject> Dialogue;

	/** The name/ID of the dialogue to complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName DialogueName;

	/** Add a waypoint to the NPC's location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bAddWaypointToAvatar;

	/** The actor tag to find the NPC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName NPCTag;

	virtual void BeginTask_Implementation() override;
	virtual void EndTask_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

	/** Call this when the dialogue finishes */
	UFUNCTION(BlueprintCallable, Category = "Task")
	void OnDialogueFinished();

protected:
	/** Bind to dialogue events */
	void BindToDialogueEvents();

	/** Unbind from dialogue events */
	void UnbindFromDialogueEvents();
};
