#include "QuestBranch.h"
#include "Quest.h"
#include "QuestState.h"
#include "QuestTask.h"
#include "QuestComponent.h"

UQuestBranch::UQuestBranch()
{
	bHidden = false;
	bIsActive = false;
	bIsComplete = false;  // ADDED
	ID = NAME_None;
	OwningQuest = nullptr;
	FromState = nullptr;
	ToState = nullptr;
	CachedQuestComp = nullptr;
}

void UQuestBranch::Initialize(UQuest* InQuest, UQuestState* InFromState, UQuestState* InToState)
{
	OwningQuest = InQuest;
	FromState = InFromState;
	ToState = InToState;

	// Initialize all tasks
	for (UQuestTask* Task : QuestTasks)
	{
		if (Task)
		{
			Task->OwningBranch = this;
			Task->OwningQuest = InQuest;
		}
	}
}

void UQuestBranch::ActivateBranch(UQuestComponent* QuestComp)
{
	if (bIsActive)
	{
		return; // Already active
	}

	bIsActive = true;
	bIsComplete = false;  // ADDED - Reset when reactivating
	CachedQuestComp = QuestComp;

	UE_LOG(LogTemp, Log, TEXT("Branch activated with %d tasks"), QuestTasks.Num());

	// Reset all task progress
	for (UQuestTask* Task : QuestTasks)
	{
		if (Task)
		{
			Task->BeginTask(QuestComp);
		}
	}

	// Check immediately if already complete (e.g., tasks completed before quest started)
	CheckTaskProgress();
}

void UQuestBranch::DeactivateBranch()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;

	// End all tasks
	for (UQuestTask* Task : QuestTasks)
	{
		if (Task)
		{
			Task->EndTask();
		}
	}

	CachedQuestComp = nullptr;

	UE_LOG(LogTemp, Log, TEXT("Branch deactivated"));
}

bool UQuestBranch::AreAllTasksComplete() const
{
	if (QuestTasks.Num() == 0)
	{
		return true; // No tasks = instant completion
	}

	for (UQuestTask* Task : QuestTasks)
	{
		if (Task && !Task->IsComplete())
		{
			return false;
		}
	}

	return true;
}

void UQuestBranch::CheckTaskProgress()
{
	if (!bIsActive || !OwningQuest || !CachedQuestComp)
	{
		return;
	}

	// Update all task progress
	for (UQuestTask* Task : QuestTasks)
	{
		if (Task && !Task->IsComplete())
		{
			Task->UpdateTaskProgress(CachedQuestComp);
		}
	}

	// Check if all tasks are now complete
	if (AreAllTasksComplete())
	{
		UE_LOG(LogTemp, Log, TEXT("Branch completed! Moving to next state."));

		bIsComplete = true;  // ADDED - Mark as complete

		// Deactivate this branch
		DeactivateBranch();

		// Tell the quest we're complete - this will trigger state change
		if (OwningQuest)
		{
			OwningQuest->OnBranchCompleted(this);
		}
	}
}

float UQuestBranch::GetCompletionPercentage() const
{
	if (QuestTasks.Num() == 0)
	{
		return 1.0f;
	}

	int32 CompletedCount = 0;
	for (UQuestTask* Task : QuestTasks)
	{
		if (Task && Task->IsComplete())
		{
			CompletedCount++;
		}
	}

	return (float)CompletedCount / (float)QuestTasks.Num();
}