// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tasks/GoToLocationTask.h"
#include "NarrativeComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UGoToLocationTask::UGoToLocationTask()
{
	DistanceTolerance = 200.0f;
	bInvert = false;
	bAddWaypoint = false;
	TickInterval = 0.5f; // Check every half second by default
}

void UGoToLocationTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	// If using an actor class, try to find it
	if (GoalActorClass && OwningComp)
	{
		UWorld* World = OwningComp->GetWorld();
		if (World)
		{
			// Find actors of the specified class
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(World, GoalActorClass, FoundActors);

			// If we have a tag, filter by tag
			if (!GoalActorTag.IsNone())
			{
				for (AActor* Actor : FoundActors)
				{
					if (Actor && Actor->Tags.Contains(GoalActorTag))
					{
						TrackedActor = Actor;
						break;
					}
				}
			}
			else if (FoundActors.Num() > 0)
			{
				// Just use the first one
				TrackedActor = FoundActors[0];
			}
		}
	}

	// TODO: Add waypoint if bAddWaypoint is true
	// This would integrate with your game's waypoint system
}

void UGoToLocationTask::EndTask_Implementation()
{
	Super::EndTask_Implementation();

	// TODO: Remove waypoint if it was added
}

void UGoToLocationTask::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	if (IsAtGoal())
	{
		SetProgress(RequiredQuantity);
	}
}

FText UGoToLocationTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	if (!FriendlyLocationName.IsEmpty())
	{
		if (bInvert)
		{
			return FText::Format(FText::FromString(TEXT("Leave {0}")), FriendlyLocationName);
		}
		else
		{
			return FText::Format(FText::FromString(TEXT("Go to {0}")), FriendlyLocationName);
		}
	}

	return bInvert ? FText::FromString(TEXT("Leave the location")) : FText::FromString(TEXT("Go to the location"));
}

FString UGoToLocationTask::GetTaskNodeDescription_Implementation() const
{
	if (!FriendlyLocationName.IsEmpty())
	{
		return FString::Printf(TEXT("Go To: %s"), *FriendlyLocationName.ToString());
	}
	return TEXT("Go To Location");
}

FVector UGoToLocationTask::GetCurrentGoalLocation() const
{
	if (TrackedActor)
	{
		return TrackedActor->GetActorLocation();
	}
	return GoalLocation;
}

bool UGoToLocationTask::IsAtGoal() const
{
	if (!OwningComp || !OwningComp->GetOwner())
	{
		return false;
	}

	AActor* Owner = OwningComp->GetOwner();
	FVector CurrentLocation = Owner->GetActorLocation();
	FVector Goal = GetCurrentGoalLocation();

	float Distance = FVector::Dist(CurrentLocation, Goal);

	if (bInvert)
	{
		// We want to be OUTSIDE the tolerance
		return Distance > DistanceTolerance;
	}
	else
	{
		// We want to be INSIDE the tolerance
		return Distance <= DistanceTolerance;
	}
}
