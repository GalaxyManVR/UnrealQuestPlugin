#include "QuestTask.h"
#include "QuestBranch.h"
#include "Quest.h"
#include "QuestComponent.h"
#include "Engine/World.h"

UQuestTask::UQuestTask()
{
	CurrentProgress = 0;
	RequiredQuantity = 1;
	TickInterval = 0.0f;
	bOptional = false;
	bIsActive = false;
	TaskName = NAME_None;
	TaskArgument = TEXT("");
	TimeSinceLastTick = 0.0f;
}

// FIXED: Added bInitializeProgress parameter
void UQuestTask::BeginTask(UQuestComponent* QuestComp, bool bInitializeProgress)
{
	bIsActive = true;
	OwningComp = QuestComp;
	TimeSinceLastTick = 0.0f;

	// FIXED: Only initialize progress from existing data if bInitializeProgress is true
	// When loading from save, bInitializeProgress will be false to preserve saved progress
	if (bInitializeProgress && QuestComp && !TaskName.IsNone())
	{
		int32 CompletedCount = QuestComp->GetDataTaskCount(TaskName, TaskArgument);
		CurrentProgress = FMath::Min(CompletedCount, RequiredQuantity);

		UE_LOG(LogTemp, Log, TEXT("Task started: %s - Found %d existing completions"),
			*GetTaskNodeDescription_Implementation(), CompletedCount);
	}
	else if (!bInitializeProgress)
	{
		// Loading from save - keep existing CurrentProgress value
		UE_LOG(LogTemp, Log, TEXT("Task started from save: %s (Progress: %d/%d)"),
			*GetTaskNodeDescription_Implementation(), CurrentProgress, RequiredQuantity);
	}
	else
	{
		// New task with no TaskName or no data
		CurrentProgress = 0;
	}

	// Call C++ override
	BeginTask_Implementation();

	// Call Blueprint event
	OnTaskBegin();

	UE_LOG(LogTemp, Log, TEXT("Task started: %s (%d/%d)"),
		*GetTaskNodeDescription_Implementation(), CurrentProgress, RequiredQuantity);
}

void UQuestTask::BeginTask_Implementation()
{
	// Override in C++ child classes
}

void UQuestTask::OnTaskBegin_Implementation()
{
	// Override in Blueprint
}

void UQuestTask::EndTask()
{
	bIsActive = false;

	// Call C++ override
	EndTask_Implementation();

	// Call Blueprint event
	OnTaskEnd();

	UE_LOG(LogTemp, Log, TEXT("Task ended: %s"), *GetTaskNodeDescription_Implementation());
}

void UQuestTask::EndTask_Implementation()
{
	// Override in C++ child classes
}

void UQuestTask::OnTaskEnd_Implementation()
{
	// Override in Blueprint
}

void UQuestTask::UpdateTaskProgress(UQuestComponent* QuestComp)
{
	if (!bIsActive || !QuestComp || TaskName.IsNone())
	{
		return;
	}

	// Check the QuestComponent for task completion
	int32 CompletedCount = QuestComp->GetDataTaskCount(TaskName, TaskArgument);

	if (CompletedCount != CurrentProgress)
	{
		SetProgress(FMath::Min(CompletedCount, RequiredQuantity));
	}
}

void UQuestTask::TickTask()
{
	if (!bIsActive)
	{
		return;
	}

	// Get world for delta time
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float DeltaTime = World->GetDeltaSeconds();
	TimeSinceLastTick += DeltaTime;

	if (TickInterval > 0.0f && TimeSinceLastTick >= TickInterval)
	{
		TimeSinceLastTick = 0.0f;
		TaskTick();
	}
	else if (TickInterval <= 0.0f)
	{
		// If TickInterval is 0 or negative, tick every frame
		TaskTick();
	}
}

void UQuestTask::TaskTick_Implementation()
{
	// Override in Blueprint
}

void UQuestTask::OnTaskProgressChanged_Implementation(int32 NewProgress)
{
	// Override in Blueprint to react to progress changes
}

FText UQuestTask::GetTaskDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride;
	}

	if (!TaskDescription.IsEmpty())
	{
		return TaskDescription;
	}

	return FText::FromString(GetTaskNodeDescription_Implementation());
}

FString UQuestTask::GetTaskNodeDescription_Implementation() const
{
	if (!DescriptionOverride.IsEmpty())
	{
		return DescriptionOverride.ToString();
	}

	if (!TaskDescription.IsEmpty())
	{
		return TaskDescription.ToString();
	}

	if (!TaskName.IsNone())
	{
		return FString::Printf(TEXT("%s: %s (%d/%d)"),
			*TaskName.ToString(),
			*TaskArgument,
			CurrentProgress,
			RequiredQuantity);
	}

	return FString::Printf(TEXT("Quest Task (%d/%d)"), CurrentProgress, RequiredQuantity);
}

bool UQuestTask::IsComplete() const
{
	return CurrentProgress >= RequiredQuantity;
}

void UQuestTask::SetProgress(int32 NewProgress)
{
	if (CurrentProgress == NewProgress)
	{
		return;
	}

	int32 OldProgress = CurrentProgress;
	CurrentProgress = FMath::Clamp(NewProgress, 0, RequiredQuantity);

	UE_LOG(LogTemp, Log, TEXT("Task progress: %s (%d/%d)"),
		*GetTaskNodeDescription_Implementation(),
		CurrentProgress,
		RequiredQuantity);

	// Fire Blueprint event
	OnTaskProgressChanged(CurrentProgress);

	// If complete, notify branch to check overall progress
	if (IsComplete() && OwningBranch)
	{
		OwningBranch->CheckTaskProgress();
	}
}

void UQuestTask::AddProgress(int32 Amount)
{
	SetProgress(CurrentProgress + Amount);
}

float UQuestTask::GetProgressPercentage() const
{
	if (RequiredQuantity <= 0)
	{
		return 1.0f;
	}

	return FMath::Clamp((float)CurrentProgress / (float)RequiredQuantity, 0.0f, 1.0f);
}