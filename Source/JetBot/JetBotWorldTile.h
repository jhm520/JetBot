// Copyright 2017, John Henry Miller, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JetBotWorldTile.generated.h"

UCLASS()
class JETBOT_API AJetBotWorldTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJetBotWorldTile();

	void SetIsSpawningObstacles(bool bInIsSpawningObstacles);

	//get if we are currently spawning obstacles
	bool GetIsSpawningObstacles();

	//get if we finished spawning obstacles
	bool GetFinishedSpawningObstacles();

	//return if this tile can spawn obstacles
	bool CanSpawnObstacles();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Choose a random location on the tile, and queue an obstacle for spawn there
	UFUNCTION(BlueprintNativeEvent, Category = WorldTileObject)
	void QueueSpawnObstacleOnTile();

	//The minimum location on a World Tile that we can spawn an obstacle
	UPROPERTY(EditDefaultsOnly, Category = WorldTileObject)
	FVector MinObstacleSpawnLocation = FVector(-5000, -5000, 0);

	//The maximum location on a World Tile that we can spawn an obstacle
	UPROPERTY(EditDefaultsOnly, Category = WorldTileObject)
	FVector MaxObstacleSpawnLocation = FVector(5000, 5000, 10000);

	//The location that we will queue the next obstacle to spawn
	FVector CurrentObstacleSpawnLocation = MinObstacleSpawnLocation;

	//Whether we are spawning an obstacle each tick
	UPROPERTY(BlueprintReadWrite, Category = WorldTileObject)
	bool bIsSpawningObstacles = false;

	//Whether we have finished spawning obstacles
	UPROPERTY(BlueprintReadWrite, Category = WorldTileObject)
	bool bFinishedSpawningObstacles = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//World tiles connected to this tile
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WorldTile)
	TArray<AJetBotWorldTile*> ConnectedTiles;

	//The modifier to the random obstacle spacing. The higher this is, the fewer the obstacles that will spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WorldTileObject)
	float ChanceToSpawnObstacle = 5000.0f;

	//How close together obstacles will spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = WorldTileObject)
	float ObstacleSpawnSpacing = 100.0f;
	
};
