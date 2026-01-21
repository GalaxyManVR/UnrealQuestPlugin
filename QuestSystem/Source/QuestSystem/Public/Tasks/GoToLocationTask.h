// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "GoToLocationTask.generated.h"

//Task that completes when the player reaches a specific location
 
UCLASS(DisplayName = "Go To Location")
class QUESTSYSTEM_API UGoToLocationTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UGoToLocationTask();

	//The location the player needs to reach
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FVector GoalLocation;

	//How close the player needs to get
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float DistanceTolerance;

	//Friendly name for the location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FText FriendlyLocationName;

	//Optional: Track an actor class instead of a fixed location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TSubclassOf<AActor> GoalActorClass;

	// Optional: Tag to find the specific actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName GoalActorTag;

	// If true, complete when leaving the location instead of arriving 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bInvert;

	//Add a waypoint marker to the location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bAddWaypoint;

	virtual void BeginTask_Implementation() override;
	virtual void EndTask_Implementation() override;
	virtual void TaskTick_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	//Get the current goal location (handles both fixed and actor-based goals)
	FVector GetCurrentGoalLocation() const;

	//Check if we're at the goal
	bool IsAtGoal() const;

	//The tracked actor if using GoalActorClass
	UPROPERTY()
	TObjectPtr<AActor> TrackedActor;
};
