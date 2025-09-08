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
	ConversionState.Defaults.Bpm = BeatMap.Settings.Bpm;
	ConversionState.Defaults.Power = BeatMap.Settings.Power;

	for (const FBeatMapNote& Data: BeatMap.Notes)
	{
		ProcessBeatData(Data, ConversionState, BeatTimes);
	}

	return BeatTimes;
}

void UBeatMapFunctionLibrary::ProcessBeatData(const FBeatMapNote& Note, FBeatConversionState& State,
                                              TArray<FTimeMapData>& OutBeatTimes)
{
	const float DeltaBeat = CalculateDeltaBeat(Note.Beat, State.PreviousBeat);

	UpdateCurrentTime(DeltaBeat, State);
	UpdateDefaultsIfChanged(Note, State);

	OutBeatTimes.Add(CreateTimeMapData(Note, State));

	State.PreviousBeat = Note.Beat;
}

float UBeatMapFunctionLibrary::CalculateDeltaBeat(const float CurrentBeat, const float PreviousBeat)
{
	return CurrentBeat - PreviousBeat;
}

void UBeatMapFunctionLibrary::UpdateCurrentTime(const float DeltaBeat, FBeatConversionState& State)
{
	if (State.Defaults.Bpm > 0.0f)
	{
		const float SecondsPerBeat = 60.0f / State.Defaults.Bpm;
		State.Time += DeltaBeat * SecondsPerBeat;
	}
}

void UBeatMapFunctionLibrary::UpdateDefaultsIfChanged(const FBeatMapNote& Note, FBeatConversionState& State)
{
	if (Note.Defaults.Bpm > 0.0f)
	{
		State.Defaults.Bpm = Note.Defaults.Bpm;
	}
	if (Note.Defaults.Power > 0.0f)
	{
		State.Defaults.Power = Note.Defaults.Power;
	}
}

FTimeMapData UBeatMapFunctionLibrary::CreateTimeMapData(const FBeatMapNote& Note, const FBeatConversionState& State)
{
	FTimeMapData NewTimeData;
	NewTimeData.SpawnerID = Note.Id;
	NewTimeData.ShotPower = Note.Power == 0 ? State.Defaults.Power : Note.Power;
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
