// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "Blueprint/UserWidget.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "QuickAssetAction.generated.h"

/**
 *  快速进行资产操作类  ：：右键蓝图（脚本化资产行为）
 *
 *  UEditorUtilityLibrary  //编辑器实用程序库
 *  
*		// Gets the set of currently selected assets
		UFUNCTION(BlueprintCallable, Category = "Development|Editor")
		static TArray<UObject*> GetSelectedAssets();

		UFUNCTION(BlueprintCallable, Category = "Development|Editor")
		static TArray<FAssetData> GetSelectedAssetData();
 *
 *
 *
 *  
 *  UEditorAssetLibrary   //编辑器资产库
*		UFUNCTION(BlueprintCallable, Category = "Editor Scripting | Asset")
		static TArray<FString> ListAssets(const FString& DirectoryPath, bool bRecursive = true, bool bIncludeFolder = false);
 */

UCLASS()
class SUPERMANAGER_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	//复制资产
	UFUNCTION(CallInEditor)
	void DuplicateAssets(int32 NumOfDuplicates);

	//增加资产前缀
	UFUNCTION(CallInEditor)
	void AddPrefixes();

	//删除没有使用资产
	UFUNCTION(CallInEditor)
	void RemoveUnusedAssets();

private:
	TMap<UClass*, FString> PrefixMap =
	{
		{UBlueprint::StaticClass(),TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(),TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(),TEXT("MF_")},

		{UParticleSystem::StaticClass(),TEXT("PS_")},
		{USoundCue::StaticClass(),TEXT("SC_")},
		{USoundWave::StaticClass(),TEXT("SW_")},
		{UTexture::StaticClass(),TEXT("T_")},
		{UTexture2D::StaticClass(),TEXT("T_")},

		{UUserWidget::StaticClass(),TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(),TEXT("SK_")},
		// {UNiagaraSystem::StaticClass(),TEXT("NS_")},
		// {UNiagaraEmitter::StaticClass(),TEXT("NE_")},
	};

	//修复重定向
	void FixUpRedirectors();
};
