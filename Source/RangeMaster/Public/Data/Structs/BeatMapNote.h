// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BeatMapDefaults.h"
#include "Engine/DataTable.h"
#include "BeatMapNote.generated.h"

USTRUCT(BlueprintType)
struct FBeatMapNote
{
	GENERATED_BODY()

	UPROPERTY()
	float Beat = 0.0f;

	UPROPERTY()
	int32 Id = 0;

	UPROPERTY()
	int32 Power = 0;

	UPROPERTY()
	FBeatMapDefaults Defaults;
};