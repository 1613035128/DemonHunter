// Fill out your copyright notice in the Description page of Project Settings.

#include "DemonHunter/Public/ClientGameInstance.h"

void UClientGameInstance::Init()
{
	Super::Init();
	
	if (GetWorld())
	{
		if (auto PC = GetWorld()->GetFirstPlayerController())
		{
			LocalPlayer = PC->GetLocalPlayer();
		}
	}

	LocalUserSubSystem = GetLocalUserSubSystem();
}
