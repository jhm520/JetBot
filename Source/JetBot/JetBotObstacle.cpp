// Copyright 2017, John Henry Miller, All Rights Reserved

#include "JetBotObstacle.h"
#include "Components/BoxComponent.h"
#include "WorldCollision.h"

// Sets default values
AJetBotObstacle::AJetBotObstacle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoundaryBox = CreateDefaultSubobject<UBoxComponent>(FName(TEXT("BoundaryBox")));

}

bool AJetBotObstacle::IsOverlappingOtherObstacle()
{
	FCollisionObjectQueryParams ObjectQueryParams;

	//ECC_GameTraceChannel3 == JetBotObstacle
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel3);

	FCollisionQueryParams CollisionQueryParams;

	CollisionQueryParams.AddIgnoredActor(GetOwner());
	CollisionQueryParams.AddIgnoredActor(this);

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	CollisionQueryParams.AddIgnoredActors(AttachedActors);

	TArray<FHitResult> HitResults;
	FHitResult HitResult;

	FCollisionShape CollisionShape;

	return GetWorld()->SweepSingleByObjectType(HitResult, BoundaryBox->GetComponentLocation(), BoundaryBox->GetComponentLocation(), BoundaryBox->GetComponentQuat(), ObjectQueryParams, BoundaryBox->GetCollisionShape(), CollisionQueryParams);
}

// Called when the game starts or when spawned
void AJetBotObstacle::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AJetBotObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UBoxComponent* AJetBotObstacle::GetBoundaryBox()
{
	return BoundaryBox;
}

FRotator AJetBotObstacle::GetRandomSpawnRotation()
{
	const float RandRoll = FMath::RandRange(0.0f, SpawnIncrementsNumberRotator.Roll) * SpawnIncrementsAngleRotator.Roll;
	const float RandPitch = FMath::RandRange(0.0f, SpawnIncrementsNumberRotator.Pitch) * SpawnIncrementsAngleRotator.Pitch;
	const float RandYaw = FMath::RandRange(0.0f, SpawnIncrementsNumberRotator.Yaw) * SpawnIncrementsAngleRotator.Yaw;

	return FRotator(RandPitch, RandYaw, RandRoll);
}
