// Copyright 2017, John Henry Miller, All Rights Reserved

#pragma once

#include "Core.h"
#include "Engine.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JetBotCharacter.generated.h"

UCLASS()
class JETBOT_API AJetBotCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AJetBotCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Input functions
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveForward(bool bInMove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveBackward(bool bInMove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveLeft(bool bInMove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveRight(bool bInMove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SetJet(const bool bInWantsToJet);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SetJetAxis(const float InJetAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void ChangeColor();

	UPROPERTY(Transient, BlueprintReadWrite)
	float JetScale;

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetAilerons(const bool bInAilerons);

	void SetMoveInput(const bool bInWantsToMove, bool& bWantsToMove, const FVector InInputVector);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnLookYaw(float YawVal);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnLookPitch(float PitchVal);
	
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetJump(bool bInWantsToJump);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetGrind(bool bInWantsToGrind);

	//Collision function
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, FHitResult Hit);

	//Collision function
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnWallRunCapsuleEndOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//The If JetBot is wallrunning
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsGrinding;

	FTimerHandle GrindTimer;

	void SetNotGrinding();

	UPROPERTY(Transient, BlueprintReadOnly)
	AActor* RunningOnActor;

	//Transient variables
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToMoveForward = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToMoveBackward = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToMoveLeft = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToMoveRight = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToJump = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToGrind = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToJet = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToAilerons = false;

	UPROPERTY(Transient)
	bool bFloorChanged = false;

	UPROPERTY(Transient)
	float YawDelta = 0.0f;

	UPROPERTY(Transient)
	float PreviousYaw = 0.0f;

	UPROPERTY(Transient)
	bool bLanded = false;

	UPROPERTY(Transient)
	float LastLandingTime = 0.0f;

	UPROPERTY(Transient, BlueprintReadWrite)
	FVector InputVector;

	UPROPERTY(Transient, BlueprintReadWrite)
	FVector MoveDirection;

	UPROPERTY(Transient, BlueprintReadWrite)
	FVector JetDirection;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector RealAcceleration = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector PreviousRealAcceleration = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector RealVelocity = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector PreviousRealVelocity = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector PreviousVelocity = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector PreviousLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector CurrentFloorNormal;

	UPROPERTY(Transient)
	FVector CurrentWallNormal;

	UPROPERTY(Transient)
	float LastWallHitTime;

	UPROPERTY(Transient)
	int32 MaterialIndex;

	/*UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* ColorBlock;*/

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	TArray<UMaterialInstance*> MaterialsArray;

	UPROPERTY(Transient)
	FVector PreviousFloorNormal;

	UPROPERTY(EditDefaultsOnly, Category = "VR")
	bool bIsVR;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float JetImpulseScale = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float MaxFlatlandWalkingSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CollisionDamageSpeedThreshold = 2000.0f;

	float DefaultWalkableFloorAngle = 0.0f;

	float DefaultWalkableFloorZ = 0.0f;

	//Sound

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* JumpSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* LandSound;

	UAudioComponent* LandAudioComp;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* RollSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* RollWallSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAudioComponent* RollSoundPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float RollSoundMaxSpeed = 4000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* WindSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	UAudioComponent* WindSoundPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float WindSoundMinSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	float WindSoundMaxSpeed = 4000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* JetSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* PeelOffSound;

	//Functions

	//Update our "Real Velocity
	void TickRealVelocity(const float DeltaTime);

	//Add a jet booster impulse
	void TickJets(const float DeltaTime);

	//Add an impulse from riding a rail/wall
	void TickGrinding(const float DeltaTime);

	//Update our floor
	void TickCharacterFloor();

	//Add a steering impulse based on the lean angle
	UFUNCTION(BlueprintNativeEvent, Category = "Movement")
	void TickLeaning(float DeltaTime);

	//Calculate the angle that our character is leaning based on the current state
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void CalculateLeanAngle();

	//Update our velocity to follow the direction of the camera
	void TickAilerons(float DeltaTime);

	//Update our looping sounds
	void TickSounds(float DeltaTime);

	FTimerHandle TimerHandle_ZeroFloor;

	bool IsUsingAilerons();

	bool bCanUseAilerons;

	FTimerHandle TimerHandle_EnableAilerons;
	void EnableAilerons();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Collision")
	UCapsuleComponent* WallRunCapsule;

	virtual void OnWalkingOffLedge_Implementation
	(
		const FVector & PreviousFloorImpactNormal,
		const FVector & PreviousFloorContactNormal,
		const FVector & PreviousLocation,
		float TimeDelta
	) override;

	virtual void NotifyHit
	(
		class UPrimitiveComponent * MyComp,
		AActor * Other,
		class UPrimitiveComponent * OtherComp,
		bool bSelfMoved,
		FVector HitLocation,
		FVector HitNormal,
		FVector NormalImpulse,
		const FHitResult & Hit
	) override;

	virtual void Landed
	(
		const FHitResult & Hit
	) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

};
