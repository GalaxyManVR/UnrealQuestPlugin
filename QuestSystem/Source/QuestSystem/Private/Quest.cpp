

#include "Quest.h"
#include "QuestState.h"
#include "QuestBranch.h"
#include "QuestTask.h"
#include "QuestComponent.h"
#include "UObject/ObjectSaveContext.h"

UQuest::UQuest()
{
	QuestStatus = EQuestStatus::NotStarted;
	bIsInProgress = false;  
	bHasSucceeded = false;   
	bHasFailed = false;     
	CurrentState = nullptr;
	bTracked = true;
	QuestStartState = nullptr;
	QuestSuccessState = nullptr;
	QuestFailureState = nullptr;
}

void UQuest::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

#if WITH_EDITOR
	// Validate that all required states exist
	if (QuestStates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest '%s' has no states! Creating default states."), *GetName());
		CreateDefaultStates();
	}

	// Ensure it has the three required states for a quest
	bool bHasStart = false;
	bool bHasSuccess = false;
	bool bHasFailure = false;

	for (UQuestState* State : QuestStates)
	{
		if (State)
		{
			switch (State->StateType)
			{
			case EQuestStateType::QuestStart:
				bHasStart = true;
				QuestStartState = State;
				break;
			case EQuestStateType::QuestSuccess:
				bHasSuccess = true;
				QuestSuccessState = State;
				break;
			case EQuestStateType::QuestFailure:
				bHasFailure = true;
				QuestFailureState = State;
				break;
			}
		}
	}

	// Warn if it is missing the required states
	if (!bHasStart)
	{
		UE_LOG(LogTemp, Error, TEXT("Quest '%s' is missing a QuestStart state!"), *GetName());
	}
	if (!bHasSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest '%s' is missing a QuestSuccess state. Quest will not be completable."), *GetName());
	}
	if (!bHasFailure)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest '%s' is missing a QuestFailure state. Quest cannot fail."), *GetName());
	}

	// Validate branches - check for orphaned branches
	TArray<UQuestBranch*> OrphanedBranches;
	for (UQuestBranch* Branch : QuestBranches)
	{
		if (Branch)
		{
			if (!Branch->FromState || !Branch->ToState)
			{
				OrphanedBranches.Add(Branch);
				UE_LOG(LogTemp, Warning, TEXT("Quest '%s' has orphaned branch: FromState=%s, ToState=%s"),
					*GetName(),
					Branch->FromState ? *Branch->FromState->ID.ToString() : TEXT("NULL"),
					Branch->ToState ? *Branch->ToState->ID.ToString() : TEXT("NULL"));
			}

			// Validate that branch tasks are valid
			for (UQuestTask* Task : Branch->QuestTasks)
			{
				if (!Task)
				{
					UE_LOG(LogTemp, Warning, TEXT("Quest '%s' has a branch with null task!"), *GetName());
				}
			}
		}
	}

	// Remove orphaned branches before saving
	if (OrphanedBranches.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Removing %d orphaned branches from quest '%s'"), OrphanedBranches.Num(), *GetName());
		for (UQuestBranch* OrphanedBranch : OrphanedBranches)
		{
			QuestBranches.Remove(OrphanedBranch);
		}
	}

	// Remove null entries from arrays
	QuestStates.RemoveAll([](UQuestState* State) { return State == nullptr; });
	QuestBranches.RemoveAll([](UQuestBranch* Branch) { return Branch == nullptr; });

	// Validate quest metadata
	if (QuestName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest '%s' has no QuestName set!"), *GetName());
	}
	if (QuestDescription.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest '%s' has no QuestDescription set!"), *GetName());
	}

	UE_LOG(LogTemp, Log, TEXT("Quest '%s' PreSave validation complete: %d states, %d branches"),
		*GetName(), QuestStates.Num(), QuestBranches.Num());
#endif
}

#if WITH_EDITOR
void UQuest::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Log whenever properties change
	UE_LOG(LogTemp, Warning, TEXT("Quest PostEditChangeProperty called for '%s'"), *GetName());
	UE_LOG(LogTemp, Warning, TEXT("Current data: %d states, %d branches"),
		QuestStates.Num(), QuestBranches.Num());
}

void UQuest::PostLoad()
{
	Super::PostLoad();

	// Log what data was loaded from the asset
	UE_LOG(LogTemp, Warning, TEXT("=== Quest PostLoad: '%s' ==="), *GetName());
	UE_LOG(LogTemp, Warning, TEXT("Loaded: %d states, %d branches"),
		QuestStates.Num(), QuestBranches.Num());

	if (QuestStates.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Quest loaded with 0 states!"));
	}
}
#endif


void UQuest::Initialize(UQuestComponent* InOwningComp)
{
	OwningComp = InOwningComp;

	// Create default states if they don't exist (for new quest assets)
	if (QuestStates.Num() == 0)
	{
		CreateDefaultStates();
	}

	// Initialize all states
	for (UQuestState* State : QuestStates)
	{
		if (State)
		{
			State->Initialize(this);

			// Cache special state references
			switch (State->StateType)
			{
			case EQuestStateType::QuestStart:
				QuestStartState = State;
				break;
			case EQuestStateType::QuestSuccess:
				QuestSuccessState = State;
				break;
			case EQuestStateType::QuestFailure:
				QuestFailureState = State;
				break;
			}
		}
	}

	// Initialize all branches
	for (UQuestBranch* Branch : QuestBranches)
	{
		if (Branch)
		{
			Branch->Initialize(this, Branch->FromState, Branch->ToState);

			if (Branch->FromState && Branch->FromState->OutgoingBranches.Num() == 0)
			{
				Branch->FromState->AddOutgoingBranch(Branch);
			}
			if (Branch->ToState && Branch->ToState->IncomingBranches.Num() == 0)
			{
				Branch->ToState->AddIncomingBranch(Branch);
			}
		}
	}
}

void UQuest::CreateDefaultStates()
{
	// Create QuestStart
	UQuestState* StartState = NewObject<UQuestState>(this, UQuestState::StaticClass(), TEXT("QuestStart"));
	StartState->StateType = EQuestStateType::QuestStart;
	StartState->ID = FName(TEXT("QuestStart"));
	StartState->DisplayName = FText::FromString(TEXT("Quest Start"));
	QuestStates.Add(StartState);
	QuestStartState = StartState;

	// Create QuestSuccess
	UQuestState* SuccessState = NewObject<UQuestState>(this, UQuestState::StaticClass(), TEXT("QuestSuccess"));
	SuccessState->StateType = EQuestStateType::QuestSuccess;
	SuccessState->ID = FName(TEXT("QuestSuccess"));
	SuccessState->DisplayName = FText::FromString(TEXT("Quest Success"));
	QuestStates.Add(SuccessState);
	QuestSuccessState = SuccessState;

	// Create QuestFailure
	UQuestState* FailureState = NewObject<UQuestState>(this, UQuestState::StaticClass(), TEXT("QuestFailure"));
	FailureState->StateType = EQuestStateType::QuestFailure;
	FailureState->ID = FName(TEXT("QuestFailure"));
	FailureState->DisplayName = FText::FromString(TEXT("Quest Failure"));
	QuestStates.Add(FailureState);
	QuestFailureState = FailureState;
}

void UQuest::BeginQuest()
{
	UE_LOG(LogTemp, Warning, TEXT("=== BeginQuest called for %s ==="), *GetName());
	UE_LOG(LogTemp, Warning, TEXT("QuestStates count: %d"), QuestStates.Num());
	UE_LOG(LogTemp, Warning, TEXT("QuestBranches count: %d"), QuestBranches.Num());
	UE_LOG(LogTemp, Warning, TEXT("QuestStartState: %s"), QuestStartState ? *QuestStartState->ID.ToString() : TEXT("NULL"));

	if (QuestStartState)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestStartState OutgoingBranches: %d"), QuestStartState->OutgoingBranches.Num());
	}
	if (QuestStatus != EQuestStatus::NotStarted)
		return;

	// Find or use cached QuestStart state
	UQuestState* StartState = QuestStartState;
	if (!StartState)
	{
		for (UQuestState* State : QuestStates)
		{
			if (State && State->StateType == EQuestStateType::QuestStart)
			{
				StartState = State;
				QuestStartState = State;
				break;
			}
		}
	}

	if (!StartState)
	{
		UE_LOG(LogTemp, Error, TEXT("Quest %s has no QuestStart state!"), *GetName());
		return;
	}

	CurrentState = StartState;
	QuestStatus = EQuestStatus::InProgress;

	// Activate all branches from start state
	ActivateBranchesFromState(StartState);

	// Fire enter event for start state
	StartState->OnStateEntered();

	OnQuestStarted.Broadcast(this);
}

void UQuest::RestartQuest()
{
	// Deactivate all active branches
	for (UQuestBranch* Branch : ActiveBranches)
	{
		if (Branch)
		{
			Branch->DeactivateBranch();
		}
	}

	// Reset all tasks
	for (UQuestBranch* Branch : QuestBranches)
	{
		if (Branch)
		{
			for (UQuestTask* Task : Branch->QuestTasks)
			{
				if (Task)
				{
					Task->CurrentProgress = 0;
				}
			}
		}
	}

	ActiveBranches.Empty();
	QuestStatus = EQuestStatus::NotStarted;
	CurrentState = nullptr;

	BeginQuest();
}

void UQuest::ForgetQuest()
{
	// Deactivate all branches
	for (UQuestBranch* Branch : ActiveBranches)
	{
		if (Branch)
		{
			Branch->DeactivateBranch();
		}
	}

	ActiveBranches.Empty();
	QuestStatus = EQuestStatus::NotStarted;
	CurrentState = nullptr;
}

void UQuest::MoveToState(UQuestState* NewState)
{
	if (!NewState) return;

	UQuestState* OldState = CurrentState;

	// Deactivate branches from old state
	if (OldState)
	{
		for (UQuestBranch* Branch : ActiveBranches)
		{
			if (Branch && Branch->FromState == OldState)
			{
				Branch->DeactivateBranch();
			}
		}

		// Remove deactivated branches
		ActiveBranches.RemoveAll([](UQuestBranch* Branch) {
			return Branch && !Branch->bIsActive;
			});

		OldState->OnStateExited();
	}

	CurrentState = NewState;
	OnQuestStateChanged.Broadcast(this, NewState);

	// Fire enter event
	NewState->OnStateEntered();

	// Check for terminal states
	if (NewState->StateType == EQuestStateType::QuestSuccess)
	{
		QuestStatus = EQuestStatus::Succeeded;

		// Deactivate all remaining branches
		for (UQuestBranch* Branch : ActiveBranches)
		{
			if (Branch)
			{
				Branch->DeactivateBranch();
			}
		}
		ActiveBranches.Empty();

		OnQuestSucceeded.Broadcast(this);
	}
	else if (NewState->StateType == EQuestStateType::QuestFailure)
	{
		QuestStatus = EQuestStatus::Failed;

		// Deactivate all remaining branches
		for (UQuestBranch* Branch : ActiveBranches)
		{
			if (Branch)
			{
				Branch->DeactivateBranch();
			}
		}
		ActiveBranches.Empty();

		OnQuestFailed.Broadcast(this);
	}
	else
	{
		// Activate branches from new state
		ActivateBranchesFromState(NewState);
	}
}

void UQuest::ActivateBranchesFromState(UQuestState* State)
{
	if (!State || !OwningComp) return;

	UE_LOG(LogTemp, Warning, TEXT("ActivateBranchesFromState: %s has %d outgoing branches"),
		*State->ID.ToString(), State->OutgoingBranches.Num());

	for (UQuestBranch* Branch : State->OutgoingBranches)
	{
		if (Branch && !Branch->bIsActive)
		{
			UE_LOG(LogTemp, Warning, TEXT("  Activating branch with %d tasks"), Branch->QuestTasks.Num());
			Branch->ActivateBranch(OwningComp);
			ActiveBranches.AddUnique(Branch);
		}
	}
}

void UQuest::OnBranchCompleted(UQuestBranch* Branch)
{
	if (!Branch) return;

	MoveToState(Branch->ToState);
}

bool UQuest::IsQuestAtState(FName StateID) const
{
	return CurrentState && CurrentState->ID == StateID;
}

TArray<UQuestBranch*> UQuest::GetActiveBranches() const
{
	TArray<UQuestBranch*> Result;
	for (UQuestBranch* Branch : ActiveBranches)
	{
		if (Branch && Branch->bIsActive)
			Result.Add(Branch);
	}
	return Result;
}