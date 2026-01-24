// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Spawner.h"
#include "Actors/Target.h"
#include "Components/ActorComponent.h"
#include "Data/Structs/TimeMapData.h"
#include "TargetSystemComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RANGEMASTER_API UTargetSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTargetSystemComponent();

private:
	UPROPERTY()
	TArray<TObjectPtr<ATarget>> ActiveTargets;

	UPROPERTY()
	TArray<TObjectPtr<ASpawner>> Spawners;

	UPROPERTY()
	TSubclassOf<ATarget> TargetClass;

	UFUNCTION()
	void OnTargetEventHandler(ATarget* Target, const FTargetEventData& EventData);

	void OnTargetDestroyed(ATarget* Target);

public:
	void SetSpawners(const TArray<ASpawner*>& SpawnerList) { Spawners = SpawnerList; }
	void SetTargetClass(const TSubclassOf<ATarget>& Class) { TargetClass = Class; }

	UFUNCTION()
	void SpawnTarget(const FTimeMapData& TimeMapData);
	
	bool IsAllTargetsDestroyed() const { return ActiveTargets.Num() == 0; }
	void DestroyAllTargets();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTargetEvent OnTargetEvent;
};
