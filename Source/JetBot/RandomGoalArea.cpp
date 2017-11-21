// Copyright 2017, John Henry Miller, All Rights Reserved

#include "RandomGoalArea.h"
#include "Components/BoxComponent.h"


// Sets default values
ARandomGoalArea::ARandomGoalArea(const FObjectInitializer& ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GoalBoxComponent = CreateDefaultSubobject<UBoxComponent>(FName(TEXT("WallRunCapsule")));
	RootComponent = GoalBoxComponent;

	//We're using this for getting random goal positions, so collision is not necessary
	GoalBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

// Called when the game starts or when spawned
void ARandomGoalArea::BeginPlay()
{
	Super::BeginPlay();
	
}



// Called every frame
void ARandomGoalArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ARandomGoalArea::GetRandomGoalPosition() const
{
	for (uint8 i = 0; i < 100; i++)
	{
		const FHitResult GoalHit = GetRandomGoalHit();

		if (GoalHit.bBlockingHit)
		{
			return GoalHit.Location;
		}
	}

	return FVector::ZeroVector;
}

FHitResult ARandomGoalArea::GetRandomGoalHit() const
{
	FVector GoalLocation = FVector::ZeroVector;

	const FVector GoalBoxExtent = GoalBoxComponent->GetScaledBoxExtent();

	//Get X and Y coordinates
	GoalLocation.X = FMath::RandRange(-GoalBoxExtent.X, GoalBoxExtent.X);

	GoalLocation.Y = FMath::RandRange(-GoalBoxExtent.Y, GoalBoxExtent.Y);

	//Get Z coordinate by raytracing from the top of the box

	const FVector TraceStart = FVector(GoalLocation.X, GoalLocation.Y, GetActorLocation().Z + GoalBoxExtent.Z);
	const FVector TraceEnd = TraceStart + (FVector(0, 0, -1)*GoalBoxExtent.Z*2.0f);


	FHitResult GoalHit;

	GetWorld()->LineTraceSingleByChannel(GoalHit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldStatic);

	return GoalHit;
}

