// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UQuestGraphNode_Branch;

/**
 * Custom Slate widget for Quest Branch nodes
 */
class SQuestGraphNode_Branch : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SQuestGraphNode_Branch) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UQuestGraphNode_Branch* InNode);

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	// End of SGraphNode interface

protected:
	// Get the node's color
	FSlateColor GetBranchColor() const;

	// Get task count text
	FText GetTaskCountText() const;
};