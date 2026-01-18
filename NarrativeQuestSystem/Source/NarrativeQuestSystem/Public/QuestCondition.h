// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Templates/SubclassOf.h"
#include "QuestCondition.generated.h"

class UNarrativeComponent;
class UQuest;

/**
 * Base class for conditions that can check quest state
 * Useful for dialogue systems, gate unlocking, etc.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class NARRATIVEQUESTSYSTEM_API UQuestCondition : public UObject
{
	GENERATED_BODY()

public:
	/** Check if the condition is met */
	UFUNCTION(BlueprintNativeEvent, Category = "Quest Condition")
	bool CheckCondition(UNarrativeComponent* NarrativeComp) const;
	virtual bool CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const;

	/** Get a description of this condition */
	UFUNCTION(BlueprintNativeEvent, Category = "Quest Condition")
	FString GetConditionDescription() const;
	virtual FString GetConditionDescription_Implementation() const;
};

/**
 * Condition that checks if a quest is active
 */
UCLASS(DisplayName = "Quest Active")
class NARRATIVEQUESTSYSTEM_API UQuestCondition_QuestActive : public UQuestCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TSubclassOf<UQuest> QuestClass;

	virtual bool CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const override;
	virtual FString GetConditionDescription_Implementation() const override;
};

/**
 * Condition that checks if a quest has succeeded
 */
UCLASS(DisplayName = "Quest Succeeded")
class NARRATIVEQUESTSYSTEM_API UQuestCondition_QuestSucceeded : public UQuestCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TSubclassOf<UQuest> QuestClass;

	virtual bool CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const override;
	virtual FString GetConditionDescription_Implementation() const override;
};

/**
 * Condition that checks if a quest has failed
 */
UCLASS(DisplayName = "Quest Failed")
class NARRATIVEQUESTSYSTEM_API UQuestCondition_QuestFailed : public UQuestCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TSubclassOf<UQuest> QuestClass;

	virtual bool CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const override;
	virtual FString GetConditionDescription_Implementation() const override;
};

/**
 * Condition that checks if a quest is at a specific state
 */
UCLASS(DisplayName = "Quest At State")
class NARRATIVEQUESTSYSTEM_API UQuestCondition_QuestAtState : public UQuestCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	TSubclassOf<UQuest> QuestClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FName StateID;

	virtual bool CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const override;
	virtual FString GetConditionDescription_Implementation() const override;
};

/**
 * Condition that checks if a data task has been completed
 */
UCLASS(DisplayName = "Data Task Completed")
class NARRATIVEQUESTSYSTEM_API UQuestCondition_DataTaskCompleted : public UQuestCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FName DataTask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FString Argument;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	int32 MinimumCount;

	UQuestCondition_DataTaskCompleted();

	virtual bool CheckCondition_Implementation(UNarrativeComponent* NarrativeComp) const override;
	virtual FString GetConditionDescription_Implementation() const override;
};