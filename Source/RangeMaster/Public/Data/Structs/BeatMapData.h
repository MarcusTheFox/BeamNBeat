// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BeatMapData.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Beat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShotPower = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BPM = 0.0f;
};