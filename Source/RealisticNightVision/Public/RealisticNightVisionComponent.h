// Copyright 2023, Maximiliam Berggren, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "RealisticNightVisionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNVSetLocalDelegate, bool, enable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNVSetRemoteDelegate, bool, enable);



UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class REALISTICNIGHTVISION_API URealisticNightVisionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URealisticNightVisionComponent();
private:
	class APostProcessVolume* PPPreviewVolume; // Only used for previewing in editor
protected:

	UPROPERTY(BlueprintReadWrite, Category = "Night Vision")
		class UPostProcessComponent* PPComponent;

	UPROPERTY(EditAnywhere, BlueprintSetter = SetNV, Category = "Night Vision")
		bool isEnabled = false;
	UPROPERTY(EditInstanceOnly, DuplicateTransient, Category = "Night Vision")
		bool previewInEditor = false;

	//Delegate for when the local player sets the NV
	UPROPERTY(BlueprintAssignable, Category = "Night Vision")
		FOnNVSetLocalDelegate OnNVSetLocalDelegate;
	//Delegate for when the remote player sets the NV
	UPROPERTY(BlueprintAssignable, Category = "Night Vision")
		FOnNVSetRemoteDelegate OnNVSetRemoteDelegate;

	// Sets the NV on the local player
	UFUNCTION(BlueprintSetter)
		void SetNV(bool enable);
	// Sets the NV on the server
	UFUNCTION(Server, Reliable)
		void ServerRPCSetNV(bool enable);
	// Sets the NV on the remote player
	UFUNCTION(NetMulticast, Reliable)
		void MulticastRPCSetNV(bool enable);

	// The settings for the post process component
	//Bloom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Bloom", meta = (UIMin = "0", UIMax = "10.0"))
		float BloomIntensity = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Bloom", meta = (UIMin = "-1.0", UIMax = "1.0"))
		float BloomThreshold = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Bloom", meta = (UIMin = "0.0", UIMax = "10.0"))
		float BloomSizeScale = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Color")
		FLinearColor ColorSaturation = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	/** The color gained in bright areas of the screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Color")
		FLinearColor ColorGain = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	/** The color offset in dark areas of the screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Color")
		FLinearColor ColorOffset = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
	//Brightness
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Brightness", meta = (DisplayName = "Exposure", UIMin = "15.0", UIMax = "30.0"))
		float AutoExposureBias = 22.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Brightness", meta = (DisplayName = "Brightness", UIMin = "0.0", UIMax = "2.0"))
		float Gamma = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Brightness", meta = (DisplayName = "Power", UIMin = "0.0", UIMax = "2.0"))
		float Luminance = 1.0f;
	//Film Grain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Film Grain", meta = (UIMin = "0.0", UIMax = "1.0"))
		float FilmGrainIntensity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Film Grain", meta = (UIMin = "0.0", UIMax = "1.0"))
		float FilmGrainIntensityShadows = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Film Grain", meta = (UIMin = "0.0", UIMax = "1.0"))
		float FilmGrainIntensityMidtones = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Film Grain", meta = (UIMin = "0.0", UIMax = "1.0"))
		float FilmGrainIntensityHighlights = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Film Grain", meta = (DisplayName = "Film Grain Size", UIMin = "0.0", UIMax = "10.0"))
		float FilmGrainTexelSize = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Film Grain")
		class UTexture2D* FilmGrainTexture = nullptr;
	// Vigette
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Night Vision|Settings|Vignette", meta = (UIMin = "0.0", UIMax = "5.0"))
		float VignetteIntensity = 0.0f;


	// Apply the settings to the post process component
	UFUNCTION(BlueprintCallable, Category = "Night Vision")
		void UpdateSettings();

	void InitializeComponent();

	virtual void BeginPlay() override;
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	// Check if this component is attached to the local player
	bool IsLocalController() const;

};
