// Fill out your copyright notice in the Description page of Project Settings.


#include "PatchingGameInstance.h"

#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include <EngineSharedPCH.h>

void UPatchingGameInstance::Init()
{
	Super::Init();
	// initialize the chunk downloader with chosen platform
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
	Downloader->Initialize("Windows", 8);
	UpdateManifestFile();
}

void UPatchingGameInstance::Shutdown()
{
	Super::Shutdown();
	// Shut down ChunkDownloader
	FChunkDownloader::Shutdown();
}

void UPatchingGameInstance::OnManifestUpdateComplete(bool bSuccess)
{
	bIsDownloadManifestUpToDate = bSuccess;
}

void UPatchingGameInstance::OnDownloadComplete(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("Download complete"));

		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		FJsonSerializableArrayInt DownloadedChunks;

		for (int32 ChunkID : ChunkDownloadList)
		{
			DownloadedChunks.Add(ChunkID);
		}

		//Mount the chunks
		TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
		Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

		OnPatchComplete.Broadcast(true);

	}
	else
	{

		UE_LOG(LogTemp, Display, TEXT("Load process failed"));

		// call the delegate
		OnPatchComplete.Broadcast(false);
	}
}

void UPatchingGameInstance::OnSingleChunkDownloadComplete(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("Download complete"));

		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();


		//Mount the chunks
		TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnSingleChunkMountComplete(bSuccess); };
		Downloader->MountChunk(ChunkToDownload, MountCompleteCallback);

		//OnPatchComplete.Broadcast(true);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Load process failed"));

		// call the delegate
		OnChunkLoadComplete.Broadcast(false);
	}
}

void UPatchingGameInstance::OnLoadingModeComplete(bool bSuccess)
{
	OnDownloadComplete(bSuccess);
}

void UPatchingGameInstance::OnMountComplete(bool bSuccess)
{
	OnPatchComplete.Broadcast(bSuccess);
}

void UPatchingGameInstance::OnSingleChunkMountComplete(bool bSuccess)
{
	Log("chunk mounted");
	OnChunkLoadComplete.Broadcast(bSuccess);
}

void UPatchingGameInstance::UpdateManifestFile()
{
	const FString DeploymentName = "paks";
	const FString ContentBuildId = "paks";
	// initialize the chunk downloader with chosen platform
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

	// load the cached build ID
	Downloader->LoadCachedBuild(DeploymentName);
	// update the build manifest file
	TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {
		bIsDownloadManifestUpToDate = bSuccess;
		if (bSuccess) {
			Log("Manifest is up to date");
		}
		else {
			Log("Manifest update failed");
		}
	};
	Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
	Log("Initialization done");
	
}

void UPatchingGameInstance::MountAll()
{
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

	TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {Log("Mount chunks callback"); };
	FJsonSerializableArrayInt DownloadedChunks;

	for (int32 ChunkID : ChunkDownloadList)
	{
		DownloadedChunks.Add(ChunkID);
	}
	//Mount the chunks
	Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);
}

void UPatchingGameInstance::Log(const FString& text)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, text);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *text);
}

void UPatchingGameInstance::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
{
	//Get a reference to ChunkDownloader
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

	//Get the loading stats struct
	FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

	//Get the bytes downloaded and bytes to download
	BytesDownloaded = LoadingStats.BytesDownloaded;
	TotalBytesToDownload = LoadingStats.TotalBytesToDownload;

	//Get the number of chunks mounted and chunks to download
	ChunksMounted = LoadingStats.ChunksMounted;
	TotalChunksToMount = LoadingStats.TotalChunksToMount;

	//Calculate the download and mount percent using the above stats
	DownloadPercent = (float)BytesDownloaded / (float)TotalBytesToDownload;
	MountPercent = (float)ChunksMounted / (float)TotalChunksToMount;
}

bool UPatchingGameInstance::PatchGame()
{
	// make sure the download manifest is up to date
	if (bIsDownloadManifestUpToDate)
	{
		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		// report current chunk status
		for (int32 ChunkID : ChunkDownloadList)
		{
			int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
			UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
		}

		TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnDownloadComplete(bSuccess); };
		Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

		// start loading mode
		TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {OnLoadingModeComplete(bSuccess); };
		Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
		return true;
	}

	// we couldn't contact the server to validate our manifest, so we can't patch
	UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));

	return false;

}

bool UPatchingGameInstance::LoadChunk()
{
	Log("Chunk requested: " + FString::FromInt(ChunkToDownload));

	//UpdateManifestFile();

	if (bIsDownloadManifestUpToDate) {
		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		//get chunk status
		FChunkDownloader::EChunkStatus chunkStatus = Downloader->GetChunkStatus(ChunkToDownload);

		/*Log(FString::FromInt(ChunkToDownload));*/
		Log(Downloader->ChunkStatusToString(chunkStatus));

		switch (chunkStatus) {

			//if chunk is cached, but not mounted
		case FChunkDownloader::EChunkStatus::Cached:
		{
			Log("chunk cached, mounting...");
			TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnSingleChunkMountComplete(bSuccess); };
			Downloader->MountChunk(ChunkToDownload, MountCompleteCallback);

			return false;
		}
		break;

		//all set
		case FChunkDownloader::EChunkStatus::Mounted:
			Log("chunk already mounted");
			return true;
			break;

		case FChunkDownloader::EChunkStatus::Remote:
		{
			Log("chunk found on server... downloading");
			TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnSingleChunkDownloadComplete(bSuccess); };
			Downloader->DownloadChunk(ChunkToDownload, DownloadCompleteCallback, 1);

			/*TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
			Downloader->MountChunk(ChunkToDownload, MountCompleteCallback);*/
			return false;
		}
		break;
		case FChunkDownloader::EChunkStatus::Partial:
		{
			Log("chunk downloaded partially");
			TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnSingleChunkDownloadComplete(bSuccess); };
			Downloader->DownloadChunk(ChunkToDownload, DownloadCompleteCallback, 1);

			/*TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
			Downloader->MountChunk(ChunkToDownload, MountCompleteCallback);*/
			return false;
		}
		break;
		case FChunkDownloader::EChunkStatus::Downloading:
			Log("downloading...");
			return false;
			break;
		default:
			Log("DAFAQ! Switch fall to default. trying to mount chunk...");

			TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnSingleChunkMountComplete(bSuccess); };
			Downloader->MountChunk(ChunkToDownload, MountCompleteCallback);
			return false;
		}
	}

	Log("manifest update failed");
	return false;
}

bool UPatchingGameInstance::IsChunkReady(int32 cID)
{
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

	//get chunk status
	FChunkDownloader::EChunkStatus chunkStatus = Downloader->GetChunkStatus(cID);
	return chunkStatus == FChunkDownloader::EChunkStatus::Mounted;
}
