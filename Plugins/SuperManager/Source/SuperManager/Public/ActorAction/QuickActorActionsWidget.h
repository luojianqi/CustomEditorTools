// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidget.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class E_DuplicationAxis :uint8
{
	EDA_XAxis UMETA(DisplayName = "X 轴"),
	EDA_YAxis UMETA(DisplayName = "Y 轴"),
	EDA_ZAxis UMETA(DisplayName = "Z 轴"),
	EDA_DefaultMAX UMETA(DisplayName = "DefaultMax"),
};

USTRUCT(BlueprintType)
struct FRandomActorRotation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation")
	bool bRandomizeRotYaw = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation",meta=(EditCondition = "bRandomizeRotYaw"))
	float RotYawMin = -45.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation",meta=(EditCondition = "bRandomizeRotYaw"))
	float RotYawMax = 45.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation")
	bool bRandomizeRotPitch = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation",meta=(EditCondition = "bRandomizeRotPitch"))
	float RotPitchMin = -45.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation",meta=(EditCondition = "bRandomizeRotPitch"))
	float RotPitchMax = 45.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation")
	bool bRandomizeRotRoll = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation",meta=(EditCondition = "bRandomizeRotRoll"))
	float RotRollMin = -45.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomActorRotation",meta=(EditCondition = "bRandomizeRotRoll"))
	float RotRollMax = 45.f;
};
UCLASS()
class SUPERMANAGER_API UQuickActorActionsWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:
	
#pragma region ActorBatchSelection
	
	UFUNCTION(BlueprintCallable,Category="ActorBatchSelection")
	void SelectAllActorsWithSimilarName(); //选择相似名字的Actor

	//是否忽略大小写
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ActorBatchSelection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::IgnoreCase;
#pragma endregion
	
#pragma region ActorBatchDuplication  //批量复制Actor

	UFUNCTION(BlueprintCallable,Category="ActorBatchDuplication")
	void DuplicateActors();

	//哪个方向
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ActorBatchDuplication")
	E_DuplicationAxis AxisForDuplication = E_DuplicationAxis::EDA_DefaultMAX;

	//复制多少个
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ActorBatchDuplication")
	int32 NumberOfDuplicates = 5;

	//偏移
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ActorBatchDuplication")
	float offsetDist = 300.f;
	
#pragma endregion

#pragma region RandomizeActorTransform

	UFUNCTION(BlueprintCallable,Category="RandomizeActorTransform")
	void RandomizeActorTransform();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomizeActorTransform")
	FRandomActorRotation RandomActorRotation;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomizeActorTransform")
	bool bRandomizeScale = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomizeActorTransform",meta=(EditCondition = "bRandomizeScale"))
	float ScaleMin = .8f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomizeActorTransform",meta=(EditCondition = "bRandomizeScale"))
	float ScaleMax = .8f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomizeActorTransform")
	bool bRandomizeOffset = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomizeActorTransform",meta=(EditCondition = "bRandomizeOffset"))
	float OffsetMin = -50.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RandomizeActorTransform",meta=(EditCondition = "bRandomizeOffset"))
	float OffsetMax = 50.f;
	
#pragma endregion
	
private:
	UPROPERTY()
	class UEditorActorSubsystem* EditorActorSubsystem;

	bool GetEditorActorSubsystem();
};
