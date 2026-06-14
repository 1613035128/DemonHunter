// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCotroller/HunterCameraManager.h"

#include "HunterActor/HunterActor.h"
#include "PlayerCotroller/HunterPlayerController.h"

static AHunterCameraManager* Instance = nullptr;
AHunterCameraManager::AHunterCameraManager()
{
	Instance = this;
	
	State = ECameraState::MainActor;
	MainCameraComponent = CreateDefaultSubobject<UCameraComponent>("MainCameraComponent");
	MainCameraComponent->SetupAttachment(RootComponent);
}

AHunterCameraManager* AHunterCameraManager::GetInstance()
{
	return Instance;
}

void AHunterCameraManager::Tick(float DeltaSeconds)
{
}

void AHunterCameraManager::UpdateCamera(float DeltaTime)
{
	if (!MainCameraComponent)
		return;
	
	if (!DeltaRotation.IsNearlyZero())
	{
		AHunterPlayerController* Controller = Cast<AHunterPlayerController>(GetWorld()->GetFirstPlayerController());
		if (!Controller)
			return;
		
		// DeltaRotation 看作度 * Sensitivity，转为弧度累加
		FVector2D Sensitivity = Controller->GetInputConfig()->Sensitivity;
		float MaxPitch = FMath::DegreesToRadians(89.f);
		CurrentRotation.X += FMath::DegreesToRadians(DeltaRotation.X * Sensitivity.X);
		CurrentRotation.Y  = FMath::Clamp(
			CurrentRotation.Y + FMath::DegreesToRadians(DeltaRotation.Y * Sensitivity.Y),
			-MaxPitch, MaxPitch);
		DeltaRotation = FVector2D::Zero();
	}
	UpdateCameraInternal(DeltaTime);
}

void AHunterCameraManager::ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
	
}

void AHunterCameraManager::SetFocusPoint(TObjectPtr<UFoucusPointComponent>& Target)
{
	if (!Target)
		return;
	
	FocusPoint = MakeWeakObjectPtr(Target);
}

void AHunterCameraManager::SetMainActor(TWeakObjectPtr<AHunterActor> InMainActor)
{
	if (!InMainActor.IsValid())
		return;
	
	MainActor = InMainActor;
	Init();
}

void AHunterCameraManager::Init()
{
	if (!MainCameraComponent)
		return;
	
	AHunterPlayerController* Controller = Cast<AHunterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!Controller)
		return;
	
	if (!MainActor.IsValid())
		return;
	
	float ArmLength = Controller->GetInputConfig()->CameraArmLength;
	FVector ActorLocation = MainActor->GetActorLocation();
	FVector CameraStartPos = ActorLocation + MainActor->GetActorRightVector() * -ArmLength + FVector(0, 0, 100);
	MainCameraComponent->SetWorldLocation(CameraStartPos);

	// 从初始相机位置计算初始弧度，存入 CurrentRotation（之后作为绝对角度使用）
	FVector InitialDir = (CameraStartPos - ActorLocation).GetSafeNormal();
	CurrentRotation.X = FMath::Atan2(InitialDir.Y, InitialDir.X);  // Yaw
	CurrentRotation.Y = FMath::Asin(InitialDir.Z);                  // Pitch
	DeltaRotation = FVector2D::Zero();
}

void AHunterCameraManager::SetRotate(const FVector2D& TargetRot)
{
	DeltaRotation = TargetRot;
}

void AHunterCameraManager::ComputeCameraTransform(const FVector& ViewCenter, FVector& OutPos, FRotator& OutRot) const
{
	if (!MainCameraComponent)
		return;
	
	AHunterPlayerController* Controller = Cast<AHunterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!Controller)
		return;
	
	float ArmLength = Controller->GetInputConfig()->CameraArmLength;
	float LimitPitch = Controller->GetInputConfig()->LimitPitch;
	
	// CurrentRotation 现为绝对弧度角（由 Init 初始化，UpdateCamera 累加输入）
	float ResultYaw   = CurrentRotation.X;
	float ResultPitch = FMath::Clamp(CurrentRotation.Y,
		FMath::DegreesToRadians(-LimitPitch), FMath::DegreesToRadians(LimitPitch));
	
	float X = ArmLength * FMath::Cos(ResultPitch) * FMath::Cos(ResultYaw);
	float Y = ArmLength * FMath::Cos(ResultPitch) * FMath::Sin(ResultYaw);
	float Z = ArmLength * FMath::Sin(ResultPitch);

	// 先构造相机旋转（弧度转度，回头看球心）
	OutRot.Yaw   = FMath::RadiansToDegrees(ResultYaw) + 180.f;
	OutRot.Pitch = -FMath::RadiansToDegrees(ResultPitch);
	OutRot.Roll  = 0.f;
	OutRot.Normalize();

	// CameraOffset 按相机本地坐标系旋转后再加
	FVector LocalOffset = OutRot.RotateVector(Controller->GetInputConfig()->CameraOffset);
	OutPos = ViewCenter + FVector(X, Y, Z) + LocalOffset;
}

void AHunterCameraManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (AHunterPlayerController* Controller = Cast<AHunterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		Controller->SetViewTarget(this);
	}
}

void AHunterCameraManager::UpdateCameraInternal(float DeltaTime)
{	
	if (!MainCameraComponent)
		return;
	
	FVector TargetPos {0,0,0};
	switch (State)
	{
		case ECameraState::MainActor:
			if (MainActor.IsValid())
			{
				TargetPos = MainActor->GetActorLocation();
			}
		break;
		
		default:
		break;
	}
	FVector	Pos;
	FRotator Rot;
	ComputeCameraTransform(TargetPos, Pos, Rot);
	SetActorLocationAndRotation(Pos, Rot);
}