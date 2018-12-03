// Copyright 2017, John Henry Miller, All Rights Reserved

#include "JetBotGameMode.h"
#include "JetBotWorldTile.h"
#include "JetBotObstacle.h"
#include "Components/BoxComponent.h"


void AJetBotGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//Spawn an obstacle each tick, until we're done spawning obstacles

	//Spawning just one obstacle per tick helps avoid hitches

	/*SpawnObstacle();*/
}

void AJetBotGameMode::QueueObstacleForSpawn(FTransform SpawnTransform, AActor* TileOwner)
{
	ObstacleSpawnQueue.Push(FObstacleSpawnInfo(SpawnTransform, TileOwner));
}

void AJetBotGameMode::OnTileFinishedSpawningObstacles(AJetBotWorldTile* WorldTile)
{
	if (WorldTile)
	{
		for (AJetBotWorldTile* JetBotWorldTile : WorldTileArray)
		{
			if (JetBotWorldTile && JetBotWorldTile->CanSpawnObstacles())
			{
				SetCurrentlySpawningTile(JetBotWorldTile);
				return;
			}
		}
	}

	SetCurrentlySpawningTile(nullptr);
}

void AJetBotGameMode::SetCurrentlySpawningTile(AJetBotWorldTile* InWorldTile)
{
	if (InWorldTile)
	{
		CurrentlySpawningTile = InWorldTile;
		InWorldTile->SetIsSpawningObstacles(true);
	}
	else
	{
		CurrentlySpawningTile = nullptr;
	}
}

AJetBotWorldTile* AJetBotGameMode::GetCurrentlySpawningTile()
{
	return CurrentlySpawningTile;
}

void AJetBotGameMode::SpawnObstacle()
{
	if (!ObstacleSpawnQueue.Num())
	{
		return;
	}

	

	const FObstacleSpawnInfo ObstacleSpawnInfo = ObstacleSpawnQueue.Pop();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ObstacleSpawnInfo.Owner;

	const FVector SpawnLocation = ObstacleSpawnInfo.Transform.GetLocation();

	UClass* ObstacleClass = ObstacleSpawnInfo.ObstacleClass;

	//if we didn't assign an obstacle class, choose one
	if (!ObstacleClass)
	{
		ObstacleClass = GetRandomObstacleClass();
	}

	AJetBotObstacle* DefaultObstacle = Cast<AJetBotObstacle>(ObstacleClass->GetDefaultObject());

	if (!DefaultObstacle)
	{
		return;
	}

	const FRotator SpawnRotation = ObstacleSpawnInfo.Owner->GetActorRotation() + DefaultObstacle->GetRandomSpawnRotation();

	//Check the area we are spawning the object in for collisions before spawning

	FCollisionObjectQueryParams ObjectQueryParams;

	//ECC_GameTraceChannel3 == JetBotObstacle
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel3);

	FCollisionQueryParams CollisionQueryParams;

	CollisionQueryParams.AddIgnoredActor(ObstacleSpawnInfo.Owner);
	CollisionQueryParams.AddIgnoredActor(DefaultObstacle);

	TArray<AActor*> AttachedActors;
	DefaultObstacle->GetAttachedActors(AttachedActors);
	CollisionQueryParams.AddIgnoredActors(AttachedActors);

	TArray<FHitResult> HitResults;
	FHitResult HitResult;

	FCollisionShape CollisionShape;

	if (GetWorld()->SweepSingleByObjectType(HitResult, SpawnLocation, SpawnLocation, SpawnRotation.Quaternion(), ObjectQueryParams, DefaultObstacle->GetBoundaryBox()->GetCollisionShape(), CollisionQueryParams))
	{
		return;
	}

	AActor* ObstacleActor = GetWorld()->SpawnActor(ObstacleClass, &SpawnLocation, &SpawnRotation, SpawnParams);
}
