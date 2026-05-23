// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCotroller/HunterCameraManager.h"

AHunterCameraManager::AHunterCameraManager()
{
}

void AHunterCameraManager::UpdateCamera(float DeltaTime)
{
	// Super::UpdateCamera(DeltaTime);
	
}

void AHunterCameraManager::SetFocusPoint(TObjectPtr<UFoucusPointComponent>& Target)
{
	if (!Target)
		return;
	
	FocusPoint = MakeWeakObjectPtr(Target);
}

void AHunterCameraManager::BeginPlay()
{
	Super::BeginPlay();
}
