// Fill out your copyright notice in the Description page of Project Settings.

#include "DemonHunter/Public/SubSystems/ULocalUserSubSystem.h"

void ULocalUserSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogTemp, Display, TEXT("Initializing LocalPlayerSubSystem"));
}

void ULocalUserSubSystem::Deinitialize()
{
	Super::Deinitialize();
}
