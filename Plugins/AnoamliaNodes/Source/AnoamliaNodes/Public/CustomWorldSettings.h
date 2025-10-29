// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "GameplayTagContainer.h"
#include "CustomWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class ANOAMLIANODES_API ACustomWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	// Editable Tag container for metadata
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FGameplayTagContainer LevelTags;
	
};
