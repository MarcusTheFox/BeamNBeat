// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Structs/BeatMap.h"
#include "Data/Structs/BeatMapNote.h"
#include "Data/Structs/TimeMapData.h"
#include "Data/Structs/TrackDataRow.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BeatMapFunctionLibrary.generated.h"

USTRUCT()
struct FBeatConversionState
{
	GENERATED_BODY()

	double Time;
	float PreviousBeat;
	FBeatMapDefaults Defaults;
};

UCLASS()
class RANGEMASTER_API UBeatMapFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TArray<FTimeMapData> ConvertBeatMapToBeatTimes(const FBeatMap& BeatMap);

private:
	static void ProcessBeatData(const FBeatMapNote& Note, FBeatConversionState& State, TArray<FTimeMapData>& OutBeatTimes);
	
	static float CalculateDeltaBeat(const float CurrentBeat, const float PreviousBeat);
	
	static void UpdateCurrentTime(const float DeltaBeat, FBeatConversionState& State);
	static void UpdateBPMIfChanged(const FBeatMapNote& Note, FBeatConversionState& State);
	
	static FTimeMapData CreateTimeMapData(const FBeatMapNote& Note, const FBeatConversionState& State);
	
	static float CalculateShotPower(float ApexHeight);
	static float CalculateApexHeight(float ShotPower);
	static float CalculateTimeToApex(float ApexHeight);
};
