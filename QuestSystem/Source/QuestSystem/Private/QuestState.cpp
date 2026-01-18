#include "QuestState.h"
#include "QuestBranch.h"
#include "Quest.h"

UQuestState::UQuestState()
{
	StateGuid = FGuid::NewGuid();
	bRewardsOnlyOnce = true;
	bIsActive = false;  // ADD THIS LINE TOO
}

void UQuestState::Initialize(UQuest* InQuest)
{
	OwningQuest = InQuest;

	if (ID.IsNone())
	{
		switch (StateType)
		{
		case EQuestStateType::QuestStart:
			ID = FName(TEXT("QuestStart"));
			break;
		case EQuestStateType::QuestSuccess:
			ID = FName(TEXT("QuestSuccess"));
			break;
		case EQuestStateType::QuestFailure:
			ID = FName(TEXT("QuestFailure"));
			break;
		default:
			ID = FName(*FGuid::NewGuid().ToString());
			break;
		}
	}
}

void UQuestState::GiveRewards()
{
	if (!OwningQuest || !OwningQuest->OwningComp)
	{
		return;
	}

	for (UQuestReward* Reward : Rewards)
	{
		if (Reward)
		{
			// Check if we should skip already-given rewards
			if (bRewardsOnlyOnce && Reward->bAlreadyGiven)
			{
				continue;
			}

			// Grant the reward
			Reward->GiveReward(OwningQuest->OwningComp, OwningQuest);

			UE_LOG(LogTemp, Log, TEXT("Quest '%s' granted reward: %s"),
				*OwningQuest->QuestName.ToString(),
				*Reward->GetRewardDescription().ToString());
		}
	}
}

// Modify your existing ActivateState function to grant rewards when entering the state
void UQuestState::ActivateState()
{
	bIsActive = true;

	// Initialize all branches in this state
	for (UQuestBranch* Branch : QuestBranches)
	{
		if (Branch)
		{
			Branch->ActivateBranch(OwningQuest->OwningComp);
		}
	}

	// Grant rewards when entering this state
	GiveRewards();

	// Broadcast state activation event
	if (OwningQuest)
	{
		OwningQuest->OnQuestStateChanged.Broadcast(OwningQuest, this);
	}

	UE_LOG(LogTemp, Log, TEXT("Quest state activated: %s"), *StateName.ToString());
}

void UQuestState::AddOutgoingBranch(UQuestBranch* Branch)
{
	if (Branch && !OutgoingBranches.Contains(Branch))
		OutgoingBranches.Add(Branch);
}

void UQuestState::AddIncomingBranch(UQuestBranch* Branch)
{
	if (Branch && !IncomingBranches.Contains(Branch))
		IncomingBranches.Add(Branch);
}

bool UQuestState::IsTerminalState() const
{
	return StateType == EQuestStateType::QuestSuccess || StateType == EQuestStateType::QuestFailure;
}

void UQuestState::OnStateEntered_Implementation()
{
	GiveRewards();
}
void UQuestState::OnStateExited_Implementation() {}
