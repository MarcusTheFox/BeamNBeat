#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Data/Structs/BeatMapData.h"
#include "Actors/SpawnerManager.h"
#include "Actors/Target.h"
#include "Data/Structs/TimeMapData.h"
#include "Data/Structs/TrackInfo.h"
#include "RhythmController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeat, const FTimeMapData&, TimeMapData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReadyToPlay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMusicFinished);

UCLASS()
class RANGEMASTER_API ARhythmController : public AActor
{
    GENERATED_BODY()
public:
    ARhythmController();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rhythm")
    UAudioComponent* MusicComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Rhythm")
    FOnBeat OnBeat;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Rhythm")
    FOnReadyToPlay OnReadyToPlay;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Rhythm")
    FOnMusicFinished OnMusicFinished;

    UFUNCTION(BlueprintCallable, Category="Rhythm")
    void PrepareTrack(USoundWave* SoundWave, const TArray<FTimeMapData>& TimeMap);

    UFUNCTION(BlueprintCallable, Category="Rhythm")
    void Play();

    UFUNCTION(BlueprintCallable, Category="Rhythm")
    void Stop();

    UFUNCTION(BlueprintCallable, Category="Rhythm")
    void ResetMusic(USoundWave* SoundWave);

    UFUNCTION(BlueprintPure, Category="Rhythm")
    bool IsPlaying() const { return bIsPlaying; }

    UFUNCTION(BlueprintCallable, Category="Rhythm")
    bool IsReadyToPlay() const { return bIsReadyToPlay; }

private:
    UFUNCTION()
    void HandleMusicPlaybackPercent(const USoundWave* PlayingSoundWave, float PlaybackPercent);

    UFUNCTION()
    void HandleMusicPlaybackFinished();
    
    UFUNCTION()
    void CheckBeatAtTime(float CurrentTime);

    UPROPERTY()
    TObjectPtr<USoundWave> CurrentSoundWave = nullptr;
    
    TArray<FTimeMapData> CachedTimeMap;
    int32 NextBeatIndex = 0;
    bool bIsReadyToPlay = false;
    bool bIsPlaying = false;
}; 