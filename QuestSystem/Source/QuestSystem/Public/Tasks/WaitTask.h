#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "WaitTask.generated.h"

/**
 * Task that completes after waiting for a specified duration
 */
UCLASS(DisplayName = "Wait")
class QUESTSYSTEM_API UWaitTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UWaitTask();

	/** Duration to wait in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (ClampMin = "0.0"))
	float WaitDuration;

	/** If true, pause the timer when the game is paused */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bPauseWithGame;

	virtual void BeginTask_Implementation() override;
	virtual void TaskTick_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	/** Time elapsed since task started */
	UPROPERTY()
	float TimeElapsed;

	/** Last tick time for delta calculation */
	UPROPERTY()
	float LastTickTime;
};
