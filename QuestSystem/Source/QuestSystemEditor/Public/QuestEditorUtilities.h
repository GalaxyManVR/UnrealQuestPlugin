// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"

class UQuest;
class UEdGraph;

class FQuestEditorUtilities
{
public:
#if WITH_EDITOR
    /** Compile the quest from a given graph */
    static void CompileQuest(UQuest* Quest, UEdGraph* Graph);
#endif
};