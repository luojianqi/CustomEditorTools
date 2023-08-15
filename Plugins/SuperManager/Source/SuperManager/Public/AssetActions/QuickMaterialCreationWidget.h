// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "QuickMaterialCreationWidget.generated.h"

UENUM(BlueprintType)
enum class E_ChannelPackingType : uint8
{
	ECPT_NoChannelPacking UMETA(DisPlayName = "无通道填充"),

	ECPT_ORM UMETA(DisPlayName = "遮挡粗糙度"),

	ECPT_MAX UMETA(DisPlayName = "DefaultMax")
};
/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UQuickMaterialCreationWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	
#pragma region QuickMaterialCreation
	
	UFUNCTION(BlueprintCallable,Category = "CreateMaterialFromSelectedTextures")
	void CreateMaterialFromSelectedTextures(); //从选择的纹理中创建材质

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="CreateMaterialFromSelectedTextures")
	E_ChannelPackingType ChannelPackingType = E_ChannelPackingType::ECPT_NoChannelPacking;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="CreateMaterialFromSelectedTextures")
	bool bCustomMaterialName = true;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= "CreateMaterialFromSelectedTextures",meta = (EditCondition = "bCustomMaterialName"))
	FString MaterialName = TEXT("M_");

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="CreateMaterialFromSelectedTextures")
	bool bCreateMaterialInstance = false;
	
#pragma endregion 

#pragma region SupportedTextureNames

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Supported Texture Names")
	TArray<FString> BaseColorArray = { //基础颜色
		TEXT("_BaseColor"),
		TEXT("_Albedo"),
		TEXT("_Diffuse"),
		TEXT("_diff")
	};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> MetallicArray = { //金属色
		TEXT("_Metallic"),
		TEXT("_metal")
	};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> RoughnessArray = {//粗糙度
		TEXT("_Roughness"),
		TEXT("_RoughnessMap"),
		TEXT("_rough")
	};
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> NormalArray = { //法线
		TEXT("_Normal"),
		TEXT("_NormalMap"),
		TEXT("_nor")
	};
	              
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> AmbientOcclusionArray = {  //环境光遮挡
		TEXT("_AmbientOcclusion"),
		TEXT("_AmbientOcclusionMap"),
		TEXT("_AO")
	};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Supported Texture Names")
	TArray<FString> ORMArray = { 
		TEXT("_arm"),
		TEXT("_OcclusionRoughnessMetallic"),
		TEXT("_ORM")
	};

#pragma endregion 
	
private:
	
#pragma region QuickMaterialCreationCore
	
	bool ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess,TArray<UTexture2D*>& OutSelectedTexturesArray,FString& OutSelectedTexturePackagePath);//处理选择的数据

	bool CheckIsNameUsed(const FString& FolderPathToCheck,const FString& MaterialNameToCheck);//检查是否有名字使用

	UMaterial* CreateMaterialAsset(const FString& NameOfTheMaterial,const FString& PathToPutMaterial); //创建材质资产

	void Default_CreateMaterialNodes(UMaterial* CreatedMaterial,UTexture2D* SelectedTexture,uint32& PinsConnectedCounter);//默认创建材质节点，PinsConnectedCounter 引脚链接计数器
	void ORM_CreateMaterialNodes(UMaterial* CreatedMaterial,UTexture2D* SelectedTexture,uint32& PinsConnectedCounter);

#pragma endregion

#pragma region CreateMaterialNodesConnectPins

	bool TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* SelectedTexture,UMaterial* CreatedMaterial);//试着连接基础颜色引脚
	bool TryConnectMetallic(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* SelectedTexture,UMaterial* CreatedMaterial);//试着连接金属色引脚
	bool TryConnectRoughness(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* SelectedTexture,UMaterial* CreatedMaterial);//试着连接粗糙度引脚
	bool TryConnectNormal(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* SelectedTexture,UMaterial* CreatedMaterial);//试着连接法线引脚
	bool TryConnectAO(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* SelectedTexture,UMaterial* CreatedMaterial);//试着连接环境遮挡引脚
	bool TryConnectORM(UMaterialExpressionTextureSample* TextureSampleNode,UTexture2D* SelectedTexture,UMaterial* CreatedMaterial);
#pragma endregion 

	class UMaterialInstanceConstant* CreateMaterialInstanceAsset(UMaterial* CreatedMaterial,FString NameOfMaterialInstance,const FString& PathToPutMI);
};


