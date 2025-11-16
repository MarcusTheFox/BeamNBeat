// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/TrackFunctionLibrary.h"

#include "JsonObjectConverter.h"
#include "Algo/Count.h"
#include "Settings/RangeMasterProjectSettings.h"
#include "Sound/SoundWaveProcedural.h"

TArray<FTrackInfo> UTrackFunctionLibrary::LoadAllTracksMetadata(const FString& Directory)
{
	TArray<FTrackInfo> FoundTracks;
	TArray<FString> InfoFiles;

	FPlatformFileManager::Get().GetPlatformFile().FindFilesRecursively(InfoFiles, *Directory, TEXT("info.json"));
	
	UE_LOG(LogTemp, Log, TEXT("Found %d potential track info files in directory: %s"), InfoFiles.Num(), *Directory);

	for (const FString& FilePath : InfoFiles)
	{
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to read track info file: %s"), *FilePath);
			continue;
		}
		
		FTrackInfo TrackInfo;
		if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &TrackInfo, 0, 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to parse JSON from file: %s"), *FilePath);
			continue;
		}
		
		const FString TrackFolder = FPaths::GetPath(FilePath);
		const FString AudioPath = FPaths::Combine(TrackFolder, TrackInfo.AudioInfo.FileName);
		const FString BeatMapPath = FPaths::Combine(TrackFolder, TrackInfo.LevelInfo.BeatmapFileName);

		FBeatMap BeatMap;
		if (!GetBeatMapFromTrackInfo(TrackInfo, BeatMap))
		{
			UE_LOG(LogTemp, Warning,
				   TEXT("Skipping track '%s' because its beatmap file could not be read"
				   ), *TrackInfo.LevelInfo.BeatmapFileName);
			continue;
		}
				
		TrackInfo.Duration = GetWavDurationSeconds(AudioPath);
		TrackInfo.Bpm = BeatMap.Settings.Bpm;
		
		const int32 DefaultPower = BeatMap.Settings.Properties.Power;
		TrackInfo.TotalTargets = Algo::CountIf(BeatMap.Notes, [&DefaultPower](const FBeatMapNote& Note)
		{
			const int32 EffectivePower = Note.Properties.Power > 0 ? Note.Properties.Power : DefaultPower;
			return EffectivePower > 0;
		});
				
		if (TrackInfo.Duration > 0 || TrackInfo.Bpm > 0)
		{
			FoundTracks.Add(TrackInfo);
			UE_LOG(LogTemp, Log, TEXT("Successfully loaded metadata for track: %s"),
			       *TrackInfo.AudioInfo.Title.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("Skipping track '%s' because its audio file could not be read or has zero duration: %s"
			       ), *TrackInfo.AudioInfo.Title.ToString(), *TrackInfo.AudioInfo.FileName);
		}
	}
	return FoundTracks;
}

bool UTrackFunctionLibrary::LoadBeatMapForTrack(const FTrackInfo& Track, const FString& TracksDirectory,
	FBeatMap& OutBeatMap)
{
	if (Track.LevelInfo.BeatmapFileName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot load beatmap. File path is invalid or file does not exist: %s"),
		       *Track.LevelInfo.BeatmapFileName);
		return false;
	}

	FString BeatMapPath = FPaths::Combine(TracksDirectory, Track.ID.ToString(), Track.LevelInfo.BeatmapFileName);

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *BeatMapPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read beatmap file to string: %s"), *Track.LevelInfo.BeatmapFileName);
		return false;
	}
	return FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutBeatMap, 0, 0);
}

USoundWaveProcedural* UTrackFunctionLibrary::CreateProceduralSoundWave(const FString& FilePath)
{
    TArray<uint8> RawFileData;
	
	if (!GetRawAudioData(FilePath, RawFileData)) return nullptr;

    return CreateProceduralSoundWaveFromData(RawFileData);
}

USoundWaveProcedural* UTrackFunctionLibrary::CreateProceduralSoundWaveFromData(const TArray<uint8>& RawAudioData)
{
	FWaveModInfo WaveInfo;
	if (!WaveInfo.ReadWaveInfo(RawAudioData.GetData(), RawAudioData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse WAV header info from cached data."));
		return nullptr;
	}
	
	const URangeMasterProjectSettings* ProjectSettings = GetDefault<URangeMasterProjectSettings>();
	if (!ProjectSettings)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateProceduralSoundWave: Failed to get Range Master Project Settings!"));
	}
	
	USoundClass* MusicSoundClass = nullptr;
	if (ProjectSettings)
	{
		MusicSoundClass = ProjectSettings->MusicSoundClass.LoadSynchronous();
	}
	
	if (!MusicSoundClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateProceduralSoundWave: Music Sound Class is not set in Project Settings! Procedural music will use master volume."));
	}

	USoundWaveProcedural* ProceduralSoundWave = NewObject<USoundWaveProcedural>();
	if (!ProceduralSoundWave)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create a new USoundWaveProcedural object."));
		return nullptr;
	}
	
	ProceduralSoundWave->SetSampleRate(*WaveInfo.pSamplesPerSec);
	ProceduralSoundWave->NumChannels = *WaveInfo.pChannels;
	ProceduralSoundWave->Duration = CalculateDurationFromWavInfo(WaveInfo);
	ProceduralSoundWave->SoundGroup = SOUNDGROUP_Default;
	ProceduralSoundWave->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
	ProceduralSoundWave->bLooping = false;
	
	if (MusicSoundClass)
	{
		ProceduralSoundWave->SoundClassObject = MusicSoundClass;
	}
	
	ProceduralSoundWave->QueueAudio(WaveInfo.SampleDataStart, WaveInfo.SampleDataSize);
	
	return ProceduralSoundWave;
}

float UTrackFunctionLibrary::GetWavDurationSeconds(const FString& FilePath)
{
	TArray<uint8> RawFileData;
	FWaveModInfo WaveInfo;
	
	if (!LoadAndParseWavInfo(FilePath, RawFileData, WaveInfo)) return 0.0f;

	return CalculateDurationFromWavInfo(WaveInfo);
}

bool UTrackFunctionLibrary::GetBeatMapFromTrackInfo(const FTrackInfo& TrackInfo, FBeatMap& OutBeatMap)
{
	if (!LoadBeatMapForTrack(TrackInfo, GetTracksDirectory(), OutBeatMap))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load beatmap for track: %s"), *TrackInfo.ID.ToString());
		return false;
	}
	return true;
}

bool UTrackFunctionLibrary::GetSoundWaveFromTrackInfo(FTrackInfo TrackInfo, USoundWave*& OutSoundWave)
{
	OutSoundWave = CreateProceduralSoundWave(GetAudioPathFromTrackInfo(TrackInfo));
	if (!OutSoundWave)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load audio for track: %s"), *TrackInfo.ID.ToString());
		return false;
	}
	return true;
}

bool UTrackFunctionLibrary::GetSoundWaveFromRawAudioData(const TArray<uint8>& RawAudioData, USoundWave*& OutSoundWave)
{
	OutSoundWave = CreateProceduralSoundWaveFromData(RawAudioData);
	if (!OutSoundWave)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load audio from raw data"));
		return false;
	}
	return true;
}

bool UTrackFunctionLibrary::GetRawAudioData(const FString AudioPath, TArray<uint8>& RawAudioData)
{
	if (!FPaths::FileExists(AudioPath) || !FFileHelper::LoadFileToArray(RawAudioData, *AudioPath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load WAV file into array: %s"), *AudioPath);
		return false;
	}
	return true;
}

bool UTrackFunctionLibrary::GetRawAudioDataFromTrackInfo(const FTrackInfo& TrackInfo, TArray<uint8>& RawAudioData)
{
	return GetRawAudioData(GetAudioPathFromTrackInfo(TrackInfo), RawAudioData);
}

FString UTrackFunctionLibrary::GetTracksDirectory()
{
	return FPaths::Combine(FPaths::ProjectDir(), "UserTracks");
}

FString UTrackFunctionLibrary::GetAudioPathFromTrackInfo(const FTrackInfo& TrackInfo)
{
	return FPaths::Combine(GetTracksDirectory(), TrackInfo.ID.ToString(), TrackInfo.AudioInfo.FileName);
}

bool UTrackFunctionLibrary::LoadAndParseWavInfo(const FString& FilePath, TArray<uint8>& OutRawData,
                                                FWaveModInfo& OutWaveInfo)
{
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("WAV file does not exist at path: %s"), *FilePath);
		return false;
	}
	if (!FFileHelper::LoadFileToArray(OutRawData, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load WAV file into array: %s"), *FilePath);
		return false;
	}
	if (!OutWaveInfo.ReadWaveInfo(OutRawData.GetData(), OutRawData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse WAV header info from file: %s"), *FilePath);
		return false;
	}

	return true;
}

float UTrackFunctionLibrary::CalculateDurationFromWavInfo(const FWaveModInfo& WaveInfo)
{
	const int32 ChannelCount = *WaveInfo.pChannels;
	const int32 SizeOfSample = *WaveInfo.pBitsPerSample / 8;
	const int32 SampleRate = *WaveInfo.pSamplesPerSec;

	if (ChannelCount == 0 || SizeOfSample == 0 || SampleRate == 0)
	{
		UE_LOG(LogTemp, Warning,
		       TEXT(
			       "Cannot calculate duration, wave info contains zero values (Channels = %d, SampleRate = %d, BitDepth = %d)."
		       ), ChannelCount, SizeOfSample, SampleRate);
		return 0.0f;
	}
	
	const int32 NumFrames = (WaveInfo.SampleDataSize / SizeOfSample) / ChannelCount;
	return static_cast<float>(NumFrames) / static_cast<float>(SampleRate);
}
