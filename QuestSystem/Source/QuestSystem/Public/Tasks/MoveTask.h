

#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "MoveTask.generated.h"

/**
 * Task that completes when the player has moved a certain distance
 */
UCLASS(DisplayName = "Move Distance")
class QUESTSYSTEM_API UMoveTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UMoveTask();

	/** The distance the player needs to move (in cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float DistanceToMove;

	virtual void BeginTask_Implementation() override;
	virtual void TaskTick_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	/** The starting location */
	FVector StartLocation;

	/** Total distance moved */
	float TotalDistance;

	/** Last recorded position */
	FVector LastPosition;
};
