// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/JudgementSystemComponent.h"
#include "Components/ScoreSystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "BeamNBeatPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class RANGEMASTER_API ABeamNBeatPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABeamNBeatPlayerState();
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UScoreSystemComponent* ScoreSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UJudgementSystemComponent* JudgementSystem;

	UFUNCTION(BlueprintCallable)
	void RegisterHit(const float HitTime) const;

	UFUNCTION(BlueprintCallable)
	void RegisterMiss() const;

	UFUNCTION(BlueprintCallable)
	void RegisterLost() const;
	
	virtual void Reset() override;
};
