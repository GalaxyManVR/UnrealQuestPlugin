#pragma once

#include "CoreMinimal.h"
#include "QuestTask.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/Actor.h"  
#include "DefeatEnemiesTask.generated.h" 

/**
 * Task that completes when a certain number of enemies are defeated
 */
UCLASS(DisplayName = "Defeat Enemies")
class QUESTSYSTEM_API UDefeatEnemiesTask : public UQuestTask
{
	GENERATED_BODY()

public:
	UDefeatEnemiesTask();

	/** Optional: Filter by enemy class (if null, counts all enemies) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TSubclassOf<AActor> EnemyClass;

	/** Optional: Filter by enemy tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	FName EnemyTag;

	/** If true, count enemies killed before this task started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bRetroactive;

	virtual void BeginTask_Implementation() override;
	virtual void EndTask_Implementation() override;
	virtual FText GetTaskDescription_Implementation() const override;
	virtual FString GetTaskNodeDescription_Implementation() const override;

protected:
	/** Called when an enemy is killed */
	UFUNCTION()
	void OnEnemyKilled(AActor* KilledEnemy, AActor* Killer);

	/** Bind to enemy death events */
	void BindToEnemyEvents();

	/** Unbind from enemy death events */
	void UnbindFromEnemyEvents();

	/** Check if an enemy matches our filter criteria */
	bool DoesEnemyMatchFilter(AActor* Enemy) const;

	/** Starting kill count for non-retroactive tracking */
	UPROPERTY()
	int32 StartingKillCount;
};
