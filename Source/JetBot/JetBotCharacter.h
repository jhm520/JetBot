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

UENUM(BlueprintType)
enum class ECauseOfDeathEnum : uint8
{
	None,
	FellToDeath,
	SlammedIntoWall
};

UENUM(BlueprintType)
enum class ETrickEnum : uint8
{
	None,
	WallJump,
	SmoothLanding,
	Wall2Wall,
	WallSliding,
	MovingFast,
	AirTime
};

UCLASS()
class JETBOT_API AJetBotCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AJetBotCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray < class FLifetimeProperty > & OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveForwardAxis(float InMoveForwardAxis);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Input")
	void ServerSetMoveForwardAxis(float InMoveForwardAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetMoveRightAxis(float InMoveRightAxis);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Input")
	void ServerSetMoveRightAxis(float InMoveForwardAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetJetAxis(const float InJetAxis);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Input")
	void ServerSetJetAxis(const float InJetAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetBrakeAxis(const float InBrakeAxis);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Input")
	void ServerSetBrakeAxis(const float InBrakeAxis);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetAilerons(const bool bInAilerons);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnLookYaw(float YawVal);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void OnLookPitch(float PitchVal);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetJump(bool bInWantsToJump);

	UFUNCTION(Server, Reliable, WithValidation, Category = "Input")
	void ServerSetJump(bool bInWantsToJump);

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

	void SetCurrentGrindState(EGrindState InGrindState);

	//The distance from a spline we need to be to grind on it
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grinding")
	float GrindDistance = 50.0f;

	//The downwards Z velocity that will remain constant when grinding on a wall.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float WallGrindFallingVelocityZ = -200.0f;

	//If JetBot is trying to grind
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsTryingToGrind;

	//The amount of time we have to hit something we can grind on, after pressing grind
	UPROPERTY(EditDefaultsOnly, Category = "Grinding")
	bool bCanGrindOnRails = false;

	//The amount of time we have to hit something we can grind on, after pressing grind
	UPROPERTY(EditDefaultsOnly, Category = "Grinding")
	float GrindLeewayTime = 0.5f;

	//For easy grinding
	UPROPERTY(Transient)
	FTimerHandle GrindTimer;

	void SetNotTryingToGrind();

	//If JetBot is trying to grind
	UPROPERTY(Transient, BlueprintReadOnly)
	bool bIsTryingToJump;

	FTimerHandle JumpTimer;

	void SetNotTryingToJump();

	//The amount of time we have to hit something to jump, and jump immediately
	UPROPERTY(EditDefaultsOnly, Category = "Jump")
	float JumpLeewayTime = 0.5f;

		UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Collision")
	UCapsuleComponent* GrindCapsule;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite)
	float JetScale;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite)
	float BrakeScale = 1.0f;

	UPROPERTY(Transient, BlueprintReadOnly)
	AActor* RunningOnActor;

	void SetRunningOnActor(AActor* InRunningOnActor);

	void TouchActor(AActor* InTouchedActor);

	UPROPERTY(Transient, BlueprintReadOnly)
	AActor* NextRunningOnActor;

	UPROPERTY(Transient, BlueprintReadOnly)
	USplineComponent* GrindingOnSpline;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly)
	bool bWantsToJump = false;

	UPROPERTY(Transient, BlueprintReadOnly)
	bool bWantsToGrind = false;

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

	UPROPERTY(Replicated, Transient, BlueprintReadWrite)
	FVector InputVector;

	UPROPERTY(Replicated, Transient, BlueprintReadWrite)
	FVector MoveDirection;

	//End transient variables

	//#Jets
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Jets")
	FVector JetDirection;

	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Jets")
	float JetMeter = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jets")
	float MaxJetMeter = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jets")
	float JetDrainRate = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jets")
	float JetRegenRate = 100.0f;

	//Time after stopping jets, that the jet meter should start regenerating
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jets")
	float JetRegenDelay = 1.0f;

	//Jet Regen timer
	UPROPERTY(Transient)
	float JetRegenTimer = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jets")
	bool bCanJet = true;
	//#EndJets

	//#Velocity

	//Update our "Real Velocity
	void TickRealVelocity(const float DeltaTime);

	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	FVector RealVelocity = FVector::ZeroVector;

	void SetRealVelocity(const FVector& InRealVelocity);

	FVector& GetRealVelocity() { return RealVelocity; }

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetRealVelocity(const FVector& InRealVelocity);

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

	UPROPERTY(Replicated, Transient)
	FVector CurrentFloorNormal;

	const FVector& GetCurrentFloorNormal() { return CurrentFloorNormal; }

	void SetCurrentFloorNormal(const FVector& InCurrentFloorNormal);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetCurrentFloorNormal(const FVector& InCurrentFloorNormal);

	//#EndFloor

	//Check if we have peeled off of a wall
	void TickWall(const float DeltaTime);

	//The normal of the wall we're on (points outward from the wall)
	UPROPERTY(Replicated, Transient)
	FVector CurrentWallNormal;

	const FVector& GetCurrentWallNormal() { return CurrentWallNormal; }

	void SetCurrentWallNormal(const FVector& InCurrentWallNormal);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetCurrentWallNormal(const FVector& InCurrentWallNormal);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WallPeelOffDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float WallJumpXYVelocity = 400.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MinWallJumpZ = 1.0f;

	UPROPERTY(Transient)
	float LastWallHitTime;

	//Time it takes for the wall normal to reset after leaving a wall (even a brush surface)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WallNormalResetTime = 0.1f;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float CollisionDamageSpeedThreshold = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	bool bHasFallDamage = false;

	float DefaultGroundFriction = 8.0f;

	//Functions

	void TickMovementInput(const float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTickMovementInput(const float DeltaTime);

	//Add a jet booster impulse
	void TickJets(const float DeltaTime);

	//Add a jet booster impulse
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTickJets(const float DeltaTime);

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
	USoundBase* CrashSound;

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
	float JetSoundMinVolume = 0.25f;

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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Death")
	void Die(ECauseOfDeathEnum CauseOfDeath);

	bool bIsDead;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Scoring")
	float Score;

	void TickScore(const float DeltaTime);

	void AddScore(ETrickEnum Trick);

	//Keep track of the last wall we jumped on
	UPROPERTY(Transient)
	FVector LastWallJumpNormal = FVector::ZeroVector;

	//For scoring, keep track of the last wall we landed on
	UPROPERTY(Transient)
	FVector LastWallHitNormal = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<ETrickEnum, float> TrickScoreMap;

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

	void TickFalling();
};
