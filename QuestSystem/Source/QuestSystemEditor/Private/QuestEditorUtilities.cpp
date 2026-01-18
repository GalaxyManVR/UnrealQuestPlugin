#include "QuestEditorUtilities.h"
#include "Quest.h"
#include "QuestState.h"
#include "QuestBranch.h"
#include "QuestGraphNode_State.h"
#include "QuestGraphNode_Branch.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#if WITH_EDITOR
void FQuestEditorUtilities::CompileQuest(UQuest* Quest, UEdGraph* Graph)
{
    if (!Quest || !Graph)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("FQuestEditorUtilities::CompileQuest for %s"), *Quest->GetName());

    // Clear old runtime data
    Quest->QuestBranches.Empty();
    for (UQuestState* State : Quest->QuestStates)
    {
        if (State)
        {
            State->OutgoingBranches.Empty();
            State->IncomingBranches.Empty();
        }
    }

    // Collect all state nodes
    TMap<UQuestGraphNode_State*, UQuestState*> NodeToStateMap;
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (UQuestGraphNode_State* StateNode = Cast<UQuestGraphNode_State>(Node))
        {
            if (StateNode->QuestState)
            {
                NodeToStateMap.Add(StateNode, StateNode->QuestState);
                if (!Quest->QuestStates.Contains(StateNode->QuestState))
                {
                    Quest->QuestStates.Add(StateNode->QuestState);
                }
            }
        }
    }

    // Collect all branch nodes
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (UQuestGraphNode_Branch* BranchNode = Cast<UQuestGraphNode_Branch>(Node))
        {
            UQuestBranch* Branch = BranchNode->QuestBranch;
            if (!Branch)
            {
                Branch = NewObject<UQuestBranch>(Quest);
                BranchNode->QuestBranch = Branch;
            }

            Branch->OwningQuest = Quest;

            // Find connected state nodes
            UEdGraphPin* InputPin = BranchNode->GetInputPin();
            UEdGraphPin* OutputPin = BranchNode->GetOutputPin();

            Branch->FromState = nullptr;
            Branch->ToState = nullptr;

            if (InputPin)
            {
                for (UEdGraphPin* LinkedPin : InputPin->LinkedTo)
                {
                    if (UQuestGraphNode_State* FromNode = Cast<UQuestGraphNode_State>(LinkedPin->GetOwningNode()))
                    {
                        Branch->FromState = NodeToStateMap.FindRef(FromNode);
                    }
                }
            }

            if (OutputPin)
            {
                for (UEdGraphPin* LinkedPin : OutputPin->LinkedTo)
                {
                    if (UQuestGraphNode_State* ToNode = Cast<UQuestGraphNode_State>(LinkedPin->GetOwningNode()))
                    {
                        Branch->ToState = NodeToStateMap.FindRef(ToNode);
                    }
                }
            }

            Quest->QuestBranches.Add(Branch);

            if (Branch->FromState)
            {
                Branch->FromState->OutgoingBranches.Add(Branch);
            }
            if (Branch->ToState)
            {
                Branch->ToState->IncomingBranches.Add(Branch);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CompileQuest finished: %d states, %d branches"),
        Quest->QuestStates.Num(), Quest->QuestBranches.Num());
}
#endif