// Copyright 2017, John Henry Miller, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RandomGoalArea.generated.h"

class UBoxComponent;

UCLASS()
class JETBOT_API ARandomGoalArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARandomGoalArea(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	UBoxComponent* GoalBoxComponent;

	FHitResult GetRandomGoalHit() const;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Goal")
	FVector GetRandomGoalPosition() const;



	
	
};
