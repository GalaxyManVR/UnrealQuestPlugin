// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestGraphSchema.h"
#include "QuestGraph.h"
#include "QuestGraphNode_State.h"
#include "QuestGraphNode_Branch.h"
#include "QuestTask.h"
#include "QuestState.h"
#include "QuestBranch.h"
#include "Quest.h"
#include "EdGraph/EdGraphPin.h"
#include "UObject/UObjectIterator.h"

#define LOCTEXT_NAMESPACE "QuestGraphSchema"

void UQuestGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	// Add Quest State action
	TSharedPtr<FQuestGraphSchemaAction_NewState> NewStateAction = MakeShared<FQuestGraphSchemaAction_NewState>(
		LOCTEXT("QuestStateCategory", "Quest States"),
		LOCTEXT("AddQuestState", "Add Quest State"),
		LOCTEXT("AddQuestStateTooltip", "Add a new quest state"),
		0
	);
	ContextMenuBuilder.AddAction(NewStateAction);

	// Get all task classes
	TArray<TSubclassOf<UQuestTask>> TaskClasses;
	GetTaskClasses(TaskClasses);

	// Add task actions
	for (TSubclassOf<UQuestTask> TaskClass : TaskClasses)
	{
		if (!TaskClass)
			continue;

		UQuestTask* TaskCDO = TaskClass->GetDefaultObject<UQuestTask>();
		if (!TaskCDO)
			continue;

		FText TaskName = FText::FromString(TaskClass->GetName().Replace(TEXT("Task_"), TEXT("")).Replace(TEXT("Task"), TEXT("")));

		TSharedPtr<FQuestGraphSchemaAction_NewTask> NewTaskAction = MakeShared<FQuestGraphSchemaAction_NewTask>(
			LOCTEXT("QuestTaskCategory", "Quest Tasks"),
			TaskName,
			FText::Format(LOCTEXT("AddTaskTooltip", "Add {0} task"), TaskName),
			0
		);
		NewTaskAction->TaskClass = TaskClass;
		ContextMenuBuilder.AddAction(NewTaskAction);
	}
}

const FPinConnectionResponse UQuestGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	// Make sure pins exist
	if (!A || !B)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("InvalidPins", "Invalid pins"));
	}

	// Make sure they're from different nodes
	if (A->GetOwningNode() == B->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SameNode", "Cannot connect a node to itself"));
	}

	// Make sure one is input and one is output
	if (A->Direction == B->Direction)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SameDirection", "Pins must be input and output"));
	}

	// Check what we're connecting
	UQuestGraphNode_State* StateNodeA = Cast<UQuestGraphNode_State>(const_cast<UEdGraphNode*>(A->GetOwningNode()));
	UQuestGraphNode_State* StateNodeB = Cast<UQuestGraphNode_State>(const_cast<UEdGraphNode*>(B->GetOwningNode()));
	UQuestGraphNode_Branch* BranchNodeA = Cast<UQuestGraphNode_Branch>(const_cast<UEdGraphNode*>(A->GetOwningNode()));
	UQuestGraphNode_Branch* BranchNodeB = Cast<UQuestGraphNode_Branch>(const_cast<UEdGraphNode*>(B->GetOwningNode()));

	// Allow State-to-State (will auto-create branch)
	if (StateNodeA && StateNodeB)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, LOCTEXT("AutoBranch", "Will create branch automatically"));
	}

	// Allow State-to-Branch and Branch-to-State
	bool bHasState = (StateNodeA != nullptr) || (StateNodeB != nullptr);
	bool bHasBranch = (BranchNodeA != nullptr) || (BranchNodeB != nullptr);

	if (bHasState && bHasBranch)
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
	}

	// Don't allow Branch-to-Branch
	return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("InvalidConnection", "Invalid connection type"));
}

bool UQuestGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	if (!A || !B)
	{
		return false;
	}

	// Check if we're connecting two states directly
	UQuestGraphNode_State* StateA = Cast<UQuestGraphNode_State>(A->GetOwningNode());
	UQuestGraphNode_State* StateB = Cast<UQuestGraphNode_State>(B->GetOwningNode());

	if (StateA && StateB)
	{
		// Auto-create a branch node between them
		UEdGraph* Graph = A->GetOwningNode()->GetGraph();
		UQuestGraph* QuestGraph = Cast<UQuestGraph>(Graph);

		if (!QuestGraph || !QuestGraph->Quest)
		{
			return false;
		}

		const FScopedTransaction Transaction(LOCTEXT("AutoCreateBranch", "Create Branch"));
		Graph->Modify();
		QuestGraph->Quest->Modify();

		// Calculate position for the branch (midpoint between the two states)
		FVector2D StartPos(StateA->NodePosX, StateA->NodePosY);
		FVector2D EndPos(StateB->NodePosX, StateB->NodePosY);
		FVector2D MidPos = (StartPos + EndPos) * 0.5f;

		// Create new branch
		FName BranchName = MakeUniqueObjectName(QuestGraph->Quest, UQuestBranch::StaticClass(), TEXT("Branch"));
		UQuestBranch* NewBranch = NewObject<UQuestBranch>(QuestGraph->Quest, UQuestBranch::StaticClass(), BranchName, RF_Transactional);

		// CRITICAL: Add to quest immediately
		QuestGraph->Quest->QuestBranches.AddUnique(NewBranch);

		// Create branch graph node
		UQuestGraphNode_Branch* BranchNode = NewObject<UQuestGraphNode_Branch>(Graph, UQuestGraphNode_Branch::StaticClass(), NAME_None, RF_Transactional);
		BranchNode->CreateNewGuid();
		BranchNode->InitializeFromBranch(NewBranch);
		BranchNode->NodePosX = MidPos.X;
		BranchNode->NodePosY = MidPos.Y;

		Graph->AddNode(BranchNode, false, false);

		// Determine which pin is output and which is input
		UEdGraphPin* OutputPin = (A->Direction == EGPD_Output) ? A : B;
		UEdGraphPin* InputPin = (A->Direction == EGPD_Input) ? A : B;

		// Connect: OutputState -> Branch -> InputState
		UEdGraphPin* BranchInPin = BranchNode->FindPin(FName(TEXT("In")));
		UEdGraphPin* BranchOutPin = BranchNode->FindPin(FName(TEXT("Out")));

		if (BranchInPin && BranchOutPin)
		{
			// Connect state output to branch input
			OutputPin->MakeLinkTo(BranchInPin);

			// Connect branch output to state input
			BranchOutPin->MakeLinkTo(InputPin);

			Graph->NotifyGraphChanged();
			QuestGraph->Quest->MarkPackageDirty();
			return true;
		}

		return false;
	}

	// Normal connection between state and branch
	const FPinConnectionResponse Response = CanCreateConnection(A, B);

	if (Response.Response == CONNECT_RESPONSE_MAKE)
	{
		A->MakeLinkTo(B);
		A->GetOwningNode()->GetGraph()->NotifyGraphChanged();
		return true;
	}

	return false;
}

FLinearColor UQuestGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor::White;
}

void UQuestGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotification) const
{
	const FScopedTransaction Transaction(LOCTEXT("BreakPinLinks", "Break Pin Links"));

	Super::BreakPinLinks(TargetPin, bSendsNodeNotification);
}

void UQuestGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	const FScopedTransaction Transaction(LOCTEXT("BreakSinglePinLink", "Break Pin Link"));

	Super::BreakSinglePinLink(SourcePin, TargetPin);
}

void UQuestGraphSchema::GetTaskClasses(TArray<TSubclassOf<UQuestTask>>& OutTaskClasses) const
{
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;

		if (Class->IsChildOf(UQuestTask::StaticClass()) &&
			!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			OutTaskClasses.Add(Class);
		}
	}
}

UEdGraphNode* FQuestGraphSchemaAction_NewState::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UQuestGraph* QuestGraph = Cast<UQuestGraph>(ParentGraph);
	if (!QuestGraph || !QuestGraph->Quest)
	{
		return nullptr;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddQuestState", "Add Quest State"));
	ParentGraph->Modify();
	QuestGraph->Quest->Modify();

	// Create new state
	FName StateName = MakeUniqueObjectName(QuestGraph->Quest, UQuestState::StaticClass(), TEXT("State"));
	UQuestState* NewState = NewObject<UQuestState>(QuestGraph->Quest, UQuestState::StaticClass(), StateName, RF_Transactional);
	NewState->StateType = EQuestStateType::QuestState;
	NewState->ID = FName(*FGuid::NewGuid().ToString());
	NewState->DisplayName = LOCTEXT("NewStateName", "New State");

	// CRITICAL: Add to quest immediately
	QuestGraph->Quest->QuestStates.AddUnique(NewState);

	// Create graph node
	UQuestGraphNode_State* GraphNode = NewObject<UQuestGraphNode_State>(ParentGraph, UQuestGraphNode_State::StaticClass(), NAME_None, RF_Transactional);
	GraphNode->CreateNewGuid();
	GraphNode->InitializeFromState(NewState);
	GraphNode->NodePosX = Location.X;
	GraphNode->NodePosY = Location.Y;

	ParentGraph->AddNode(GraphNode, true, bSelectNewNode);

	// If dragging from a pin, create connection
	if (FromPin != nullptr)
	{
		GraphNode->AutowireNewNode(FromPin);
	}

	ParentGraph->NotifyGraphChanged();
	QuestGraph->Quest->MarkPackageDirty();

	return GraphNode;
}

UEdGraphNode* FQuestGraphSchemaAction_NewTask::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	UQuestGraph* QuestGraph = Cast<UQuestGraph>(ParentGraph);
	if (!QuestGraph || !QuestGraph->Quest || !TaskClass)
	{
		return nullptr;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddQuestTask", "Add Quest Task"));
	ParentGraph->Modify();
	QuestGraph->Quest->Modify();

	// Create new task
	UQuestTask* NewTask = NewObject<UQuestTask>(QuestGraph->Quest, TaskClass, NAME_None, RF_Transactional);

	// Create new branch
	FName BranchName = MakeUniqueObjectName(QuestGraph->Quest, UQuestBranch::StaticClass(), TEXT("Branch"));
	UQuestBranch* NewBranch = NewObject<UQuestBranch>(QuestGraph->Quest, UQuestBranch::StaticClass(), BranchName, RF_Transactional);
	NewBranch->QuestTasks.Add(NewTask);
	NewBranch->ID = FName(*FGuid::NewGuid().ToString());

	// CRITICAL: Add to quest immediately
	QuestGraph->Quest->QuestBranches.AddUnique(NewBranch);

	// Create graph node
	UQuestGraphNode_Branch* GraphNode = NewObject<UQuestGraphNode_Branch>(ParentGraph, UQuestGraphNode_Branch::StaticClass(), NAME_None, RF_Transactional);
	GraphNode->CreateNewGuid();
	GraphNode->InitializeFromBranch(NewBranch);
	GraphNode->NodePosX = Location.X;
	GraphNode->NodePosY = Location.Y;

	ParentGraph->AddNode(GraphNode, true, bSelectNewNode);

	// If we're dragging from a pin, create the connection
	if (FromPin != nullptr)
	{
		// Connect the branch to the pin we dragged from
		UEdGraphPin* ConnectToPin = nullptr;

		if (FromPin->Direction == EGPD_Output)
		{
			// Dragged from output, connect to our input
			ConnectToPin = GraphNode->FindPin(FName(TEXT("In")));
		}
		else
		{
			// Dragged from input, connect to our output
			ConnectToPin = GraphNode->FindPin(FName(TEXT("Out")));
		}

		if (ConnectToPin)
		{
			GetDefault<UQuestGraphSchema>()->TryCreateConnection(FromPin, ConnectToPin);
		}

		// Auto-create a state on the other end
		FVector2D NewStateLocation = Location;
		if (FromPin->Direction == EGPD_Output)
		{
			// We connected input, create state on output side
			NewStateLocation.X += 300;

			UQuestState* NewState = NewObject<UQuestState>(QuestGraph->Quest, UQuestState::StaticClass(), NAME_None, RF_Transactional);
			NewState->StateType = EQuestStateType::QuestState;
			NewState->ID = FName(*FGuid::NewGuid().ToString());
			NewState->DisplayName = LOCTEXT("NewStateName", "New State");

			// CRITICAL: Add to quest immediately
			QuestGraph->Quest->QuestStates.AddUnique(NewState);

			UQuestGraphNode_State* StateNode = NewObject<UQuestGraphNode_State>(ParentGraph, UQuestGraphNode_State::StaticClass(), NAME_None, RF_Transactional);
			StateNode->CreateNewGuid();
			StateNode->InitializeFromState(NewState);
			StateNode->NodePosX = NewStateLocation.X;
			StateNode->NodePosY = NewStateLocation.Y;
			ParentGraph->AddNode(StateNode, false, false);

			// Connect branch output to new state input
			UEdGraphPin* BranchOutPin = GraphNode->FindPin(FName(TEXT("Out")));
			UEdGraphPin* StateInPin = StateNode->FindPin(FName(TEXT("In")));
			if (BranchOutPin && StateInPin)
			{
				GetDefault<UQuestGraphSchema>()->TryCreateConnection(BranchOutPin, StateInPin);
			}
		}
		else
		{
			// We connected output, create state on input side
			NewStateLocation.X -= 300;

			UQuestState* NewState = NewObject<UQuestState>(QuestGraph->Quest, UQuestState::StaticClass(), NAME_None, RF_Transactional);
			NewState->StateType = EQuestStateType::QuestState;
			NewState->ID = FName(*FGuid::NewGuid().ToString());
			NewState->DisplayName = LOCTEXT("NewStateName", "New State");

			// CRITICAL: Add to quest immediately
			QuestGraph->Quest->QuestStates.AddUnique(NewState);

			UQuestGraphNode_State* StateNode = NewObject<UQuestGraphNode_State>(ParentGraph, UQuestGraphNode_State::StaticClass(), NAME_None, RF_Transactional);
			StateNode->CreateNewGuid();
			StateNode->InitializeFromState(NewState);
			StateNode->NodePosX = NewStateLocation.X;
			StateNode->NodePosY = NewStateLocation.Y;
			ParentGraph->AddNode(StateNode, false, false);

			// Connect new state output to branch input
			UEdGraphPin* StateOutPin = StateNode->FindPin(FName(TEXT("Out")));
			UEdGraphPin* BranchInPin = GraphNode->FindPin(FName(TEXT("In")));
			if (StateOutPin && BranchInPin)
			{
				GetDefault<UQuestGraphSchema>()->TryCreateConnection(StateOutPin, BranchInPin);
			}
		}
	}

	ParentGraph->NotifyGraphChanged();
	QuestGraph->Quest->MarkPackageDirty();

	return GraphNode;
}

void UQuestGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	// Call parent implementation to get standard actions (delete, copy, paste, etc.)
	Super::GetContextMenuActions(Menu, Context);
}

#undef LOCTEXT_NAMESPACE