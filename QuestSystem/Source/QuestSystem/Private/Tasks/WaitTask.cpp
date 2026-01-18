#include "Tasks/WaitTask.h"
#include "QuestComponent.h"
#include "Engine/World.h"  // ADD THIS LINE

UWaitTask::UWaitTask()
{
	WaitDuration = 10.0f;
	bPauseWithGame = true;
	TimeElapsed = 0.0f;
	LastTickTime = 0.0f;
	TickInterval = 0.1f; // Update 10 times per second for smooth progress
}

void UWaitTask::BeginTask_Implementation()
{
	Super::BeginTask_Implementation();

	TimeElapsed = 0.0f;
	
	if (OwningComp && OwningComp->GetWorld())
	{
		LastTickTime = OwningComp->GetWorld()->GetTimeSeconds();
	}

	SetProgress(0);
}

void UWaitTask::TaskTick_Implementation()
{
	Super::TaskTick_Implementation();

	if (!OwningComp || IsComplete())
	{
		return;
	}

	UWorld* World = OwningComp->GetWorld();
	if (!World)
	{
		return;
	}

	// Calculate delta time
	float CurrentTime = World->GetTimeSeconds();
	float DeltaTime = CurrentTime - LastTickTime;
	LastTickTime = CurrentTime;

	// If pausing with game, check if game is paused
	if (bPauseWithGame && World->IsPaused())
	{
		return;
	}

	// Update time elapsed
	TimeElapsed += DeltaTime;

	// Calculate progress (0 to RequiredQuantity)
	int32 NewProgress = FMath::Clamp(
		FMath::RoundToInt((TimeElapsed / WaitDuration) * RequiredQuantity),
		0,
		RequiredQuantity
	);

	// Update progress
	if (NewProgress != CurrentProgress)
	{
		SetProgress(NewProgress);
	}
}

FText UWaitTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	float TimeRemaining = FMath::Max(0.0f, WaitDuration - TimeElapsed);
	
	// Format time nicely
	if (TimeRemaining >= 60.0f)
	{
		int32 Minutes = FMath::FloorToInt(TimeRemaining / 60.0f);
		int32 Seconds = FMath::FloorToInt(TimeRemaining - (Minutes * 60.0f));
		return FText::FromString(FString::Printf(TEXT("Wait %d:%02d"), Minutes, Seconds));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("Wait %.1f seconds"), TimeRemaining));
	}
}

FString UWaitTask::GetTaskNodeDescription_Implementation() const
{
	if (WaitDuration >= 60.0f)
	{
		int32 Minutes = FMath::FloorToInt(WaitDuration / 60.0f);
		int32 Seconds = FMath::FloorToInt(WaitDuration - (Minutes * 60.0f));
		return FString::Printf(TEXT("Wait %d:%02d"), Minutes, Seconds);
	}
	return FString::Printf(TEXT("Wait %.0f seconds"), WaitDuration);
}
