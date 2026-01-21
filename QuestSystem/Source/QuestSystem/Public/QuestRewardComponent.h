// Copyright (c) 2026 Logan Chambers. All Rights Reserved.\n

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/UnrealType.h"
#include "QuestRewardComponent.generated.h"

class UInventoryComponent;

// Configuration for how to handle a stat/variable reward

USTRUCT(BlueprintType)
struct FStatRewardConfig
{
	GENERATED_BODY()

	// Name of the property/variable to modify (e.g., "Health", "Experience", "Gold") 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Config")
	FName PropertyName;

	// Optional: Specific component class to look for this property in 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Config")
	TSubclassOf<UActorComponent> ComponentClass;

	// If true, search in the owner actor directly instead of components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Config")
	bool bSearchInOwnerActor = true;

	// If true, multiply the reward value by this multiplier (for bonuses)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Config")
	float Multiplier = 1.0f;
};

//Configuration for inventory-based rewards

USTRUCT(BlueprintType)
struct FInventoryRewardConfig
{
	GENERATED_BODY()

	// The inventory component class to use 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Config")
	TSubclassOf<UActorComponent> InventoryComponentClass;

	// Name of the function to call to add items (e.g., "AddItem") 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Config")
	FName AddItemFunctionName = "AddItem";
};

/**
 * Component that automatically detects and modifies variables for quest rewards
 * Add this to any actor that should receive quest rewards
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class QUESTSYSTEM_API UQuestRewardComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestRewardComponent();


	// STAT/VALUE REWARDS

	/**
	 * Configuration for common stats (Experience, Gold, Health, etc.)
	 * The component will auto-detect these and add values to them
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Config|Stats")
	TArray<FStatRewardConfig> StatConfigurations;

	//
	// Automatically find and modify a stat/property by name
	// Returns true if successful
	//
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	bool ModifyStatByName(FName StatName, int32 Amount);

	//Find a numeric property and add a value to it Searches owner actor and all components
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	bool FindAndModifyNumericProperty(FName PropertyName, int32 Amount);

	// INVENTORY REWARDS

	//Configuration for inventory system 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Config|Inventory")
	FInventoryRewardConfig InventoryConfig;

	//Try to add an item to the inventory Automatically finds the inventory component based on config
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	bool AddItemToInventory(TSubclassOf<UObject> ItemClass, FName ItemID, int32 Quantity);

	//Get the inventory component (if any)
	
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	UActorComponent* GetInventoryComponent();


	// AUTO-DETECTION

	// If true, automatically search for common property names 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Config|Auto Detection")
	bool bAutoDetectCommonStats = true;

	// Common stat names to auto-detect (used when bAutoDetectCommonStats is true) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward Config|Auto Detection")
	TArray<FName> CommonStatNames = { "Experience", "XP", "Gold", "Currency", "Health", "Mana", "Stamina" };

	//Initialize auto-detection - scans the actor for available properties Called automatically on BeginPlay if bAutoDetectCommonStats is true
	
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	void InitializeAutoDetection();

protected:
	virtual void BeginPlay() override;

private:
	// Helper: Find a property in an object 
	FProperty* FindPropertyInObject(UObject* Object, FName PropertyName);

	// Helper: Modify a numeric property 
	bool ModifyNumericProperty(UObject* Object, FProperty* Property, int32 Amount);

	// Cached property locations for faster lookups
	UPROPERTY()
	TMap<FName, TObjectPtr<UObject>> CachedPropertyOwners;

	// Non-UPROPERTY because FProperty cannot be serialized
	TMap<FName, FProperty*> CachedProperties;
};