// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "JetBotLibrary.generated.h"

/**
 * 
 */
UCLASS()
class JETBOT_API UJetBotLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	//Return angle between vectors (in radians)
	static float AngleBetweenVectors(const FVector& V1, const FVector&V2);
	
};
