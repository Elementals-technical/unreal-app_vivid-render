// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "FrameGrabber.h"
#include "Engine/GameEngine.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "FrameGrabberActor.generated.h"

class FFrameGrabber;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGrabberCompleteDelegateSignature);

UCLASS()
class UECONFIGURATOR_API AFrameGrabberActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFrameGrabberActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool StartFrameGrab(int32 mask);

	UFUNCTION(BlueprintCallable)
	void StopFrameGrab();

	UFUNCTION(BlueprintCallable)
	void SetMaterialInstanceDynamic(UMaterialInstanceDynamic *MI);

	UFUNCTION(BlueprintCallable)
	void ReleaseFrameGrabber();

	UFUNCTION(BlueprintCallable, Category = "Grab")
	TArray<uint8> GetPNG();

	UFUNCTION(BlueprintCallable, Category = "Grab")
	void ClearMask();

	/*UFUNCTION(BlueprintCallable)
		void SetCallback(FGrabberCompleteDelegateSignature grabDelegate);*/

	// private:
	//	TArray<uint8> EncodeToPNG(const TArray<uint8>& Pixels, int Width, int Height);

public:
	UPROPERTY()
	UMaterialInstanceDynamic *MaterialInstanceDynamic;

	UPROPERTY(BlueprintReadOnly, Category = "Grabber")
	TArray<uint8> CaptureFrameData;

	/*UPROPERTY(BlueprintReadOnly, Category = "Grabber")*/
	TArray<uint8> PNGBinaryData;

	UPROPERTY()
	UTexture2D *CaptureFrameTexture;

	UPROPERTY(BlueprintAssignable, Category = "Grabber")
	FGrabberCompleteDelegateSignature GrabDelegateVariable;
	UPROPERTY(BlueprintAssignable, Category = "Grabber")
	FGrabberCompleteDelegateSignature OnMaskCaptured;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabber")
	int32 FinalImageWidth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabber")
	int32 FinalImageHeight;

	// UPROPERTY(EditAnywhere, Category = "Grabber")
	// 	UCurveBase* AlphaCurve;

	UPROPERTY(EditAnywhere, Category = "Grabber", meta = (ClampMin = "0", ClampMax = "255", UIMin = "0", UIMax = "255"))
	int32 AlphaMinTrashhold = 10;
	UPROPERTY(EditAnywhere, Category = "Grabber", meta = (ClampMin = "0", ClampMax = "255", UIMin = "0", UIMax = "255"))
	int32 AlphaMaxTrashhold = 200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabber", meta = (ClampMin = "0", ClampMax = "255", UIMin = "0", UIMax = "255"))
	int32 WarupMaskFrameCount = 16;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabber", meta = (ClampMin = "0", ClampMax = "255", UIMin = "0", UIMax = "255"))
	int32 WarupColorFrameCount = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabber", meta = (ClampMin = "1", ClampMax = "100", UIMin = "1", UIMax = "100"))
	int32 JPEGQuality = 75;

private:
	TSharedPtr<FFrameGrabber> FrameGrabber;
	// bool grab;
	void GetPNGData();
	void GetPNGDataNew();
	void GetMaskData();
	uint8 ApplyAlphaTrashhold(uint8 input);
	uint8 debugDisplayTime;
	uint8 bufferFrameCount;
	TArray<uint8> MaskPixelData;
	uint8 GrabberState = 0; // 0: Idle, 1: Mask, 2: Color Masked, 3: Color Opaque
};