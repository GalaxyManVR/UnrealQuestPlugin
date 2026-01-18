#include "QuestState.h"
#include "QuestBranch.h"
#include "Quest.h"

UQuestState::UQuestState()
{
	StateGuid = FGuid::NewGuid();
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

void UQuestState::OnStateEntered_Implementation() {}
void UQuestState::OnStateExited_Implementation() {}
