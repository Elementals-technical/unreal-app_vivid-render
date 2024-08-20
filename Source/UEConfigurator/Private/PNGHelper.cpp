// Fill out your copyright notice in the Description page of Project Settings.


#include "PNGHelper.h"
// #include <ThirdParty/libPNG/libPNG-1.5.2/png.h>
// #include <ThirdParty/zlib/1.2.12/include/zlib.h>
// #include <ThirdParty/libjpeg-turbo/include/turbojpeg.h>
#include <libPNG/libPNG-1.5.2/png.h>
#include <zlib/1.2.12/include/zlib.h>
#include <libjpeg-turbo/include/turbojpeg.h>


PNGHelper::PNGHelper()
{
}

PNGHelper::~PNGHelper()
{
}

TArray<uint8> PNGHelper::EncodeToPNG(const TArray<uint8>& Pixels, int Width, int Height, int CompressionLevel)
{
	UE_LOG(LogTemp, Warning, TEXT("Using PNG encoder, includes changed!!!"));
	// Create an output buffer for the PNG-encoded data
	TArray<uint8> PNGData;

	// Initialize the libpng structures
	png_structp PNGPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!PNGPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize PNG write struct"));
		return PNGData;
	}

	png_infop InfoPtr = png_create_info_struct(PNGPtr);
	if (!InfoPtr)
	{
		png_destroy_write_struct(&PNGPtr, NULL);
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize PNG info struct"));
		return PNGData;
	}

	//// Set up error handling
	//if (setjmp(png_jmpbuf(PNGPtr)))
	//{
	//	png_destroy_write_struct(&PNGPtr, &InfoPtr);
	//	UE_LOG(LogTemp, Error, TEXT("Failed to encode PNG data"));
	//	return PNGData;
	//}

	// Set up the output buffer
	png_set_write_fn(PNGPtr, &PNGData, [](png_structp png_ptr, png_bytep data, png_size_t length)
		{
			TArray<uint8>* PNGDataPtr = reinterpret_cast<TArray<uint8>*>(png_get_io_ptr(png_ptr));
	PNGDataPtr->Append(data, length);
		}, NULL);

	// Set up the PNG header
	png_set_IHDR(PNGPtr, InfoPtr, Width, Height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Set the compression level
	png_set_compression_level(PNGPtr, CompressionLevel);

	// Write the PNG header
	png_write_info(PNGPtr, InfoPtr);

	// Write the PNG image data
	uint8* RowPtr = const_cast<uint8*>(Pixels.GetData());
	for (int32 Y = 0; Y < Height; ++Y)
	{
		png_write_row(PNGPtr, RowPtr);
		RowPtr += Width * sizeof(uint32);
	}

	// End the PNG write process
	png_write_end(PNGPtr, NULL);

	// Clean up the libpng structures
	png_destroy_write_struct(&PNGPtr, &InfoPtr);

	// Return the PNG-encoded data
	return PNGData;
}

TArray<uint8> PNGHelper::EncodeToPNGAndDownscale(const TArray<uint8>& Pixels, int Width, int Height, int NewWidth, int NewHeight, int CompressionLevel)
{
	UE_LOG(LogTemp, Warning, TEXT("Using PNG encoder!!!"));
	// Create an output buffer for the PNG-encoded data
	TArray<uint8> PNGData;

	// int NewWidth = Width / 2;
	// int NewHeight = Height / 2;

	// Resize the image
	TArray<uint8> ResizedPixels;
	ResizedPixels.SetNumUninitialized(NewWidth * NewHeight * 4); // Assuming 24-bit RGBA format
	ResizedPixels = ResizeImage(Pixels.GetData(), Width, Height, NewWidth, NewHeight, 4);

	// Initialize the libpng structures
	png_structp PNGPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!PNGPtr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize PNG write struct"));
		return PNGData;
	}

	png_infop InfoPtr = png_create_info_struct(PNGPtr);
	if (!InfoPtr)
	{
		png_destroy_write_struct(&PNGPtr, NULL);
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize PNG info struct"));
		return PNGData;
	}

	//// Set up error handling
	//if (setjmp(png_jmpbuf(PNGPtr)))
	//{
	//	png_destroy_write_struct(&PNGPtr, &InfoPtr);
	//	UE_LOG(LogTemp, Error, TEXT("Failed to encode PNG data"));
	//	return PNGData;
	//}

	// Set up the output buffer
	png_set_write_fn(PNGPtr, &PNGData, [](png_structp png_ptr, png_bytep data, png_size_t length)
		{
			TArray<uint8>* PNGDataPtr = reinterpret_cast<TArray<uint8>*>(png_get_io_ptr(png_ptr));
			PNGDataPtr->Append(data, length);
		}, NULL);

	// Set up the PNG header
	png_set_IHDR(PNGPtr, InfoPtr, NewWidth, NewHeight, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// Set the compression level
	png_set_compression_level(PNGPtr, CompressionLevel);

	// Write the PNG header
	png_write_info(PNGPtr, InfoPtr);

	// Write the PNG image data
	uint8* RowPtr = const_cast<uint8*>(ResizedPixels.GetData());
	for (int32 Y = 0; Y < NewHeight; ++Y)
	{
		png_write_row(PNGPtr, RowPtr);
		RowPtr += NewWidth * sizeof(uint32);
	}

	// End the PNG write process
	png_write_end(PNGPtr, NULL);

	// Clean up the libpng structures
	png_destroy_write_struct(&PNGPtr, &InfoPtr);

	// Return the PNG-encoded data
	return PNGData;
}

TArray<uint8> PNGHelper::EncodeToJPEG(const TArray<uint8>& Pixels, int Width, int Height, int Quality)
{
	UE_LOG(LogTemp, Warning, TEXT("Using JPEG encoder!!!"));
	//GEngine->AddOnScreenDebugMessage(13, 20, FColor::Cyan, FString::Printf(TEXT("incoming pixel data %d"), Pixels.Num()));
	// Create an output buffer for the JPEG-encoded data
	TArray<uint8> JPEGData;
	unsigned char* _compressedImage = NULL;
	long unsigned int _jpegSize = 0;

	// Initialize the libjpeg-turbo structures
	tjhandle CInfo = tjInitCompress();
	if (!CInfo)
	{
		//GEngine->AddOnScreenDebugMessage(10, 20, FColor::Cyan, TEXT("Failed to initialize libjpeg-turbo compression"));
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize libjpeg-turbo compression"));
		return JPEGData;
	}
	//GEngine->AddOnScreenDebugMessage(11, 20, FColor::Cyan, TEXT("libjpeg-turbo initialized successfully"));
	UE_LOG(LogTemp, Error, TEXT("libjpeg-turbo initialized successfully"));
	// Set up the JPEG compression parameters
	int32 Subsamp = TJSAMP_444;
	int32 Flags = TJFLAG_FASTDCT;
	int success = tjCompress2(CInfo, Pixels.GetData(), Width, 0, Height, TJPF_RGBA, &_compressedImage, &_jpegSize, Subsamp, Quality, Flags);
	//GEngine->AddOnScreenDebugMessage(15, 20, FColor::Cyan, FString::Printf(TEXT("_jpegSize var value %d"), _jpegSize));
	UE_LOG(LogTemp, Warning, TEXT("The integer value is: %d"), _jpegSize);
	
	if (success == 1) {
		FString s = tjGetErrorStr2(CInfo);
		//GEngine->AddOnScreenDebugMessage(14, 20, FColor::Cyan, s);
	}
	else {
		//GEngine->AddOnScreenDebugMessage(11, 20, FColor::Cyan, TEXT("no errors during compression"));
		JPEGData.Append(_compressedImage, _jpegSize);
	}


	// Clean up the libjpeg-turbo structures
	tjFree(_compressedImage);
	tjDestroy(CInfo);
	//GEngine->AddOnScreenDebugMessage(12, 20, FColor::Cyan, FString::Printf(TEXT("compressed jpeg data %d"), JPEGData.Num()));
	return JPEGData;
}

TArray<uint8> PNGHelper::EncodeToJPEGAndDownScale(const TArray<uint8>& Pixels, int Width, int Height, int NewWidth, int NewHeight, int Quality)
{
	UE_LOG(LogTemp, Warning, TEXT("Using JPEG encoder!!!"));
	//GEngine->AddOnScreenDebugMessage(13, 20, FColor::Cyan, FString::Printf(TEXT("incoming pixel data %d"), Pixels.Num()));
	// Create an output buffer for the JPEG-encoded data
	TArray<uint8> JPEGData;
	unsigned char* _compressedImage = NULL;
	long unsigned int _jpegSize = 0;

	// int NewWidth = Width / 2;
	// int NewHeight = Height / 2;
	

	// Initialize the libjpeg-turbo structures
	tjhandle CInfo = tjInitCompress();
	if (!CInfo)
	{
		//GEngine->AddOnScreenDebugMessage(10, 20, FColor::Cyan, TEXT("Failed to initialize libjpeg-turbo compression"));
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize libjpeg-turbo compression"));
		return JPEGData;
	}
	//GEngine->AddOnScreenDebugMessage(11, 20, FColor::Cyan, TEXT("libjpeg-turbo initialized successfully"));
	UE_LOG(LogTemp, Error, TEXT("libjpeg-turbo initialized successfully"));

	// Resize the image
	TArray<uint8> ResizedPixels;
	ResizedPixels.SetNumUninitialized(NewWidth * NewHeight * 3); // Assuming 24-bit RGB format
	ResizedPixels = ResizeImage(Pixels.GetData(), Width, Height, NewWidth, NewHeight, 3);

	


	// Set up the JPEG compression parameters
	int32 Subsamp = TJSAMP_444;
	int32 Flags = TJFLAG_FASTDCT;
	int success = tjCompress2(CInfo, ResizedPixels.GetData(), NewWidth, 0, NewHeight, TJPF_RGBA, &_compressedImage, &_jpegSize, Subsamp, Quality, Flags);
	//GEngine->AddOnScreenDebugMessage(15, 20, FColor::Cyan, FString::Printf(TEXT("_jpegSize var value %d"), _jpegSize));
	UE_LOG(LogTemp, Warning, TEXT("The integer value is: %d"), _jpegSize);

	if (success == 1) {
		FString s = tjGetErrorStr2(CInfo);
		//GEngine->AddOnScreenDebugMessage(14, 20, FColor::Cyan, s);
	}
	else {
		//GEngine->AddOnScreenDebugMessage(11, 20, FColor::Cyan, TEXT("no errors during compression"));
		JPEGData.Append(_compressedImage, _jpegSize);
	}


	// Clean up the libjpeg-turbo structures
	tjFree(_compressedImage);
	tjDestroy(CInfo);
	//GEngine->AddOnScreenDebugMessage(12, 20, FColor::Cyan, FString::Printf(TEXT("compressed jpeg data %d"), JPEGData.Num()));
	return JPEGData;
}

TArray<uint8> ResizeImage(const uint8* InputPixels, int InputWidth, int InputHeight, int NewWidth, int NewHeight, int ChannelsNum) {
	TArray<uint8> OutputPixels;
	OutputPixels.SetNumUninitialized(NewWidth * NewHeight * ChannelsNum); // Assuming RGB format

	// Calculate scaling factors
	float XScale = static_cast<float>(InputWidth) / NewWidth;
	float YScale = static_cast<float>(InputHeight) / NewHeight;

	for (int Y = 0; Y < NewHeight; ++Y) {
		for (int X = 0; X < NewWidth; ++X) {
			// Calculate corresponding position in the original image
			float SourceX = X * XScale;
			float SourceY = Y * YScale;

			// Get the four nearest neighbors
			int X0 = static_cast<int>(floor(SourceX));
			int X1 = X0 + 1;
			int Y0 = static_cast<int>(floor(SourceY));
			int Y1 = Y0 + 1;

			// Ensure we're within bounds
			X0 = FMath::Clamp(X0, 0, InputWidth - 1);
			X1 = FMath::Clamp(X1, 0, InputWidth - 1);
			Y0 = FMath::Clamp(Y0, 0, InputHeight - 1);
			Y1 = FMath::Clamp(Y1, 0, InputHeight - 1);

			// Bilinear interpolation
			float Alpha = SourceX - X0;
			float Beta = SourceY - Y0;

			for (int Channel = 0; Channel < ChannelsNum; ++Channel) { // RGB channels
				float InterpolatedValue = (1.0f - Alpha) * (1.0f - Beta) * InputPixels[(Y0 * InputWidth + X0) * ChannelsNum + Channel] +
					Alpha * (1.0f - Beta) * InputPixels[(Y0 * InputWidth + X1) * ChannelsNum + Channel] +
					(1.0f - Alpha) * Beta * InputPixels[(Y1 * InputWidth + X0) * ChannelsNum + Channel] +
					Alpha * Beta * InputPixels[(Y1 * InputWidth + X1) * ChannelsNum + Channel];

				// Clamp the result to [0, 255]
				OutputPixels[(Y * NewWidth + X) * ChannelsNum + Channel] = static_cast<uint8>(FMath::Clamp(InterpolatedValue, 0.0f, 255.0f));
			}
		}
	}

	return OutputPixels;
}

// TArray<uint8> ResizeImagePNG(const uint8* InputPixels, int InputWidth, int InputHeight, int NewWidth, int NewHeight) {
// 	TArray<uint8> OutputPixels;
// 	OutputPixels.SetNumUninitialized(NewWidth * NewHeight * 4); // Assuming RGB format

// 	// Calculate scaling factors
// 	float XScale = static_cast<float>(InputWidth) / NewWidth;
// 	float YScale = static_cast<float>(InputHeight) / NewHeight;

// 	for (int Y = 0; Y < NewHeight; ++Y) {
// 		for (int X = 0; X < NewWidth; ++X) {
// 			// Calculate corresponding position in the original image
// 			float SourceX = X * XScale;
// 			float SourceY = Y * YScale;

// 			// Get the four nearest neighbors
// 			int X0 = static_cast<int>(floor(SourceX));
// 			int X1 = X0 + 1;
// 			int Y0 = static_cast<int>(floor(SourceY));
// 			int Y1 = Y0 + 1;

// 			// Ensure we're within bounds
// 			X0 = FMath::Clamp(X0, 0, InputWidth - 1);
// 			X1 = FMath::Clamp(X1, 0, InputWidth - 1);
// 			Y0 = FMath::Clamp(Y0, 0, InputHeight - 1);
// 			Y1 = FMath::Clamp(Y1, 0, InputHeight - 1);

// 			// Bilinear interpolation
// 			float Alpha = SourceX - X0;
// 			float Beta = SourceY - Y0;

// 			for (int Channel = 0; Channel < 4; ++Channel) { // RGB channels
// 				float InterpolatedValue = (1.0f - Alpha) * (1.0f - Beta) * InputPixels[(Y0 * InputWidth + X0) * 4 + Channel] +
// 					Alpha * (1.0f - Beta) * InputPixels[(Y0 * InputWidth + X1) * 4 + Channel] +
// 					(1.0f - Alpha) * Beta * InputPixels[(Y1 * InputWidth + X0) * 4 + Channel] +
// 					Alpha * Beta * InputPixels[(Y1 * InputWidth + X1) * 4 + Channel];

// 				// Clamp the result to [0, 255]
// 				OutputPixels[(Y * NewWidth + X) * 4 + Channel] = static_cast<uint8>(FMath::Clamp(InterpolatedValue, 0.0f, 255.0f));
// 			}
// 		}
// 	}

// 	return OutputPixels;
// }

