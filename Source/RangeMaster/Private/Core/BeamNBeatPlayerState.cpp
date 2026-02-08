// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/BeamNBeatPlayerState.h"
#include "Settings/BeamNBeatScoreSettings.h"

ABeamNBeatPlayerState::ABeamNBeatPlayerState()
{
	ScoreSystem = CreateDefaultSubobject<UScoreSystemComponent>(TEXT("ScoreSystem"));
	JudgementSystem = CreateDefaultSubobject<UJudgementSystemComponent>(TEXT("JudgementSystem"));
}

void ABeamNBeatPlayerState::RegisterHit(const float HitTime) const
{
	float JudgementMultiplier = 1;
	
	if (FMath::Abs(HitTime) < 0.25f)
	{
		JudgementSystem->RegisterJudgement(EJudgement::Perfect);
	}
	else if (FMath::Abs(HitTime) < 0.5f)
	{
		JudgementSystem->RegisterJudgement(EJudgement::Great);
		JudgementMultiplier = 0.9f;
	}
	else if (FMath::Abs(HitTime) < 0.75f)
	{
		JudgementSystem->RegisterJudgement(EJudgement::Good);
		JudgementMultiplier = 0.8f;
	}
	else if (HitTime <= -0.75f)
	{
		JudgementSystem->RegisterJudgement(EJudgement::Early);
		JudgementMultiplier = 0.7f;
	}
	else if (HitTime >= 0.75f)
	{
		JudgementSystem->RegisterJudgement(EJudgement::Late);
		JudgementMultiplier = 0.7f;
	}
	
	ScoreSystem->IncreaseCombo();

	const UBeamNBeatScoreSettings* Settings = UBeamNBeatScoreSettings::Get();
	const int32 Points = Settings->BasePoints * JudgementMultiplier;
	const int32 ComboMultiplier = ScoreSystem->GetComboMultiplier();

	ScoreSystem->AddScore(Points * ComboMultiplier);
}

void ABeamNBeatPlayerState::RegisterMiss() const
{
	JudgementSystem->RegisterJudgement(EJudgement::Miss);
	ScoreSystem->ResetCombo();
}

void ABeamNBeatPlayerState::RegisterLost() const
{
	JudgementSystem->RegisterJudgement(EJudgement::Lost);
	ScoreSystem->ResetCombo();
	ScoreSystem->SubtractScore(10);
}

void ABeamNBeatPlayerState::Reset()
{
	Super::Reset();

	JudgementSystem->ClearJudgements();
	ScoreSystem->ResetAllStats();
}
