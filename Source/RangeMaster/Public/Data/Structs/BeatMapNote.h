// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BeatMapNote.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapNote
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Beat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Power = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Bpm = 0.0f;
};