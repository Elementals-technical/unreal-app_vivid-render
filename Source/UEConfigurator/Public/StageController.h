// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LevelStreamingDynamic.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StageController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStageControllerLoadingDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UECONFIGURATOR_API UStageController : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStageController();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:

	UPROPERTY(BlueprintAssignable, Category="StageController")
	FLevelStreamingLoadedStatus OnStageLoaded;
	

	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<UWorld> CastObjectToWorld(UObject* input) const;
	UFUNCTION(BlueprintCallable)
	void LoadStage(UObject* input, int32 stageId);
	UFUNCTION(BlueprintCallable)
	void UnloadStage();
	UFUNCTION()
	void OnLevelLoadingComplete();
	UFUNCTION()
	void OnLevelUnloaded();

private:
	ULevelStreamingDynamic* Stage = nullptr;
	int32 latestStageId = -1;

	

};
