// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "HunterActor/Component/FoucusPointComponent.h"
#include "HunterCameraManager.generated.h"

UENUM()
enum class ECameraState : uint8
{
	Default = 0,
	Free,
	Lock,
};
/**
 * 
 */
UCLASS()
class DEMONHUNTER_API AHunterCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:
	
	AHunterCameraManager();
	virtual void UpdateCamera(float DeltaTime) override;
	
	virtual void SetFocusPoint(TObjectPtr<UFoucusPointComponent>& Target);
	
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	TWeakObjectPtr<UFoucusPointComponent> FocusPoint;
	
	ECameraState CameraState;
};
