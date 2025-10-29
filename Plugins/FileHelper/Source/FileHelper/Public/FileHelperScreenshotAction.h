// Copyright 2024 RLoris

#pragma once

#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "FileHelperScreenshotAction.generated.h"

USTRUCT(BlueprintType)
struct FFileHelperScreenshotActionOptions
{
	GENERATED_BODY()

	/** File name without extension or path information */
	UPROPERTY(BlueprintReadWrite, Category = "FileHelper|Screenshot")
	FString Filename;

	/** Prefix filename with a custom timestamp */
	UPROPERTY(BlueprintReadWrite, Category = "FileHelper|Screenshot")
	bool bPrefixTimestamp = true;

	/** Include the UI in the screenshot */
	UPROPERTY(BlueprintReadWrite, Category = "FileHelper|Screenshot")
	bool bShowUI = false;

	/**
	 * Uses this option only if the scene has HDR enabled,
	 * extension of screenshot file will be exr instead of png
	 */
	UPROPERTY(BlueprintReadWrite, Category = "FileHelper|Screenshot")
	bool bWithHDR = false;

	/**
	 * Leave this empty for default screenshot,
	 * a different type of screenshot will be taken with a render target if set,
	 * options and settings quality may differ from regular screenshot, no UI shown
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Screenshot")
	TObjectPtr<ACameraActor> CustomCameraActor = nullptr;
};

UCLASS()
class FILEHELPER_API UFileHelperScreenshotAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOutputPin, UTexture2D*, Screenshot, FString, Path);

	UPROPERTY(BlueprintAssignable)
	FOutputPin Completed;

	UPROPERTY(BlueprintAssignable)
	FOutputPin Failed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "InWorldContextObject", Keywords = "File plugin screenshot save load texture", ToolTip = "Take a screenshot, save and load it"), Category = "FileHelper|Screenshot")
	static UFileHelperScreenshotAction* TakeScreenshot(UObject* InWorldContextObject, const FFileHelperScreenshotActionOptions& InOptions);

	UFUNCTION(BlueprintCallable, meta = (Keywords = "screenshot load texture FileHelper", ToolTip = "Load a screenshot into a texture"), Category = "FileHelper|Screenshot")
	static UTexture2D* LoadScreenshot(const FString& InFilePath);

	virtual void Activate() override;

private:
	UFUNCTION()
	void OnTaskCompleted();

	void OnTaskFailed();

	void CreateCustomCameraScreenshot();

	void Reset();

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY()
	TObjectPtr<UTexture2D> ScreenshotTexture;

	UPROPERTY()
	FFileHelperScreenshotActionOptions Options;

	/** The file path of the new screenshot taken */
	FString FilePath;

	/** Is this node active */
	bool bActive = false;
};
