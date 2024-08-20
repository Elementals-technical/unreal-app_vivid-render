// Fill out your copyright notice in the Description page of Project Settings.

#include "StageController.h"

// Sets default values for this component's properties
UStageController::UStageController()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UStageController::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

// Called every frame
void UStageController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

TSoftObjectPtr<UWorld> UStageController::CastObjectToWorld(UObject *input) const
{
	return TSoftObjectPtr<UWorld>(Cast<UWorld>(input));
}

void UStageController::LoadStage(UObject *input, int32 stageId)
{
	UE_LOG(LogTemp, Warning, TEXT("Loading stage %d called!"), stageId);
	if(stageId == latestStageId){
		OnLevelLoadingComplete();
		return;
	}

	if(latestStageId > 0 && Stage){
		UE_LOG(LogTemp, Error, TEXT("Previous stage was different. Unloading..."));
		UnloadStage();
	}

	TSoftObjectPtr<UWorld> stageSoftPtr = CastObjectToWorld(input);
	// Get a reference to the WorldContextObject (for example, a player controller or the game mode)
	UObject *WorldContextObject = GetWorld();

	// Define the soft reference to the level you want to load
	// TSoftObjectPtr<UWorld> LevelSoftObjectPtr("/Game/Maps/YourLevel");

	// Define the location and rotation where you want to load the level
	FVector Location(0, 0, 0);
	FRotator Rotation(0, 0, 0);

	// Output variable to store whether the level loading was successful
	bool bOutSuccess = false;

	// // Optionally, you can provide a level name override
	// const FString OptionalLevelNameOverride = "YourLevelOverride";

	// Load the level instance asynchronously
	Stage = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		WorldContextObject,
		stageSoftPtr,
		Location,
		Rotation,
		bOutSuccess
		// OptionalLevelNameOverride
	);

	if (bOutSuccess && Stage)
	{
		latestStageId = stageId;
		Stage->OnLevelLoaded.AddDynamic(this, &UStageController::OnLevelLoadingComplete);
		// Stage->OnLevelUnloaded.AddDynamic(this, &UStageController::OnLevelUnloaded);

		// Level loading was successful
		UE_LOG(LogTemp, Warning, TEXT("Level loaded successfully!"));
		UE_LOG(LogTemp, Warning, TEXT("Latest stageId is %d"), latestStageId);
	}
	else
	{
		// Level loading failed
		UE_LOG(LogTemp, Error, TEXT("Failed to load level!"));
	}
}

void UStageController::UnloadStage()
{
	if (Stage)
	{
		Stage->OnLevelLoaded.Clear();
		Stage->SetIsRequestingUnloadAndRemoval(true);
	}
}

void UStageController::OnLevelLoadingComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("OnLevelLoadingComplete"));
	UE_LOG(LogTemp, Warning, TEXT("Latest stageId is %d"), latestStageId);
	if (OnStageLoaded.IsBound())
	{
		OnStageLoaded.Broadcast();
	}
}

void UStageController::OnLevelUnloaded()
{
	UE_LOG(LogTemp, Warning, TEXT("Level unloaded. Latest stageId is %d"), latestStageId);
}
