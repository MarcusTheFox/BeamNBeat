// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/GameUtilityFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "Core/RangeMasterGameMode.h"

FText UGameUtilityFunctionLibrary::SecondsToTime(float Seconds)
{
	if (Seconds < 0.0f)
	{
		Seconds = 0.0f;
	}
	
	int32 TotalSeconds = FMath::FloorToInt(Seconds);
	int32 Minutes = TotalSeconds / 60;
	int32 RemainingSeconds = TotalSeconds % 60;
	
	FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, RemainingSeconds);
	
	return FText::FromString(TimeString);
}

ARangeMasterGameMode* UGameUtilityFunctionLibrary::GetRangeMasterGameMode(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return nullptr;
    const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    return Cast<ARangeMasterGameMode>(UGameplayStatics::GetGameMode(World));
}

ABeamNBeatPlayerState* UGameUtilityFunctionLibrary::GetBeamNBeatPlayerState(const UObject* WorldContextObject,
	const int32 PlayerStateIndex)
{
    if (!WorldContextObject) return nullptr;
    const UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	return Cast<ABeamNBeatPlayerState>(UGameplayStatics::GetPlayerState(World, PlayerStateIndex));
}

FString UGameUtilityFunctionLibrary::GetGameVersion()
{
	FString GameVersion;

	GConfig->GetString(
        TEXT("/Script/EngineSettings.GeneralProjectSettings"),
        TEXT("ProjectVersion"),
        GameVersion,
        GGameIni
    );

	if (GameVersion.IsEmpty())
	{
		return TEXT("0.0.0");
	}

	return GameVersion;
}
