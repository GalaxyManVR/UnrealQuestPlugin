#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "InteractWithObjectTask.generated.h"

class AActor;

/**
 * Task that completes when the player interacts with specific objects
 */
UCLASS(DisplayName = "Interact With Object")
class QUESTSYSTEM_API UInteractWithObjectTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UInteractWithObjectTask();

	/** The class of object to interact with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TSubclassOf<AActor> ObjectClass;

	/** Optional: Filter by object tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName ObjectTag;

	/** Optional: Specific actor to interact with (overrides class and tag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	AActor* SpecificActor;

	/** If true, count interactions that happened before this task started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bRetroactive;

	/** Add a waypoint marker to the object */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bAddWaypoint;

	virtual void BeginTask_Implementation() override;
	virtual void EndTask_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	/** Called when an object is interacted with */
	UFUNCTION()
	void OnObjectInteracted(AActor* InteractedObject, AActor* Interactor);

	/** Bind to interaction events */
	void BindToInteractionEvents();

	/** Unbind from interaction events */
	void UnbindFromInteractionEvents();

	/** Check if an object matches our filter criteria */
	bool DoesObjectMatchFilter(AActor* Object) const;

	/** Starting interaction count for non-retroactive tracking */
	UPROPERTY()
	int32 StartingInteractionCount;
};
