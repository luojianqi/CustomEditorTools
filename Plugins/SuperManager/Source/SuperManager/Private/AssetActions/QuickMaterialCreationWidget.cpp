// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"

#include "AssetToolsModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialInstanceConstant.h"

#pragma region QuickMaterialCreationCore

void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	//DebugHeader::PrintLog(TEXT("UQuickMaterialCreationWidget Working"));
	if (bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("请输入正确的名称"));
			return;
		}
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData(); //生成在主内容浏览器中选择的资产列表
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;

	uint32 PinsConnectedCounter = 0;

	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath)) //过滤器--只能选择纹理
	{
		MaterialName = TEXT("M_");
		return;
	}

	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName)) //检查名字是否相同（如果材质名称被指定文件夹下的资源使用）
	{
		MaterialName = TEXT("M_");
		return;
	}
	
	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath); //创建材质

	if (!CreatedMaterial)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("创建材质失败"));
		return;
	}

	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if (!SelectedTexture) continue;
		switch (ChannelPackingType)
		{
		case E_ChannelPackingType::ECPT_NoChannelPacking:

			Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;

		case E_ChannelPackingType::ECPT_ORM:
			ORM_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;

		case E_ChannelPackingType::ECPT_MAX:

			break;

		default:
			break;
		}
	}

	if (PinsConnectedCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("成功连接") + FString::FromInt(PinsConnectedCounter) + TEXT("对引脚"));
	}

	
	//MaterialName = TEXT("M_");

	if(bCreateMaterialInstance)
	{
		CreateMaterialInstanceAsset(CreatedMaterial,MaterialName,SelectedTextureFolderPath);
	}

	
	//DebugHeader::PrintLog(*SelectedTextureFolderPath);
}
#pragma endregion

#pragma region QuickMaterialCreation

//处理选中的数据，将过滤纹理，如果未选中则返回false
bool UQuickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess,
                                                       TArray<UTexture2D*>& OutSelectedTexturesArray,
                                                       FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProcess.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("没有图片选择"));
		return false;
	}

	bool bMaterialNameSet = false;
	for (const FAssetData& SelectedData : SelectedDataToProcess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset(); //选择的资产
		if (!SelectedAsset) continue;

		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);
		if (!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("仅仅只能选择纹理,") + SelectedAsset->GetName() + TEXT("不是纹理!"));

			return false;
		}

		OutSelectedTexturesArray.Add(SelectedTexture);

		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();
		}

		if (!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectedAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0,TEXT("M_"));
			bMaterialNameSet = true;
		}
	}

	return true;
}

//如果材质名称被指定文件夹下的资源使用，则返回true
bool UQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{
	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);
	//返回在DirectoryPath中找到的所有资源的列表。

	for (const FString& ExistingAssetPath : ExistingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath); //返回文件名

		if (ExistingAssetName.Equals(MaterialNameToCheck))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, MaterialNameToCheck + TEXT("已被资产使用"));

			return true;
		}
	}

	return false;
}

UMaterial* UQuickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfTheMaterial,
                                                             const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();

	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterial, PathToPutMaterial,
	                                                            UMaterial::StaticClass(), MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

void UQuickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture,
                                                               uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
	//材质中的TextureSample节点
	if (!TextureSampleNode) return;

	if (!CreatedMaterial->HasBaseColorConnected()) //创建的材质基础颜色引脚没有连接
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasMetallicConnected()) //创建的材质金属色引脚没有连接
	{
		if (TryConnectMetallic(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasRoughnessConnected()) //创建的粗糙度没有引脚连接
	{
		if (TryConnectRoughness(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasNormalConnected()) //创建的法线没有引脚连接
	{
		if (TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasAmbientOcclusionConnected()) //创建的环境遮挡没有引脚连接
	{
		if (TryConnectAO(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("连接纹理") + SelectedTexture->GetName() + TEXT("失败"), false);
}

void UQuickMaterialCreationWidget::ORM_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture,
                                                           uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);
	//材质中的TextureSample节点
	if (!TextureSampleNode) return;

	if (!CreatedMaterial->HasBaseColorConnected()) //创建的材质基础颜色引脚没有连接
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasNormalConnected()) //创建的法线没有引脚连接
	{
		if (TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasRoughnessConnected()) //
	{
		if (TryConnectORM(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter+=3;
			return;
		}
	}
}
#pragma endregion

#pragma region CreateMaterialNodesConnectPins
bool UQuickMaterialCreationWidget::TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode,
                                                       UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray) //循环遍历基本颜色数组
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			TextureSampleNode->Texture = SelectedTexture; //设置图片

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_BaseColor)->Connect(0, TextureSampleNode); //连接引脚
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600; //隔离位置
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectMetallic(UMaterialExpressionTextureSample* TextureSampleNode,
                                                      UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& MetallicName : MetallicArray) //循环遍历基本颜色数组
	{
		if (SelectedTexture->GetName().Contains(MetallicName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture; //设置图片
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(0, TextureSampleNode); //连接引脚
			CreatedMaterial->PostEditChange(); //对象属性被外部值修改时调用

			TextureSampleNode->MaterialExpressionEditorX -= 600; //隔离间隔X
			TextureSampleNode->MaterialExpressionEditorY += 240; //隔离间隔Y

			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectRoughness(UMaterialExpressionTextureSample* TextureSampleNode,
                                                       UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& RoughnessName : RoughnessArray)
	{
		if (SelectedTexture->GetName().Contains(RoughnessName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default; //压缩设置
			SelectedTexture->SRGB = false; //srgb
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture; //设置图片
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor; //材质线性

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange(); //对象属性被外部值修改时调用

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 480; //隔离间隔Y

			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectNormal(UMaterialExpressionTextureSample* TextureSampleNode,
                                                    UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& NormalName : NormalArray)
	{
		if (SelectedTexture->GetName().Contains(NormalName))
		{
			TextureSampleNode->Texture = SelectedTexture; //设置图片
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Normal; //

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Normal)->Connect(0, TextureSampleNode);
			CreatedMaterial->PostEditChange(); //对象属性被外部值修改时调用

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 720; //隔离间隔Y

			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectAO(UMaterialExpressionTextureSample* TextureSampleNode,
                                                UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& AOName : AmbientOcclusionArray) //循环遍历环境遮挡数组
	{
		if (SelectedTexture->GetName().Contains(AOName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture; //设置图片
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(0, TextureSampleNode); //连接引脚
			CreatedMaterial->PostEditChange(); //对象属性被外部值修改时调用

			TextureSampleNode->MaterialExpressionEditorX -= 600; //隔离间隔X
			TextureSampleNode->MaterialExpressionEditorY += 960; //隔离间隔Y

			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectORM(UMaterialExpressionTextureSample* TextureSampleNode,
                                                 UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& ORM_Name : ORMArray) //循环遍历基本颜色数组
	{
		if (SelectedTexture->GetName().Contains(ORM_Name))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture; //设置图片
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Masks;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(1, TextureSampleNode); //连接引脚
			CreatedMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(2, TextureSampleNode);
			CreatedMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(3, TextureSampleNode);
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600; //隔离间隔X
			TextureSampleNode->MaterialExpressionEditorY += 960; //隔离间隔Y
			return true;
		}
	}
	return false;
}

#pragma endregion

UMaterialInstanceConstant* UQuickMaterialCreationWidget::CreateMaterialInstanceAsset(UMaterial* CreatedMaterial,
	FString NameOfMaterialInstance, const FString& PathToPutMI)
{
	NameOfMaterialInstance.RemoveFromStart(TEXT("M_"));  //处理名字

	
	NameOfMaterialInstance.InsertAt(0,TEXT("MI_"));

	UE_LOG(LogTemp,Warning,TEXT("1111 %s"),*NameOfMaterialInstance);

	UMaterialInstanceConstantFactoryNew* MIFactoryNew = NewObject<UMaterialInstanceConstantFactoryNew>();  //创建工厂材质实例，建造新资产的工厂
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")); //加载模块

	UObject* CreateObject = AssetToolsModule.Get().CreateAsset(NameOfMaterialInstance,PathToPutMI,UMaterialInstanceConstant::StaticClass(),MIFactoryNew); //创建材质实例

	if(UMaterialInstanceConstant* CreatedMI = Cast<UMaterialInstanceConstant>(CreateObject))  
	{
		CreatedMI->SetParentEditorOnly(CreatedMaterial);

		CreatedMI->PostEditChange();
		CreatedMaterial->PostEditChange();

		return CreatedMI;
	}

	return nullptr;
}
