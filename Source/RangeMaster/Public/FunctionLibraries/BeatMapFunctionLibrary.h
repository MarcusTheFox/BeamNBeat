// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Structs/BeatMapData.h"
#include "Data/Structs/TimeMapData.h"
#include "Data/Structs/TrackDataRow.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BeatMapFunctionLibrary.generated.h"

USTRUCT()
struct FBeatConversionState
{
	GENERATED_BODY()

	double CurrentTime = 0.0;
	float PreviousBeat = 0.0f;
	float CurrentBPM = 120.0f;
};

UCLASS()
class RANGEMASTER_API UBeatMapFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template <typename T>
	static TArray<T> GetDataFromDataTable(UDataTable* DataTable);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TArray<FTrackDataRow> GetTrackData(UDataTable* TrackDataTable);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TArray<FBeatMapData> GetBeatMapData(UDataTable* BeatMapTable);

	static TArray<FTimeMapData> GetTimeMapData(UDataTable* BeatMapTable);
	
	static TArray<FTimeMapData> ConvertBeatMapToBeatTimes(TArray<FBeatMapData> BeatMapData, float TimeOffsetMs);

private:
	static float GetInitialBPM(const FBeatMapData& FirstBeatData);
	static void ProcessBeatData(const FBeatMapData& BeatData, FBeatConversionState& ConversionState, TArray<FTimeMapData>& OutBeatTimes);
	
	static float CalculateTotalBeat(const FBeatMapData& BeatData);
	static float CalculateDeltaBeat(const float CurrentBeat, const float PreviousBeat);
	
	static void UpdateCurrentTime(const float DeltaBeat, FBeatConversionState& ConversionState);
	static void UpdateBPMIfChanged(const FBeatMapData& BeatData, FBeatConversionState& ConversionState);
	
	static FTimeMapData CreateTimeMapData(const FBeatMapData& BeatData, const double CurrentTime);
};
