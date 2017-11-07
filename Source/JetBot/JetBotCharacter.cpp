// Copyright 2017, John Henry Miller, All Rights Reserved

#include "JetBotCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "JetBotLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "JetBotObstacle.h"

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

	//ColorBlock = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("ColorBlock")));

}

// Called when the game starts or when spawned
void AJetBotCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;

	DefaultWalkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
	DefaultWalkableFloorZ = GetCharacterMovement()->GetWalkableFloorZ();
	
	TSubclassOf<UActorComponent> FeetComponentClass = USceneComponent::StaticClass();

	TArray<UActorComponent*> FeetComponents = GetComponentsByTag(FeetComponentClass, FName(TEXT("FeetComponent")));

	if (FeetComponents.Num() > 0)
	{
		FeetComponent = Cast<USceneComponent>(FeetComponents[0]);
	}

	TickCharacterFloor();

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

void AJetBotCharacter::ChangeColor()
{
	MaterialIndex++;

	if (MaterialIndex > MaterialsArray.Num() - 1)
	{
		MaterialIndex = 0;
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

			/*if (CurrentWallNormal != FVector::ZeroVector && !bIsGrinding)
			{
				bIsGrinding = true;
				bWantsToJump = false;
				GetWorldTimerManager().ClearTimer(GrindTimer);
			}
			else if (!bIsGrinding && CurrentFloorNormal == FVector::ZeroVector)
			{
				bIsGrinding = true;

				GetWorldTimerManager().SetTimer(GrindTimer, this, &AJetBotCharacter::SetNotGrinding, 0.5f, false);
			}*/
		}
		else
		{
			bool bJumped = false;
			if (GrindingOnSpline || CurrentFloorNormal != FVector::ZeroVector || CurrentWallNormal != FVector::ZeroVector)
			{
				UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
				bJumped = true;
			}

			if (bJumped)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);

				FVector JumpDirection = FVector(0, 0, 1);


				// If we are grinding a rail
				if (GrindingOnSpline != nullptr && FeetComponent != nullptr)
				{
					JumpDirection = GetActorLocation() - GrindingOnSpline->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);

					JumpDirection.Normalize();
					/*GetCharacterMovement()->Velocity = RealVelocity + JumpDirection*GetCharacterMovement()->JumpZVelocity;*/

					//GetCharacterMovement()->Velocity = RealVelocity + JumpDirection*GetCharacterMovement()->JumpZVelocity;

					JumpDirection.Z = FMath::Max(CurrentWallNormal.Z, 0.8f);

					GetCharacterMovement()->Velocity += JumpDirection*GetCharacterMovement()->JumpZVelocity;

					/*if (GetCharacterMovement()->Velocity.Z < 0.0f)
					{
						GetCharacterMovement()->Velocity.Z = 0.0f;
					}*/
					GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("_-_")));
					GrindingOnSpline = nullptr;

					bIsTryingToGrind = false;
					GetWorldTimerManager().ClearTimer(GrindTimer);
				}
				// If we are on a floor
				else if (CurrentFloorNormal != FVector::ZeroVector)
				{
					JumpDirection = CurrentFloorNormal;
					JumpDirection.Normalize();
					/*GetCharacterMovement()->Velocity = RealVelocity + JumpDirection*GetCharacterMovement()->JumpZVelocity;*/

					GetCharacterMovement()->Velocity = RealVelocity + JumpDirection*GetCharacterMovement()->JumpZVelocity;
					if (GetCharacterMovement()->Velocity.Z < 0.0f)
					{
						GetCharacterMovement()->Velocity.Z = 0.0f;
					}

					//GetCharacterMovement()->Velocity.Z += GetCharacterMovement()->JumpZVelocity;
					
				}
				//If we are on a wall
				else if (CurrentWallNormal != FVector::ZeroVector)
				{
					JumpDirection = CurrentWallNormal;
					JumpDirection.Normalize();
					/*GetCharacterMovement()->Velocity.X = RealVelocity.X + (JumpDirection*GetCharacterMovement()->JumpZVelocity).X;
					GetCharacterMovement()->Velocity.Y = RealVelocity.Y + (JumpDirection*GetCharacterMovement()->JumpZVelocity).Y;*/

					GetCharacterMovement()->Velocity.X = RealVelocity.X + (JumpDirection*GetCharacterMovement()->JumpZVelocity).X;
					GetCharacterMovement()->Velocity.Y = RealVelocity.Y + (JumpDirection*GetCharacterMovement()->JumpZVelocity).Y;

					/*if (GetCharacterMovement()->Velocity.Z < 0.0f)
					{
						GetCharacterMovement()->Velocity.Z = 0.0f;
					}*/

					GetCharacterMovement()->Velocity.Z += GetCharacterMovement()->JumpZVelocity*FMath::Max(CurrentWallNormal.Z, 0.8f);

					GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("</")));
					//bIsGrinding = false;
					bIsGrinding = false;
				}

				CurrentWallNormal = FVector::ZeroVector;
				CurrentFloorNormal = FVector::ZeroVector;
			}
		}
	}
}

void AJetBotCharacter::SetGrind(bool bInWantsToGrind)
{
	/*if (bInWantsToGrind != bWantsToGrind)
	{
		bWantsToGrind = bInWantsToGrind;

		if (bWantsToGrind)
		{
			bIsTryingToGrind = true;
			bWantsToGrind = false;
			GetWorldTimerManager().SetTimer(GrindTimer, this, &AJetBotCharacter::SetNotTryingToGrind, 0.5f, false);	
		}
		else
		{
			if (GrindingOnSpline)
			{
				GetWorldTimerManager().ClearTimer(GrindTimer);
				bIsTryingToGrind = false;
				GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("_-_")));
				GrindingOnSpline = nullptr;
			}
		}
	}*/
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
		TickCharacterFloor();
	}

	//Add movement input
	TickMovementInput(DeltaTime);

	TickBrakes(DeltaTime);

	//Add jet impulse
	TickJets(DeltaTime);

	//Add floor slope impulse
	TickRolling(DeltaTime);

	TickGrinding(DeltaTime);

	//Update our looping sounds
	TickSounds(DeltaTime);

	//Set "previous" variables for next tick
	PreviousLocation = GetActorLocation();
	PreviousRealVelocity = RealVelocity;
	PreviousVelocity = GetCharacterMovement()->Velocity;
	PreviousRealAcceleration = RealAcceleration;
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
	if (RealVelocity != FVector::ZeroVector)
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

				const FVector NewVelocity = FVector::VectorPlaneProject(RealVelocity, Hit.Normal);

				//Check for collision damage

				if (PreviousSpeed > CollisionDamageSpeedThreshold)
				{
					const float NewSpeed = NewVelocity.Size();

					const float DeltaSpeed = NewSpeed - PreviousSpeed;

					if (DeltaSpeed < CollisionDamageSpeedThreshold)
					{
						/*TSubclassOf<UDamageType> DamageType;
						UGameplayStatics::ApplyDamage(this, DeltaSpeed * 100.0f, nullptr, this, DamageType);*/
						GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("X")));
					}
				}

				const FVector PerpindicularVelocity = RealVelocity.ProjectOnTo(Hit.Normal);

				const float WallHitVolume = 1.0f * (FMath::Min(GetCharacterMovement()->MaxWalkSpeed, PerpindicularVelocity.Size()) / GetCharacterMovement()->MaxWalkSpeed);
				UGameplayStatics::PlaySoundAtLocation(this, WallHitSound, Hit.Location, WallHitVolume);

				LaunchCharacter(NewVelocity, true, true);
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, Hit.Normal.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT(">/")));

				//if (bIsTryingToGrind)
				//{
				//	bIsGrinding = true;
				//	GetWorldTimerManager().ClearTimer(GrindTimer);
				//	/*bWantsToJump = false;*/
				//}
			}

			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			CurrentFloorNormal = FVector::ZeroVector;
			CurrentWallNormal = Hit.Normal;

			/*if (bIsTryingToGrind)
			{
				bIsGrinding = true;
				GetWorldTimerManager().ClearTimer(GrindTimer);
			}*/

			RunningOnActor = Hit.GetActor();

			const float CurrentTime = GetWorld()->GetTimeSeconds();
			LastWallHitTime = CurrentTime;
		}
	}

}

void AJetBotCharacter::OnGrindCapsuleBeginOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult SweepResult)
{
	if (!RunningOnActor)
	{
		RunningOnActor = OtherActor;
	}
	else if (OtherActor != RunningOnActor)
	{
		NextRunningOnActor = OtherActor;
	}

	/*if (!RunningOnObstacle)
	{
		RunningOnObstacle = Cast<AJetBotObstacle>(OtherActor);
	}
	else if (OtherActor != RunningOnObstacle)
	{
		NextRunningOnObstacle = Cast<AJetBotObstacle>(OtherActor);
	}*/
}

void AJetBotCharacter::OnGrindCapsuleEndOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == RunningOnActor)
	{
		if (NextRunningOnActor)
		{
			RunningOnActor = NextRunningOnActor;
			NextRunningOnActor = nullptr;
		}
		else
		{
			RunningOnActor = nullptr;
			CurrentWallNormal = FVector::ZeroVector;
			GrindingOnSpline = nullptr;
			/*GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("_-_")));

			UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));*/
		}
	}
	else if (OtherActor == NextRunningOnActor)
	{
		NextRunningOnActor = nullptr;
	}


	//if (OtherActor == RunningOnObstacle)
	//{
	//	if (NextRunningOnObstacle)
	//	{
	//		RunningOnObstacle = NextRunningOnObstacle;
	//		NextRunningOnObstacle = nullptr;
	//	}
	//	else
	//	{
	//		RunningOnObstacle = nullptr;
	//		CurrentWallNormal = FVector::ZeroVector;
	//		GrindingOnSpline = nullptr;
	//		/*GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("_-_")));

	//		UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));*/
	//	}
	//}
	//else if (OtherActor == NextRunningOnObstacle)
	//{
	//	NextRunningOnObstacle = nullptr;
	//}
}

void AJetBotCharacter::SetNotTryingToGrind()
{
	if (!bWantsToJump && CurrentWallNormal == FVector::ZeroVector)
	{
		bIsTryingToGrind = false;
	}
}

void AJetBotCharacter::TickRealVelocity(const float DeltaTime)
{
	//Calculate "real" velocity from distance between two locations
	if (PreviousLocation != FVector::ZeroVector)
	{
		RealVelocity = (GetActorLocation() - PreviousLocation) / DeltaTime;
		
		if (PreviousRealVelocity != FVector::ZeroVector)
		{
			RealAcceleration = RealVelocity - PreviousRealVelocity;
		}
		else
		{
			RealAcceleration = FVector::ZeroVector;
		}
	}
}

void AJetBotCharacter::TickMovementInput(const float DeltaTime)
{
	if (!bIsVR)
	{
		//Add horizontal movement input
		MoveDirection = InputVector.RotateAngleAxis(GetActorRotation().Yaw, InputVectors::Up);
	}

	AddMovementInput(MoveDirection);
}

void AJetBotCharacter::TickJets(const float DeltaTime)
{
	bool bShouldJet = false;
	JetDirection = FVector::ZeroVector;

	if (bWantsToJet || JetScale > 0.01f)
	{
		JetDirection = MoveDirection;
		JetDirection.Z = 1.0f - JetDirection.Size();
		JetDirection.Normalize();

		bShouldJet = true;		
	}

	if (bWantsToJump && CurrentFloorNormal == FVector::ZeroVector)
	{
		bShouldJet = true;
		JetDirection.Z = 1.0f;
		JetDirection.Normalize();

		if (CurrentWallNormal != FVector::ZeroVector)
		{
			JetDirection = FVector::VectorPlaneProject(JetDirection, CurrentWallNormal);
			JetDirection.Normalize();
		}


		JetScale = 1.0f;
	}

	if (bShouldJet)
	{
		GetCharacterMovement()->AddImpulse(JetDirection*JetImpulseScale*JetScale*DeltaTime, true);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, JetDirection.ToString());
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
	GetCharacterMovement()->FallingLateralFriction = BrakeScale*DefaultGroundFriction;
}

void AJetBotCharacter::TickGrinding(const float DeltaTime)
{
	//#WallRun #Grinding

	static FVector CurrentRailDirection;

	//If we are trying to find a spline to grind on
	if (bIsTryingToGrind)
	{
		USplineComponent* ClosestGrindableSpline = nullptr;

		if (RunningOnObstacle && FeetComponent)
		{
			ClosestGrindableSpline = RunningOnObstacle->FindGrindSplineClosestToLocation(FeetComponent->GetComponentLocation(), 100.0f);
		}

		if (ClosestGrindableSpline)
		{
			if (!GrindingOnSpline)
			{
				//Hit Rail
				//Started grinding on a spline
				GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("___")));

				FVector GrindPoint = ClosestGrindableSpline->FindLocationClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World);

				SetActorLocation(GrindPoint - (FeetComponent->GetComponentLocation() - GetActorLocation()));

				CurrentRailDirection = ClosestGrindableSpline->FindDirectionClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World);
			}

			GrindingOnSpline = ClosestGrindableSpline;
		}
		//else
		//{
		//	if (GrindingOnSpline)
		//	{
		//		//Stopped grinding on a spline
		//		GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("_-_")));
		//	}

		//	GrindingOnSpline = nullptr;
		//	GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("_-_")));
		//}
	}

	if (GrindingOnSpline)
	{
		if (CurrentRailDirection != FVector::ZeroVector)
		{
			FVector GrindPoint = GrindingOnSpline->FindLocationClosestToWorldLocation(FeetComponent->GetComponentLocation(), ESplineCoordinateSpace::World);


			GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.ProjectOnTo(CurrentRailDirection);
		}
	}
	else
	{
		CurrentRailDirection = FVector::ZeroVector;
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

void AJetBotCharacter::TickCharacterFloor()
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
					if (!bWantsToJump)
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

		JetSoundPlayer->SetVolumeMultiplier(JetScale);

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
			if (bIsGrinding && RollSoundPlayer->Sound != GrindWallSound)
			{
				RollSoundPlayer->Stop();
				RollSoundPlayer->Sound = GrindWallSound;
			}

			if (!bIsGrinding && RollSoundPlayer->Sound != SlideWallSound)
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
		if (JetScale > 0.1f)
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

	if (FloorResult.bWalkableFloor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT("|___|")));

		CurrentFloorNormal = FloorResult.HitResult.Normal;
		CurrentWallNormal = FVector::ZeroVector;
		bLanded = true;
		LastLandingTime = CurrentTime;

		if (!LandAudioComp || (LandAudioComp && !LandAudioComp->IsPlaying()))
		{
			LandAudioComp = UGameplayStatics::SpawnSoundAttached(LandSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, false);
		}
	}
}

void AJetBotCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode /*= 0*/)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement()->MovementMode == MOVE_Walking && PrevMovementMode == MOVE_Falling)
	{
		if (CurrentFloorNormal.Z < 1.0f)
		{
			if (bLanded)
			{
				GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(RealVelocity, CurrentFloorNormal);
			}

			bFloorChanged = false;
		}
	}
}
