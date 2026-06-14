// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "HunterActor/Component/FoucusPointComponent.h"
#include "HunterCameraManager.generated.h"

class AHunterActor;

UENUM()
enum class ECameraState : uint8
{
	Default = 0,
	MainActor,
	LockTarget,
	PlayingAnim,
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
	
	static AHunterCameraManager* GetInstance();
	
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void UpdateCamera(float DeltaTime) override;
	
	virtual void ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot) override;
	
	virtual void SetFocusPoint(TObjectPtr<UFoucusPointComponent>& Target);
	
	void SetMainActor(TWeakObjectPtr<AHunterActor> InMainActor);
	
	void Init();
	
	void SetRotate(const FVector2D& TargetRot);
	
	void ComputeCameraTransform(const FVector& ViewCenter, FVector& OutPos, FRotator& OutRot) const;
	
protected:
	virtual void BeginPlay() override;
	
	virtual void UpdateCameraInternal(float DeltaTime);
public:
protected:
	UPROPERTY()
	TWeakObjectPtr<UFoucusPointComponent> FocusPoint {nullptr};
	
	UPROPERTY()
	TWeakObjectPtr<AHunterActor> MainActor {nullptr};
	
	ECameraState CameraState {ECameraState::Default};
	
	ECameraState State {ECameraState::MainActor};
	
	FVector2D DeltaRotation {FVector2D::Zero()};
	
	FVector2D CurrentRotation {FVector2D::Zero()};

	UPROPERTY()
	UCameraComponent* MainCameraComponent {nullptr};
};