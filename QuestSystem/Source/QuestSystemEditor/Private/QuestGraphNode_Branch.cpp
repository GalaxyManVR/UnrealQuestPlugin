// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "QuestGraphNode_Branch.h"
#include "QuestGraphNode_State.h"
#include "Quest.h"
#include "QuestTask.h"
#include "EdGraph/EdGraphPin.h"
#include "ToolMenu.h"

#define LOCTEXT_NAMESPACE "QuestGraphNode_Branch"

UQuestGraphNode_Branch::UQuestGraphNode_Branch()
{
}

void UQuestGraphNode_Branch::AllocateDefaultPins()
{
	// Input pin (from state)
	CreatePin(EGPD_Input, TEXT("StateConnection"), TEXT("In"));

	// Output pin (to state)
	CreatePin(EGPD_Output, TEXT("StateConnection"), TEXT("Out"));
}

FText UQuestGraphNode_Branch::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (QuestBranch)
	{
		if (!QuestBranch->Description.IsEmpty())
		{
			return QuestBranch->Description;
		}

		// Show task count
		int32 TaskCount = QuestBranch->QuestTasks.Num();
		return FText::Format(LOCTEXT("BranchTitle", "Branch ({0} tasks)"), FText::AsNumber(TaskCount));
	}

	return LOCTEXT("BranchDefault", "Quest Branch");
}

FLinearColor UQuestGraphNode_Branch::GetNodeTitleColor() const
{
	if (QuestBranch && QuestBranch->bHidden)
	{
		return FLinearColor(0.5f, 0.5f, 0.5f, 0.5f); // Translucent gray for hidden
	}

	return FLinearColor(0.8f, 0.6f, 0.2f); // Orange/yellow for branches
}

FText UQuestGraphNode_Branch::GetTooltipText() const
{
	if (QuestBranch)
	{
		FString TaskList;
		for (int32 i = 0; i < QuestBranch->QuestTasks.Num(); ++i)
		{
			UQuestTask* Task = QuestBranch->QuestTasks[i];
			if (Task)
			{
				TaskList += FString::Printf(TEXT("%d. %s\n"), i + 1, *Task->GetTaskNodeDescription());
			}
		}

		return FText::FromString(TaskList);
	}

	return LOCTEXT("BranchTooltip", "A branch containing quest tasks");
}

void UQuestGraphNode_Branch::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{

}

void UQuestGraphNode_Branch::CompileToQuestBranch(UQuest* Quest)
{

}

void UQuestGraphNode_Branch::InitializeFromBranch(UQuestBranch* InBranch)
{
	QuestBranch = InBranch;
	AllocateDefaultPins();
}

UEdGraphPin* UQuestGraphNode_Branch::GetInputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin && Pin->Direction == EGPD_Input)
		{
			return Pin;
		}
	}
	return nullptr;
}

UEdGraphPin* UQuestGraphNode_Branch::GetOutputPin() const
{
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin && Pin->Direction == EGPD_Output)
		{
			return Pin;
		}
	}
	return nullptr;
}

UEdGraphPin* UQuestGraphNode_Branch::FindPin(const FName& PinName) const
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

#undef LOCTEXT_NAMESPACE