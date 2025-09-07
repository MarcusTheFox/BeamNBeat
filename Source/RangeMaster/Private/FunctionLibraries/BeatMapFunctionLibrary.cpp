// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/BeatMapFunctionLibrary.h"

#include "PhysicsEngine/PhysicsSettings.h"

template <typename T>
TArray<T> UBeatMapFunctionLibrary::GetDataFromDataTable(UDataTable* DataTable)
{
	TArray<T> Data;
	
	if (DataTable)
	{
		TArray<T*> RowPtrs;
		DataTable->GetAllRows<T>(TEXT("GetDataFromDataTable"), RowPtrs);
		
		for (T* RowPtr : RowPtrs)
		{
			if (RowPtr)
			{
				Data.Add(*RowPtr);
			}
		}
	}
	
	return Data;
}

TArray<FTrackDataRow> UBeatMapFunctionLibrary::GetTrackData(UDataTable* TrackDataTable)
{
	return GetDataFromDataTable<FTrackDataRow>(TrackDataTable);
}

TArray<FBeatMapData> UBeatMapFunctionLibrary::GetBeatMapData(UDataTable* BeatMapTable)
{
	return GetDataFromDataTable<FBeatMapData>(BeatMapTable);
}

TArray<FTimeMapData> UBeatMapFunctionLibrary::GetTimeMapData(UDataTable* BeatMapTable)
{
	return ConvertBeatMapToBeatTimes(GetBeatMapData(BeatMapTable), 0);
}

TArray<FTimeMapData> UBeatMapFunctionLibrary::ConvertBeatMapToBeatTimes(TArray<FBeatMapData> BeatMapData, float TimeOffsetMs)
{
	TArray<FTimeMapData> BeatTimes;
	if (BeatMapData.Num() == 0) return BeatTimes;

	BeatTimes.Reserve(BeatMapData.Num());

	FBeatConversionState ConversionState;
	ConversionState.CurrentTime = TimeOffsetMs / 1000.0;
	ConversionState.PreviousBeat = 0.0f;
	ConversionState.CurrentBPM = GetInitialBPM(BeatMapData[0]);

	for (const FBeatMapData& Data: BeatMapData)
	{
		ProcessBeatData(Data, ConversionState, BeatTimes);
	}

	return BeatTimes;
}

float UBeatMapFunctionLibrary::GetInitialBPM(const FBeatMapData& FirstBeatData)
{
	return FirstBeatData.BPM > 0.0f ? FirstBeatData.BPM : 120.0f;
}

void UBeatMapFunctionLibrary::ProcessBeatData(const FBeatMapData& BeatData, FBeatConversionState& ConversionState,
                                              TArray<FTimeMapData>& OutBeatTimes)
{
	const float Beat = CalculateTotalBeat(BeatData);
	const float DeltaBeat = CalculateDeltaBeat(Beat, ConversionState.PreviousBeat);

	UpdateCurrentTime(DeltaBeat, ConversionState);
	UpdateBPMIfChanged(BeatData, ConversionState);

	OutBeatTimes.Add(CreateTimeMapData(BeatData, ConversionState.CurrentTime));

	ConversionState.PreviousBeat = Beat;
}

float UBeatMapFunctionLibrary::CalculateTotalBeat(const FBeatMapData& BeatData)
{
	return BeatData.BeatIndex + BeatData.BeatFraction;
}

float UBeatMapFunctionLibrary::CalculateDeltaBeat(const float CurrentBeat, const float PreviousBeat)
{
	return CurrentBeat - PreviousBeat;
}

void UBeatMapFunctionLibrary::UpdateCurrentTime(const float DeltaBeat, FBeatConversionState& ConversionState)
{
	if (ConversionState.CurrentBPM > 0.0f)
	{
		const float SecondsPerBeat = 60.0f / ConversionState.CurrentBPM;
		ConversionState.CurrentTime += DeltaBeat * SecondsPerBeat;
	}
}

void UBeatMapFunctionLibrary::UpdateBPMIfChanged(const FBeatMapData& BeatData, FBeatConversionState& ConversionState)
{
	if (BeatData.BPM > 0.0f)
	{
		ConversionState.CurrentBPM = BeatData.BPM;
	}
}

FTimeMapData UBeatMapFunctionLibrary::CreateTimeMapData(const FBeatMapData& BeatData, const double CurrentTime)
{
	FTimeMapData NewTimeData;
	NewTimeData.SpawnerID = BeatData.SpawnerID;
	NewTimeData.ShotPower = BeatData.ShotPower;
	NewTimeData.Time = CurrentTime;
	return NewTimeData;
}
