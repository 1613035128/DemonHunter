// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ClientGameInstance.generated.h"

class ULocalUserSubSystem;
/**
 * 
 */
UCLASS()
class DEMONHUNTER_API UClientGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
	ULocalUserSubSystem* GetLocalUserSubSystem() const { return LocalUserSubSystem; }
	
	ULocalPlayer* GetLocalPlayer() const { return LocalPlayer; }
protected:
	
protected:
	UPROPERTY()
	ULocalUserSubSystem* LocalUserSubSystem;
	
	UPROPERTY()
	ULocalPlayer* LocalPlayer;
	
	int test;
};
