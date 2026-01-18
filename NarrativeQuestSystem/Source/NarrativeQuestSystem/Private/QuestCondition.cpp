// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuestCondition.h"
#include "NarrativeComponent.h"
#include "Quest.h"

bool UQuestCondition::CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const
{
	return false;
}

FString UQuestCondition::GetConditionDescription_Implementation() const
{
	return TEXT("Quest Condition");
}

// QuestCondition_QuestActive

bool UQuestCondition_QuestActive::CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const
{
	if (!NarrativeComp || !QuestClass)
	{
		return false;
	}

	// FIXED: Use ByClass version
	return NarrativeComp->IsQuestActiveByClass(QuestClass);
}

FString UQuestCondition_QuestActive::GetConditionDescription_Implementation() const
{
	if (QuestClass)
	{
		UQuest* QuestCDO = QuestClass->GetDefaultObject<UQuest>();
		if (QuestCDO)
		{
			return FString::Printf(TEXT("Quest Active: %s"), *QuestCDO->QuestName.ToString());
		}
		return FString::Printf(TEXT("Quest Active: %s"), *QuestClass->GetName());
	}
	return TEXT("Quest Active");
}

// QuestCondition_QuestSucceeded

bool UQuestCondition_QuestSucceeded::CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const
{
	if (!NarrativeComp || !QuestClass)
	{
		return false;
	}

	// FIXED: Use ByClass version
	UQuest* Quest = NarrativeComp->GetQuestByClass(QuestClass);
	return Quest && Quest->HasQuestSucceeded();
}

FString UQuestCondition_QuestSucceeded::GetConditionDescription_Implementation() const
{
	if (QuestClass)
	{
		UQuest* QuestCDO = QuestClass->GetDefaultObject<UQuest>();
		if (QuestCDO)
		{
			return FString::Printf(TEXT("Quest Succeeded: %s"), *QuestCDO->QuestName.ToString());
		}
		return FString::Printf(TEXT("Quest Succeeded: %s"), *QuestClass->GetName());
	}
	return TEXT("Quest Succeeded");
}

// QuestCondition_QuestFailed

bool UQuestCondition_QuestFailed::CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const
{
	if (!NarrativeComp || !QuestClass)
	{
		return false;
	}

	// FIXED: Use ByClass version
	UQuest* Quest = NarrativeComp->GetQuestByClass(QuestClass);
	return Quest && Quest->HasQuestFailed();
}

FString UQuestCondition_QuestFailed::GetConditionDescription_Implementation() const
{
	if (QuestClass)
	{
		UQuest* QuestCDO = QuestClass->GetDefaultObject<UQuest>();
		if (QuestCDO)
		{
			return FString::Printf(TEXT("Quest Failed: %s"), *QuestCDO->QuestName.ToString());
		}
		return FString::Printf(TEXT("Quest Failed: %s"), *QuestClass->GetName());
	}
	return TEXT("Quest Failed");
}

// QuestCondition_QuestAtState

bool UQuestCondition_QuestAtState::CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const
{
	if (!NarrativeComp || !QuestClass)
	{
		return false;
	}

	// FIXED: Use ByClass version
	UQuest* Quest = NarrativeComp->GetQuestByClass(QuestClass);
	return Quest && Quest->IsQuestAtState(StateID);
}

FString UQuestCondition_QuestAtState::GetConditionDescription_Implementation() const
{
	if (QuestClass)
	{
		return FString::Printf(TEXT("Quest At State: %s (%s)"), *QuestClass->GetName(), *StateID.ToString());
	}
	return TEXT("Quest At State");
}

// QuestCondition_DataTaskCompleted

UQuestCondition_DataTaskCompleted::UQuestCondition_DataTaskCompleted()
{
	MinimumCount = 1;
}

bool UQuestCondition_DataTaskCompleted::CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const
{
	if (!NarrativeComp)
	{
		return false;
	}

	int32 Count = NarrativeComp->GetDataTaskCount(DataTask, Argument);
	return Count >= MinimumCount;
}

FString UQuestCondition_DataTaskCompleted::GetConditionDescription_Implementation() const
{
	if (!Argument.IsEmpty())
	{
		return FString::Printf(TEXT("Data Task: %s (%s) >= %d"), *DataTask.ToString(), *Argument, MinimumCount);
	}
	return FString::Printf(TEXT("Data Task: %s >= %d"), *DataTask.ToString(), MinimumCount);
}