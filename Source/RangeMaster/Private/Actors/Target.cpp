#include "Actors/Target.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"

ATarget::ATarget()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATarget::OnHit_Implementation()
{
    const float HitTime = GetWorld()->GetTimerManager().GetTimerElapsed(MissedStateTimerHandle);
    const float PerfectTime = Lifetime / 2;
    const float NormalizedTime = ( HitTime - PerfectTime ) / PerfectTime;
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    OnTargetEvent.Broadcast(this, FTargetEventData::CreateHit(NormalizedTime));
    DestroyTarget();
}

void ATarget::Initialize(const float LifetimeValue)
{
    Lifetime = LifetimeValue;
}

void ATarget::SetTargetState_Implementation()
{
    GetWorld()->GetTimerManager().SetTimer(MissedStateTimerHandle, this, &ATarget::SetMissedState, Lifetime);
}

void ATarget::SetMissedState_Implementation()
{
    Lost();
}

void ATarget::DestroyTarget_Implementation()
{
    OnTargetEvent.Broadcast(this, FTargetEventData(ETargetEventType::Destroyed));
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    Destroy();
}

void ATarget::SetImpulse(const FVector& Impulse)
{
    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(GetRootComponent()))
    {
        Prim->AddImpulse(Impulse, NAME_None, true);
    }

    const float ApexTime = FMath::Abs(Impulse.Z / UPhysicsSettings::Get()->DefaultGravityZ);
    GetWorld()->GetTimerManager().SetTimer(TargetStateTimerHandle, this, &ATarget::SetTargetState, ApexTime - Lifetime / 2);
}

void ATarget::Lost()
{
    OnTargetEvent.Broadcast(this, FTargetEventData::CreateLost());
    DestroyTarget();
}