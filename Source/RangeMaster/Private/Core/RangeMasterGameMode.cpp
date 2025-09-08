#include "Core/RangeMasterGameMode.h"

#include "Data/Enums/TrackRank.h"
#include "FunctionLibraries/BeatMapFunctionLibrary.h"
#include "FunctionLibraries/GameSaveFunctionLibrary.h"
#include "FunctionLibraries/RankFunctionLibrary.h"
#include "FunctionLibraries/TrackFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/RangeMasterProjectSettings.h"

ARangeMasterGameMode::ARangeMasterGameMode()
{
    ScoreSystem = CreateDefaultSubobject<UScoreSystemComponent>(TEXT("ScoreSystem"));
}

void ARangeMasterGameMode::BeginPlay()
{
    Super::BeginPlay();
    RhythmController = Cast<ARhythmController>(UGameplayStatics::GetActorOfClass(this, ARhythmController::StaticClass()));
    SpawnerManager = Cast<ASpawnerManager>(UGameplayStatics::GetActorOfClass(this, ASpawnerManager::StaticClass()));
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
    
    TArray<FBeatMapData> BeatMap;
    USoundWave* SoundWave;
    FBeatMapSettings BeatMapSettings;
    
    if (!UTrackFunctionLibrary::GetBeatMapFromTrackInfo(TrackInfo, BeatMap, BeatMapSettings)) return;
    if (!UTrackFunctionLibrary::GetRawAudioDataFromTrackInfo(TrackInfo, CachedRawAudioData)) return;
    if (!UTrackFunctionLibrary::GetSoundWaveFromRawAudioData(CachedRawAudioData, SoundWave)) return;

    const TArray<FTimeMapData> TimeMap = UBeatMapFunctionLibrary::ConvertBeatMapToBeatTimes(
        BeatMap, BeatMapSettings.TimeOffset);
    
    RhythmController->PrepareTrack(SoundWave, TimeMap);
}

void ARangeMasterGameMode::StartGameRequest_Implementation()
{
    if (!RhythmController->IsReadyToPlay() || !bPlayerInGame) return;
    
    bWasForceStopped = false;
    bMusicHasFinished = false;
    
    ResetJudgementCounts();
    TrackJudgements.Empty();
    if (ScoreSystem)
    {
        ScoreSystem->ResetAllStats();
    }
    
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
    
    GetWorld()->GetTimerManager().ClearTimer(PrepareTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(EndGameTimerHandle);
    
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

    const FName TrackID = CurrentTrackData.TrackID;
    const int32 Score = ScoreSystem ? ScoreSystem->GetScore() : 0;
    const int32 MaxScore = URankFunctionLibrary::CalculateMaxScore(CurrentTrackData.TotalTargets);
    const ETrackRank Rank = URankFunctionLibrary::CalculateTrackRank(Score, MaxScore);
    
    if (!bWasForceStopped)
    {
        UGameSaveFunctionLibrary::SaveTrackResult(TrackID, Score, Rank, TrackJudgements);
    }
    
    FGameResultData Result;
    Result.Score = Score;
    Result.Rank = Rank;
    Result.MaxCombo = ScoreSystem ? ScoreSystem->GetMaxCombo() : 0;
    Result.JudgementCounts = JudgementCounts;

    OnGameFinished.Broadcast(Result);
}

void ARangeMasterGameMode::OnBeatReceived(const FTimeMapData& TimeMapData)
{
    ASpawner* Spawner = CachedSpawners[TimeMapData.SpawnerID];
    ATarget* SpawnedTarget = Spawner->SpawnTarget(TargetClass, TimeMapData.ShotPower);

    if (SpawnedTarget)
    {
        SpawnedTarget->OnTargetDestroyed.AddDynamic(this, &ARangeMasterGameMode::OnTargetDestroyed);
        ActiveTargets.Add(SpawnedTarget);
    }
}

void ARangeMasterGameMode::RegisterJudgement(EJudgement Judgement)
{
    if (!bIsGameInProgress) return;

    TrackJudgements.Add(Judgement);
    
    int32& Count = JudgementCounts.FindOrAdd(Judgement);
    Count++;
    OnJudgementRegistered.Broadcast(Judgement);
}

int32 ARangeMasterGameMode::GetJudgementCount(EJudgement Judgement) const
{
    const int32* Count = JudgementCounts.Find(Judgement);
    return Count ? *Count : 0;
}

void ARangeMasterGameMode::ResetJudgementCounts()
{
    JudgementCounts.Empty();
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

void ARangeMasterGameMode::OnTargetDestroyed(ATarget* Target)
{
    if (Target)
    {
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