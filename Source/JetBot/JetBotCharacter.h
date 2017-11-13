// Copyright 2017, John Henry Miller, All Rights Reserved

#pragma once

#include "Core.h"
#include "Engine.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "JetBotCharacter.generated.h"

class USplineComponent;
class AJetBotObstacle;

UENUM(BlueprintType)
enum class EGrindState : uint8
{
	None,
	Wall,
	Rail
};

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
	void SetMoveForwardAxis(float InMoveForwardAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveBackward(bool bInMove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveLeft(bool bInMove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveRight(bool bInMove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveRightAxis(float InMoveRightAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SetJet(const bool bInWantsToJet);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SetJetAxis(const float InJetAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void SetBrakeAxis(const float InBrakeAxis);

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

	//Collision
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, FHitResult Hit);

	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnGrindCapsuleBeginOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Collision")
	void OnGrindCapsuleEndOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//End Collision

	//Transient variables

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsGrinding;

	//What we are grinding on
	UPROPERTY(Transient, BlueprintReadOnly)
	EGrindState CurrentGrindState = EGrindState::None;

	//If JetBot is trying to grind
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsTryingToGrind;

	//The amount of time we have to hit something we can grind on, after pressing grind
	UPROPERTY(EditDefaultsOnly, Category = "Grinding")
	float GrindLeewayTime = 0.5f;

	//For easy grinding
	UPROPERTY(Transient)
	FTimerHandle GrindTimer;

	void SetNotTryingToGrind();

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Collision")
	UCapsuleComponent* GrindCapsule;

	UPROPERTY(Transient, BlueprintReadWrite)
	float JetScale;

	UPROPERTY(Transient, BlueprintReadWrite)
	float BrakeScale;

	UPROPERTY(Transient, BlueprintReadOnly)
	AActor* RunningOnActor;

	UPROPERTY(Transient, BlueprintReadOnly)
	AActor* NextRunningOnActor;
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
	bool bWantsToBrake = false;

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

	

	//End transient variables

	//#Jets
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Jets")
	FVector JetDirection;

	UPROPERTY(Transient, BlueprintReadWrite, Category = "Jets")
	float JetMeter = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Jets")
	float MaxJetMeter = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Jets")
	float JetDrainRate = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Jets")
	bool bCanJet = true;
	//#EndJets

	//#Velocity

	//Update our "Real Velocity
	void TickRealVelocity(const float DeltaTime);

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector RealVelocity = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly)
	FVector PreviousRealVelocity = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector PreviousLocation = FVector::ZeroVector;

	//End Velocity

	//#Floor

	float DefaultWalkableFloorAngle;

	float DefaultWalkableFloorZ;

	virtual void OnWalkingOffLedge_Implementation
	(
		const FVector & PreviousFloorImpactNormal,
		const FVector & PreviousFloorContactNormal,
		const FVector & PreviousLocation,
		float TimeDelta
	) override;

	UPROPERTY(Transient)
	FVector PreviousFloorNormal;

	//Update our floor
	void TickFloor();

	UPROPERTY(Transient)
	FVector CurrentFloorNormal;

	//#EndFloor

	UPROPERTY(Transient)
	FVector CurrentWallNormal;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WallJumpXYVelocity = 800.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MinWallJumpZ = 0.8f;

	UPROPERTY(Transient)
	float LastWallHitTime;

	UPROPERTY(Transient)
	int32 MaterialIndex;
	
	UPROPERTY(BlueprintReadWrite, Category = "Movement")
	USceneComponent* FeetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	TArray<UMaterialInstance*> MaterialsArray;

	UPROPERTY(EditDefaultsOnly, Category = "VR")
	bool bIsVR;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float JetImpulseScale = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float MaxFlatlandWalkingSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CollisionDamageSpeedThreshold = 2000.0f;

	float DefaultGroundFriction;

	//Functions

	void TickMovementInput(const float DeltaTime);

	//Add a jet booster impulse
	void TickJets(const float DeltaTime);

	//Add an impulse based on the floor's slope
	void TickRolling(const float DeltaTime);

	//Add a braking impulse
	void TickBrakes(const float DeltaTime);

	//Add an impulse from riding a rail/wall
	void TickGrinding(const float DeltaTime);

	//Add a steering impulse based on the lean angle (we may not implement this)
	UFUNCTION(BlueprintNativeEvent, Category = "Movement")
	void TickLeaning(float DeltaTime);

	//Calculate the angle that our character is leaning based on the current state
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void CalculateLeanAngle();

	//#Sounds

	
	//Update our looping sounds
	void TickSounds(float DeltaTime);

	void InitializeSoundPlayers();

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* JumpSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* WallHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* LandSound;

	UAudioComponent* LandAudioComp;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* RollSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* GrindWallSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* SlideWallSound;

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
	UAudioComponent* JetSoundPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* PeelOffSound;

	//#EndSound

	//#Ailerons

	void TickAilerons(float DeltaTime);

	bool IsUsingAilerons();

	bool bCanUseAilerons;

	FTimerHandle TimerHandle_EnableAilerons;
	void EnableAilerons();

	//#EndAilerons


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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
