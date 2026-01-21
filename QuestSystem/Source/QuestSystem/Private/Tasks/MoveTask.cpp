// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "Tasks/MoveTask.h"
#include "QuestComponent.h"
#include "GameFramework/Pawn.h"

UMoveTask::UMoveTask()
{
	DistanceToMove = 1000.0f; // Default 10 meters
	TotalDistance = 0.0f;
	TickInterval = 0.1f; // Check 10 times per second
}

void UMoveTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	if (!OwningComp)
	{
		return;
	}

	// Get the pawn's current location as starting point
	APawn* OwnerPawn = Cast<APawn>(OwningComp->GetOwner());
	if (OwnerPawn)
	{
		StartLocation = OwnerPawn->GetActorLocation();
		LastPosition = StartLocation;
		TotalDistance = 0.0f;

		// Set initial progress to 0
		SetProgress(0);
	}
}

void UMoveTask::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	if (!OwningComp || IsComplete())
	{
		return;
	}

	// Get the pawn's current position
	APawn* OwnerPawn = Cast<APawn>(OwningComp->GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	FVector CurrentPosition = OwnerPawn->GetActorLocation();

	// Calculate distance moved since last tick
	float DistanceThisTick = FVector::Dist(LastPosition, CurrentPosition);
	TotalDistance += DistanceThisTick;

	// Update last position
	LastPosition = CurrentPosition;

	// Calculate progress as a value between 0 and RequiredQuantity
	// RequiredQuantity acts as "100%" in the base class
	int32 NewProgress = FMath::Clamp(
		FMath::RoundToInt((TotalDistance / DistanceToMove) * RequiredQuantity),
		0,
		RequiredQuantity
	);

	// Only update if progress has changed
	if (NewProgress != CurrentProgress)
	{
		SetProgress(NewProgress);
	}
}

FText UMoveTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	// Convert cm to meters for display
	float MetersToMove = DistanceToMove / 100.0f;
	float MetersMoved = TotalDistance / 100.0f;

	return FText::FromString(FString::Printf(
		TEXT("Move %.1f / %.1f meters"),
		MetersMoved,
		MetersToMove
	));
}

FString UMoveTask::GetTaskNodeDescription_Implementation() const
{
	float MetersToMove = DistanceToMove / 100.0f;
	return FString::Printf(TEXT("Move %.1f meters"), MetersToMove);
}