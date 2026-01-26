#include "Components/TargetSystemComponent.h"

UTargetSystemComponent::UTargetSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTargetSystemComponent::SpawnTarget(const FTimeMapData& TimeMapData)
{
	ASpawner* Spawner = Spawners[TimeMapData.SpawnerID];

	if (ATarget* Target = Spawner->SpawnTarget(TargetClass, TimeMapData.ShotPower))
	{
		Target->OnTargetEvent.AddDynamic(this, &UTargetSystemComponent::OnTargetEventHandler);
		ActiveTargets.Add(Target);
	}
}

void UTargetSystemComponent::OnTargetEventHandler(ATarget* Target, const FTargetEventData& EventData)
{
	if (EventData.TargetEventType == ETargetEventType::Destroyed)
	{
		OnTargetDestroyed(Target);
	}
	
	OnTargetEvent.Broadcast(Target, EventData);
}

void UTargetSystemComponent::OnTargetDestroyed(ATarget* Target)
{
	if (Target)
	{
		Target->OnTargetEvent.RemoveDynamic(this, &UTargetSystemComponent::OnTargetEventHandler);
		ActiveTargets.Remove(Target);
	}
}

void UTargetSystemComponent::DestroyAllTargets()
{
	for (ATarget* Target : ActiveTargets)
	{
		Target->DestroyTarget();
	}
}
