// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/BeatMapFunctionLibrary.h"

#include "PhysicsEngine/PhysicsSettings.h"

TArray<FTimeMapData> UBeatMapFunctionLibrary::ConvertBeatMapToBeatTimes(const FBeatMap& BeatMap)
{
	TArray<FTimeMapData> BeatTimes;
	if (BeatMap.Notes.Num() == 0) return BeatTimes;

	BeatTimes.Reserve(BeatMap.Notes.Num());

	FBeatConversionState ConversionState;
	ConversionState.Time = BeatMap.Settings.Offset;
	ConversionState.PreviousBeat = 0.0f;
	ConversionState.Bpm = BeatMap.Settings.Bpm;
	ConversionState.Power = BeatMap.Settings.Power;

	for (const FBeatMapNote& Data: BeatMap.Notes)
	{
		ProcessBeatData(Data, ConversionState, BeatTimes);
	}

	return BeatTimes;
}

float UBeatMapFunctionLibrary::GetInitialBPM(const FBeatMapNote& FirstBeatData)
{
	return FirstBeatData.Bpm > 0.0f ? FirstBeatData.Bpm : 120.0f;
}

void UBeatMapFunctionLibrary::ProcessBeatData(const FBeatMapNote& BeatData, FBeatConversionState& ConversionState,
                                              TArray<FTimeMapData>& OutBeatTimes)
{
	const float DeltaBeat = CalculateDeltaBeat(BeatData.Beat, ConversionState.PreviousBeat);

	UpdateCurrentTime(DeltaBeat, ConversionState);
	UpdateBPMIfChanged(BeatData, ConversionState);

	OutBeatTimes.Add(CreateTimeMapData(BeatData, ConversionState));

	ConversionState.PreviousBeat = BeatData.Beat;
}

float UBeatMapFunctionLibrary::CalculateDeltaBeat(const float CurrentBeat, const float PreviousBeat)
{
	return CurrentBeat - PreviousBeat;
}

void UBeatMapFunctionLibrary::UpdateCurrentTime(const float DeltaBeat, FBeatConversionState& ConversionState)
{
	if (ConversionState.Bpm > 0.0f)
	{
		const float SecondsPerBeat = 60.0f / ConversionState.Bpm;
		ConversionState.Time += DeltaBeat * SecondsPerBeat;
	}
}

void UBeatMapFunctionLibrary::UpdateBPMIfChanged(const FBeatMapNote& BeatData, FBeatConversionState& ConversionState)
{
	if (BeatData.Bpm > 0.0f)
	{
		ConversionState.Bpm = BeatData.Bpm;
	}
}

FTimeMapData UBeatMapFunctionLibrary::CreateTimeMapData(const FBeatMapNote& BeatData, const FBeatConversionState& State)
{
	FTimeMapData NewTimeData;
	NewTimeData.SpawnerID = BeatData.Id;
	NewTimeData.ShotPower = BeatData.Power == 0 ? State.Power : BeatData.Power;
	const float Apex = CalculateApexHeight(NewTimeData.ShotPower);
	NewTimeData.Time = State.Time - CalculateTimeToApex(Apex);
	UE_LOG(LogTemp, Display, TEXT("SpawnerID: %d | Time: %f"), NewTimeData.SpawnerID, NewTimeData.Time);
	return NewTimeData;
}

float UBeatMapFunctionLibrary::CalculateShotPower(float ApexHeight)
{
	float G = UPhysicsSettings::Get()->DefaultGravityZ;
	return FMath::Abs(FMath::Sqrt(2 * G * ApexHeight));
}

float UBeatMapFunctionLibrary::CalculateApexHeight(float ShotPower)
{
	float G = UPhysicsSettings::Get()->DefaultGravityZ;
	return ShotPower * ShotPower / (2 * G);
}

float UBeatMapFunctionLibrary::CalculateTimeToApex(float ApexHeight)
{
	float G = UPhysicsSettings::Get()->DefaultGravityZ;
	return FMath::Abs(FMath::Sqrt(2 * ApexHeight / G));
}
