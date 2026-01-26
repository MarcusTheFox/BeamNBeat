#include "Actors/Target.h"
#include "Components/PrimitiveComponent.h"

ATarget::ATarget()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATarget::OnHit_Implementation()
{
    OnTargetEvent.Broadcast(this, FTargetEventData::CreateHit());
    DestroyTarget();
}

void ATarget::DestroyTarget_Implementation()
{
    OnTargetEvent.Broadcast(this, FTargetEventData(ETargetEventType::Destroyed));
    Destroy();
}

void ATarget::SetImpulse(const FVector& Impulse)
{
    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        Prim->AddImpulse(Impulse, NAME_None, true);
    }
}

void ATarget::Lost()
{
    OnTargetEvent.Broadcast(this, FTargetEventData::CreateLost());
    DestroyTarget();
}