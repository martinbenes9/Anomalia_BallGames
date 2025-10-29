// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnoamliaNodesBPLibrary.h"
#include "AnoamliaNodes.h"

UAnoamliaNodesBPLibrary::UAnoamliaNodesBPLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

FGameplayTagContainer UAnoamliaNodesBPLibrary::GetWorldTags(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return FGameplayTagContainer();

    UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
    if (!World) return FGameplayTagContainer();

    ACustomWorldSettings* CustomSettings = Cast<ACustomWorldSettings>(World->GetWorldSettings());
    if (CustomSettings)
    {
        return CustomSettings->LevelTags;
    }

    return FGameplayTagContainer();
}