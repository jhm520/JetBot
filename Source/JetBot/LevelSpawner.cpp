// Copyright 2017, John Henry Miller, All Rights Reserved

#include "LevelSpawner.h"


// Sets default values
ALevelSpawner::ALevelSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALevelSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALevelSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelSpawner::SpawnObstacles()
{
	//Calculate the location of the next Obstacle
	FVector CurrentObstacleSpawnLocation = BottomLeftRearCorner;

	while (CurrentObstacleSpawnLocation.X < TopRightFrontCorner.X)
	{
		//Calculate next X coordinate randomly
		const int XRand = (int)FMath::RandRange(ObstacleSpawnSpacing, ChanceToSpawnObstacle);

		const int XMod = XRand - (XRand % (int)ObstacleSpawnSpacing);

		CurrentObstacleSpawnLocation.X += XMod;

		while (CurrentObstacleSpawnLocation.Y < TopRightFrontCorner.Y)
		{
			//Calculate next Y coordinate randomly
			const int YRand = (int)FMath::RandRange(ObstacleSpawnSpacing, ChanceToSpawnObstacle);

			const int YMod = YRand - (YRand % (int)ObstacleSpawnSpacing);

			CurrentObstacleSpawnLocation.Y += YMod;

			while (CurrentObstacleSpawnLocation.Z < TopRightFrontCorner.Z)
			{
				const FTransform ObstacleSpawnTransform = FTransform(CurrentObstacleSpawnLocation + GetActorLocation());

				SpawnObstacleAtTransform(ObstacleSpawnTransform);

				const int ZRand = (int)FMath::RandRange(ObstacleSpawnSpacing, ChanceToSpawnObstacle);
				const int ZMod = ZRand - (ZRand % (int)ObstacleSpawnSpacing);

				CurrentObstacleSpawnLocation.Z += ZMod;
			}

			CurrentObstacleSpawnLocation.Z = BottomLeftRearCorner.Z;
		}

		CurrentObstacleSpawnLocation.Y = BottomLeftRearCorner.Y;
	}
	CurrentObstacleSpawnLocation.X = BottomLeftRearCorner.X;
}

