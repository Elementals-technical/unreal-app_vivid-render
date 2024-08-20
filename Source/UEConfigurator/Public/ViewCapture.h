// ViewCapture.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ViewCapture.generated.h"

// An enumeration for execution pins in Blueprint functions
UENUM(BlueprintType)
enum EViewCaptureOutcomes
{
    Failure,
    Success
};

UCLASS()
class UECONFIGURATOR_API AViewCapture : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    //AViewCapture();
    // Sets default values for this actor's properties
    AViewCapture(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "Outcome", Category = "View Capture", ToolTip = "Align the camera of this View Capture actor with the player's camera."))
        void SetCameraToPlayerView(TEnumAsByte<EViewCaptureOutcomes>& Outcome);
    bool SetCameraToPlayerView();

    UFUNCTION(BlueprintCallable, meta = (ExpandEnumAsExecs = "Outcome", Category = "View Capture", ToolTip = "Capture the player's view.\n\nResolution - a power of 2 resolution for the view capture, like 512"))
        void CapturePlayersView(TEnumAsByte<EViewCaptureOutcomes>& Outcome, int32 Resolution, TArray<FColor>& ColorData);
    bool CapturePlayersView(int32 Resolution, TArray<FColor>& ColorData);

    UFUNCTION(BlueprintCallable, meta = (Category = "View Capture PNG", ToolTip = "Capture the player's view and converts it to PNG.\n\nResolution - a power of 2 resolution for the view capture, like 512"))
        void CapturePlayersViewPNG(int32 Resolution);

    // The pointer to our "Camera" USceneCaptureComponent2D. 
    UPROPERTY(EditAnywhere, Transient)
    class USceneCaptureComponent2D* Camera;

    UPROPERTY(EditAnywhere, Transient)
        class UTextureRenderTarget2D* RenderTexture;
   
};