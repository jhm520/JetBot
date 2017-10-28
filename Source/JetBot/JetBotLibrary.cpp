// Fill out your copyright notice in the Description page of Project Settings.

#include "JetBotLibrary.h"




float UJetBotLibrary::AngleBetweenVectors(const FVector& V1, const FVector& V2)
{
	return FMath::Acos(FVector::DotProduct(V1, V2) / (V1.Size()*V2.Size()));
}
