// Copyright 2017, John Henry Miller, All Rights Reserved

#include "NewJetBotCharacter.h"
#include "JetBotCharacterMovementComponent.h"

// Sets default values
ANewJetBotCharacter::ANewJetBotCharacter(const FObjectInitializer& ObjectInitializer) : Super (ObjectInitializer.SetDefaultSubobjectClass<UJetBotCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANewJetBotCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANewJetBotCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANewJetBotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

