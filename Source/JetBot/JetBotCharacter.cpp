// Copyright 2017, John Henry Miller, All Rights Reserved

#include "JetBotCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "JetBotLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "JetBotObstacle.h"
#include "JetBotGameMode.h"
#include "JetBotCharacterMovementComponent.h"
#include "UnrealNetwork.h"

namespace InputVectors
{
	const FVector Forward(1, 0, 0);
	const FVector Backward(-1, 0, 0);
	const FVector Left(0, -1, 0);
	const FVector Right(0, 1, 0);
	const FVector Up(0, 0, 1);
}

// Sets default values
AJetBotCharacter::AJetBotCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer/*.SetDefaultSubobjectClass<UJetBotCharacterMovementComponent>(ACharacter::CharacterMovementComponentName)*/)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GrindCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName(TEXT("WallRunCapsule")));
	FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
	GrindCapsule->AttachToComponent(GetRootComponent(), AttachRules);


	TrickScoreMap.Emplace(ETrickEnum::None, 0.0f);
	TrickScoreMap.Emplace(ETrickEnum::SmoothLanding, 1.0f);
	TrickScoreMap.Emplace(ETrickEnum::Wall2Wall, 2.0f);
	TrickScoreMap.Emplace(ETrickEnum::WallJump, 1.0f);
	TrickScoreMap.Emplace(ETrickEnum::WallSliding, 1.0f);
	TrickScoreMap.Emplace(ETrickEnum::MovingFast, 1.0f);
	TrickScoreMap.Emplace(ETrickEnum::AirTime, 1.0f);
}

void AJetBotCharacter::GetLifetimeReplicatedProps(TArray < FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AJetBotCharacter, bWantsToJump);
	DOREPLIFETIME(AJetBotCharacter, BrakeScale);
	DOREPLIFETIME(AJetBotCharacter, MoveDirection);
	DOREPLIFETIME(AJetBotCharacter, JetMeter);
	DOREPLIFETIME(AJetBotCharacter, InputVector);
	DOREPLIFETIME(AJetBotCharacter, CurrentFloorNormal);
	DOREPLIFETIME(AJetBotCharacter, CurrentWallNormal);
	DOREPLIFETIME(AJetBotCharacter, RealVelocity);
}

// Called when the game starts or when spawned
void AJetBotCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;

	DefaultWalkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
	DefaultWalkableFloorZ = GetCharacterMovement()->GetWalkableFloorZ();

	JetMeter = MaxJetMeter;
	
	TSubclassOf<UActorComponent> FeetComponentClass = USceneComponent::StaticClass();

	TArray<UActorComponent*> FeetComponents = GetComponentsByTag(FeetComponentClass, FName(TEXT("FeetComponent")));

	if (FeetComponents.Num() > 0)
	{
		FeetComponent = Cast<USceneComponent>(FeetComponents[0]);
	}

	TickFloor();

	InitializeSoundPlayers();
}

void AJetBotCharacter::SetMoveForwardAxis(float InMoveForwardAxis)
{
	if (Role != ROLE_Authority)
	{
		ServerSetMoveForwardAxis(InMoveForwardAxis);
	}

	InputVector.X = InMoveForwardAxis;
}

void AJetBotCharacter::ServerSetMoveForwardAxis_Implementation(float InMoveForwardAxis)
{
	SetMoveForwardAxis(InMoveForwardAxis);
}

bool AJetBotCharacter::ServerSetMoveForwardAxis_Validate(float InMoveForwardAxis)
{
	return true;
}

void AJetBotCharacter::SetMoveRightAxis(float InMoveRightAxis)
{
	if (Role != ROLE_Authority)
	{
		ServerSetMoveRightAxis(InMoveRightAxis);
	}

	InputVector.Y = InMoveRightAxis;
}

void AJetBotCharacter::ServerSetMoveRightAxis_Implementation(float InMoveForwardAxis)
{
	SetMoveRightAxis(InMoveForwardAxis);
}

bool AJetBotCharacter::ServerSetMoveRightAxis_Validate(float InMoveForwardAxis)
{
	return true;
}

void AJetBotCharacter::SetJetAxis(const float InJetAxis)
{
	if (Role != ROLE_Authority)
	{
		ServerSetJetAxis(InJetAxis);
	}

	JetScale = InJetAxis;
}

void AJetBotCharacter::ServerSetJetAxis_Implementation(const float InJetAxis)
{
	SetJetAxis(InJetAxis);
}

bool AJetBotCharacter::ServerSetJetAxis_Validate(const float InJetAxis)
{
	return true;
}

void AJetBotCharacter::SetBrakeAxis(const float InBrakeAxis)
{
	if (Role != ROLE_Authority)
	{
		ServerSetBrakeAxis(InBrakeAxis);
	}

	if (!bWantsToBrake)
	{
		BrakeScale = InBrakeAxis;
	}
}

void AJetBotCharacter::ServerSetBrakeAxis_Implementation(const float InBrakeAxis)
{
	SetBrakeAxis(InBrakeAxis);
}

bool AJetBotCharacter::ServerSetBrakeAxis_Validate(const float InBrakeAxis)
{
	return true;
}

void AJetBotCharacter::SetAilerons(const bool bInAilerons)
{
	if (bInAilerons != bWantsToAilerons)
	{
		bWantsToAilerons = bInAilerons;
	}
}

void AJetBotCharacter::OnLookYaw(float YawVal)
{
	AddControllerYawInput(YawVal);

	YawDelta = YawVal - PreviousYaw;
	PreviousYaw = YawVal;
}

void AJetBotCharacter::OnLookPitch(float PitchVal)
{
	AddControllerPitchInput(PitchVal);
}

void AJetBotCharacter::SetJump(bool bInWantsToJump)
{
	if (Role != ROLE_Authority)
	{
		ServerSetJump(bInWantsToJump);
	}

	if (bInWantsToJump != bWantsToJump)
	{
		bWantsToJump = bInWantsToJump;

		if (bWantsToJump)
		{
			if (GetCurrentFloorNormal().Z < 1)
			{

			}
		}
		else
		{
			bool bJumped = false;

			GetCharacterMovement()->SetMovementMode(MOVE_Falling);

			FVector JumpDirection = FVector(0, 0, 1);

			if (bCanGrindOnRails && CurrentGrindState == EGrindState::Rail)
			{
				if (GetRealVelocity().Z < 0.0f)
				{
					GetRealVelocity().Z = 0.0f;
				}

				SetCurrentGrindState(EGrindState::None);
				GrindingOnSpline = nullptr;
				bIsTryingToGrind = false;

				GetCharacterMovement()->Velocity = GetRealVelocity() + GetCharacterMovement()->JumpZVelocity*JumpDirection;

				
			}
			// If we are on a floor
			else if (GetCurrentFloorNormal() != FVector::ZeroVector)
			{

				if (GetRealVelocity().Z < 0.0f)
				{
					GetRealVelocity().Z = 0.0f;
				}

				GetCharacterMovement()->Velocity = GetRealVelocity() + GetCharacterMovement()->JumpZVelocity*JumpDirection;

				bJumped = true;

			}
			//If we are on a wall && the wall is different from the last wall we jumped off
			else if (GetCurrentWallNormal() != FVector::ZeroVector/* && GetCurrentWallNormal() != LastWallJumpNormal*/)
			{
				JumpDirection = GetCurrentWallNormal();

				JumpDirection.Z = FMath::Max(GetCurrentWallNormal().Z, MinWallJumpZ);
				//JumpDirection.Normalize();

				if (GetCharacterMovement()->Velocity.Z < 0.0f)
				{
					GetCharacterMovement()->Velocity.Z = 0.0f;
				}

				GetCharacterMovement()->Velocity.X += WallJumpXYVelocity*JumpDirection.X;
				GetCharacterMovement()->Velocity.Y += WallJumpXYVelocity*JumpDirection.Y;
				GetCharacterMovement()->Velocity.Z += GetCharacterMovement()->JumpZVelocity*JumpDirection.Z;

				GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("</")));

				LastWallJumpNormal = GetCurrentWallNormal();

				SetCurrentGrindState(EGrindState::None);

				bJumped = true;
			}

			if (bJumped)
			{
				UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
				SetCurrentWallNormal(FVector::ZeroVector);
				SetCurrentFloorNormal(FVector::ZeroVector);
			}
			else
			{
				bIsTryingToJump = true;
				GetWorldTimerManager().SetTimer(JumpTimer, this, &AJetBotCharacter::SetNotTryingToJump, JumpLeewayTime);
			}

		}
	}
}

void AJetBotCharacter::ServerSetJump_Implementation(bool bInWantsToJump)
{
	SetJump(bInWantsToJump);
}

bool AJetBotCharacter::ServerSetJump_Validate(bool bInWantsToJump)
{
	return true;
}

void AJetBotCharacter::SetGrind(bool bInWantsToGrind)
{
	if (bInWantsToGrind != bWantsToGrind)
	{
		bWantsToGrind = bInWantsToGrind;

		if (bWantsToGrind)
		{
			//TODO: Implement rail grinding
			//if we found an adequate spline to grind on
			if (bCanGrindOnRails && GrindingOnSpline)
			{
				SetCurrentGrindState(EGrindState::Rail);
			}
			//See if we are grinding on a wall
			else if (GetCurrentWallNormal() != FVector::ZeroVector)
			{
				SetCurrentGrindState(EGrindState::Wall);
			}
			else
			{
				SetCurrentGrindState(EGrindState::None);
			}

			switch (CurrentGrindState)
			{
				case EGrindState::None:
				{
					bIsTryingToGrind = true;
					GetWorldTimerManager().SetTimer(GrindTimer, this, &AJetBotCharacter::SetNotTryingToGrind, GrindLeewayTime);
					break;
				}
				default:
				{
					bIsTryingToGrind = false;
					GetWorldTimerManager().ClearTimer(GrindTimer);
					break;
				}
			}

			bWantsToGrind = false;
		}
		else
		{

		}
	}
}

// Called every frame
void AJetBotCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickRealVelocity(DeltaTime);

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	//Update our floor, adjust our velocity accordingly
	if (GetCurrentFloorNormal() != FVector::ZeroVector)
	{
		TickFloor();
	}

	if (!bIsDead)
	{
		//Add movement input
		TickMovementInput(DeltaTime);

		TickBrakes(DeltaTime);

		//Add jet impulse
		TickJets(DeltaTime);

		TickGrinding(DeltaTime);

		TickScore(DeltaTime);

		TickFalling();
	}

	//Add floor slope impulse
	TickRolling(DeltaTime);

	//Update our looping sounds
	TickSounds(DeltaTime);

	TickWall(DeltaTime);

	if (bIsTryingToJump && bLanded)
	{
		bIsTryingToJump = false;
		GetWorldTimerManager().ClearTimer(JumpTimer);
		SetJump(true);
		SetJump(false);
	}

	//Set "previous" variables for next tick
	PreviousLocation = GetActorLocation();
	PreviousRealVelocity = GetRealVelocity();
	PreviousFloorNormal = GetCurrentFloorNormal();
	bLanded = false;
}

// Called to bind functionality to input
void AJetBotCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AJetBotCharacter::OnWalkingOffLedge_Implementation(const FVector & PreviousFloorImpactNormal, const FVector & PreviousFloorContactNormal, const FVector & PreviousLocation, float TimeDelta)
{
	Super::OnWalkingOffLedge_Implementation(PreviousFloorImpactNormal, PreviousFloorContactNormal, PreviousLocation, TimeDelta);

	//if we walk off a ledge, set our velocity to what it was before
	if (GetRealVelocity() != FVector::ZeroVector && CurrentGrindState != EGrindState::Rail)
	{
		GetCharacterMovement()->Velocity = GetRealVelocity();
		const float VolumeMultiplier = FMath::Min(GetRealVelocity().Size() / 2000.0f, 1.0f);
		UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), VolumeMultiplier);
	}
}

void AJetBotCharacter::NotifyHit(class UPrimitiveComponent * MyComp, AActor * Other, class UPrimitiveComponent * OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult & Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

void AJetBotCharacter::OnCapsuleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, FHitResult Hit)
{

	//Check if we hit a wall #HitWall
	if (!GetCharacterMovement()->IsWalkable(Hit))
	{
		//If we hit an obstacle that is not walkable, try to project our velocity to slide along the surface of that obstacle

		if (Hit.Normal != GetCurrentFloorNormal() && !FMath::IsNearlyEqual(Hit.Normal.Z, 1.0f, 0.02f))
		{
			if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling || GetCurrentWallNormal() == FVector::ZeroVector)
			{
				const float PreviousSpeed = GetRealVelocity().Size();

				const FVector WallHitVelocity = GetRealVelocity().ProjectOnTo(Hit.Normal);

				const FVector NewVelocity = FVector::VectorPlaneProject(GetRealVelocity(), Hit.Normal);

				//Check for collision damage

				if (bHasFallDamage && WallHitVelocity.Size() > CollisionDamageSpeedThreshold)
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("X")));
					Die(ECauseOfDeathEnum::SlammedIntoWall);
				}

				const FVector PerpindicularVelocity = GetRealVelocity().ProjectOnTo(Hit.Normal);

				const float WallHitVolume = 1.0f * (FMath::Min(GetCharacterMovement()->MaxWalkSpeed, PerpindicularVelocity.Size()) / GetCharacterMovement()->MaxWalkSpeed);
				UGameplayStatics::PlaySoundAtLocation(this, WallHitSound, Hit.Location, WallHitVolume);


				if (bIsTryingToJump)
				{
					bIsTryingToJump = false;
					GetWorldTimerManager().ClearTimer(JumpTimer);
					SetCurrentWallNormal(Hit.Normal);
					SetJump(true);
					SetJump(false);
				}
				else
				{
					LaunchCharacter(NewVelocity, true, true);
					GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT(">/")));
				}


				//If we have jumped from another wall, add points
				if (LastWallHitNormal == FVector::ZeroVector)
				{
					LastWallHitNormal = Hit.Normal;
				}
				else if (LastWallHitNormal != Hit.Normal)
				{
					AddScore(ETrickEnum::Wall2Wall);
				}

				
			}

			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			SetCurrentFloorNormal(FVector::ZeroVector);


			SetCurrentWallNormal(Hit.Normal);

			if (bIsTryingToGrind && GetCurrentWallNormal() != FVector::ZeroVector)
			{
				SetCurrentGrindState(EGrindState::Wall);
			}

			const float CurrentTime = GetWorld()->GetTimeSeconds();
			LastWallHitTime = CurrentTime;

			
		}
	}

}

void AJetBotCharacter::SetNotTryingToGrind()
{
	if (!bWantsToJump && GetCurrentWallNormal() == FVector::ZeroVector)
	{
		bIsTryingToGrind = false;
	}
}

void AJetBotCharacter::SetNotTryingToJump()
{
	bIsTryingToJump = false;
}

void AJetBotCharacter::TickRealVelocity(const float DeltaTime)
{
	//Calculate "real" velocity from distance between two locations
	if (PreviousLocation != FVector::ZeroVector)
	{
		SetRealVelocity((GetActorLocation() - PreviousLocation) / DeltaTime);
	}
}

void AJetBotCharacter::SetRealVelocity(const FVector& InRealVelocity)
{
	if (Role != ROLE_Authority)
	{
		ServerSetRealVelocity(InRealVelocity);
	}

	RealVelocity = InRealVelocity;
}

void AJetBotCharacter::ServerSetRealVelocity_Implementation(const FVector& InRealVelocity)
{
	SetRealVelocity(InRealVelocity);
}

bool AJetBotCharacter::ServerSetRealVelocity_Validate(const FVector& InRealVelocity)
{
	return true;
}

void AJetBotCharacter::TickMovementInput(const float DeltaTime)
{
	if (Role != ROLE_Authority)
	{
		ServerTickMovementInput(DeltaTime);
	}

	if (!bIsVR)
	{
		//Add horizontal movement input
		MoveDirection = InputVector.RotateAngleAxis(GetActorRotation().Yaw, InputVectors::Up);
	}

	if (BrakeScale < 0.1f)
	{
		AddMovementInput(MoveDirection);
	}
}

void AJetBotCharacter::ServerTickMovementInput_Implementation(const float DeltaTime)
{
	TickMovementInput(DeltaTime);
}

bool AJetBotCharacter::ServerTickMovementInput_Validate(const float DeltaTime)
{
	return true;
}

void AJetBotCharacter::TickJets(const float DeltaTime)
{

	if (Role != ROLE_Authority)
	{
		ServerTickJets(DeltaTime);
	}

	if (!bCanJet)
	{
		return;
	}

	bool bShouldJet = false;
	JetDirection = FVector::ZeroVector;

	if (JetScale > 0.01f)
	{
		JetDirection = MoveDirection;
		JetDirection.Z = 1.0f - JetDirection.Size();
		JetDirection.Normalize();

		bShouldJet = true;		
	}

	if (bShouldJet)
	{
		if (JetMeter > 0.0f)
		{
			GetCharacterMovement()->AddImpulse(JetDirection*JetImpulseScale*JetScale*DeltaTime, true);
			JetRegenTimer = JetRegenDelay;

			//Drain jets
			JetMeter -= JetScale*JetDrainRate*DeltaTime;

			if (JetMeter < 0.0f)
			{
				JetMeter = 0.0f;
			}
		}
	}
	else
	{
		JetRegenTimer -= DeltaTime;

		//if we have waited long enough to regenerate the jets
		if (JetRegenTimer <= 0.0f)
		{
			//Regenerate jets
			JetMeter += JetRegenRate*DeltaTime;

			if (JetMeter > MaxJetMeter)
			{
				JetMeter = MaxJetMeter;
			}

			JetRegenTimer = 0.0f;
		}
		
	}

	//GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString::SanitizeFloat(JetMeter));
}

void AJetBotCharacter::ServerTickJets_Implementation(const float DeltaTime)
{
	TickJets(DeltaTime);
}

bool AJetBotCharacter::ServerTickJets_Validate(const float DeltaTime)
{
	return true;
}

void AJetBotCharacter::TickRolling(const float DeltaTime)
{ 
	//Add "rolling" impulse from our floor
	if (GetCurrentFloorNormal() != FVector::ZeroVector)
	{
		if (GetCurrentFloorNormal().Z < 1.0f)
		{
			const FVector RollingNormal = FVector(GetCurrentFloorNormal().X * -1.0f, GetCurrentFloorNormal().Y * -1.0f, GetCurrentFloorNormal().Z);

			const FVector RollingImpulse = RollingNormal * GetCharacterMovement()->GetGravityZ() * DeltaTime;

			GetCharacterMovement()->AddImpulse(RollingImpulse.ProjectOnTo(GetRealVelocity()), true);
		}
	}
}

void AJetBotCharacter::TickBrakes(const float DeltaTime)
{
	GetCharacterMovement()->GroundFriction = BrakeScale*DefaultGroundFriction;
	//GetCharacterMovement()->FallingLateralFriction = BrakeScale*DefaultGroundFriction;
}

void AJetBotCharacter::OnGrindCapsuleBeginOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult)
{

	TouchActor(OtherActor);

	if (!RunningOnActor)
	{
		SetRunningOnActor(OtherActor);

		if (FeetComponent)
		{
			AJetBotObstacle* RunningOnObstacle = Cast<AJetBotObstacle>(RunningOnActor);

			if (RunningOnObstacle)
			{
				GrindingOnSpline = RunningOnObstacle->FindGrindSplineClosestToLocation(FeetComponent->GetComponentLocation(), GrindDistance);

				if (bCanGrindOnRails && bIsTryingToGrind && GrindingOnSpline)
				{
					SetCurrentGrindState(EGrindState::Rail);
				}
			}
		}
	}
	else if (OtherActor != RunningOnActor)
	{
		NextRunningOnActor = OtherActor;
	}
}

void AJetBotCharacter::SetRunningOnActor(AActor* InRunningOnActor)
{
	RunningOnActor = InRunningOnActor;
}

void AJetBotCharacter::TouchActor(AActor* InTouchedActor)
{
	AJetBotObstacle* RunningOnObstacle = Cast<AJetBotObstacle>(InTouchedActor);

	if (RunningOnObstacle)
	{
		RunningOnObstacle->OnPlayerTouched();
	}
}

void AJetBotCharacter::OnGrindCapsuleEndOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == RunningOnActor)
	{
		if (NextRunningOnActor)
		{
			SetRunningOnActor(NextRunningOnActor);

			if (FeetComponent)
			{
				AJetBotObstacle* RunningOnObstacle = Cast<AJetBotObstacle>(RunningOnActor);

				if (RunningOnObstacle)
				{
					GrindingOnSpline = RunningOnObstacle->FindGrindSplineClosestToLocation(FeetComponent->GetComponentLocation(), GrindDistance);
				}
			}

			NextRunningOnActor = nullptr;
		}
		else
		{
			SetRunningOnActor(nullptr);
			GrindingOnSpline = nullptr;
			SetCurrentWallNormal(FVector::ZeroVector);
			bIsTryingToGrind = false;

			SetCurrentGrindState(EGrindState::None);
		}
	}
	else if (OtherActor == NextRunningOnActor)
	{
		NextRunningOnActor = nullptr;
	}
}

void AJetBotCharacter::SetCurrentGrindState(EGrindState InGrindState)
{
	CurrentGrindState = InGrindState;

	if (FeetComponent)
	{
		const static FVector FeetComponentOffset = GetActorLocation() - FeetComponent->GetComponentLocation();

		if (bCanGrindOnRails && InGrindState == EGrindState::Rail && GrindingOnSpline)
		{
			if (GetCharacterMovement()->Velocity.Z > 0.0f)
			{
				GetCharacterMovement()->Velocity.Z = 0.0f;
			}

			GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.ProjectOnTo(GrindingOnSpline->FindDirectionClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World));

			SetActorLocation(GrindingOnSpline->FindLocationClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World) + FeetComponentOffset);
		}
	}
}

void AJetBotCharacter::TickGrinding(const float DeltaTime)
{
	const static FVector FeetComponentOffset = GetActorLocation() - FeetComponent->GetComponentLocation();

	//Check for grindable splines
	if (FeetComponent && bCanGrindOnRails)
	{
		AJetBotObstacle* RunningOnObstacle = Cast<AJetBotObstacle>(RunningOnActor);

		if (RunningOnObstacle)
		{

			USplineComponent* FoundSpline = RunningOnObstacle->FindGrindSplineClosestToLocation(FeetComponent->GetComponentLocation(), GrindDistance);

			if (FoundSpline)
			{
				////if this spline is different than the current spline we are grinding on
				//if (GrindingOnSpline != FoundSpline)
				//{
				//	SetActorLocation(FoundSpline->FindLocationClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World) + FeetComponentOffset);
				//}

				GrindingOnSpline = FoundSpline;

				if (bIsTryingToGrind && bCanGrindOnRails)
				{
					SetCurrentGrindState(EGrindState::Rail);
				}
			}
		}
	}

	//For later, if we want to implement grinding

	switch (CurrentGrindState)
	{
		case EGrindState::None:
		{
			//Nothing
			GetCharacterMovement()->SetPlaneConstraintEnabled(false);
			break;
		}
		case EGrindState::Rail:
		{
			if (!GrindingOnSpline)
			{
				break;
			}

			GetCharacterMovement()->SetPlaneConstraintEnabled(true);

			const FVector SplineDirection = GrindingOnSpline->FindDirectionClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World);

			const FVector PlaneConstraintNormal = SplineDirection.RotateAngleAxis(90.0, FVector(0, 0, 1));

			GetCharacterMovement()->SetPlaneConstraintNormal(PlaneConstraintNormal);
			GetCharacterMovement()->SetPlaneConstraintOrigin(GrindingOnSpline->GetWorldLocationAtSplinePoint(0));
			GetCharacterMovement()->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Custom);
			//GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.ProjectOnTo(GrindingOnSpline->FindDirectionClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World));

			////SetActorLocation(GrindingOnSpline->FindLocationClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World) + FeetComponentOffset);

			break;
		}
		case EGrindState::Wall:
		{
			GetCharacterMovement()->SetPlaneConstraintEnabled(false);
			if (GetCurrentWallNormal() != FVector::ZeroVector)
			{
				GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(GetCharacterMovement()->Velocity, GetCurrentWallNormal());

				if (GetCharacterMovement()->Velocity.Z < 0.0f)
				{
					//Limit our downwards slide by adding an impulse until a threshold, then make downwards velocity constant afterward
					if (GetCharacterMovement()->Velocity.Z > WallGrindFallingVelocityZ)
					{
						if (JetScale == 0.0f)
						{
							GetCharacterMovement()->Velocity.Z = WallGrindFallingVelocityZ;
						}
					}
					else
					{
						GetCharacterMovement()->AddImpulse(JetImpulseScale*InputVectors::Up*DeltaTime, true);
					}
				}
			}
			break;
		}
	}

	if (RunningOnActor)
	{
		//If we are touching a wall
		if (GetCurrentWallNormal() != FVector::ZeroVector)
		{

		}
	}
}

bool AJetBotCharacter::IsUsingAilerons()
{
	return bWantsToAilerons && bCanUseAilerons;
}

void AJetBotCharacter::EnableAilerons()
{
	bCanUseAilerons = true;
}

void AJetBotCharacter::Die_Implementation(ECauseOfDeathEnum CauseOfDeath)
{
	if (bIsDead)
	{
		return;
	}

	switch (CauseOfDeath)
	{
		case ECauseOfDeathEnum::FellToDeath:
		case ECauseOfDeathEnum::SlammedIntoWall:
			UGameplayStatics::SpawnSoundAttached(CrashSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::SnapToTarget, true);
	}

	bIsDead = true;
	SetLifeSpan(2.0);
}

void AJetBotCharacter::TickScore(const float DeltaTime)
{
	static float AirTime;

	const float CurrentSpeed = GetCharacterMovement()->Velocity.Size();

	//Air Time Score Tick
	if (GetCurrentWallNormal() == FVector::ZeroVector && GetCurrentFloorNormal() == FVector::ZeroVector)
	{
		AirTime += DeltaTime;
	}
	else
	{
		AirTime = 0.0f;
	}

	if (AirTime > 2.0f)
	{
		const float* FoundTrickScore = TrickScoreMap.Find(ETrickEnum::AirTime);

		if (FoundTrickScore)
		{
			Score += *FoundTrickScore*DeltaTime;
		}
	}

	//Wall Sliding Score Tick
	if (GetCurrentWallNormal() != FVector::ZeroVector)
	{
		const float* FoundTrickScore = TrickScoreMap.Find(ETrickEnum::WallSliding);

		if (FoundTrickScore)
		{
			Score += *FoundTrickScore*DeltaTime*(CurrentSpeed/1000);
		}
	}

	//Moving Fast Score Tick
	if (GetRealVelocity().Size() > CollisionDamageSpeedThreshold)
	{
		const float* FoundTrickScore = TrickScoreMap.Find(ETrickEnum::MovingFast);

		if (FoundTrickScore)
		{
			Score += *FoundTrickScore*DeltaTime*(CurrentSpeed / 1000);
		}
	}
}

void AJetBotCharacter::AddScore(ETrickEnum Trick)
{
	const float* FoundTrickScore = TrickScoreMap.Find(Trick);

	if (FoundTrickScore)
	{
		Score += *FoundTrickScore;
	}
}

void AJetBotCharacter::TickFloor()
{
	//Update our floor
	FFindFloorResult FloorResult;
	GetCharacterMovement()->FindFloor(GetCapsuleComponent()->GetComponentLocation(), FloorResult, false);

	if (FloorResult.bWalkableFloor)
	{
		if (bPressedJump)
		{
			SetCurrentFloorNormal(FVector::ZeroVector);
		}
		else
		{
			if (!FloorResult.HitResult.Normal.Equals(GetCurrentFloorNormal(), 0.01f))
			{
				/*		/TT We went off a ramp, 
				*/
				if (GetCurrentFloorNormal() != FVector::ZeroVector && GetRealVelocity().Z < PreviousRealVelocity.Z)
				{
					if (!bWantsToJump && !(CurrentGrindState != EGrindState::Rail))
					{
						GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("/TT")));
						LaunchCharacter(PreviousRealVelocity, true, true);
						SetCurrentFloorNormal(FVector::ZeroVector);

						const float VolumeMultiplier = FMath::Min(GetRealVelocity().Size() / 2000.0f, 1.0f);

						UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), VolumeMultiplier);
					}
				}
				/*		__/ We hit a steeper slope, project our velocity onto the new slope
				*/
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("__/")));
					SetCurrentFloorNormal(FloorResult.HitResult.Normal);
					GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(GetRealVelocity(), GetCurrentFloorNormal());

				}
			}
		}
	}
	else
	{

		SetCurrentFloorNormal(FVector::ZeroVector);
	}
}

void AJetBotCharacter::SetCurrentFloorNormal(const FVector& InCurrentFloorNormal)
{
	if (Role != ROLE_Authority)
	{
		ServerSetCurrentFloorNormal(InCurrentFloorNormal);
	}

	CurrentFloorNormal = InCurrentFloorNormal;
}

void AJetBotCharacter::ServerSetCurrentFloorNormal_Implementation(const FVector& InCurrentFloorNormal)
{
	SetCurrentFloorNormal(InCurrentFloorNormal);
}

bool AJetBotCharacter::ServerSetCurrentFloorNormal_Validate(const FVector& InCurrentFloorNormal)
{
	return true;
}

void AJetBotCharacter::TickWall(const float DeltaTime)
{
	//See if we still have a wall to ride on
	if (GetCurrentWallNormal() != FVector::ZeroVector)
	{
		FHitResult Hit;

		const FVector TraceStart = FeetComponent->GetComponentLocation();
		const FVector TraceEnd = TraceStart + GetCurrentWallNormal()*-1.0f*WallPeelOffDistance;

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(this);

		GetWorld()->LineTraceSingleByObjectType(Hit, TraceStart, TraceEnd, ObjectQueryParams, CollisionQueryParams);

		if (!Hit.bBlockingHit)
		{
			SetCurrentWallNormal(FVector::ZeroVector);
		}
	}
}

void AJetBotCharacter::SetCurrentWallNormal(const FVector& InCurrentWallNormal)
{
	if (Role != ROLE_Authority)
	{
		ServerSetCurrentWallNormal(InCurrentWallNormal);
	}

	CurrentWallNormal = InCurrentWallNormal;
}

void AJetBotCharacter::ServerSetCurrentWallNormal_Implementation(const FVector& InCurrentWallNormal)
{
	SetCurrentWallNormal(InCurrentWallNormal);
}

bool AJetBotCharacter::ServerSetCurrentWallNormal_Validate(const FVector& InCurrentWallNormal)
{
	return true;
}

void AJetBotCharacter::TickLeaning_Implementation(float DeltaTime)
{
	
}

void AJetBotCharacter::TickAilerons(float DeltaTime)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	//Update our velocity to follow the direction of the camera

	//Check for peeling off
	if (!bWantsToJump && GetRealVelocity().Size() > 500.0f && PreviousFloorNormal != FVector::ZeroVector && GetCurrentFloorNormal().Z > 0.0f && GetCurrentFloorNormal().Z < 1.0f)
	{

		//Project our Current real velocity and previous real velocity onto our Floor plane
		const FVector PreviousRealVelocityFloor = FVector::VectorPlaneProject(PreviousRealVelocity, GetCurrentFloorNormal());
		const FVector RealVelocityFloor = FVector::VectorPlaneProject(GetRealVelocity(), GetCurrentFloorNormal());

		float Theta = UJetBotLibrary::AngleBetweenVectors(RealVelocityFloor, PreviousRealVelocityFloor);

		Theta = FMath::RadiansToDegrees(Theta);

		if (!bLanded && CurrentTime - LastLandingTime  > 0.25f && PreviousRealVelocityFloor.Z > RealVelocityFloor.Z && FMath::Abs(Theta / DeltaTime) > 200.0f)
		{
			//Peeled off
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("/_)_/")));
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			GetCharacterMovement()->Velocity = GetRealVelocity() + GetRealVelocity().ProjectOnTo(GetCurrentFloorNormal());
			SetCurrentFloorNormal(FVector::ZeroVector);

			const float VolumeMultiplier = FMath::Min(GetRealVelocity().Size() / 2000.0f, 1.0f);
			UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), VolumeMultiplier);

			return;
		}
	}

	FVector AileronsNormal = GetCapsuleComponent()->GetForwardVector();

	FVector AileronsVelocity = GetCharacterMovement()->Velocity.ProjectOnToNormal(AileronsNormal);

	GetCharacterMovement()->Velocity.X = AileronsVelocity.X;
	GetCharacterMovement()->Velocity.Y = AileronsVelocity.Y;
}

void AJetBotCharacter::TickSounds(float DeltaTime)
{
	if (RollSoundPlayer && WindSoundPlayer && JetSoundPlayer)
	{

		//Set volumes
		const float MaxRollSpeed = FMath::Min(GetRealVelocity().Size(), RollSoundMaxSpeed);
		RollSoundPlayer->SetVolumeMultiplier(MaxRollSpeed / RollSoundMaxSpeed);

		float MaxWindSpeed = FMath::Min(GetRealVelocity().Size(), WindSoundMaxSpeed);
		MaxWindSpeed -= WindSoundMinSpeed;
		WindSoundPlayer->SetVolumeMultiplier(MaxWindSpeed / WindSoundMaxSpeed);

		////Limit Jet volume based on JetMeter 
		//if (JetMeter < MaxJetMeter)
		//{
		//	JetSoundPlayer->SetVolumeMultiplier(FMath::Max(JetMeter/MaxJetMeter, JetSoundMinVolume)*JetScale);
		//}
		//else
		//{
			JetSoundPlayer->SetVolumeMultiplier(JetScale);
		/*}*/
		/*else
		{
			JetSoundPlayer->SetVolumeMultiplier(JetScale);
		}
		JetSoundPlayer->SetVolumeMultiplier(JetSoundPlayer
		FMath::Max(*/

		//Wind sound
		if (GetRealVelocity().Size() > WindSoundMinSpeed)
		{
			if (!WindSoundPlayer->IsPlaying())
			{
				WindSoundPlayer->Play();
			}
		}
		else
		{
			WindSoundPlayer->Stop();
		}


		//Rolling and sliding sounds
		if (GetCurrentFloorNormal() != FVector::ZeroVector)
		{
			if (RollSoundPlayer->Sound != RollSound)
			{
				RollSoundPlayer->Stop();
				RollSoundPlayer->Sound = RollSound;
			}

			if (!RollSoundPlayer->IsPlaying())
			{
				RollSoundPlayer->Play();
			}
		}
		else if (GetCurrentWallNormal() != FVector::ZeroVector)
		{
			if (CurrentGrindState == EGrindState::Wall && RollSoundPlayer->Sound != GrindWallSound)
			{
				RollSoundPlayer->Stop();
				RollSoundPlayer->Sound = GrindWallSound;
			}

			if (CurrentGrindState == EGrindState::None && RollSoundPlayer->Sound != SlideWallSound)
			{
				RollSoundPlayer->Stop();
				RollSoundPlayer->Sound = SlideWallSound;
			}

			if (!RollSoundPlayer->IsPlaying())
			{
				RollSoundPlayer->Play();
			}
		}
		else
		{
			RollSoundPlayer->Stop();
		}

		//Jet Sound
		if (JetScale > 0.1f && JetMeter > 0.0f)
		{
			if (!JetSoundPlayer->IsPlaying())
			{
				JetSoundPlayer->FadeIn(0.2f);
			}
		}
		else
		{
			JetSoundPlayer->FadeOut(0.2f, 0.0f);
		}
	}
}

void AJetBotCharacter::InitializeSoundPlayers()
{
	RollSoundPlayer = UGameplayStatics::SpawnSoundAttached(RollSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, false);
	if (RollSoundPlayer)
	{
		RollSoundPlayer->Sound = RollSound;
		RollSoundPlayer->Stop();
	}

	WindSoundPlayer = UGameplayStatics::SpawnSoundAttached(WindSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, false);
	if (WindSoundPlayer)
	{
		WindSoundPlayer->Sound = WindSound;
		WindSoundPlayer->Stop();
	}

	JetSoundPlayer = UGameplayStatics::SpawnSoundAttached(JetSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, false);

	if (JetSoundPlayer)
	{
		JetSoundPlayer->Sound = JetSound;
		JetSoundPlayer->Stop();
	}
}

void AJetBotCharacter::Landed(const FHitResult & Hit)
{
	//Update our floor
	FFindFloorResult FloorResult;
	GetCharacterMovement()->FindFloor(GetCapsuleComponent()->GetComponentLocation(), FloorResult, false);
	
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if (FloorResult.bWalkableFloor && CurrentGrindState != EGrindState::Rail)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("|___|")));

		SetCurrentFloorNormal(FloorResult.HitResult.Normal);
		SetCurrentWallNormal(FVector::ZeroVector);
		LastWallHitNormal = FVector::ZeroVector;
		LastWallJumpNormal = FVector::ZeroVector;
		LastLandingTime = CurrentTime;

		SetCurrentGrindState(EGrindState::None);

		const FVector FloorHitVelocity = GetRealVelocity().ProjectOnTo(GetCurrentFloorNormal());

		if (bHasFallDamage && FloorHitVelocity.Size() > CollisionDamageSpeedThreshold)
		{
			Die(ECauseOfDeathEnum::FellToDeath);
			return;
		}

		if (!LandAudioComp || (LandAudioComp && !LandAudioComp->IsPlaying()))
		{
			LandAudioComp = UGameplayStatics::SpawnSoundAttached(LandSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, false);
		}

		bLanded = true;

		if (GetCurrentFloorNormal().Z < 1.0f)
		{
			if (bLanded)
			{
				GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(GetRealVelocity(), GetCurrentFloorNormal());
			}
		}
	}
}

void AJetBotCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode /*= 0*/)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement()->MovementMode == MOVE_Walking && PrevMovementMode == MOVE_Falling)
	{
		
	}
}

void AJetBotCharacter::TickFalling()
{
	AJetBotGameMode* GameMode = Cast<AJetBotGameMode>(GetWorld()->GetAuthGameMode());

	if (GameMode)
	{
		const float KillZ = GameMode->GetKillZ();
		if (GetActorLocation().Z < KillZ)
		{
			Die(ECauseOfDeathEnum::FellToDeath);
		}
	}
	
}
