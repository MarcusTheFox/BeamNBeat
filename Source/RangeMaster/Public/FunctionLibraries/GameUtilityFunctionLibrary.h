// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/BeamNBeatPlayerState.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameUtilityFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class RANGEMASTER_API UGameUtilityFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FText SecondsToTime(float Seconds);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Game", meta=(WorldContext="WorldContextObject"))
	static class ARangeMasterGameMode* GetRangeMasterGameMode(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContextObject"))
	static ABeamNBeatPlayerState* GetBeamNBeatPlayerState(const UObject* WorldContextObject,
		const int32 PlayerStateIndex = 0);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FString GetGameVersion();
};
