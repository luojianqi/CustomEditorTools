// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"

#include "AssetToolsModule.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"


void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		//Print(TEXT("请输入有效值"), FColor::Red);

		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("请输入有效值"));
		return;
	}

	TArray<FAssetData> SelectAssetsData = UEditorUtilityLibrary::GetSelectedAssetData(); //获取当前选定的资产数据集
	uint32 Counter = 0;

	for (const auto& SelectAssetData : SelectAssetsData)
	{
		for (int i = 0; i < NumOfDuplicates; ++i)
		{
			const FString SourceAssetPath = SelectAssetData.GetObjectPathString(); //获取对象路径
			const FString NewDuplicateAssetName = SelectAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1); //新的资产路径名
			const FString NewPathName = FPaths::Combine(SelectAssetData.PackagePath.ToString(), NewDuplicateAssetName);//联合路径
			
			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		//Print((TEXT("成功复制") + FString::FromInt(Counter) + TEXT("个文件")), FColor::Green);
		DebugHeader::ShowNotifyInfo(TEXT("成功复制") + FString::FromInt(Counter) + TEXT("个文件"));
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets(); //获取当前选定资产的集合
	uint32 Counter = 0;

	for (const auto& SelectedObject : SelectedObjects)
	{
		if(!SelectedObject) continue;
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if(!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("没有找到前缀,对类 ") + SelectedObject->GetClass()->GetName(),FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();
		
		if(OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(OldName + TEXT("已经存在前缀"),FColor::Red);
			continue;
		}

		if(SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			//删除字符**开头的字符
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject,NewNameWithPrefix);

		++Counter;
	}
	if(Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("成功命名") + FString::FromInt(Counter) + TEXT("个资产"));
	}
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData(); //你选中的所有资产
	TArray<FAssetData> UnusedAssetsData; //未使用的资产

	FixUpRedirectors();//如果移动资产到其他文件夹会删除失败
	
	for (const auto& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString());//找到包引用

		if(AssetReferences.Num() == 0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	if(UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("没有从选择的资产中,发现未使用的资产"),false);
		return;
	}

	//删除未使用的资产
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);
	
	if(NumOfAssetsDeleted == 0)  return;

	DebugHeader::ShowNotifyInfo(TEXT("已经成功删除") + FString::FromInt(NumOfAssetsDeleted) + TEXT("个未使用的资产"));
	
	
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	FARFilter Filter; //过滤器
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter,OutRedirectors);

	for (const auto& RedirectorData : OutRedirectors)
	{
		if(UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray); //修复对指定重定向器的引用。
}
