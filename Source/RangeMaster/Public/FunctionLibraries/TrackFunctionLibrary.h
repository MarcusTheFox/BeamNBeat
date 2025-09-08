// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Structs/BeatMap.h"
#include "Data/Structs/BeatMapNote.h"
#include "Data/Structs/BeatMapSettings.h"
#include "Data/Structs/TrackInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundWaveProcedural.h"
#include "TrackFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class RANGEMASTER_API UTrackFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static TArray<FTrackInfo> LoadAllTracksMetadata(const FString& Directory);
	
	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static bool LoadBeatMapForTrack(const FTrackInfo& Track, const FString& TracksDirectory,
		FBeatMap& OutBeatMap);
	
	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static USoundWaveProcedural* CreateProceduralSoundWave(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static USoundWaveProcedural* CreateProceduralSoundWaveFromData(const TArray<uint8>& RawAudioData);
	
	UFUNCTION(BlueprintCallable, Category = "Track Management|Metadata")
	static float GetWavDurationSeconds(const FString& FilePath);
	
	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static bool GetBeatMapFromTrackInfo(const FTrackInfo& TrackInfo, FBeatMap& OutBeatMap);

	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static bool GetSoundWaveFromTrackInfo(FTrackInfo TrackInfo, USoundWave*& OutSoundWave);

	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static bool GetSoundWaveFromRawAudioData(const TArray<uint8>& RawAudioData, USoundWave*& OutSoundWave);

	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static bool GetRawAudioData(const FString AudioPath, TArray<uint8>& RawAudioData);

	UFUNCTION(BlueprintCallable, Category = "Track Management")
	static bool GetRawAudioDataFromTrackInfo(const FTrackInfo& TrackInfo, TArray<uint8>& RawAudioData);

private:
	static FString GetTracksDirectory();
	static FString GetAudioPathFromTrackInfo(const FTrackInfo& TrackInfo);
	static bool LoadAndParseWavInfo(const FString& FilePath, TArray<uint8>& OutRawData, FWaveModInfo& OutWaveInfo);
	static float CalculateDurationFromWavInfo(const FWaveModInfo& WaveInfo);
};