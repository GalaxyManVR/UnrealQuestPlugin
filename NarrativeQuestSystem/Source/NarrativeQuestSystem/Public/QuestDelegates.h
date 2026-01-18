// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "QuestDelegates.generated.h"

// Forward declarations
class UQuest;
class UQuestState;

// Quest event delegates - shared between Quest and NarrativeComponent
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, class UQuest*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestSucceeded, class UQuest*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, class UQuest*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, class UQuest*, Quest, class UQuestState*, NewState);

// Dummy class to force UHT to process this file and generate QuestDelegates.generated.h
// This class serves no other purpose
UCLASS(Abstract, NotBlueprintable, Hidden)
class UQuestDelegatesHelper : public UObject
{
	GENERATED_BODY()
};