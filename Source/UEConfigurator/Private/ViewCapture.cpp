// ViewCapture.cpp
#include "ViewCapture.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include <lodepng.h>
#include <PNGHelper.h>
// Sets default values
AViewCapture::AViewCapture(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;
    // Create our SceneCaptureComponent2D and make it the root component of this Actor.
    Camera = ObjectInitializer.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("Camera"));
    SetRootComponent(Camera);
    
    // Make sure we don't capture every frame for performance, and because our render target will be made to be GC'd.
    Camera->bCaptureEveryFrame = false;
    // Set the right format to not deal with alpha issues
    Camera->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
    Camera->bUseRayTracingIfEnabled = true;
    Camera->bAlwaysPersistRenderingState = true;
    
    
}
bool AViewCapture::SetCameraToPlayerView()
{
    APlayerCameraManager* PlayerCamera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

    const FVector CameraLocation = PlayerCamera->GetCameraLocation();
    const FRotator CameraRotation = PlayerCamera->GetCameraRotation();

    SetActorLocationAndRotation(CameraLocation, CameraRotation);
    Camera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);

    Camera->FOVAngle = PlayerCamera->GetFOVAngle();

    return true;
}
bool AViewCapture::CapturePlayersView(int32 Resolution, TArray<FColor>& ColorData)
{
    // Make the resolution a power of two.
    Resolution = FGenericPlatformMath::Pow((float)2, (float)FGenericPlatformMath::FloorLog2(FGenericPlatformMath::Max(Resolution, 1) * 2 - 1));

    // Move our actor and its camera component to player's camera.
    if (!SetCameraToPlayerView()) return false;

    // Create a temporary object that we will let die in GC in a moment after this scope ends.
    UTextureRenderTarget2D* TextureRenderTarget = NewObject<UTextureRenderTarget2D>();
    TextureRenderTarget->InitCustomFormat(Resolution, Resolution, PF_B8G8R8A8, false);
    // Take the capture.
    Camera->TextureTarget = TextureRenderTarget;
    Camera->CaptureScene();
    // Output the capture to a pixel array.
    ColorData.Empty();
    ColorData.Reserve(Resolution * Resolution);
    TextureRenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(ColorData);
    ColorData.Shrink();

    return true;
}

void AViewCapture::CapturePlayersViewPNG(int32 Resolution)
{
    // Make the resolution a power of two.
    Resolution = FGenericPlatformMath::Pow((float)2, (float)FGenericPlatformMath::FloorLog2(FGenericPlatformMath::Max(Resolution, 1) * 2 - 1));

    // Move our actor and its camera component to player's camera.
    if (!SetCameraToPlayerView()) return;

    // Create a temporary object that we will let die in GC in a moment after this scope ends.
    UTextureRenderTarget2D* TextureRenderTarget = NewObject<UTextureRenderTarget2D>();
    TextureRenderTarget->InitCustomFormat(Resolution, Resolution, PF_B8G8R8A8, false);
    // Take the capture.
    Camera->TextureTarget = TextureRenderTarget;
    Camera->CaptureScene();
    // Output the capture to a pixel array.
    TArray<FColor> ColorData;
    TArray<uint8> PixelData;
    
    ColorData.Empty();
    ColorData.Reserve(Resolution * Resolution);
    TextureRenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(ColorData);
    ColorData.Shrink();

    
    for (int32 i = 0; i < ColorData.Num(); i++)
    {
        PixelData.Add(ColorData[i].R);
        PixelData.Add(ColorData[i].G);
        PixelData.Add(ColorData[i].B);
        PixelData.Add(ColorData[i].A);
    }
    //TArray<uint8> PNGData = PNGHelper::EncodeToPNG(PixelData, Resolution, Resolution);
}

// Function wrappers for Blueprint functions.
void AViewCapture::SetCameraToPlayerView(TEnumAsByte<EViewCaptureOutcomes>& Outcome)
{
    Outcome = SetCameraToPlayerView() ? Success : Failure;
}
void AViewCapture::CapturePlayersView(TEnumAsByte<EViewCaptureOutcomes>& Outcome, const int32 Resolution,
    TArray<FColor>& ColorData)
{
    Outcome = CapturePlayersView(Resolution, ColorData) ? Success : Failure;
}