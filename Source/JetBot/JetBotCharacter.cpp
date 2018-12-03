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

namespace InputVectors
{
	const FVector Forward(1, 0, 0);
	const FVector Backward(-1, 0, 0);
	const FVector Left(0, -1, 0);
	const FVector Right(0, 1, 0);
	const FVector Up(0, 0, 1);
}

// Sets default values
AJetBotCharacter::AJetBotCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

void AJetBotCharacter::SetMoveForward(bool bInMove)
{
	SetMoveInput(bInMove, bWantsToMoveForward, InputVectors::Forward);
}

void AJetBotCharacter::SetMoveForwardAxis(float InMoveForwardAxis)
{
	if (!(bWantsToMoveForward || bWantsToMoveBackward))
	{
		InputVector.X = InMoveForwardAxis;
	}
}

void AJetBotCharacter::SetMoveBackward(bool bInMove)
{
	SetMoveInput(bInMove, bWantsToMoveBackward, InputVectors::Backward);
}

void AJetBotCharacter::SetMoveLeft(bool bInMove)
{
	SetMoveInput(bInMove, bWantsToMoveLeft, InputVectors::Left);
}

void AJetBotCharacter::SetMoveRight(bool bInMove)
{
	SetMoveInput(bInMove, bWantsToMoveRight, InputVectors::Right);
}

void AJetBotCharacter::SetMoveRightAxis(float InMoveRightAxis)
{
	if (!(bWantsToMoveRight || bWantsToMoveLeft))
	{
		InputVector.Y = InMoveRightAxis;
	}
}

void AJetBotCharacter::SetJet(const bool bInWantsToJet)
{
	if (bInWantsToJet != bWantsToJet)
	{
		bWantsToJet = bInWantsToJet;
		bWantsToJet ? JetScale = 1.0f : JetScale = 0.0f;
	}
}

void AJetBotCharacter::SetJetAxis(const float InJetAxis)
{
	if (!bWantsToJet)
	{
		JetScale = InJetAxis;
	}
}

void AJetBotCharacter::SetBrakeAxis(const float InBrakeAxis)
{
	if (!bWantsToBrake)
	{
		BrakeScale = InBrakeAxis;
	}
}

void AJetBotCharacter::SetAilerons(const bool bInAilerons)
{
	if (bInAilerons != bWantsToAilerons)
	{
		bWantsToAilerons = bInAilerons;
	}
}

void AJetBotCharacter::SetMoveInput(const bool bInWantsToMove, bool& bWantsToMove, const FVector InInputVector)
{
	if (bInWantsToMove != bWantsToMove)
	{
		bWantsToMove = bInWantsToMove;
		bWantsToMove ? InputVector += InInputVector : InputVector -= InInputVector;
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
	if (bInWantsToJump != bWantsToJump)
	{
		bWantsToJump = bInWantsToJump;

		if (bWantsToJump)
		{
			if (CurrentFloorNormal.Z < 1)
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
				if (RealVelocity.Z < 0.0f)
				{
					//JumpDirection = CurrentFloorNormal;
					RealVelocity.Z = 0.0f;
				}

				SetCurrentGrindState(EGrindState::None);
				GrindingOnSpline = nullptr;
				bIsTryingToGrind = false;

				GetCharacterMovement()->Velocity = RealVelocity + GetCharacterMovement()->JumpZVelocity*JumpDirection;

				
			}
			// If we are on a floor
			else if (CurrentFloorNormal != FVector::ZeroVector)
			{

				//JumpDirection.Z = FMath::Max(CurrentWallNormal.Z, MinWallJumpZ);
				////JumpDirection.Normalize();

				if (RealVelocity.Z < 0.0f)
				{
					//JumpDirection = CurrentFloorNormal;
					RealVelocity.Z = 0.0f;
				}

				GetCharacterMovement()->Velocity = RealVelocity + GetCharacterMovement()->JumpZVelocity*JumpDirection;

				bJumped = true;

			}
			//If we are on a wall && the wall is different from the last wall we jumped off
			else if (CurrentWallNormal != FVector::ZeroVector/* && CurrentWallNormal != LastWallJumpNormal*/)
			{
				JumpDirection = CurrentWallNormal;

				JumpDirection.Z = FMath::Max(CurrentWallNormal.Z, MinWallJumpZ);
				//JumpDirection.Normalize();

				if (GetCharacterMovement()->Velocity.Z < 0.0f)
				{
					GetCharacterMovement()->Velocity.Z = 0.0f;
				}

				GetCharacterMovement()->Velocity.X += WallJumpXYVelocity*JumpDirection.X;
				GetCharacterMovement()->Velocity.Y += WallJumpXYVelocity*JumpDirection.Y;
				GetCharacterMovement()->Velocity.Z += GetCharacterMovement()->JumpZVelocity*JumpDirection.Z;

				GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("</")));

				LastWallJumpNormal = CurrentWallNormal;

				SetCurrentGrindState(EGrindState::None);

				bJumped = true;
			}

			if (bJumped)
			{
				UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
				CurrentWallNormal = FVector::ZeroVector;
				CurrentFloorNormal = FVector::ZeroVector;
			}
			else
			{
				bIsTryingToJump = true;
				GetWorldTimerManager().SetTimer(JumpTimer, this, &AJetBotCharacter::SetNotTryingToJump, JumpLeewayTime);
			}

		}
	}
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
			else if (CurrentWallNormal != FVector::ZeroVector)
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
	if (CurrentFloorNormal != FVector::ZeroVector)
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

	//Old tickwall
	/*if (CurrentWallNormal != FVector::ZeroVector && CurrentTime - LastWallHitTime > WallNormalResetTime)
	{
		CurrentWallNormal = FVector::ZeroVector;
	}*/

	//Set "previous" variables for next tick
	PreviousLocation = GetActorLocation();
	PreviousRealVelocity = RealVelocity;
	PreviousFloorNormal = CurrentFloorNormal;
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
	if (RealVelocity != FVector::ZeroVector && CurrentGrindState != EGrindState::Rail)
	{
		GetCharacterMovement()->Velocity = RealVelocity;
		const float VolumeMultiplier = FMath::Min(RealVelocity.Size() / 2000.0f, 1.0f);
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

		if (Hit.Normal != CurrentFloorNormal && !FMath::IsNearlyEqual(Hit.Normal.Z, 1.0f, 0.02f))
		{
			if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_Falling || CurrentWallNormal == FVector::ZeroVector)
			{
				const float PreviousSpeed = RealVelocity.Size();

				const FVector WallHitVelocity = RealVelocity.ProjectOnTo(Hit.Normal);

				const FVector NewVelocity = FVector::VectorPlaneProject(RealVelocity, Hit.Normal);

				//Check for collision damage

				if (bHasFallDamage && WallHitVelocity.Size() > CollisionDamageSpeedThreshold)
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("X")));
					Die(ECauseOfDeathEnum::SlammedIntoWall);
				}

				const FVector PerpindicularVelocity = RealVelocity.ProjectOnTo(Hit.Normal);

				const float WallHitVolume = 1.0f * (FMath::Min(GetCharacterMovement()->MaxWalkSpeed, PerpindicularVelocity.Size()) / GetCharacterMovement()->MaxWalkSpeed);
				UGameplayStatics::PlaySoundAtLocation(this, WallHitSound, Hit.Location, WallHitVolume);


				if (bIsTryingToJump)
				{
					bIsTryingToJump = false;
					GetWorldTimerManager().ClearTimer(JumpTimer);
					CurrentWallNormal = Hit.Normal;
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
			CurrentFloorNormal = FVector::ZeroVector;


			CurrentWallNormal = Hit.Normal;

			if (bIsTryingToGrind && CurrentWallNormal != FVector::ZeroVector)
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
	if (!bWantsToJump && CurrentWallNormal == FVector::ZeroVector)
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
		RealVelocity = (GetActorLocation() - PreviousLocation) / DeltaTime;
	}
}

void AJetBotCharacter::TickMovementInput(const float DeltaTime)
{
	if (!bIsVR)
	{
		//Add horizontal movement input
		MoveDirection = InputVector.RotateAngleAxis(GetActorRotation().Yaw, InputVectors::Up);


		////if we are on a wall, and we are not giving input, move into the wall, causing more hit events to occur
		//if (CurrentWallNormal != FVector::ZeroVector && MoveDirection.IsNearlyZero(0.01))
		//{
		//	MoveDirection.X = CurrentWallNormal.X*-1;
		//	MoveDirection.Y = CurrentWallNormal.Y*-1;
		//	MoveDirection.Z = 0.0f;

		//	MoveDirection.Normalize();
		//}
	}

	AddMovementInput(MoveDirection);
}

void AJetBotCharacter::TickJets(const float DeltaTime)
{
	if (!bCanJet)
	{
		return;
	}

	bool bShouldJet = false;
	JetDirection = FVector::ZeroVector;

	if (bWantsToJet || JetScale > 0.01f)
	{
		JetDirection = MoveDirection;
		JetDirection.Z = 1.0f - JetDirection.Size();
		JetDirection.Normalize();

		bShouldJet = true;		
	}

	//if (bWantsToJump && CurrentFloorNormal == FVector::ZeroVector && CurrentWallNormal == FVector::ZeroVector)
	//{
	//	bShouldJet = true;
	//	JetDirection.Z = 1.0f;
	//	JetDirection.Normalize();

	//	/*if (CurrentWallNormal != FVector::ZeroVector)
	//	{
	//		JetDirection = FVector::VectorPlaneProject(JetDirection, CurrentWallNormal);
	//		JetDirection.Normalize();
	//	}*/


	//	JetScale = 1.0f;
	//}

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

void AJetBotCharacter::TickRolling(const float DeltaTime)
{ 
	//Add "rolling" impulse from our floor
	if (CurrentFloorNormal != FVector::ZeroVector)
	{
		if (CurrentFloorNormal.Z < 1.0f)
		{
			const FVector RollingNormal = FVector(CurrentFloorNormal.X * -1.0f, CurrentFloorNormal.Y * -1.0f, CurrentFloorNormal.Z);

			const FVector RollingImpulse = RollingNormal * GetCharacterMovement()->GetGravityZ() * DeltaTime;

			GetCharacterMovement()->AddImpulse(RollingImpulse.ProjectOnTo(RealVelocity), true);
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
			CurrentWallNormal = FVector::ZeroVector;
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
			if (CurrentWallNormal != FVector::ZeroVector)
			{
				GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(GetCharacterMovement()->Velocity, CurrentWallNormal);

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
		if (CurrentWallNormal != FVector::ZeroVector)
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
	if (CurrentWallNormal == FVector::ZeroVector && CurrentFloorNormal == FVector::ZeroVector)
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
	if (CurrentWallNormal != FVector::ZeroVector)
	{
		const float* FoundTrickScore = TrickScoreMap.Find(ETrickEnum::WallSliding);

		if (FoundTrickScore)
		{
			Score += *FoundTrickScore*DeltaTime*(CurrentSpeed/1000);
		}
	}

	//Moving Fast Score Tick
	if (RealVelocity.Size() > CollisionDamageSpeedThreshold)
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
			CurrentFloorNormal = FVector::ZeroVector;
		}
		else
		{
			if (!FloorResult.HitResult.Normal.Equals(CurrentFloorNormal, 0.01f))
			{
				/*		/TT We went off a ramp, 
				*/
				if (CurrentFloorNormal != FVector::ZeroVector && RealVelocity.Z < PreviousRealVelocity.Z)
				{
					if (!bWantsToJump && !(CurrentGrindState != EGrindState::Rail))
					{
						GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("/TT")));
						LaunchCharacter(PreviousRealVelocity, true, true);
						CurrentFloorNormal = FVector::ZeroVector;

						const float VolumeMultiplier = FMath::Min(RealVelocity.Size() / 2000.0f, 1.0f);

						UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), VolumeMultiplier);
					}
				}
				/*		__/ We hit a steeper slope, project our velocity onto the new slope
				*/
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("__/")));
					CurrentFloorNormal = FloorResult.HitResult.Normal;
					GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(RealVelocity, CurrentFloorNormal);

				}
			}
		}
	}
	else
	{

		CurrentFloorNormal = FVector::ZeroVector;
	}
}

void AJetBotCharacter::TickWall(const float DeltaTime)
{
	//See if we still have a wall to ride on
	if (CurrentWallNormal != FVector::ZeroVector)
	{
		FHitResult Hit;

		const FVector TraceStart = FeetComponent->GetComponentLocation();
		const FVector TraceEnd = TraceStart + CurrentWallNormal*-1.0f*WallPeelOffDistance;

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		FCollisionQueryParams CollisionQueryParams;
		CollisionQueryParams.AddIgnoredActor(this);

		GetWorld()->LineTraceSingleByObjectType(Hit, TraceStart, TraceEnd, ObjectQueryParams, CollisionQueryParams);

		if (!Hit.bBlockingHit)
		{
			CurrentWallNormal = FVector::ZeroVector;
		}
	}
}

void AJetBotCharacter::TickLeaning_Implementation(float DeltaTime)
{
	
}

void AJetBotCharacter::TickAilerons(float DeltaTime)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	//Update our velocity to follow the direction of the camera

	//Check for peeling off
	if (!bWantsToJump && RealVelocity.Size() > 500.0f && PreviousFloorNormal != FVector::ZeroVector && CurrentFloorNormal.Z > 0.0f && CurrentFloorNormal.Z < 1.0f)
	{

		//Project our Current real velocity and previous real velocity onto our Floor plane
		const FVector PreviousRealVelocityFloor = FVector::VectorPlaneProject(PreviousRealVelocity, CurrentFloorNormal);
		const FVector RealVelocityFloor = FVector::VectorPlaneProject(RealVelocity, CurrentFloorNormal);

		float Theta = UJetBotLibrary::AngleBetweenVectors(RealVelocityFloor, PreviousRealVelocityFloor);

		Theta = FMath::RadiansToDegrees(Theta);

		if (!bLanded && CurrentTime - LastLandingTime  > 0.25f && PreviousRealVelocityFloor.Z > RealVelocityFloor.Z && FMath::Abs(Theta / DeltaTime) > 200.0f)
		{
			//Peeled off
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("/_)_/")));
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			GetCharacterMovement()->Velocity = RealVelocity + RealVelocity.ProjectOnTo(CurrentFloorNormal);
			CurrentFloorNormal = FVector::ZeroVector;

			const float VolumeMultiplier = FMath::Min(RealVelocity.Size() / 2000.0f, 1.0f);
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
		const float MaxRollSpeed = FMath::Min(RealVelocity.Size(), RollSoundMaxSpeed);
		RollSoundPlayer->SetVolumeMultiplier(MaxRollSpeed / RollSoundMaxSpeed);

		float MaxWindSpeed = FMath::Min(RealVelocity.Size(), WindSoundMaxSpeed);
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
		if (RealVelocity.Size() > WindSoundMinSpeed)
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
		if (CurrentFloorNormal != FVector::ZeroVector)
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
		else if (CurrentWallNormal != FVector::ZeroVector)
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

		CurrentFloorNormal = FloorResult.HitResult.Normal;
		CurrentWallNormal = FVector::ZeroVector;
		LastWallHitNormal = FVector::ZeroVector;
		LastWallJumpNormal = FVector::ZeroVector;
		LastLandingTime = CurrentTime;

		SetCurrentGrindState(EGrindState::None);

		const FVector FloorHitVelocity = RealVelocity.ProjectOnTo(CurrentFloorNormal);

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

		if (CurrentFloorNormal.Z < 1.0f)
		{
			if (bLanded)
			{
				GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(RealVelocity, CurrentFloorNormal);
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

	const float KillZ = GameMode->GetKillZ();
	if (GetActorLocation().Z < KillZ)
	{
		Die(ECauseOfDeathEnum::FellToDeath);
	}
}
