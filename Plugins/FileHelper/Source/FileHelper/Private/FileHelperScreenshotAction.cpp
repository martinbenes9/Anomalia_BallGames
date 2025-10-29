// Copyright 2024 RLoris

#include "FileHelperScreenshotAction.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TextureResource.h"
#include "UnrealClient.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

UFileHelperScreenshotAction* UFileHelperScreenshotAction::TakeScreenshot(UObject* InWorldContextObject, const FFileHelperScreenshotActionOptions& InOptions)
{
	UFileHelperScreenshotAction* Node = NewObject<UFileHelperScreenshotAction>();
	Node->Options = InOptions;
	Node->Options.Filename = FPaths::GetBaseFilename(Node->Options.Filename);
	Node->bActive = false;
	Node->WorldContextObject = InWorldContextObject;
	// not garbage collected
	Node->AddToRoot();
	return Node;
}

UTexture2D* UFileHelperScreenshotAction::LoadScreenshot(const FString& InFilePath)
{
	return FImageUtils::ImportFileAsTexture2D(InFilePath);
}

void UFileHelperScreenshotAction::Activate()
{
	if (bActive)
	{
		FFrame::KismetExecutionMessage(TEXT("ScreenshotUtility is already running"), ELogVerbosity::Warning);
		OnTaskFailed();
		return;
	}

	if (!WorldContextObject || !WorldContextObject->GetWorld() || !WorldContextObject->GetWorld()->GetGameViewport())
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid WorldContextObject. Cannot execute ScreenshotUtility"), ELogVerbosity::Error);
		OnTaskFailed();
		return;
	}

	FText ErrorFilename;
	if (!FFileHelper::IsFilenameValidForSaving(Options.Filename, ErrorFilename))
	{
		FFrame::KismetExecutionMessage(TEXT("Filename is not valid"), ELogVerbosity::Warning);
		OnTaskFailed();
		return;
	}

	const FString FinalFilename = (Options.bPrefixTimestamp ? (FDateTime::Now().ToString(TEXT("%Y_%m_%d__%H_%M_%S__"))) : "") + Options.Filename;
	if (!FScreenshotRequest::IsScreenshotRequested())
	{
		bActive = true;
		ScreenshotTexture = nullptr;

		if (!Options.CustomCameraActor)
		{
			constexpr bool bAddFilenameSuffix = false;
			const FViewport* Viewport = WorldContextObject->GetWorld()->GetGameViewport()->Viewport;
			const bool bHDREnabled = Viewport->GetSceneHDREnabled();

			FScreenshotRequest::Reset();
			FScreenshotRequest::RequestScreenshot(FinalFilename, Options.bShowUI, bAddFilenameSuffix, bHDREnabled && Options.bWithHDR);

			FilePath = FScreenshotRequest::GetFilename();

			FScreenshotRequest::OnScreenshotRequestProcessed().RemoveAll(this);
			FScreenshotRequest::OnScreenshotRequestProcessed().AddUObject(this, &UFileHelperScreenshotAction::OnTaskCompleted);
		}
		else
		{
			FilePath = FPaths::ScreenShotDir() + FinalFilename + TEXT(".png");
			CreateCustomCameraScreenshot();
		}
	}
}

void UFileHelperScreenshotAction::OnTaskCompleted()
{
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	if (!ScreenshotTexture && FileManager.FileExists(*FilePath))
	{
		ScreenshotTexture = FImageUtils::ImportFileAsTexture2D(FilePath);
	}

	if (ScreenshotTexture)
	{
		Completed.Broadcast(ScreenshotTexture, FilePath);
	}
	else
	{
		OnTaskFailed();
	}

	Reset();
}

void UFileHelperScreenshotAction::OnTaskFailed()
{
	Reset();
	Failed.Broadcast(ScreenshotTexture, FilePath);
}

void UFileHelperScreenshotAction::CreateCustomCameraScreenshot()
{
	UWorld* World = WorldContextObject->GetWorld();
	if (!World || !World->GetGameViewport() || !World->GetGameViewport()->Viewport)
	{
		OnTaskFailed();
		return;
	}

	const ACameraActor* Camera = Options.CustomCameraActor;
	if (!Camera)
	{
		OnTaskFailed();
		return;
	}

	const UCameraComponent* CameraComponent = Camera->GetCameraComponent();
	if (!CameraComponent)
	{
		OnTaskFailed();
		return;
	}

	/*const APlayerCameraManager* PlayerCamera = UGameplayStatics::GetPlayerCameraManager(WorldContextObject, 0);
	if (!PlayerCamera)
	{
		OnTaskFailed();
		return;
	}*/

	const FViewport* GameViewport = World->GetGameViewport()->Viewport;
	const FIntRect ViewRect(0, 0, GameViewport->GetSizeXY().X, GameViewport->GetSizeXY().Y);
	const FVector CameraLocation = Camera->GetActorLocation();
	const FRotator CameraRotation = Camera->GetActorRotation();

	USceneCaptureComponent2D* SceneComponent = NewObject<USceneCaptureComponent2D>(this, TEXT("SceneComponent"));
	SceneComponent->RegisterComponentWithWorld(World);
	SceneComponent->bCaptureEveryFrame = false;
	SceneComponent->bCaptureOnMovement = false;
	SceneComponent->bAlwaysPersistRenderingState = true;
	SceneComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
	SceneComponent->FOVAngle = CameraComponent->FieldOfView;
	SceneComponent->ProjectionType = CameraComponent->ProjectionMode;
	SceneComponent->OrthoWidth = CameraComponent->OrthoWidth;
	SceneComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);

	UTextureRenderTarget2D* TextureRenderTarget = NewObject<UTextureRenderTarget2D>();
	TextureRenderTarget->InitCustomFormat(ViewRect.Width(),ViewRect.Height(),PF_B8G8R8A8,false);
	TextureRenderTarget->UpdateResourceImmediate();
	SceneComponent->TextureTarget = TextureRenderTarget;
	SceneComponent->CaptureScene();

	TArray<FColor> OutColors;
	OutColors.Reserve(ViewRect.Width() * ViewRect.Height());
	TextureRenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(OutColors);
	OutColors.Shrink();
	SceneComponent->UnregisterComponent();

	if (OutColors.Num() == 0)
	{
		OnTaskFailed();
		return;
	}

	TArray<uint8> OutImage;
	FImageUtils::ThumbnailCompressImageArray(ViewRect.Width(), ViewRect.Height(), OutColors, OutImage);

	if (OutImage.Num() == 0)
	{
		OnTaskFailed();
		return;
	}

	if (!FFileHelper::SaveArrayToFile(OutImage, *FilePath))
	{
		OnTaskFailed();
		return;
	}

	ScreenshotTexture = FImageUtils::ImportBufferAsTexture2D(OutImage);
	OnTaskCompleted();
}

void UFileHelperScreenshotAction::Reset()
{
	WorldContextObject = nullptr;
	ScreenshotTexture = nullptr;
	bActive = false;
	FilePath.Empty();
	FScreenshotRequest::Reset();
	FScreenshotRequest::OnScreenshotRequestProcessed().RemoveAll(this);
	RemoveFromRoot();
}
