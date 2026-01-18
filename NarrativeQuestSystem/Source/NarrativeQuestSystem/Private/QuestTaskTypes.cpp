// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestTaskTypes.h"
#include "NarrativeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

// ============================================================================
// UQuestTask_DataTask
// ============================================================================

UQuestTask_DataTask::UQuestTask_DataTask()
{
	// Data tasks work automatically through the NarrativeComponent system
	// No special implementation needed
}

// ============================================================================
// UQuestTask_BlueprintCondition
// ============================================================================

UQuestTask_BlueprintCondition::UQuestTask_BlueprintCondition()
{
	CheckInterval = 1.0f;
	TimeUntilNextCheck = 0.0f;
	TickInterval = 0.1f; // Check 10 times per second
}

bool UQuestTask_BlueprintCondition::CheckTaskCondition_Implementation()
{
	// Override this in Blueprint to implement custom logic
	return false;
}

void UQuestTask_BlueprintCondition::OnTaskBegin_Implementation()
{
	Super::OnTaskBegin_Implementation();
	TimeUntilNextCheck = 0.0f;
}

void UQuestTask_BlueprintCondition::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TimeUntilNextCheck += World->GetDeltaSeconds();

	if (TimeUntilNextCheck >= CheckInterval)
	{
		TimeUntilNextCheck = 0.0f;

		// Check the Blueprint condition
		if (CheckTaskCondition())
		{
			SetProgress(RequiredQuantity);
		}
	}
}

// ============================================================================
// UQuestTask_Location
// ============================================================================

UQuestTask_Location::UQuestTask_Location()
{
	TargetLocation = FVector::ZeroVector;
	AcceptanceRadius = 500.0f;
	CheckInterval = 0.5f;
	TimeUntilNextCheck = 0.0f;
	TickInterval = 0.1f;
	TaskDescription = FText::FromString(TEXT("Reach the target location"));
}

void UQuestTask_Location::OnTaskBegin_Implementation()
{
	Super::OnTaskBegin_Implementation();
	TimeUntilNextCheck = 0.0f;
}

void UQuestTask_Location::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	if (!OwningComp)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TimeUntilNextCheck += World->GetDeltaSeconds();

	if (TimeUntilNextCheck >= CheckInterval)
	{
		TimeUntilNextCheck = 0.0f;

		// Get the player pawn
		AActor* Owner = OwningComp->GetOwner();
		APawn* Pawn = Cast<APawn>(Owner);

		if (Pawn)
		{
			float Distance = FVector::Dist(Pawn->GetActorLocation(), TargetLocation);

			if (Distance <= AcceptanceRadius)
			{
				SetProgress(RequiredQuantity);
			}
		}
	}
}

FString UQuestTask_Location::GetTaskNodeDescription_Implementation() const
{
	if (!TaskDescription.IsEmpty())
	{
		return TaskDescription.ToString();
	}

	return FString::Printf(TEXT("Reach location (%.0f, %.0f, %.0f) within %.0fm"),
		TargetLocation.X, TargetLocation.Y, TargetLocation.Z, AcceptanceRadius);
}

// ============================================================================
// UQuestTask_Wait
// ============================================================================

UQuestTask_Wait::UQuestTask_Wait()
{
	WaitTime = 5.0f;
	TimeElapsed = 0.0f;
	TickInterval = 0.1f;
	TaskDescription = FText::FromString(TEXT("Wait"));
}

void UQuestTask_Wait::OnTaskBegin_Implementation()
{
	Super::OnTaskBegin_Implementation();
	TimeElapsed = 0.0f;
}

void UQuestTask_Wait::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TimeElapsed += World->GetDeltaSeconds();

	// Update progress based on time
	float Progress = FMath::Clamp(TimeElapsed / WaitTime, 0.0f, 1.0f);
	SetProgress(FMath::FloorToInt(Progress * RequiredQuantity));
}

FString UQuestTask_Wait::GetTaskNodeDescription_Implementation() const
{
	if (!TaskDescription.IsEmpty())
	{
		return FString::Printf(TEXT("%s (%.1fs)"), *TaskDescription.ToString(), WaitTime);
	}

	return FString::Printf(TEXT("Wait %.1f seconds"), WaitTime);
}

// ============================================================================
// UQuestTask_Manual
// ============================================================================

UQuestTask_Manual::UQuestTask_Manual()
{
	TaskDescription = FText::FromString(TEXT("Manual Task"));
}

void UQuestTask_Manual::CompleteTask()
{
	SetProgress(RequiredQuantity);
}

void UQuestTask_Manual::SetTaskProgress(int32 Progress)
{
	SetProgress(Progress);
}