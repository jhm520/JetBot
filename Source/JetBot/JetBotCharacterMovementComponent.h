// Copyright 2017, John Henry Miller, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "JetBotCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class JETBOT_API UJetBotCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()


	friend class FSavedMove_ExtendedFSavedMove_JetBotCharacterMovementComponent;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	
};

class FSavedMove_JetBotCharacterMovementComponent : public FSavedMove_Character
{
public:

	typedef FSavedMove_Character Super;

	///@brief Resets all saved variables.
	virtual void Clear() override;

	///@brief Store input commands in the compressed flags.
	virtual uint8 GetCompressedFlags() const override;

	///@brief This is used to check whether or not two moves can be combined into one.
	///Basically you just check to make sure that the saved variables are the same.
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;

	///@brief Sets up the move before sending it to the server. 
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	///@brief Sets variables on character movement component before making a predictive correction.
	virtual void PrepMoveFor(class ACharacter* Character) override;
};

class FNetworkPredictionData_Client_JetBotCharacterMovementComponent : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_JetBotCharacterMovementComponent(const UCharacterMovementComponent& ClientMovement);

	typedef FNetworkPredictionData_Client_Character Super;

	///@brief Allocates a new copy of our custom saved move
	virtual FSavedMovePtr AllocateNewMove() override;
};
