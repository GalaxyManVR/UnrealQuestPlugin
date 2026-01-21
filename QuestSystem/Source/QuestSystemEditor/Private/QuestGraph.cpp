// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#include "QuestGraph.h"
#include "Quest.h"
#include "QuestState.h"
#include "QuestBranch.h"
#include "QuestTask.h"
#include "QuestGraphNode_State.h"
#include "QuestGraphNode_Branch.h"
#include "EdGraph/EdGraphPin.h"

void UQuestGraph::InitializeGraph(UQuest* InQuest)
{
	Quest = InQuest;

	if (Quest)
	{
		// Auto-compile before save
		Quest->SetPreSaveListener([this]()
			{
				CompileQuestFromGraph();
			});
	}

	// CRITICAL FIX: Only rebuild if graph is empty
	// This preserves node positions when reopening the editor
	if (Nodes.Num() > 0)
	{
		// Graph already has nodes - just update their data
		for (UEdGraphNode* Node : Nodes)
		{
			if (UQuestGraphNode_State* StateNode = Cast<UQuestGraphNode_State>(Node))
			{
				// Node already exists with position - just refresh its data
				if (StateNode->QuestState)
				{
					StateNode->QuestState->Initialize(Quest);
				}
			}
			else if (UQuestGraphNode_Branch* BranchNode = Cast<UQuestGraphNode_Branch>(Node))
			{
				// Refresh branch data
				if (BranchNode->QuestBranch)
				{
					BranchNode->QuestBranch->Initialize(Quest, BranchNode->QuestBranch->FromState, BranchNode->QuestBranch->ToState);
				}
			}
		}
		return;
	}

	// First time opening or graph was cleared - build from scratch

	// Ensure default states exist
	if (Quest->QuestStates.Num() == 0)
	{
		Quest->Initialize(nullptr);
	}

	// Create a map to track state nodes
	TMap<UQuestState*, UQuestGraphNode_State*> StateNodeMap;

	// First pass: Create all state nodes
	int32 RegularStateCount = 0;

	for (int32 i = 0; i < Quest->QuestStates.Num(); ++i)
	{
		UQuestState* State = Quest->QuestStates[i];
		if (!State)
		{
			continue;
		}

		UQuestGraphNode_State* StateNode = NewObject<UQuestGraphNode_State>(this, UQuestGraphNode_State::StaticClass(), NAME_None, RF_Transactional);
		StateNode->CreateNewGuid();
		StateNode->InitializeFromState(State);

		// Layout states in a sensible way
		switch (State->StateType)
		{
		case EQuestStateType::QuestStart:
			// Start at top left
			StateNode->NodePosX = 0;
			StateNode->NodePosY = 0;
			break;

		case EQuestStateType::QuestSuccess:
			// Success at bottom left
			StateNode->NodePosX = 0;
			StateNode->NodePosY = 600;
			break;

		case EQuestStateType::QuestFailure:
			// Failure at bottom right
			StateNode->NodePosX = 400;
			StateNode->NodePosY = 600;
			break;

		case EQuestStateType::QuestState:
			// Regular states in the middle
			StateNode->NodePosX = 200;
			StateNode->NodePosY = 200 + (RegularStateCount * 150);
			RegularStateCount++;
			break;
		}

		AddNode(StateNode, false, false);
		StateNodeMap.Add(State, StateNode);
	}

	// Second pass: Create all branch nodes and connections
	for (int32 i = 0; i < Quest->QuestBranches.Num(); ++i)
	{
		UQuestBranch* Branch = Quest->QuestBranches[i];
		if (!Branch)
		{
			continue;
		}

		UQuestGraphNode_Branch* BranchNode = NewObject<UQuestGraphNode_Branch>(this, UQuestGraphNode_Branch::StaticClass(), NAME_None, RF_Transactional);
		BranchNode->CreateNewGuid();
		BranchNode->InitializeFromBranch(Branch);

		// Position branch between its connected states
		if (Branch->FromState && Branch->ToState &&
			StateNodeMap.Contains(Branch->FromState) &&
			StateNodeMap.Contains(Branch->ToState))
		{
			UQuestGraphNode_State* FromNode = StateNodeMap[Branch->FromState];
			UQuestGraphNode_State* ToNode = StateNodeMap[Branch->ToState];

			BranchNode->NodePosX = (FromNode->NodePosX + ToNode->NodePosX) / 2;
			BranchNode->NodePosY = (FromNode->NodePosY + ToNode->NodePosY) / 2;
		}
		else
		{
			// Default position
			BranchNode->NodePosX = 400;
			BranchNode->NodePosY = 200 + (i * 150);
		}

		AddNode(BranchNode, false, false);

		// Create connections
		if (Branch->FromState && StateNodeMap.Contains(Branch->FromState))
		{
			UEdGraphPin* FromStatePin = StateNodeMap[Branch->FromState]->FindPin(FName(TEXT("Out")));
			UEdGraphPin* BranchInPin = BranchNode->FindPin(FName(TEXT("In")));
			if (FromStatePin && BranchInPin)
			{
				FromStatePin->MakeLinkTo(BranchInPin);
			}
		}

		if (Branch->ToState && StateNodeMap.Contains(Branch->ToState))
		{
			UEdGraphPin* BranchOutPin = BranchNode->FindPin(FName(TEXT("Out")));
			UEdGraphPin* ToStatePin = StateNodeMap[Branch->ToState]->FindPin(FName(TEXT("In")));
			if (BranchOutPin && ToStatePin)
			{
				BranchOutPin->MakeLinkTo(ToStatePin);
			}
		}
	}

	NotifyGraphChanged();
}

void UQuestGraph::CompileQuestFromGraph()
{
	if (!Quest)
	{
		UE_LOG(LogTemp, Error, TEXT("CompileQuestFromGraph: No quest!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("=== Starting Quest Compilation ==="));

	// Mark quest as modified BEFORE compilation
	Quest->Modify();

	// Build new arrays (don't clear the main arrays yet - we'll swap at the end)
	TArray<UQuestState*> NewStates;
	TArray<UQuestBranch*> NewBranches;

	// Rebuild quest from graph nodes
	TArray<UEdGraphNode*> GraphNodes;
	GetNodesOfClass<UEdGraphNode>(GraphNodes);

	UE_LOG(LogTemp, Warning, TEXT("Found %d graph nodes"), GraphNodes.Num());

	// First pass: Collect/create all states
	for (UEdGraphNode* Node : GraphNodes)
	{
		if (UQuestGraphNode_State* StateNode = Cast<UQuestGraphNode_State>(Node))
		{
			if (StateNode->QuestState)
			{
				// Ensure proper ownership
				if (StateNode->QuestState->GetOuter() != Quest)
				{
					UE_LOG(LogTemp, Warning, TEXT("Fixing state outer: %s"), *StateNode->QuestState->ID.ToString());
					StateNode->QuestState->Rename(nullptr, Quest, REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
				}
				StateNode->QuestState->Initialize(Quest);
				NewStates.AddUnique(StateNode->QuestState);
				UE_LOG(LogTemp, Warning, TEXT("Added state: %s"), *StateNode->QuestState->ID.ToString());
			}
		}
	}

	// Second pass: Collect/create all branches and connect them
	for (UEdGraphNode* Node : GraphNodes)
	{
		if (UQuestGraphNode_Branch* BranchNode = Cast<UQuestGraphNode_Branch>(Node))
		{
			if (BranchNode->QuestBranch)
			{
				// Ensure proper ownership
				if (BranchNode->QuestBranch->GetOuter() != Quest)
				{
					UE_LOG(LogTemp, Warning, TEXT("Fixing branch outer"));
					BranchNode->QuestBranch->Rename(nullptr, Quest, REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
				}

				// Fix task ownership
				for (UQuestTask* Task : BranchNode->QuestBranch->QuestTasks)
				{
					if (Task && Task->GetOuter() != Quest)
					{
						UE_LOG(LogTemp, Warning, TEXT("Fixing task outer"));
						Task->Rename(nullptr, Quest, REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
					}
				}

				// Find connected states by looking at pin connections
				UEdGraphPin* InputPin = nullptr;
				UEdGraphPin* OutputPin = nullptr;

				for (UEdGraphPin* Pin : BranchNode->Pins)
				{
					if (Pin && Pin->Direction == EGPD_Input)
					{
						InputPin = Pin;
					}
					else if (Pin && Pin->Direction == EGPD_Output)
					{
						OutputPin = Pin;
					}
				}

				BranchNode->QuestBranch->FromState = nullptr;
				BranchNode->QuestBranch->ToState = nullptr;

				if (InputPin && InputPin->LinkedTo.Num() > 0)
				{
					UEdGraphPin* SourcePin = InputPin->LinkedTo[0];
					if (UQuestGraphNode_State* SourceNode = Cast<UQuestGraphNode_State>(SourcePin->GetOwningNode()))
					{
						BranchNode->QuestBranch->FromState = SourceNode->QuestState;
						UE_LOG(LogTemp, Warning, TEXT("Branch FromState: %s"), *SourceNode->QuestState->ID.ToString());
					}
				}

				if (OutputPin && OutputPin->LinkedTo.Num() > 0)
				{
					UEdGraphPin* TargetPin = OutputPin->LinkedTo[0];
					if (UQuestGraphNode_State* TargetNode = Cast<UQuestGraphNode_State>(TargetPin->GetOwningNode()))
					{
						BranchNode->QuestBranch->ToState = TargetNode->QuestState;
						UE_LOG(LogTemp, Warning, TEXT("Branch ToState: %s"), *TargetNode->QuestState->ID.ToString());
					}
				}

				BranchNode->QuestBranch->Initialize(Quest, BranchNode->QuestBranch->FromState, BranchNode->QuestBranch->ToState);
				NewBranches.AddUnique(BranchNode->QuestBranch);
				UE_LOG(LogTemp, Warning, TEXT("Added branch with %d tasks"), BranchNode->QuestBranch->QuestTasks.Num());
			}
		}
	}

	// Update quest arrays
	Quest->QuestStates = NewStates;
	Quest->QuestBranches = NewBranches;
	Quest->QuestStartState = nullptr;
	Quest->QuestSuccessState = nullptr;
	Quest->QuestFailureState = nullptr;

	// Third pass - populate state branch arrays
	// Clear all branch arrays first
	for (UQuestState* State : Quest->QuestStates)
	{
		if (State)
		{
			State->OutgoingBranches.Empty();
			State->IncomingBranches.Empty();
		}
	}

	// Now populate them from the branches
	for (UQuestBranch* Branch : Quest->QuestBranches)
	{
		if (Branch)
		{
			if (Branch->FromState)
			{
				Branch->FromState->AddOutgoingBranch(Branch);
			}
			if (Branch->ToState)
			{
				Branch->ToState->AddIncomingBranch(Branch);
			}
		}
	}

	// Update quest state references
	for (UQuestState* State : Quest->QuestStates)
	{
		if (State)
		{
			switch (State->StateType)
			{
			case EQuestStateType::QuestStart:
				Quest->QuestStartState = State;
				break;
			case EQuestStateType::QuestSuccess:
				Quest->QuestSuccessState = State;
				break;
			case EQuestStateType::QuestFailure:
				Quest->QuestFailureState = State;
				break;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== Quest Compiled ==="));
	UE_LOG(LogTemp, Warning, TEXT("States: %d, Branches: %d"), Quest->QuestStates.Num(), Quest->QuestBranches.Num());

	for (UQuestBranch* Branch : Quest->QuestBranches)
	{
		if (Branch)
		{
			UE_LOG(LogTemp, Warning, TEXT("Branch: From=%s To=%s Tasks=%d"),
				Branch->FromState ? *Branch->FromState->ID.ToString() : TEXT("NULL"),
				Branch->ToState ? *Branch->ToState->ID.ToString() : TEXT("NULL"),
				Branch->QuestTasks.Num());
		}
	}

	// Log state connections for verification
	for (UQuestState* State : Quest->QuestStates)
	{
		if (State)
		{
			UE_LOG(LogTemp, Warning, TEXT("State %s: Outgoing=%d, Incoming=%d"),
				*State->ID.ToString(),
				State->OutgoingBranches.Num(),
				State->IncomingBranches.Num());
		}
	}

	// CRITICAL: Mark package dirty AFTER compilation
	Quest->MarkPackageDirty();

	UPackage* Package = Quest->GetOutermost();
	if (Package)
	{
		Package->SetDirtyFlag(true);
		UE_LOG(LogTemp, Warning, TEXT("Package marked dirty: %s"), *Package->GetName());
	}

	UE_LOG(LogTemp, Warning, TEXT("=== Compilation Complete ==="));
}