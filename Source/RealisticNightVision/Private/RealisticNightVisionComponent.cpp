// Copyright 2023, Maximiliam Berggren, All Rights Reserved.


#include "RealisticNightVisionComponent.h"
#include "Components/PostProcessComponent.h"
#include "UObject/UObjectGlobals.h"
#include <Net/UnrealNetwork.h>
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include <Engine/PostProcessVolume.h>
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"


URealisticNightVisionComponent::URealisticNightVisionComponent()
{

#if WITH_EDITOR // Only used for previewing in editor
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
#else
	PrimaryComponentTick.bCanEverTick = false;
#endif
	SetIsReplicated(true);
	PPComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"), true);

	//Safety Measure so preview is not active on play. Don't know why but it prevents crashes
	if (GetWorld() != nullptr && GetWorld()->WorldType != EWorldType::Editor)
		previewInEditor = false;
}

// Called every frame
void URealisticNightVisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//If in editor, allow for preview of NV
#if WITH_EDITOR
	if (GetWorld() == nullptr || GetWorld()->WorldType != EWorldType::Editor)
		return;

	if (previewInEditor)
	{
		//Create a post process volume if it doesn't exist
		if (!PPPreviewVolume)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.bHideFromSceneOutliner = true;
			SpawnInfo.bTemporaryEditorActor = true;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.ObjectFlags |= RF_Transient;
			PPPreviewVolume = GetWorld()->SpawnActor<APostProcessVolume>(SpawnInfo);
			PPPreviewVolume->Priority = 1000;
			PPPreviewVolume->BlendRadius = 0.0f;
			PPPreviewVolume->BlendWeight = 1.0f;
		}
		// Set the post process settings
		UpdateSettings();
		PPPreviewVolume->Settings = PPComponent->Settings;
	}
	else if (PPPreviewVolume) // If preview is not active, destroy the post process volume
	{
		GetWorld()->EditorDestroyActor(PPPreviewVolume, false);
		PPPreviewVolume = nullptr;
	}
#endif
}

void URealisticNightVisionComponent::UpdateSettings()
{
	// Set exposure to manual
	PPComponent->Settings.bOverride_AutoExposureMethod = true;
	PPComponent->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	PPComponent->Priority = 1000; //Make sure it's on top of everything

	//apply bloom
	PPComponent->Settings.bOverride_BloomIntensity = true;
	PPComponent->Settings.bOverride_BloomThreshold = true;
	PPComponent->Settings.bOverride_BloomSizeScale = true;
	PPComponent->Settings.BloomIntensity = BloomIntensity;
	PPComponent->Settings.BloomThreshold = BloomThreshold;
	PPComponent->Settings.BloomSizeScale = BloomSizeScale;
	//Apply Color
	PPComponent->Settings.bOverride_ColorSaturation = true;
	PPComponent->Settings.bOverride_ColorGain = true;
	PPComponent->Settings.bOverride_ColorOffset = true;
	PPComponent->Settings.ColorSaturation = ColorSaturation;
	PPComponent->Settings.ColorGain = ColorGain;
	PPComponent->Settings.ColorOffset = ColorOffset;
	//Apply Brightness
	PPComponent->Settings.bOverride_ColorGamma = true;
	PPComponent->Settings.bOverride_AutoExposureBias = true;
	PPComponent->Settings.ColorGamma = FLinearColor(Gamma, Gamma, Gamma, Luminance);
	PPComponent->Settings.AutoExposureBias = AutoExposureBias;
	//Apply Filmgrain
	PPComponent->Settings.bOverride_FilmGrainIntensity = true;
	PPComponent->Settings.bOverride_FilmGrainTexelSize = true;
	PPComponent->Settings.bOverride_FilmGrainTexture = true;
	PPComponent->Settings.bOverride_FilmGrainIntensityShadows = true;
	PPComponent->Settings.bOverride_FilmGrainIntensityMidtones = true;
	PPComponent->Settings.bOverride_FilmGrainIntensityHighlights = true;
	PPComponent->Settings.FilmGrainIntensity = FilmGrainIntensity;
	PPComponent->Settings.FilmGrainTexelSize = FilmGrainTexelSize;
	PPComponent->Settings.FilmGrainTexture = FilmGrainTexture;
	PPComponent->Settings.FilmGrainIntensityShadows = FilmGrainIntensityShadows;
	PPComponent->Settings.FilmGrainIntensityMidtones = FilmGrainIntensityMidtones;
	PPComponent->Settings.FilmGrainIntensityHighlights = FilmGrainIntensityHighlights;
	// Apply vignette
	PPComponent->Settings.bOverride_VignetteIntensity = true;
	PPComponent->Settings.VignetteIntensity = VignetteIntensity;
}

void URealisticNightVisionComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Check if a post process component exists and if not, create one
	PPComponent = GetOwner()->FindComponentByClass<UPostProcessComponent>();
	if (PPComponent)
		return;
	PPComponent = NewObject<UPostProcessComponent>(GetOwner());
	PPComponent->SetupAttachment(GetOwner()->GetRootComponent());
	PPComponent->CreationMethod = EComponentCreationMethod::Instance;
	PPComponent->RegisterComponent();

}

void URealisticNightVisionComponent::SetNV(bool enable)
{
	// Enable/Disable post proccessing
	isEnabled = enable;
	PPComponent->SetVisibility(enable);

	// If we are the local player, then we need to call the server to update the remote players
	if (IsLocalController())
	{
		OnNVSetLocalDelegate.Broadcast(enable);
		ServerRPCSetNV(enable);
	}
}

void URealisticNightVisionComponent::ServerRPCSetNV_Implementation(bool enable)
{
	MulticastRPCSetNV(enable);
}

void URealisticNightVisionComponent::MulticastRPCSetNV_Implementation(bool enable)
{
	if (IsLocalController())
		return;

	isEnabled = enable;
	OnNVSetRemoteDelegate.Broadcast(enable);

}

// Called when the game starts
void URealisticNightVisionComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateSettings();
	PPComponent->SetVisibility(isEnabled);

}




bool URealisticNightVisionComponent::IsLocalController() const
{
	const ENetMode NetMode = GetNetMode();
	const AActor* Owner = GetOwner();
	if (NetMode == NM_Standalone)
	{
		// Not networked.
		return true;
	}

	if (NetMode == NM_Client && Owner->GetLocalRole() == ROLE_AutonomousProxy)
	{
		// Networked client in control.
		return true;
	}

	if (Owner->GetRemoteRole() != ROLE_AutonomousProxy && Owner->GetLocalRole() == ROLE_Authority)
	{
		// Local authority in control.
		return true;
	}

	return false;
}