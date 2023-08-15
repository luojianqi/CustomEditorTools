// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	//文件浏览器菜单扩展
#pragma region ContentBrowserMenuExtention

	//删除未使用的资产按钮（三个）
	void InitCBMMenuExtension();

	TArray<FString> FolderPathsSelected; //选择的文件路径
	
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
	
	void OnDeleteUnusedAssetButtonClicked();  //删除未使用的资产按钮点击事件
	void OnDeleteEmptyFoldersButtonClicked();  //删除空文件夹按钮点击事件
	void OnAdvanceDeletionButtonClicked();  //高级删除
	
	void FixUpRedirectors();

	
#pragma endregion

	//自定义编辑器标签
#pragma region CustomEditorTab

	//注册高级删除标签
	void RegisterAdvanceDeletionTab();

	//回调
	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<SDockTab> ConstructDockTab;

	void OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
#pragma endregion

#pragma region LevelEditorMenuExtension

	void InitLevelEditorExtension(); //初始化关卡编辑器扩展

	TSharedRef<FExtender> CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);  //自定义关卡编辑器菜单扩展

	void AddLevelEditorMenuEntry(class FMenuBuilder& MenuBuilder);

	void OnLockActorsSelectionButtonClicked(); //锁定的选择Actor按钮点击事件

	void OnUnlockActorsSelectionButtonClicked(); //取消选择Actor锁定的按钮点击事件
	
#pragma endregion

#pragma  region SelectionLock  //选择锁定

	void InitCustomSelectionEvent(); //初始化自定义选择事件

	void OnActorSelected(UObject* SelectedObject); //

	void LockActorsSelection(AActor* ActorToProcess); //锁定选择的Actor
	void UnlockActorsSelection(AActor* ActorToProcess); //解锁选定的Actor

	void RefreshSceneOutliner();
	
#pragma endregion

#pragma region CustomEditorUICommands

	TSharedPtr<FUICommandList> CustomUICommands;

	void InitCustomUICommands(); //初始化自定义UI命令

	void OnSelectionLockHotKeyPressed();
	
	void OnUnlockActorSelectionHotKeyPressed();
	
#pragma endregion
	
#pragma region SceneOutlinerExtension

	void InitSceneOutlinerColumnExtension(); //初始化场景大纲扩展器

	TSharedRef<class ISceneOutlinerColumn> OnCreateSelectionLockColumn(class ISceneOutliner& SceneOutliner);

	void UnRegisterSceneOutlinerColumnExtension();

#pragma endregion
	
	
	TWeakObjectPtr<class UEditorActorSubsystem> WeakEditorActorsSubsystem;
	
	bool GetEditorActorSubSystem();
	
public:
	
#pragma region ProcessDataForAdvanceDeletionTab

	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);  //删除单个资产

	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetDataToDelete);  //删除多个资产

	//第一个参数：存储的资产 。第二个参数 ：应该显示的资产。
	void ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);//列出未使用的资产
	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);//列出相同名称的资产

	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);//同步内容浏览器 ,(点击列表中的一个资产，定位到对应的内容浏览器中)
#pragma endregion
	
	bool CheckIsActorsSelectionLocked(AActor* ActorToProcess); //检查选择的Actor是否锁定

	void ProcessLockingForOutliner(AActor* ActorToProcess,bool bShouldLock);
	
};


