// Copyright 2017, John Henry Miller, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelSpawner.generated.h"

UCLASS()
class JETBOT_API ALevelSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	void SpawnObstacles();

	UFUNCTION(BlueprintImplementableEvent, Category = "Spawning")
	void SpawnObstacleAtTransform(const FTransform SpawnTransform);

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float ObstacleSpawnSpacing = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float ChanceToSpawnObstacle = 5000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	FVector BottomLeftRearCorner = FVector(-5000,-5000,0);

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	FVector TopRightFrontCorner = FVector(5000,5000,5000);
	
};
