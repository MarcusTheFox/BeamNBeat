#include "Actors/Target.h"
#include "Core/RangeMasterGameMode.h"
#include "Components/ScoreSystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FunctionLibraries/GameUtilityFunctionLibrary.h"

ATarget::ATarget()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATarget::OnHit_Implementation()
{
    OnTargetHit.Broadcast(this);
    DestroyTarget();
}

void ATarget::DestroyTarget_Implementation()
{
    OnTargetDestroyed.Broadcast(this);
    Destroy();
}

void ATarget::SetImpulse(const FVector& Impulse)
{
    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        Prim->AddImpulse(Impulse, NAME_None, true);
    }
} 