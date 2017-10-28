// Copyright 2017, John Henry Miller, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JetBotObstacle.generated.h"

class UBoxComponent;
UCLASS()
class JETBOT_API AJetBotObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJetBotObstacle(const FObjectInitializer& ObjectInitializer);

	//Get whether or not this obstacle is overlapping another obstacle, for use when spawning obstacles
	bool IsOverlappingOtherObstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Get this obstacle's "boundary box"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	UBoxComponent* BoundaryBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	FRotator SpawnIncrementsAngleRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	FRotator SpawnIncrementsNumberRotator;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UBoxComponent* GetBoundaryBox();

	UFUNCTION(BlueprintCallable, Category = "Spawning")
	FRotator GetRandomSpawnRotation();
	
};
