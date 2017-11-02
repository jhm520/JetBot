// Copyright 2017, John Henry Miller, All Rights Reserved

#include "JetBotCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "JetBotLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

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

	WallRunCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName(TEXT("WallRunCapsule")));
	FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, false);
	WallRunCapsule->AttachToComponent(GetRootComponent(), AttachRules);

	//ColorBlock = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("ColorBlock")));

}

// Called when the game starts or when spawned
void AJetBotCharacter::BeginPlay()
{
	Super::BeginPlay();

	TickCharacterFloor();

	DefaultWalkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
	DefaultWalkableFloorZ = GetCharacterMovement()->GetWalkableFloorZ();

	RollSoundPlayer = UGameplayStatics::SpawnSoundAttached(RollSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, false);
	if (RollSoundPlayer)
	{
		RollSoundPlayer->Sound = RollSound;
		RollSoundPlayer->Stop();
	}
	
	WindSoundPlayer = UGameplayStatics::SpawnSoundAttached(WindSound, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.0f, 0.0f, nullptr, nullptr, false);	if (WindSoundPlayer)
	{
		WindSoundPlayer->Sound = WindSound;
		WindSoundPlayer->Stop();
	}
}

void AJetBotCharacter::SetMoveForward(bool bInMove)
{
	SetMoveInput(bInMove, bWantsToMoveForward, InputVectors::Forward);
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
	
	/*if (!bWantsToJet)
	{
		if (JetScale > 0.01)
		{
			bWantsToJet = true;
		}
		else
		{
			bWantsToJet = false;
		}
	}*/
}

void AJetBotCharacter::ChangeColor()
{
	MaterialIndex++;

	if (MaterialIndex > MaterialsArray.Num() - 1)
	{
		MaterialIndex = 0;
	}

	/*if (!MaterialsArray[MaterialIndex].IsValidLowLevel())
	{
		return;
	}*/

	//ColorBlock->SetMaterial(0, MaterialsArray[MaterialIndex]);
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
			if (CurrentFloorNormal != FVector::ZeroVector || CurrentWallNormal != FVector::ZeroVector)
			{
				UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
				bJumped = true;
			}

			if (bJumped)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Falling);

				//FVector JumpDirection = MoveDirection;

				FVector JumpDirection = FVector(0, 0, 1);


				// If we are on a floor
				if (CurrentFloorNormal != FVector::ZeroVector)
				{
					JumpDirection += CurrentFloorNormal;
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

					if (GetCharacterMovement()->Velocity.Z < 0.0f)
					{
						GetCharacterMovement()->Velocity.Z = 0.0f;
					}

					GetCharacterMovement()->Velocity.Z += GetCharacterMovement()->JumpZVelocity*FMath::Max(CurrentWallNormal.Z, 0.8f);

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
	if (bInWantsToGrind != bWantsToGrind)
	{
		bWantsToGrind = bInWantsToGrind;

		if (bWantsToGrind)
		{
			if (CurrentWallNormal != FVector::ZeroVector && !bIsGrinding)
			{
				bIsGrinding = true;
				bWantsToGrind = false;
			}
			else if (!bIsGrinding && CurrentFloorNormal == FVector::ZeroVector)
			{
				bIsGrinding = true;
				bWantsToGrind = false;
				GetWorldTimerManager().SetTimer(GrindTimer, this, &AJetBotCharacter::SetNotGrinding, 0.5f, false);
			}
		}
		else
		{
			if (bIsGrinding)
			{
				GetWorldTimerManager().ClearTimer(GrindTimer);
				bIsGrinding = false;
			}
		}
	}
}

// Called every frame
void AJetBotCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickRealVelocity(DeltaTime);

	//Clamp velocity to maximum
	if (GetCharacterMovement()->Velocity.Size() > GetCharacterMovement()->MaxWalkSpeed)
	{
		GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetClampedToMaxSize(GetCharacterMovement()->MaxWalkSpeed);
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	//Update our floor, adjust our velocity accordingly
	if (CurrentFloorNormal != FVector::ZeroVector)
	{
		TickCharacterFloor();
	}

	//Add steering impulse from "leaning", implemented in blueprints
	//TickLeaning(DeltaTime);


	//Add "ailerons" steering impulse. Not sure if I want to use this or not

	/*if (bWantsToAilerons && CurrentFloorNormal != FVector::ZeroVector && CurrentWallNormal == FVector::ZeroVector)
	{
		TickAilerons(DeltaTime);
	}*/

	//Update our looping sounds
	TickSounds(DeltaTime);

	//if we aren't in VR mode, update move direction here

	if (!bIsVR)
	{
		//Add horizontal movement input
		MoveDirection = InputVector.RotateAngleAxis(GetActorRotation().Yaw, InputVectors::Up);
		//MoveDirection.Normalize();
	}
	
	JetDirection = MoveDirection;

	AddMovementInput(MoveDirection);

	////Add friction if below a certain speed
	//if (GetCharacterMovement()->Velocity.Size() < 300.0f)
	//{
	//	GetCharacterMovement()->GroundFriction = 8.0f;
	//}
	//else
	//{
	//	GetCharacterMovement()->GroundFriction = 0.0f;
	//}

	/*if (CurrentWallNormal != FVector::ZeroVector)
	{
		GetCharacterMovement()->FallingLateralFriction = 1.0f;
	}
	else
	{
		GetCharacterMovement()->FallingLateralFriction = 0.0f;
	}*/

	/*if (GetCharacterMovement()->Velocity.Size() < 1000.0f)
	{
		GetCharacterMovement()->AirControl = 0.2f;
	}
	else
	{
		GetCharacterMovement()->AirControl = 1.0f;
	}*/

	/*if (CurrentFloorNormal == FVector::ZeroVector)
	{	
		GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->Velocity.Size();
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 2000.0f;
	}*/


	//Add jet impulse
	if (bWantsToJet || JetScale > 0.01f)
	{
		TickJets(DeltaTime);
		/*if (JetDirection != FVector::ZeroVector && GetCharacterMovement()->Velocity.Size() > GetCharacterMovement()->MaxWalkSpeed)
		{
			JetDirection -= LateralVelocity.GetSafeNormal();
		}*/

		GetCharacterMovement()->AddImpulse(JetDirection*JetImpulseScale*JetScale*DeltaTime, true);
	}
	
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

	TickGrinding(DeltaTime);

	////Check if we have "peeled off" of a wall
	//if (CurrentWallNormal != FVector::ZeroVector && CurrentTime - LastWallHitTime > 0.25)
	//{
	//	CurrentWallNormal = FVector::ZeroVector;
	//	UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	//}

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
		UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
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

				LaunchCharacter(NewVelocity, true, true);
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, Hit.Normal.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString(TEXT(">/")));

				if (bWantsToGrind)
				{
					bIsGrinding = true;
					GetWorldTimerManager().ClearTimer(GrindTimer);
					/*bWantsToJump = false;*/
				}
			}

			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			CurrentFloorNormal = FVector::ZeroVector;
			CurrentWallNormal = Hit.Normal;

			if (bWantsToGrind)
			{
				bIsGrinding = true;
				GetWorldTimerManager().ClearTimer(GrindTimer);
			}

			RunningOnActor = Hit.GetActor();

			const float CurrentTime = GetWorld()->GetTimeSeconds();
			LastWallHitTime = CurrentTime;
		}
	}

}

void AJetBotCharacter::OnWallRunCapsuleEndOverlap(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Yellow, FString(TEXT("OffWall")));
	
	if (CurrentWallNormal != FVector::ZeroVector && RunningOnActor != nullptr && RunningOnActor == OtherActor)
	{
		CurrentWallNormal = FVector::ZeroVector;

		RunningOnActor = nullptr;

		UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
	}
}

void AJetBotCharacter::SetNotGrinding()
{
	if (!bWantsToJump && CurrentWallNormal == FVector::ZeroVector)
	{
		bIsGrinding = false;
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

void AJetBotCharacter::TickJets(const float DeltaTime)
{
	
}

void AJetBotCharacter::TickGrinding(const float DeltaTime)
{
	//#WallRun #Grinding
	//If wallrunning, Zero Z velocity, constrain wall movement (if less than zero)
	if (CurrentWallNormal != FVector::ZeroVector && bIsGrinding)
	{
		GetCharacterMovement()->Velocity = FVector::VectorPlaneProject(GetCharacterMovement()->Velocity, CurrentWallNormal);

		if (GetCharacterMovement()->Velocity.Z < 0.0f)
		{
			if (FMath::Abs(GetCharacterMovement()->Velocity.Z) < 100.0f)
			{
				GetCharacterMovement()->Velocity.Z = 0.0f;
			}
			else
			{
				GetCharacterMovement()->AddImpulse(InputVectors::Up*JetImpulseScale*DeltaTime, true);
			}
			//GetCharacterMovement()->Velocity.Z = 0.0f;
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
						UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
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

			UGameplayStatics::PlaySoundAtLocation(this, PeelOffSound, GetActorLocation() - FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

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
	if (RollSoundPlayer && WindSoundPlayer)
	{
		const float MaxRollSpeed = FMath::Min(RealVelocity.Size(), RollSoundMaxSpeed);
		RollSoundPlayer->SetVolumeMultiplier(MaxRollSpeed / RollSoundMaxSpeed);

		float MaxWindSpeed = FMath::Min(RealVelocity.Size(), WindSoundMaxSpeed);
		MaxWindSpeed -= WindSoundMinSpeed;
		WindSoundPlayer->SetVolumeMultiplier(MaxWindSpeed / WindSoundMaxSpeed);

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
		else if (CurrentWallNormal != FVector::ZeroVector && bIsGrinding)
		{
			if (RollSoundPlayer->Sound != RollWallSound)
			{
				RollSoundPlayer->Stop();
				RollSoundPlayer->Sound = RollWallSound;
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

		bIsGrinding = false;

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
