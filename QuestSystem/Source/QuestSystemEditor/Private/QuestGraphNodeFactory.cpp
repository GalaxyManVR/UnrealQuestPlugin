

#include "QuestGraphNodeFactory.h"
#include "QuestGraphNode_State.h"
#include "QuestGraphNode_Branch.h"
#include "SQuestGraphNode_State.h"
#include "SQuestGraphNode_Branch.h"

TSharedPtr<SGraphNode> FQuestGraphNodeFactory::CreateNode(UEdGraphNode* Node) const
{
	if (UQuestGraphNode_State* StateNode = Cast<UQuestGraphNode_State>(Node))
	{
		return SNew(SQuestGraphNode_State, StateNode);
	}

	if (UQuestGraphNode_Branch* BranchNode = Cast<UQuestGraphNode_Branch>(Node))
	{
		return SNew(SQuestGraphNode_Branch, BranchNode);
	}

	return nullptr;
}