#include "Core/RangeMasterGameMode.h"

#include "Data/Enums/TrackRank.h"
#include "FunctionLibraries/BeatMapFunctionLibrary.h"
#include "FunctionLibraries/GameSaveFunctionLibrary.h"
#include "FunctionLibraries/RankFunctionLibrary.h"
#include "FunctionLibraries/TrackFunctionLibrary.h"
#include "TimerManager.h"
#include "FunctionLibraries/GameUtilityFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/BeamNBeatScoreSettings.h"
#include "Settings/RangeMasterProjectSettings.h"

ARangeMasterGameMode::ARangeMasterGameMode()
{
    PlayerStateClass = ABeamNBeatPlayerState::StaticClass();
}

void ARangeMasterGameMode::BeginPlay()
{
    Super::BeginPlay();
    RhythmController = Cast<ARhythmController>(UGameplayStatics::GetActorOfClass(this, ARhythmController::StaticClass()));
    SpawnerManager = Cast<ASpawnerManager>(UGameplayStatics::GetActorOfClass(this, ASpawnerManager::StaticClass()));
    PlayerState = UGameUtilityFunctionLibrary::GetBeamNBeatPlayerState(this);
    const URangeMasterProjectSettings* ProjectSettings = URangeMasterProjectSettings::Get();

    if (!RhythmController || !SpawnerManager || !ProjectSettings) return;

    RhythmController->OnBeat.AddDynamic(this, &ARangeMasterGameMode::OnBeatReceived);
    RhythmController->OnMusicFinished.AddDynamic(this, &ARangeMasterGameMode::HandleMusicFinished);
    
    CachedSpawners = SpawnerManager->GetSpawners();
    TargetClass = ProjectSettings->TargetClass;
}

void ARangeMasterGameMode::InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer)
{
    Super::InitStartSpot_Implementation(StartSpot, NewPlayer);
    InitialPlayerTransform = StartSpot->GetActorTransform();
}

void ARangeMasterGameMode::JoinTheGame(APlayerController* PlayerController)
{
    PlayerController->SetControlRotation(InitialPlayerTransform.Rotator());
    PlayerController->GetPawn()->SetActorLocation(InitialPlayerTransform.GetLocation());
    
    bPlayerInGame = true;
    OnPlayerJoined.Broadcast();
}

void ARangeMasterGameMode::LeaveTheGame()
{
    bPlayerInGame = false;
    OnPlayerLeaved.Broadcast();
}

void ARangeMasterGameMode::SetGameTrack(const FTrackInfo& TrackInfo)
{
    CurrentTrackData = TrackInfo;
    CachedRawAudioData.Empty();
    CachedPreSpawnTargets.Empty();

    FBeatMap BeatMap;
    USoundWave* SoundWave;
    
    if (!UTrackFunctionLibrary::GetBeatMapFromTrackInfo(TrackInfo, BeatMap)) return;
    if (!UTrackFunctionLibrary::GetRawAudioDataFromTrackInfo(TrackInfo, CachedRawAudioData)) return;
    if (!UTrackFunctionLibrary::GetSoundWaveFromRawAudioData(CachedRawAudioData, SoundWave)) return;

    const TArray<FTimeMapData> FullTimeMap = UBeatMapFunctionLibrary::ConvertBeatMapToBeatTimes(BeatMap);
    
    TArray<FTimeMapData> InGameTimeMap;

    for (const FTimeMapData& Data : FullTimeMap)
    {
        if (Data.Time < 0.0f)
        {
            CachedPreSpawnTargets.Add(Data);
        }
        else
        {
            InGameTimeMap.Add(Data);
        }
    }
    
    RhythmController->PrepareTrack(SoundWave, InGameTimeMap);
}

void ARangeMasterGameMode::StartGameRequest_Implementation()
{
    if (!RhythmController->IsReadyToPlay() || !bPlayerInGame) return;
    
    bWasForceStopped = false;
    bMusicHasFinished = false;

    PlayerState->JudgementSystem->ClearJudgements();
    PlayerState->ScoreSystem->ResetAllStats();
    
    StartPreparePhase();
}

void ARangeMasterGameMode::ResetGameRequest()
{
    bIsGameInProgress = false;
    
    GetWorld()->GetTimerManager().ClearTimer(PrepareTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(EndGameTimerHandle);

    USoundWave* SoundWave;
    if (!UTrackFunctionLibrary::GetSoundWaveFromRawAudioData(CachedRawAudioData, SoundWave)) return;
    
    if (RhythmController)
    {
        RhythmController->Stop();
        RhythmController->ResetMusic(SoundWave);
    }
    DestroyAllActiveTargets();
    OnGameReset.Broadcast();
}


void ARangeMasterGameMode::ForceStopGame_Implementation()
{
    bIsGameInProgress = false;
    bWasForceStopped = true;
    
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    
    if (RhythmController)
    {
        RhythmController->Stop();
    }
    DestroyAllActiveTargets();
    OnGameStopped.Broadcast();
}

void ARangeMasterGameMode::EndGame()
{
    GetWorld()->GetTimerManager().ClearTimer(EndGameTimerHandle);

    bIsGameInProgress = false;

    const int32 Score = PlayerState->ScoreSystem->GetScore();
    const int32 MaxCombo = PlayerState->ScoreSystem->GetMaxCombo();
    
    const FName TrackID = CurrentTrackData.ID;
    const int32 MaxScore = URankFunctionLibrary::CalculateMaxScore(CurrentTrackData.TotalTargets);
    const ETrackRank Rank = URankFunctionLibrary::CalculateTrackRank(Score, MaxScore);
    
    if (!bWasForceStopped)
    {
        UGameSaveFunctionLibrary::SaveTrackResult(TrackID, Score, Rank, PlayerState->JudgementSystem->GetJudgementEvents());
    }
    
    FGameResultData Result;
    Result.Score = Score;
    Result.Rank = Rank;
    Result.MaxCombo = MaxCombo;

    OnGameFinished.Broadcast(Result);
}

void ARangeMasterGameMode::OnBeatReceived(const FTimeMapData& TimeMapData)
{
    ASpawner* Spawner = CachedSpawners[TimeMapData.SpawnerID];
    ATarget* SpawnedTarget = Spawner->SpawnTarget(TargetClass, TimeMapData.ShotPower);

    if (SpawnedTarget)
    {
        SpawnedTarget->OnTargetDestroyed.AddDynamic(this, &ARangeMasterGameMode::OnTargetDestroyed);
        SpawnedTarget->OnTargetHit.AddDynamic(this, &ARangeMasterGameMode::OnTargetHit);
        ActiveTargets.Add(SpawnedTarget);
    }
}

void ARangeMasterGameMode::HandleMusicFinished()
{
    bMusicHasFinished = true;

    if (ActiveTargets.Num() == 0)
    {
        GetWorld()->GetTimerManager().SetTimer(
            EndGameTimerHandle, this, &ARangeMasterGameMode::EndGame, EndGameTime, false);
    }
}

void ARangeMasterGameMode::OnTargetHit(ATarget* Target)
{
    if (Target)
    {
        PlayerState->ScoreSystem->IncreaseCombo();

        const UBeamNBeatScoreSettings* Settings = UBeamNBeatScoreSettings::Get();
        const int32 Points = Settings->BasePoints;
        const int32 ComboMultiplier = PlayerState->ScoreSystem->GetComboMultiplier();

        PlayerState->ScoreSystem->AddScore(Points * ComboMultiplier);
        PlayerState->JudgementSystem->RegisterJudgement(EJudgement::Perfect);
    }
}

void ARangeMasterGameMode::OnTargetDestroyed(ATarget* Target)
{
    if (Target)
    {
        Target->OnTargetHit.RemoveDynamic(this, &ARangeMasterGameMode::OnTargetHit);
        Target->OnTargetDestroyed.RemoveDynamic(this, &ARangeMasterGameMode::OnTargetDestroyed);
        ActiveTargets.Remove(Target);
    }

    if (bMusicHasFinished && ActiveTargets.Num() == 0)
    {
        GetWorld()->GetTimerManager().SetTimer(
            EndGameTimerHandle, this, &ARangeMasterGameMode::EndGame, EndGameTime, false);
    }
}

void ARangeMasterGameMode::DestroyAllActiveTargets()
{
    for (ATarget* Target : ActiveTargets)
    {
        Target->DestroyTarget();
    }
}

void ARangeMasterGameMode::StartPreparePhase()
{
    OnPreparePhaseStarted.Broadcast(); // UI: "Готовьтесь!"
    const float TimeUntilMusicStarts = PreparePhaseTime + CountdownTime;

    for (const FTimeMapData& PreSpawnData : CachedPreSpawnTargets)
    {
        const float SpawnDelay = TimeUntilMusicStarts + PreSpawnData.Time;

        if (SpawnDelay > 0.0f)
        {
            FTimerHandle PreSpawnTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(PreSpawnTimerHandle, [this, PreSpawnData]()
            {
                OnBeatReceived(PreSpawnData);
            }, SpawnDelay, false);
        }
    }

    GetWorld()->GetTimerManager().SetTimer(PrepareTimerHandle, this, &ARangeMasterGameMode::StartCountdown, PreparePhaseTime, false);
}

void ARangeMasterGameMode::StartCountdown()
{
    GetWorld()->GetTimerManager().ClearTimer(PrepareTimerHandle);
    
    OnPreparePhaseFinished.Broadcast(); // UI: подготовка закончилась
    OnCountdownStarted.Broadcast(); // UI: показать отсчёт
    CurrentCountdown = CountdownTime;
    
    GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ARangeMasterGameMode::CountdownTick, CountdownTickInterval, true);
}

void ARangeMasterGameMode::CountdownTick()
{
    CurrentCountdown -= CountdownTickInterval;
    FCountdownInfo Info;
    Info.TimeLeft = FMath::Max(0.0f, CurrentCountdown);
    Info.TotalTime = CountdownTime;
    Info.Progress = (CountdownTime > 0.f) ? 1.0f - (Info.TimeLeft / CountdownTime) : 1.0f;
    OnCountdownTick.Broadcast(Info);
    if (CurrentCountdown <= 0.0f)
    {
        FinishCountdown();
    }
}

void ARangeMasterGameMode::FinishCountdown()
{
    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
    
    OnCountdownFinished.Broadcast(); // UI: "Старт!"
    OnGameStarted.Broadcast(); // Сигнал о начале игры

    bIsGameInProgress = true;

    if (RhythmController)
    {
        RhythmController->Play();
    }
}