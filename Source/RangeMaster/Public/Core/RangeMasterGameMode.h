#pragma once

#include "CoreMinimal.h"
#include "Actors/RhythmController.h"
#include "Components/ScoreSystemComponent.h"
#include "Data/Enums/Judgement.h"
#include "Data/Structs/CountdownInfo.h"
#include "Data/Structs/GameResultData.h"
#include "Data/Structs/TimeMapData.h"
#include "Data/Structs/TrackInfo.h"
#include "GameFramework/GameModeBase.h"
#include "RangeMasterGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJudgementRegistered, EJudgement, Judgement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameFinished, const FGameResultData&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, const FCountdownInfo&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreparePhaseStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreparePhaseFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerJoined);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLeaved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStopped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameReset);

UCLASS()
class ARangeMasterGameMode : public AGameModeBase
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer) override;
    
public:
    ARangeMasterGameMode();

    UFUNCTION(BlueprintCallable, Category="Game")
    void JoinTheGame(APlayerController* PlayerController);

    UFUNCTION(BlueprintCallable, Category="Game")
    void LeaveTheGame();

    UFUNCTION(BlueprintCallable, Category="Game")
    void SetGameTrack(const FTrackInfo& TrackInfo);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Game")
    void StartGameRequest();

    UFUNCTION(BlueprintCallable, Category="Game")
    void ResetGameRequest();
    
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Game")
    void ForceStopGame();

    UFUNCTION(BlueprintCallable, Category="Game")
    void RegisterJudgement(EJudgement Judgement);

    UFUNCTION(BlueprintCallable, Category="Game")
    int32 GetJudgementCount(EJudgement Judgement) const;

    UFUNCTION(BlueprintCallable, Category="Game")
    void ResetJudgementCounts();

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game")
    FOnJudgementRegistered OnJudgementRegistered;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game")
    FOnGameFinished OnGameFinished;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game")
    FOnGameStarted OnGameStarted;
    
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game")
    FOnGameStopped OnGameStopped;
    
    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game")
    FOnGameReset OnGameReset;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game")
    FOnPlayerJoined OnPlayerJoined;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game")
    FOnPlayerLeaved OnPlayerLeaved;

    UPROPERTY(BlueprintReadOnly, Category="Game")
    TMap<EJudgement, int32> JudgementCounts;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game")
    UScoreSystemComponent* ScoreSystem;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game")
    FTrackInfo CurrentTrackData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Game")
    bool bIsGameInProgress = false;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game|Countdown")
    FOnCountdownStarted OnCountdownStarted;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game|Countdown")
    FOnCountdownTick OnCountdownTick;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game|Countdown")
    FOnCountdownFinished OnCountdownFinished;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game|Countdown")
    FOnPreparePhaseStarted OnPreparePhaseStarted;

    UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Game|Countdown")
    FOnPreparePhaseFinished OnPreparePhaseFinished;

private:
    void EndGame();
    void DestroyAllActiveTargets();
    
    UFUNCTION()
    void OnBeatReceived(const FTimeMapData& TimeMapData);

    UFUNCTION()
    void HandleMusicFinished();

    UFUNCTION()
    void OnTargetDestroyed(ATarget* Target);

    void StartPreparePhase();
    void StartCountdown();
    void CountdownTick();
    void FinishCountdown();

    UPROPERTY()
    ARhythmController* RhythmController = nullptr;

    UPROPERTY()
    ASpawnerManager* SpawnerManager = nullptr;

    UPROPERTY()
    TArray<FTimeMapData> CachedPreSpawnTargets;

    UPROPERTY()
    TArray<TObjectPtr<ASpawner>> CachedSpawners;

    UPROPERTY()
    TArray<TObjectPtr<ATarget>> ActiveTargets;

    UPROPERTY()
    TSubclassOf<ATarget> TargetClass;

    FTransform InitialPlayerTransform;

    bool bPlayerInGame = false;
    bool bWasForceStopped = false;
    bool bMusicHasFinished = false;
    
    int32 TotalTargets = 0;
    int32 LastSpawnedTargetIndex = 0;
    
    TArray<uint8> CachedRawAudioData;
    TArray<EJudgement> TrackJudgements;
    
    FTimerHandle PrepareTimerHandle;
    FTimerHandle CountdownTimerHandle;
    FTimerHandle EndGameTimerHandle;
    
    float CountdownTime = 3.0f;
    float CurrentCountdown = 0.0f;
    float CountdownTickInterval = 0.01f;
    float PreparePhaseTime = 2.0f;
    float EndGameTime = 1.0f;
}; 