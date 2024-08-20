// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class UECONFIGURATOR_API PNGHelper
{
public:
	PNGHelper();
	~PNGHelper();

public:
	static TArray<uint8> EncodeToPNG(const TArray<uint8>& Pixels, int Width, int Height, int CompressionLevel);
	static TArray<uint8> EncodeToPNGAndDownscale(const TArray<uint8>& Pixels, int Width, int Height, int NewWidth, int NewHeight, int CompressionLevel);
	static TArray<uint8> EncodeToJPEG(const TArray<uint8>& Pixels, int Width, int Height, int Quality);
	static TArray<uint8> EncodeToJPEGAndDownScale(const TArray<uint8>& Pixels, int Width, int Height, int NewWidth, int NewHeight, int Quality);
};

TArray<uint8> ResizeImage(const uint8* InputPixels, int InputWidth, int InputHeight, int NewWidth, int NewHeight, int ChannelsNum);
// TArray<uint8> ResizeImagePNG(const uint8* InputPixels, int InputWidth, int InputHeight, int NewWidth, int NewHeight);
