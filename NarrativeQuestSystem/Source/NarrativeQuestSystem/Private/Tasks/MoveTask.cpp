// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tasks/MoveTask.h"
#include "NarrativeComponent.h"
#include "GameFramework/Actor.h"

UMoveTask::UMoveTask()
{
	DistanceToMove = 10000.0f; // 100 meters default
	TotalDistance = 0.0f;
	TickInterval = 0.1f; // Check 10 times per second
}

void UMoveTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	if (OwningComp && OwningComp->GetOwner())
	{
		StartLocation = OwningComp->GetOwner()->GetActorLocation();
		LastPosition = StartLocation;
		TotalDistance = 0.0f;
	}
}

void UMoveTask::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	if (!OwningComp || !OwningComp->GetOwner() || IsComplete())
	{
		return;
	}

	AActor* Owner = OwningComp->GetOwner();
	FVector CurrentPosition = Owner->GetActorLocation();

	// Calculate distance moved since last check
	float DistanceMoved = FVector::Dist(LastPosition, CurrentPosition);
	TotalDistance += DistanceMoved;
	LastPosition = CurrentPosition;

	// Update progress (use distance as a percentage)
	int32 ProgressPercent = FMath::Min(100, (int32)((TotalDistance / DistanceToMove) * 100.0f));
	
	if (ProgressPercent >= 100)
	{
		SetProgress(RequiredQuantity);
	}
}

FText UMoveTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	// Convert cm to meters
	float DistanceInMeters = DistanceToMove / 100.0f;
	return FText::Format(
		FText::FromString(TEXT("Move {0} meters")),
		FText::AsNumber((int32)DistanceInMeters)
	);
}

FString UMoveTask::GetTaskNodeDescription_Implementation() const
{
	float DistanceInMeters = DistanceToMove / 100.0f;
	return FString::Printf(TEXT("Move: %.0f meters"), DistanceInMeters);
}
