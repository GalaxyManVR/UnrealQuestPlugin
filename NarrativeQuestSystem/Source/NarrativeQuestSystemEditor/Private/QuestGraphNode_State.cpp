// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestGraphNode_State.h"
#include "Quest.h"
#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "QuestGraphNode_State"

UQuestGraphNode_State::UQuestGraphNode_State()
{
	bCanRenameNode = true;

	// Required for delete / undo / redo
	SetFlags(RF_Transactional);
}


void UQuestGraphNode_State::AllocateDefaultPins()
{
	// Input pin (branches leading TO this state)
	if (!QuestState || QuestState->StateType != EQuestStateType::QuestStart)
	{
		CreatePin(EGPD_Input, TEXT("StateConnection"), TEXT("In"));
	}

	// Output pin (branches leading FROM this state)
	if (!QuestState || !QuestState->IsTerminalState())
	{
		CreatePin(EGPD_Output, TEXT("StateConnection"), TEXT("Out"));
	}
}

FText UQuestGraphNode_State::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!QuestState)
	{
		return LOCTEXT("QuestStateDefault", "Quest State");
	}

	// Special states: fixed label + optional user subtitle
	switch (QuestState->StateType)
	{
	case EQuestStateType::QuestStart:
		return LOCTEXT("QuestStartTitle", "Quest Start");

	case EQuestStateType::QuestSuccess:
		return LOCTEXT("QuestSuccessTitle", "Quest Success");

	case EQuestStateType::QuestFailure:
		return LOCTEXT("QuestFailureTitle", "Quest Failure");

	case EQuestStateType::QuestState:
		break;
	}

	// Normal states: user-controlled naming
	if (!QuestState->DisplayName.IsEmpty())
	{
		return QuestState->DisplayName;
	}

	if (!QuestState->Description.IsEmpty())
	{
		return QuestState->Description;
	}

	// Default to "Quest State" instead of showing the GUID
	return LOCTEXT("QuestStateTitle", "Quest State");
}


FLinearColor UQuestGraphNode_State::GetNodeTitleColor() const
{
	if (QuestState)
	{
		switch (QuestState->StateType)
		{
		case EQuestStateType::QuestStart:
			return FLinearColor::Green;
		case EQuestStateType::QuestSuccess:
			return FLinearColor::Blue;
		case EQuestStateType::QuestFailure:
			return FLinearColor::Red;
		default:
			return FLinearColor::Gray;
		}
	}

	return FLinearColor::White;
}

void UQuestGraphNode_State::OnRenameNode(const FString& NewName)
{
	if (QuestState)
	{
		QuestState->Modify(); // Required for undo/redo
		QuestState->DisplayName = FText::FromString(NewName);
	}
}


FText UQuestGraphNode_State::GetTooltipText() const
{
	if (QuestState)
	{
		return QuestState->Description;
	}
	return LOCTEXT("QuestStateTooltip", "A state in the quest");
}

void UQuestGraphNode_State::CompileToQuestState(UQuest* Quest)
{
	// NOTE: This function is now deprecated - compilation happens in QuestGraph::CompileQuestFromGraph()
	// Keeping it for backwards compatibility but it does nothing
	// All compilation logic is now centralized in QuestGraph.cpp
}

void UQuestGraphNode_State::InitializeFromState(UQuestState* InState)
{
	QuestState = InState;
	AllocateDefaultPins();
}

UEdGraphPin* UQuestGraphNode_State::FindPin(const FName& PinName) const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin && Pin->PinName == PinName)
		{
			return Pin;
		}
	}
	return nullptr;
}

bool UQuestGraphNode_State::CanUserDeleteNode() const
{
	if (!QuestState) return false;

	switch (QuestState->StateType)
	{
	case EQuestStateType::QuestStart:
	case EQuestStateType::QuestSuccess:
	case EQuestStateType::QuestFailure:
		return false; // cannot delete these
	default:
		return true;
	}
}

bool UQuestGraphNode_State::CanDuplicateNode() const
{
	return true;
}

bool UQuestGraphNode_State::CanRenameNode() const
{
	return this->QuestState && this->QuestState->StateType == EQuestStateType::QuestState;
}


#undef LOCTEXT_NAMESPACE