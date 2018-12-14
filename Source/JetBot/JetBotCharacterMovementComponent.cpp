// Copyright 2017, John Henry Miller, All Rights Reserved

#include "JetBotCharacterMovementComponent.h"
#include "GameFramework/Character.h"

UJetBotCharacterMovementComponent::UJetBotCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

void UJetBotCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
}

class FNetworkPredictionData_Client* UJetBotCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);
	check(PawnOwner->Role < ROLE_Authority);

	if (!ClientPredictionData)
	{
		UJetBotCharacterMovementComponent* MutableThis = const_cast<UJetBotCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_JetBotCharacterMovementComponent(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void FSavedMove_JetBotCharacterMovementComponent::Clear()
{
	Super::Clear();
}

uint8 FSavedMove_JetBotCharacterMovementComponent::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();


	return Result;
}

bool FSavedMove_JetBotCharacterMovementComponent::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_JetBotCharacterMovementComponent::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UJetBotCharacterMovementComponent* CharMov = Cast<UJetBotCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{

	}
}

void FSavedMove_JetBotCharacterMovementComponent::PrepMoveFor(class ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UJetBotCharacterMovementComponent* CharMov = Cast<UJetBotCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharMov)
	{

	}
}

FNetworkPredictionData_Client_JetBotCharacterMovementComponent::FNetworkPredictionData_Client_JetBotCharacterMovementComponent(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_JetBotCharacterMovementComponent::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_JetBotCharacterMovementComponent());
}
