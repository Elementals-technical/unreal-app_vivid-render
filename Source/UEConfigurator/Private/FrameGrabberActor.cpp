// Fill out your copyright notice in the Description page of Project Settings.

#include "FrameGrabberActor.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "IAssetViewport.h"
#endif

#include <PNGHelper.h>
#include "ImageUtils.h"

// Sets default values
AFrameGrabberActor::AFrameGrabberActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// grab = false;
	GrabberState = 0;
	debugDisplayTime = 20;
}

// Called when the game starts or when spawned
void AFrameGrabberActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] FrameGrabber constructor called"));
}

void AFrameGrabberActor::BeginDestroy()
{
	Super::BeginDestroy();

	ReleaseFrameGrabber();

	if (CaptureFrameTexture)
	{
		CaptureFrameTexture->ConditionalBeginDestroy();
		CaptureFrameTexture = nullptr;
	}
}

// Called every frame
void AFrameGrabberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GrabberState == 0)
	{
		return;
	}

	if (FrameGrabber.IsValid())
	{
		GetMaskData();
		GetPNGData();
	}
}

TArray<uint8> AFrameGrabberActor::GetPNG()
{
	return PNGBinaryData;
}

void AFrameGrabberActor::ClearMask()
{
	MaskPixelData.Empty();
}

// void AFrameGrabberActor::SetCallback()
//{
//	GrabDelegateVariable = grabDelegate;
// }

void AFrameGrabberActor::GetPNGDataNew()
{
	bool ApplyDownscale = false;
	if (GrabberState != 2 && GrabberState != 3)
	{
		return;
	}
	if (FrameGrabber.IsValid())
	{
		UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] FrameGrabber is valid"));
		// GEngine->AddOnScreenDebugMessage(1, debugDisplayTime, FColor::Cyan, TEXT("FrameGrabber is valid"));
		bufferFrameCount++;
		// hack for preventing returning old frames
		if (bufferFrameCount < WarupColorFrameCount)
		{
			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Skipping first 32 frames. Current count is %d"), bufferFrameCount);
			return;
		}

		FrameGrabber->CaptureThisFrame(FFramePayloadPtr());
		TArray<FCapturedFrameData> Frames = FrameGrabber->GetCapturedFrames();

		if (Frames.Num() > 0)
		{
			// PNGBinaryData.Empty();
			// GEngine->AddOnScreenDebugMessage(-1, debugDisplayTime, FColor::Cyan, FString::Printf(TEXT("[FrameGrabber] INFO: Frame count: %d"), Frames.Num()));
			FCapturedFrameData &LastFrame = Frames.Last();
			// TArray<uint8> PixelData;

			int32 Width = Frames.Last().BufferSize.X;
			int32 Height = Frames.Last().BufferSize.Y;
			TArray<FColor> resizedImage;
			// int32 NewWidth = Frames.Last().BufferSize.X / 2;
			// int32 NewHeight = Frames.Last().BufferSize.Y / 2;
			if (FinalImageWidth != Width && FinalImageHeight != Height)
			{
				ApplyDownscale = true;
				resizedImage.SetNumUninitialized(FinalImageWidth * FinalImageHeight);
				FImageUtils::ImageResize(Width, Height, LastFrame.ColorBuffer, FinalImageWidth, FinalImageHeight, resizedImage, false, true);
			}
			else
			{
				resizedImage = Frames.Last().ColorBuffer;
			}

			// from now on our buffer is stored in resizedImage;
			int32 pixelDataLegnth = resizedImage.Num();
			int32 maskDataLegnth = MaskPixelData.Num();

			if (GrabberState == 2 && pixelDataLegnth != maskDataLegnth)
			{
				UE_LOG(LogTemp, Error, TEXT("Mask and Color pixel count do not match! Color Num=%d, Mask Num=%d"), pixelDataLegnth, maskDataLegnth);
				GrabberState = 0;
				return;
			}

			for (int32 i = 0; i < pixelDataLegnth; i++)
			{
				// if(i % DebugFrameFrequency == 0){
				// 	UE_LOG(LogTemp, Warning, TEXT("Color of %dth pixel: R:%d, G:%d, B:%d, A:%d"), i, resizedImage[i].R, resizedImage[i].G, resizedImage[i].B, resizedImage[i].A);
				// }

				// PixelData.Add(resizedImage[i].R);
				// PixelData.Add(resizedImage[i].G);
				// PixelData.Add(resizedImage[i].B);
				resizedImage[i].A = GrabberState == 2 ? ApplyAlphaTrashhold(MaskPixelData[i]) : 255;

				// PixelData.Add(GrabberState == 2 ? ApplyAlphaTrashhold(MaskPixelData[i]) : 255);
			}

			// Create an output buffer for the PNG-encoded data
			// TArray<uint8> PNGData = PNGHelper::EncodeToPNG(PixelData, ApplyDownscale ? NewWidth : Width, ApplyDownscale ? NewHeight : Height, 6);
			TArray<uint8> PNGData;
			FImageUtils::CompressImageArray(FinalImageWidth, FinalImageHeight, resizedImage, PNGData);

			// we decided to use JPEG instead
			//  TArray<uint8> PNGData = PNGHelper::EncodeToJPEG(PixelData, Width, Height, 75);

			if (GrabDelegateVariable.IsBound())
			{
				bufferFrameCount = 0;
				// grab = false;
				GrabberState = 0;
				//				GEngine->AddOnScreenDebugMessage(-1, debugDisplayTime, FColor::Green, TEXT("[FrameGrabber] SUCESS: image will be sent to server"));
				UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Image grabbed, invoking callback"));
				PNGBinaryData = PNGData;
				GrabDelegateVariable.Broadcast();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber] Image grabbed, but callback invokation failed"));
			}
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber] No frames are captured"));
		// GEngine->AddOnScreenDebugMessage(6, debugDisplayTime, FColor::Cyan, TEXT("No frames are captured"));
	}
	UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber] Something wrong with frame grabber"));
	// GEngine->AddOnScreenDebugMessage(7, debugDisplayTime, FColor::Cyan, TEXT("Something wrong with frame grabber"));
	return;
}
void AFrameGrabberActor::GetPNGData()
{
	if (GrabberState != 2 && GrabberState != 3)
	{
		return;
	}
	if (FrameGrabber.IsValid())
	{
		bool ApplyDownscale = false;
		// GEngine->AddOnScreenDebugMessage(1, debugDisplayTime, FColor::Cyan, TEXT("FrameGrabber is valid"));
		bufferFrameCount++;



		// hack for preventing returning old frames
		if (bufferFrameCount < WarupColorFrameCount - 5)
		{
			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Totally skipping first %d frames. Current count is %d"), WarupColorFrameCount, bufferFrameCount);
			return;
		}


		FrameGrabber->CaptureThisFrame(FFramePayloadPtr());
		TArray<FCapturedFrameData> Frames = FrameGrabber->GetCapturedFrames();
		
		// hack for preventing returning old frames
		if (bufferFrameCount < WarupColorFrameCount)
		{
			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Last 5 with capture. Current count is %d"), WarupColorFrameCount, bufferFrameCount);
			return;
		}

		if (Frames.Num() > 0)
		{
			// GEngine->AddOnScreenDebugMessage(-1, debugDisplayTime, FColor::Cyan, FString::Printf(TEXT("[FrameGrabber] INFO: Frame count: %d"), Frames.Num()));
			FCapturedFrameData &LastFrame = Frames.Last();
			TArray<uint8> PixelData;

			int32 Width = Frames.Last().BufferSize.X;
			int32 Height = Frames.Last().BufferSize.Y;
			TArray<FColor> resizedImage;
			// int32 NewWidth = Frames.Last().BufferSize.X / 2;
			// int32 NewHeight = Frames.Last().BufferSize.Y / 2;
			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Need resizing?"));
			if (FinalImageWidth != Width && FinalImageHeight != Height)
			{
				UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Yes, starting resize"));
				ApplyDownscale = true;
				resizedImage.SetNumUninitialized(FinalImageWidth * FinalImageHeight);
				FImageUtils::ImageResize(Width, Height, LastFrame.ColorBuffer, FinalImageWidth, FinalImageHeight, resizedImage, false, true);
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] No need to resize"));
				resizedImage = Frames.Last().ColorBuffer;
			}

			// from now on our buffer is stored in resizedImage;
			int32 pixelDataLegnth = resizedImage.Num();
			int32 maskDataLegnth = MaskPixelData.Num();

			if (GrabberState == 2 && pixelDataLegnth != maskDataLegnth)
			{
				UE_LOG(LogTemp, Error, TEXT("Mask and Color pixel count do not match! Color Num=%d, Mask Num=%d"), pixelDataLegnth, maskDataLegnth);
				GrabberState = 0;
				return;
			}

			for (int32 i = 0; i < pixelDataLegnth; i++)
			{
				// if(i % DebugFrameFrequency == 0){
				// 	UE_LOG(LogTemp, Warning, TEXT("Color of %dth pixel: R:%d, G:%d, B:%d, A:%d"), i, resizedImage[i].R, resizedImage[i].G, resizedImage[i].B, resizedImage[i].A);
				// }

				PixelData.Add(resizedImage[i].R);
				PixelData.Add(resizedImage[i].G);
				PixelData.Add(resizedImage[i].B);

				PixelData.Add(GrabberState == 2 ? ApplyAlphaTrashhold(MaskPixelData[i]) : 255);
			}

			TArray<uint8> PNGData;

			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] encoding to..."));

			// if (GrabberState == 3)
			// {
			// 	UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] JPEG"));
			// 	// we decided to use JPEG instead
			// 	PNGData = PNGHelper::EncodeToJPEG(PixelData, FinalImageWidth, FinalImageHeight, JPEGQuality);
			// }
			// else
			// {
			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] PNG"));
			// Create an output buffer for the PNG-encoded data
			PNGData = PNGHelper::EncodeToPNG(PixelData, FinalImageWidth, FinalImageHeight, 6);
			// }

			// we decided to use JPEG instead
			//  TArray<uint8> PNGData = PNGHelper::EncodeToJPEG(PixelData, Width, Height, 75);

			if (GrabDelegateVariable.IsBound())
			{
				bufferFrameCount = 0;
				// grab = false;
				GrabberState = 0;
				//				GEngine->AddOnScreenDebugMessage(-1, debugDisplayTime, FColor::Green, TEXT("[FrameGrabber] SUCESS: image will be sent to server"));
				UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Image grabbed, invoking callback"));
				PNGBinaryData = PNGData;
				GrabDelegateVariable.Broadcast();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber] Image grabbed, but callback invokation failed"));
			}
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber] No frames are captured"));
		// GEngine->AddOnScreenDebugMessage(6, debugDisplayTime, FColor::Cyan, TEXT("No frames are captured"));
	}
	UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber] Something wrong with frame grabber"));
	// GEngine->AddOnScreenDebugMessage(7, debugDisplayTime, FColor::Cyan, TEXT("Something wrong with frame grabber"));
	return;
}

void AFrameGrabberActor::GetMaskData()
{
	if (GrabberState != 1)
	{
		return;
	}
	if (FrameGrabber.IsValid())
	{
		bool ApplyDownscale = false;
		
		bufferFrameCount++;
		

		if (bufferFrameCount < WarupMaskFrameCount - 5)
		{
			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Mask. Totally skipping first %d frames. Current count is %d"), WarupMaskFrameCount, bufferFrameCount);
			return;
		}


		FrameGrabber->CaptureThisFrame(FFramePayloadPtr());
		TArray<FCapturedFrameData> Frames = FrameGrabber->GetCapturedFrames();
		
		
		if (bufferFrameCount < WarupMaskFrameCount)
		{
			UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Mask. Last 5 with capture. Current count is %d"), WarupMaskFrameCount, bufferFrameCount);
			return;
		}

		UE_LOG(LogTemp, Display, TEXT("[FrameGrabber-Mask] Creating a mask... FrameGrabber is valid"));

		if (Frames.Num() > 0)
		{
			MaskPixelData.Empty();
			// GEngine->AddOnScreenDebugMessage(-1, debugDisplayTime, FColor::Cyan, FString::Printf(TEXT("[FrameGrabber] INFO: Frame count: %d"), Frames.Num()));
			FCapturedFrameData &LastFrame = Frames.Last();

			int32 Width = Frames.Last().BufferSize.X;
			int32 Height = Frames.Last().BufferSize.Y;
			TArray<FColor> resizedImage;
			// int32 NewWidth = Frames.Last().BufferSize.X / 2;
			// int32 NewHeight = Frames.Last().BufferSize.Y / 2;
			if (FinalImageWidth != Width && FinalImageHeight != Height)
			{
				ApplyDownscale = true;
				resizedImage.SetNumUninitialized(FinalImageWidth * FinalImageHeight);
				FImageUtils::ImageResize(Width, Height, LastFrame.ColorBuffer, FinalImageWidth, FinalImageHeight, resizedImage, false, true);
			}
			else
			{
				resizedImage = Frames.Last().ColorBuffer;
			}

			// from now on our buffer is stored in resizedImage;
			int32 pixelDataLegnth = resizedImage.Num();

			for (int32 i = 0; i < pixelDataLegnth; i++)
			{

				// if(i % DebugFrameFrequency == 0){
				// 	UE_LOG(LogTemp, Warning, TEXT("Color of %dth pixel: R:%d, G:%d, B:%d, A:%d"), i, resizedImage[i].R, resizedImage[i].G, resizedImage[i].B, resizedImage[i].A);
				// }
				// assuming image is black and white
				MaskPixelData.Add((resizedImage[i].R + resizedImage[i].G + resizedImage[i].B) / 3);

				// MaskPixelData.Add(resizedImage[i].G);
				// MaskPixelData.Add(resizedImage[i].B);
				// MaskPixelData.Add(resizedImage[i].A);
				// PixelData.Add((resizedImage[i].R < 3 && resizedImage[i].G < 3 && resizedImage[i].B < 3) ? 0 : resizedImage[i].A);
			}

			if (OnMaskCaptured.IsBound())
			{
				bufferFrameCount = 0;
				GrabberState = 0;

				UE_LOG(LogTemp, Display, TEXT("[FrameGrabber-Mask] Mask has been created, invoking callback"));

				OnMaskCaptured.Broadcast();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber-Mask] Failed to invoke callback after mask creation"));
			}
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber-Mask] No frames are captured"));
		// GEngine->AddOnScreenDebugMessage(6, debugDisplayTime, FColor::Cyan, TEXT("No frames are captured"));
	}
	UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber-Mask] Something's wrong with frame grabber"));
	// GEngine->AddOnScreenDebugMessage(7, debugDisplayTime, FColor::Cyan, TEXT("Something wrong with frame grabber"));
	return;
}

uint8 AFrameGrabberActor::ApplyAlphaTrashhold(uint8 input)
{
	if (input < AlphaMinTrashhold)
	{
		return 0;
	}
	if (input > AlphaMaxTrashhold)
	{
		return 255;
	}
	return input;
}

bool AFrameGrabberActor::StartFrameGrab(int32 mask)
{
	UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Start frame grab"));
	// GEngine->AddOnScreenDebugMessage(21, debugDisplayTime, FColor::Cyan, TEXT("Start frame grab"));
	GrabberState = mask;
	bufferFrameCount = 0;
	TSharedPtr<FSceneViewport> SceneViewport;

	// Get SceneViewport
	// ( quoted from FRemoteSessionHost::OnCreateChannels() )
#if WITH_EDITOR
	if (GIsEditor)
	{
		for (const FWorldContext &Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE)
			{
				FSlatePlayInEditorInfo *SlatePlayInEditorSession = GEditor->SlatePlayInEditorMap.Find(Context.ContextHandle);
				if (SlatePlayInEditorSession)
				{
					if (SlatePlayInEditorSession->DestinationSlateViewport.IsValid())
					{
						TSharedPtr<IAssetViewport> DestinationLevelViewport = SlatePlayInEditorSession->DestinationSlateViewport.Pin();
						SceneViewport = DestinationLevelViewport->GetSharedActiveViewport();
					}
					else if (SlatePlayInEditorSession->SlatePlayInEditorWindowViewport.IsValid())
					{
						SceneViewport = SlatePlayInEditorSession->SlatePlayInEditorWindowViewport;
					}
				}
			}
		}
	}
	else
#endif
	{
		UGameEngine *GameEngine = Cast<UGameEngine>(GEngine);
		SceneViewport = GameEngine->SceneViewport;
	}
	if (!SceneViewport.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[FrameGrabber] Scene viewport is invalid"));
		// GEngine->AddOnScreenDebugMessage(8, debugDisplayTime, FColor::Cyan, TEXT("Scene viewport is invalid"));
		return false;
	}

	// Setup Texture
	/*if (!CaptureFrameTexture)
	{
		CaptureFrameTexture = UTexture2D::CreateTransient(SceneViewport.Get()->GetSize().X, SceneViewport.Get()->GetSize().Y, PF_B8G8R8A8);
		CaptureFrameTexture->UpdateResource();

		MaterialInstanceDynamic->SetTextureParameterValue(FName("Texture"), CaptureFrameTexture);
	}*/

	// Capture Start
	ReleaseFrameGrabber();
	FrameGrabber = MakeShareable(new FFrameGrabber(SceneViewport.ToSharedRef(), SceneViewport->GetSize()));
	FrameGrabber->StartCapturingFrames();

	UE_LOG(LogTemp, Display, TEXT("[FrameGrabber] Frame grabber initialized"));
	// GEngine->AddOnScreenDebugMessage(9, debugDisplayTime, FColor::Cyan, TEXT("Frame grabber initialized"));

	return true;
}

void AFrameGrabberActor::StopFrameGrab()
{
	ReleaseFrameGrabber();
}

void AFrameGrabberActor::ReleaseFrameGrabber()
{
	if (FrameGrabber.IsValid())
	{
		FrameGrabber->StopCapturingFrames();
		FrameGrabber->Shutdown();
		FrameGrabber.Reset();
	}
}

void AFrameGrabberActor::SetMaterialInstanceDynamic(UMaterialInstanceDynamic *MI)
{
	MaterialInstanceDynamic = MI;
}