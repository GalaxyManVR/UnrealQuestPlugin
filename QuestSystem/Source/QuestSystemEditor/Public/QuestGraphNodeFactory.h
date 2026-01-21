// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

// Forward declarations to avoid circular includes
class UEdGraphNode;
class SGraphNode;

class FQuestGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
    virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};