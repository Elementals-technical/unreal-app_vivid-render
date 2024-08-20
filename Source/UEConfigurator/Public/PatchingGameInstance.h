// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PatchingGameInstance.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPatchCompleteDelegate, bool, Succeeded);

/**
 * 
 */
UCLASS()
class UECONFIGURATOR_API UPatchingGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
	

protected:
	//Tracks if our local manifest file is up to date with the one hosted on our website
	bool bIsDownloadManifestUpToDate;
	//Called when the chunk download process finishes
	void OnManifestUpdateComplete(bool bSuccess);
	// Called when the chunk download process finishes
	void OnDownloadComplete(bool bSuccess);
	void OnSingleChunkDownloadComplete(bool bSuccess);

	// Called whenever ChunkDownloader's loading mode is finished
	void OnLoadingModeComplete(bool bSuccess);

	// Called when ChunkDownloader finishes mounting chunks
	void OnMountComplete(bool bSuccess);
	void OnSingleChunkMountComplete(bool bSuccess);
	void UpdateManifestFile();
	void Log(const FString& text);

public:
	UFUNCTION(BlueprintPure, Category = "Patching|Stats")
	void GetLoadingProgress(int32& FilesDownloaded, int32& TotalFilesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

	// Delegates
	// Fired when the patching process succeeds or fails
	UPROPERTY(BlueprintAssignable, Category = "Patching");
	FPatchCompleteDelegate OnPatchComplete;
	UPROPERTY(BlueprintAssignable, Category = "Patching");
	FPatchCompleteDelegate OnChunkLoadComplete;
	UPROPERTY(BlueprintReadWrite, Category = "Patching");
	int32 ChunkToDownload;
	UFUNCTION(BlueprintCallable, Category = "Patching")
		void MountAll();
	// Starts the game patching process. Returns false if the patching manifest is not up to date. */
	UFUNCTION(BlueprintCallable, Category = "Patching")
		bool PatchGame();
	UFUNCTION(BlueprintCallable, Category = "Patching")
		bool LoadChunk();
	UFUNCTION(BlueprintCallable, Category = "Patching")
		bool IsChunkReady(int32 cID);

protected:
	// List of Chunk IDs to try and download
	UPROPERTY(BlueprintReadWrite, Category = "Patching")
		TArray<int32> ChunkDownloadList;
};
