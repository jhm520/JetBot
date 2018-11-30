// Copyright 2017, John Henry Miller, All Rights Reserved

#include "JetBotWorldTile.h"
#include "JetBotGameMode.h"


// Sets default values
AJetBotWorldTile::AJetBotWorldTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AJetBotWorldTile::SetIsSpawningObstacles(bool bInIsSpawningObstacles)
{
	bIsSpawningObstacles = bInIsSpawningObstacles;
}

bool AJetBotWorldTile::GetIsSpawningObstacles()
{
	return bIsSpawningObstacles;
}

bool AJetBotWorldTile::GetFinishedSpawningObstacles()
{
	return bFinishedSpawningObstacles;
}

bool AJetBotWorldTile::CanSpawnObstacles()
{
	if (!GetIsSpawningObstacles() && !GetFinishedSpawningObstacles())
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Called when the game starts or when spawned
void AJetBotWorldTile::BeginPlay()
{
	Super::BeginPlay();

	/*AJetBotGameMode* JetBotGameMode = Cast<AJetBotGameMode>(GetWorld()->GetAuthGameMode());

	if (JetBotGameMode)
	{
		JetBotGameMode->QueueTileForSpawn(this);
	}*/
}

// Called every frame
void AJetBotWorldTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*if (bIsSpawningObstacles)
	{
		QueueSpawnObstacleOnTile();
	}*/
}

void AJetBotWorldTile::QueueSpawnObstacleOnTile_Implementation()
{

	if (!bIsSpawningObstacles)
	{
		return;
	}

	AJetBotGameMode* JetBotGameMode = Cast<AJetBotGameMode>(GetWorld()->GetAuthGameMode());

	if (!JetBotGameMode)
	{
		return;
	}
	
	//Calculate the location of the next Obstacle

	//Calculate next X coordinate randomly
	const int XRand = (int)FMath::RandRange(ObstacleSpawnSpacing, ChanceToSpawnObstacle);
	
	const int XMod = XRand - (XRand % (int)ObstacleSpawnSpacing);

	if (CurrentObstacleSpawnLocation.X == MinObstacleSpawnLocation.X)
	{
		CurrentObstacleSpawnLocation.X += XMod;
	}

	if (CurrentObstacleSpawnLocation.X < MaxObstacleSpawnLocation.X)
	{

		//Calculate next Y coordinate randomly
		const int YRand = (int)FMath::RandRange(ObstacleSpawnSpacing, ChanceToSpawnObstacle);

		const int YMod = YRand - (YRand % (int)ObstacleSpawnSpacing);

		if (CurrentObstacleSpawnLocation.Y == MinObstacleSpawnLocation.Y)
		{
			CurrentObstacleSpawnLocation.Y += YMod;
		}

		if (CurrentObstacleSpawnLocation.Y < MaxObstacleSpawnLocation.Y)
		{

			if (CurrentObstacleSpawnLocation.Z < MaxObstacleSpawnLocation.Z)
			{
				const FTransform ObstacleSpawnTransform = FTransform(CurrentObstacleSpawnLocation + GetActorLocation());

				//Queue Obstacle for spawn
				JetBotGameMode->QueueObstacleForSpawn(ObstacleSpawnTransform, this);

				//Calculate next Z coordinate (you guessed it) randomly!
				//Calculate the Z afterwards so we always have at least some of the obstacles neatly on the ground floor (Z == 0.0f)

				const int ZRand = (int)FMath::RandRange(ObstacleSpawnSpacing, ChanceToSpawnObstacle);
				const int ZMod = ZRand - (ZRand % (int)ObstacleSpawnSpacing);

				CurrentObstacleSpawnLocation.Z += ZMod;
			}
			else
			{
				CurrentObstacleSpawnLocation.Y += YMod;
				CurrentObstacleSpawnLocation.Z = MinObstacleSpawnLocation.Z;
			}
		}
		else
		{
			CurrentObstacleSpawnLocation.X += XMod;
			CurrentObstacleSpawnLocation.Y = MinObstacleSpawnLocation.Y;
		}
	}
	else
	{
		JetBotGameMode->OnTileFinishedSpawningObstacles(this);
		bIsSpawningObstacles = false;
		bFinishedSpawningObstacles = true;
		PrimaryActorTick.bCanEverTick = false;
	}

}

