// Copyright 2017, John Henry Miller, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JetBotGameMode.generated.h"

/**
 * 
 */

USTRUCT()
struct FObstacleSpawnInfo
{
	GENERATED_BODY()

	UClass* ObstacleClass;
	FTransform Transform;
	AActor* Owner;

	FObstacleSpawnInfo(){}

	FObstacleSpawnInfo(FTransform InTransform, AActor* InOwner)
	{
		ObstacleClass = nullptr;
		Transform = InTransform;
		Owner = InOwner;
	}

	FObstacleSpawnInfo(UClass* InObstacleClass, FTransform InTransform, AActor* InOwner)
	{
		ObstacleClass = InObstacleClass;
		Transform = InTransform;
		Owner = InOwner;
	}
};

class AJetBotWorldTile;

UCLASS()
class JETBOT_API AJetBotGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	//Get a random obstacle class
	UFUNCTION(BlueprintImplementableEvent, Category = WorldTileObstacle)
	UClass* GetRandomObstacleClass();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Add another obstacle to be queued for spawn
	void QueueObstacleForSpawn(FTransform SpawnTransform, AActor* TileOwner);

	//Called when a tile finishes spawning obstacles
	void OnTileFinishedSpawningObstacles(AJetBotWorldTile* WorldTile);

	//Set the tile that is currently spawning obstacles
	UFUNCTION(BlueprintCallable, Category = WorldTile)
	void SetCurrentlySpawningTile(AJetBotWorldTile* InWorldTile);

	//Get the tile that is currently spawning obstacles
	UFUNCTION(BlueprintCallable, Category = WorldTile)
	AJetBotWorldTile* GetCurrentlySpawningTile();

	//Get the tile that is currently spawning obstacles
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintImplementableEvent, Category = WorldTile)
	float GetKillZ();

protected:

	//Array of the types of world tile classes we can spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WorldTile)
	TArray<TSubclassOf<AJetBotWorldTile>> WorldTileTypeArray;

	//Array of pointers to each world tile in the game right now
	UPROPERTY(Transient, BlueprintReadWrite, Category = WorldTile)
	TArray<AJetBotWorldTile*> WorldTileArray;

	//Spawn a random obstacle at a random location on the tile
	void SpawnObstacle();

	//Queue of obstacles that will be spawned
	UPROPERTY(Transient)
	TArray<FObstacleSpawnInfo> ObstacleSpawnQueue;

	//The world tile that obstacles are currently spawning on
	UPROPERTY(Transient)
	AJetBotWorldTile* CurrentlySpawningTile;
};
