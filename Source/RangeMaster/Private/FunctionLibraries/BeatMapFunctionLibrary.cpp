// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/BeatMapFunctionLibrary.h"

#include "PhysicsEngine/PhysicsSettings.h"

TArray<FTimeMapData> UBeatMapFunctionLibrary::ConvertBeatMapToBeatTimes(const FBeatMap& BeatMap)
{
	TArray<FTimeMapData> BeatTimes;
	if (BeatMap.Notes.Num() == 0) return BeatTimes;

	BeatTimes.Reserve(BeatMap.Notes.Num());

	FBeatConversionState ConversionState;
	ConversionState.CurrentTime = BeatMap.Settings.TimeOffset;
	ConversionState.PreviousBeat = 0.0f;
	ConversionState.CurrentBPM = BeatMap.Settings.StartBPM;
	ConversionState.CurrentPower = BeatMap.Settings.DefaultPower;

	for (const FBeatMapData& Data: BeatMap.Notes)
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

FTimeMapData UBeatMapFunctionLibrary::CreateTimeMapData(const FBeatMapData& BeatData, const FBeatConversionState& State)
{
	FTimeMapData NewTimeData;
	NewTimeData.SpawnerID = BeatData.SpawnerID;
	NewTimeData.ShotPower = BeatData.ShotPower == 0 ? State.CurrentPower : BeatData.ShotPower;
	const float Apex = CalculateApexHeight(NewTimeData.ShotPower);
	NewTimeData.Time = State.CurrentTime - CalculateTimeToApex(Apex);
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
