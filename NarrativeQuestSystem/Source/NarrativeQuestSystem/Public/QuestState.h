#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestState.generated.h"

class UQuestBranch;
class UQuest;

UENUM(BlueprintType)
enum class EQuestStateType : uint8
{
	QuestStart,
	QuestState,
	QuestSuccess,
	QuestFailure
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class NARRATIVEQUESTSYSTEM_API UQuestState : public UObject
{
	GENERATED_BODY()

public:
	UQuestState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	EQuestStateType StateType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FName ID;  // Unique ID for the state

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FText DisplayName;  // Shown in editor and graph nodes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (MultiLine = true))
	FText Description;  // Editor shows description

	// CRITICAL FIX: These need to be serialized to persist at runtime
	// Using VisibleAnywhere instead of SaveGame for asset serialization
	UPROPERTY(EditAnywhere, Instanced, Category = "State")
	TArray<TObjectPtr<UQuestBranch>> OutgoingBranches;

	UPROPERTY(EditAnywhere, Instanced, Category = "State")
	TArray<TObjectPtr<UQuestBranch>> IncomingBranches;


	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UQuest> OwningQuest;

	UPROPERTY()
	FGuid StateGuid;

	// Initialize the state
	void Initialize(UQuest* InQuest);

	void AddOutgoingBranch(UQuestBranch* Branch);
	void AddIncomingBranch(UQuestBranch* Branch);

	UFUNCTION(BlueprintPure, Category = "State")
	bool IsTerminalState() const;

	UFUNCTION(BlueprintNativeEvent, Category = "State")
	void OnStateEntered();
	virtual void OnStateEntered_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "State")
	void OnStateExited();
	virtual void OnStateExited_Implementation();
};