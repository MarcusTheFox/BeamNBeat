// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibraries/RankFunctionLibrary.h"

#include "Settings/BeamNBeatScoreSettings.h"

FText URankFunctionLibrary::GetRankText(ETrackRank Rank)
{
	switch (Rank)
	{
	case ETrackRank::None:
		return FText::FromString(TEXT("-"));
	case ETrackRank::E:
		return FText::FromString(TEXT("E"));
	case ETrackRank::D:
		return FText::FromString(TEXT("D"));
	case ETrackRank::C:
		return FText::FromString(TEXT("C"));
	case ETrackRank::B:
		return FText::FromString(TEXT("B"));
	case ETrackRank::A:
		return FText::FromString(TEXT("A"));
	case ETrackRank::S:
		return FText::FromString(TEXT("S"));
	case ETrackRank::SS:
		return FText::FromString(TEXT("SS"));
	default:
		return FText::FromString(TEXT("-"));
	}
}

FText URankFunctionLibrary::GetRankComment(ETrackRank Rank)
{
	switch (Rank)
	{
	case ETrackRank::None:
		return FText::FromString(TEXT("No Rank"));
	case ETrackRank::E:
		return FText::FromString(TEXT("Terrible"));
	case ETrackRank::D:
		return FText::FromString(TEXT("Bad"));
	case ETrackRank::C:
		return FText::FromString(TEXT("Okay"));
	case ETrackRank::B:
		return FText::FromString(TEXT("Good"));
	case ETrackRank::A:
		return FText::FromString(TEXT("Great"));
	case ETrackRank::S:
		return FText::FromString(TEXT("Amazing"));
	case ETrackRank::SS:
		return FText::FromString(TEXT("Perfect"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FLinearColor URankFunctionLibrary::GetRankColor(ETrackRank Rank)
{
	switch (Rank)
	{
	case ETrackRank::None:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Серый
	case ETrackRank::E:
		return FLinearColor(0.8f, 0.2f, 0.2f, 1.0f); // Красный
	case ETrackRank::D:
		return FLinearColor(0.9f, 0.4f, 0.1f, 1.0f); // Оранжевый
	case ETrackRank::C:
		return FLinearColor(0.9f, 0.9f, 0.2f, 1.0f); // Желтый
	case ETrackRank::B:
		return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f); // Зеленый
	case ETrackRank::A:
		return FLinearColor(0.2f, 0.6f, 0.9f, 1.0f); // Синий
	case ETrackRank::S:
		return FLinearColor(0.8f, 0.2f, 0.8f, 1.0f); // Фиолетовый
	case ETrackRank::SS:
		return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Золотой
	default:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Серый по умолчанию
	}
}

FText URankFunctionLibrary::GetJudgementText(EJudgement Judgement)
{
	switch (Judgement)
	{
	case EJudgement::Lost:
		return FText::FromString(TEXT("Lost"));
	case EJudgement::Miss:
		return FText::FromString(TEXT("Miss"));
	case EJudgement::Early:
		return FText::FromString(TEXT("Early"));
	case EJudgement::Late:
		return FText::FromString(TEXT("Late"));
	case EJudgement::Good:
		return FText::FromString(TEXT("Good"));
	case EJudgement::Great:
		return FText::FromString(TEXT("Great"));
	case EJudgement::Perfect:
		return FText::FromString(TEXT("Perfect"));
	default:
		return FText::FromString(TEXT("Unknown"));
	}
}

FLinearColor URankFunctionLibrary::GetJudgementColor(EJudgement Judgement)
{
	switch (Judgement)
	{
	case EJudgement::Lost:
		return FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);
	case EJudgement::Miss:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
	case EJudgement::Late:
	case EJudgement::Early:
		return FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
	case EJudgement::Good:
		return FLinearColor(0.8f, 0.8f, 0.2f, 1.0f);
	case EJudgement::Great:
		return FLinearColor(0.4f, 0.8f, 0.2f, 1.0f);
	case EJudgement::Perfect:
		return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
	default:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
	}
}

ETrackRank URankFunctionLibrary::CalculateTrackRank(const int32 Score, const int32 MaxScore)
{
	if (MaxScore <= 0) return ETrackRank::None;
	
	const float Accuracy = static_cast<float>(Score) / static_cast<float>(MaxScore);
	
	if (Accuracy >= 1.0f) return ETrackRank::SS;
	if (Accuracy >= 0.95f) return ETrackRank::S;
	if (Accuracy >= 0.90f) return ETrackRank::A;
	if (Accuracy >= 0.80f) return ETrackRank::B;
	if (Accuracy >= 0.70f) return ETrackRank::C;
	if (Accuracy >= 0.50f) return ETrackRank::D;
	return ETrackRank::E;
}

int32 URankFunctionLibrary::CalculateMaxScore(const int32 TargetCount)
{
	const UBeamNBeatScoreSettings* Settings = UBeamNBeatScoreSettings::Get();
	if (!Settings || Settings->ComboTiers.IsEmpty()) return 0;

	const TArray<FComboTierData>& ComboTiers = Settings->ComboTiers;
	const int32 BasePoints = Settings->BasePoints;
    
	int32 MaxScore = 0;
	int32 TargetsLeft = TargetCount;
	int32 PreviousThreshold = 1;
	int32 CurrentMultiplier = Settings->BaseMultiplier;
    
	for (const FComboTierData& Tier : ComboTiers)
	{
		if (TargetsLeft <= 0) break;

		const int32 TargetsInTier = FMath::Min(Tier.Threshold - PreviousThreshold, TargetsLeft);
		MaxScore += TargetsInTier * BasePoints * CurrentMultiplier;
        
		TargetsLeft -= TargetsInTier;
		PreviousThreshold = Tier.Threshold;
		CurrentMultiplier = Tier.Multiplier;
	}

	if (TargetsLeft > 0)
	{
		MaxScore += TargetsLeft * BasePoints * CurrentMultiplier;
	}

	return MaxScore;
}
