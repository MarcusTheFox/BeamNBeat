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
    TargetSystemComponent = CreateDefaultSubobject<UTargetSystemComponent>("TargetSystemComponent");
}

void ARangeMasterGameMode::BeginPlay()
{
    Super::BeginPlay();
    RhythmController = Cast<ARhythmController>(UGameplayStatics::GetActorOfClass(this, ARhythmController::StaticClass()));
    SpawnerManager = Cast<ASpawnerManager>(UGameplayStatics::GetActorOfClass(this, ASpawnerManager::StaticClass()));
    PlayerState = UGameUtilityFunctionLibrary::GetBeamNBeatPlayerState(this);
    const URangeMasterProjectSettings* ProjectSettings = URangeMasterProjectSettings::Get();

    if (!RhythmController || !SpawnerManager || !ProjectSettings) return;

    RhythmController->OnBeat.AddDynamic(TargetSystemComponent, &UTargetSystemComponent::SpawnTarget);
    RhythmController->OnMusicFinished.AddDynamic(this, &ARangeMasterGameMode::HandleMusicFinished);

    TargetSystemComponent->SetSpawners(SpawnerManager->GetSpawners());
    TargetSystemComponent->SetTargetClass(ProjectSettings->TargetClass);
    TargetSystemComponent->OnTargetEvent.AddDynamic(this, &ARangeMasterGameMode::OnTargetEvent);
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
    GetWorld()->GetTimerManager().ClearTimer(PrepareTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(EndGameTimerHandle);

    for (FTimerHandle& Handle : PreSpawnTimerHandles)
    {
        GetWorld()->GetTimerManager().ClearTimer(Handle);
    }
    PreSpawnTimerHandles.Empty();

    USoundWave* SoundWave;
    if (!UTrackFunctionLibrary::GetSoundWaveFromRawAudioData(CachedRawAudioData, SoundWave)) return;
    
    if (RhythmController)
    {
        RhythmController->Stop();
        RhythmController->ResetMusic(SoundWave);
    }
    TargetSystemComponent->DestroyAllTargets();
    OnGameReset.Broadcast();
}


void ARangeMasterGameMode::ForceStopGame_Implementation()
{
    bWasForceStopped = true;
    
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    
    for (FTimerHandle& Handle : PreSpawnTimerHandles)
    {
        GetWorld()->GetTimerManager().ClearTimer(Handle);
    }
    PreSpawnTimerHandles.Empty();
    
    if (RhythmController)
    {
        RhythmController->Stop();
    }
    TargetSystemComponent->DestroyAllTargets();
    OnGameStopped.Broadcast();
}

void ARangeMasterGameMode::EndGame()
{
    GetWorld()->GetTimerManager().ClearTimer(EndGameTimerHandle);

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

void ARangeMasterGameMode::HandleMusicFinished()
{
    bMusicHasFinished = true;

    if (TargetSystemComponent->IsAllTargetsDestroyed())
    {
        GetWorld()->GetTimerManager().SetTimer(
            EndGameTimerHandle, this, &ARangeMasterGameMode::EndGame, EndGameTime, false);
    }
}

void ARangeMasterGameMode::OnTargetEvent(ATarget* Target, const FTargetEventData& EventData)
{
    if (Target)
    {
        switch (EventData.TargetEventType) {
            case ETargetEventType::Hit:
                PlayerState->RegisterHit(EventData.NormalizedTime);
                break;
            case ETargetEventType::Lost:
                PlayerState->RegisterLost();
                break;
            case ETargetEventType::Destroyed:
                OnTargetDestroyed();
                break;
        }
    }
}

void ARangeMasterGameMode::OnTargetDestroyed()
{
    if (bMusicHasFinished && TargetSystemComponent->IsAllTargetsDestroyed())
    {
        GetWorld()->GetTimerManager().SetTimer(
            EndGameTimerHandle, this, &ARangeMasterGameMode::EndGame, EndGameTime, false);
    }
}

void ARangeMasterGameMode::StartPreparePhase()
{
    OnPreparePhaseStarted.Broadcast(); // UI: "Готовьтесь!"
    const float TimeUntilMusicStarts = PreparePhaseTime + CountdownTime;
    
    for (FTimerHandle& Handle : PreSpawnTimerHandles)
    {
        GetWorld()->GetTimerManager().ClearTimer(Handle);
    }
    PreSpawnTimerHandles.Empty();

    for (const FTimeMapData& PreSpawnData : CachedPreSpawnTargets)
    {
        const float SpawnDelay = TimeUntilMusicStarts + PreSpawnData.Time;

        if (SpawnDelay > 0.0f)
        {
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(TargetSystemComponent, FName("SpawnTarget"), PreSpawnData);

            FTimerHandle Handle;
            GetWorld()->GetTimerManager().SetTimer(Handle, TimerDelegate, SpawnDelay, false);
            PreSpawnTimerHandles.Add(Handle);
        }
        else
        {
            TargetSystemComponent->SpawnTarget(PreSpawnData);
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

    if (RhythmController)
    {
        RhythmController->Play();
    }
}