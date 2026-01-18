#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Interface.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/Actor.h"  // ADD THIS LINE
#include "QuestReward.generated.h"

class UQuest;
class UQuestComponent;

/**
 * Enum to define the type of reward target
 */
UENUM(BlueprintType)
enum class ERewardTargetType : uint8
{
	/** Give reward to the quest owner (default) */
	QuestOwner UMETA(DisplayName = "Quest Owner"),

	/** Give reward to a specific actor */
	SpecificActor UMETA(DisplayName = "Specific Actor"),

	/** Give reward to an actor found by tag */
	ActorWithTag UMETA(DisplayName = "Actor With Tag"),

	/** Custom Blueprint logic to determine target */
	Custom UMETA(DisplayName = "Custom")
};

// ========================================
// REWARD RECEIVER INTERFACE
// ========================================

/**
 * Interface for actors that can receive quest rewards
 * Implement this on your characters/objects to handle rewards
 */
UINTERFACE(MinimalAPI, Blueprintable)
class URewardReceiver : public UInterface
{
	GENERATED_BODY()
};

class QUESTSYSTEM_API IRewardReceiver
{
	GENERATED_BODY()

public:
	/** Called when the actor should receive experience */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewards")
	void ReceiveExperience(int32 Amount);

	/** Called when the actor should receive currency */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewards")
	void ReceiveCurrency(FName CurrencyType, int32 Amount);

	/** Called when the actor should receive an item */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewards")
	void ReceiveItem(TSubclassOf<UObject> ItemClass, FName ItemID, int32 Quantity);

	/** Called when a quest should be unlocked for this actor */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Rewards")
	void ReceiveQuestUnlock(UQuest* QuestToUnlock, bool bAutoStart);
};

// ========================================
// REWARD DELEGATES
// ========================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRewardGiven, AActor*, Target, UQuestReward*, Reward, UQuest*, Quest);

// ========================================
// BASE REWARD CLASS
// ========================================

/**
 * Base class for quest rewards
 * Extend this to create custom reward types
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class QUESTSYSTEM_API UQuestReward : public UObject
{
	GENERATED_BODY()

public:
	/** Called when the reward should be granted */
	UFUNCTION(BlueprintNativeEvent, Category = "Reward")
	void GiveReward(UQuestComponent* QuestComponent, UQuest* Quest);
	virtual void GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest);

	/** Get a description of this reward for UI display */
	UFUNCTION(BlueprintNativeEvent, Category = "Reward")
	FText GetRewardDescription() const;
	virtual FText GetRewardDescription_Implementation() const;

	/** Whether this reward has already been given */
	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	bool bAlreadyGiven;

	/** Broadcast when a reward is successfully given */
	UPROPERTY(BlueprintAssignable, Category = "Reward")
	FOnRewardGiven OnRewardGiven;

	// ===== TARGET SYSTEM =====

	/** How to determine who/what receives this reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Target")
	ERewardTargetType TargetType;

	/** Specific actor to give reward to (when TargetType is SpecificActor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Target", meta = (EditCondition = "TargetType == ERewardTargetType::SpecificActor", EditConditionHides))
	TObjectPtr<AActor> SpecificTargetActor;

	/** Tag to find actor by (when TargetType is ActorWithTag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Target", meta = (EditCondition = "TargetType == ERewardTargetType::ActorWithTag", EditConditionHides))
	FName TargetActorTag;

	/**
	 * Override this in Blueprint to provide custom target logic
	 * Only called when TargetType is Custom
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Reward Target")
	AActor* GetCustomRewardTarget(UQuestComponent* QuestComponent, UQuest* Quest);
	virtual AActor* GetCustomRewardTarget_Implementation(UQuestComponent* QuestComponent, UQuest* Quest);

	/**
	 * Helper function to get the final reward target based on settings
	 * This is called internally before applying the reward
	 */
	UFUNCTION(BlueprintCallable, Category = "Reward Target")
	AActor* GetRewardTarget(UQuestComponent* QuestComponent, UQuest* Quest);

	UQuestReward() : bAlreadyGiven(false), TargetType(ERewardTargetType::QuestOwner) {}
};

/**
 * Reward that gives experience points
 */
UCLASS(DisplayName = "Experience Reward")
class QUESTSYSTEM_API UExperienceReward : public UQuestReward
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ExperienceAmount;

	UExperienceReward() : ExperienceAmount(100) {}

	virtual void GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest) override;
	virtual FText GetRewardDescription_Implementation() const override;
};

/**
 * Reward that gives currency/gold
 */
UCLASS(DisplayName = "Currency Reward")
class QUESTSYSTEM_API UCurrencyReward : public UQuestReward
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 CurrencyAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName CurrencyType; // e.g., "Gold", "Gems", etc.

	UCurrencyReward() : CurrencyAmount(100), CurrencyType("Gold") {}

	virtual void GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest) override;
	virtual FText GetRewardDescription_Implementation() const override;
};

/**
 * Reward that gives items
 */
UCLASS(DisplayName = "Item Reward")
class QUESTSYSTEM_API UItemReward : public UQuestReward
{
	GENERATED_BODY()

public:
	/** The class of item to give (your item class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TSubclassOf<UObject> ItemClass;

	/** Number of items to give */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 Quantity;

	/** Optional: Item data/ID if using a data-driven system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName ItemID;

	UItemReward() : Quantity(1) {}

	virtual void GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest) override;
	virtual FText GetRewardDescription_Implementation() const override;
};

/**
 * Reward that unlocks another quest
 */
UCLASS(DisplayName = "Unlock Quest Reward")
class QUESTSYSTEM_API UUnlockQuestReward : public UQuestReward
{
	GENERATED_BODY()

public:
	/** The quest to unlock/start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<UQuest> QuestToUnlock;

	/** If true, automatically start the quest. If false, just make it available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	bool bAutoStartQuest;

	UUnlockQuestReward() : bAutoStartQuest(false) {}

	virtual void GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest) override;
	virtual FText GetRewardDescription_Implementation() const override;
};

/**
 * Reward that executes Blueprint logic
 */
UCLASS(DisplayName = "Blueprint Reward")
class QUESTSYSTEM_API UBlueprintReward : public UQuestReward
{
	GENERATED_BODY()

public:
	/** Description shown in UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FText RewardDescription;

	// Override in Blueprint to implement custom reward logic
	virtual void GiveReward_Implementation(UQuestComponent* QuestComponent, UQuest* Quest) override;
	virtual FText GetRewardDescription_Implementation() const override;
};