// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "DebugHeader.h"
#include "LevelEditor.h"
#include "LevelEditorMenuContext.h"
#include "SceneOutlinerModule.h"
#include "Selection.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "CustomUICommand/SuperManagerUICommands.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "Subsystems/EditorActorSubsystem.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	FSuperManagerStyle::InitializeIcons();

	InitCBMMenuExtension();

	RegisterAdvanceDeletionTab();

	FSuperManagerUICommands::Register();

	InitCustomUICommands();

	InitLevelEditorExtension();

	InitCustomSelectionEvent();

	InitSceneOutlinerColumnExtension();
}

#pragma region ContentBrowserMenuExtention
void FSuperManagerModule::InitCBMMenuExtension()
{
	//向菜单扩展器添加一个自定义代理
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		TEXT("ContentBrowser"));

	//获取所有的菜单扩展器
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.
		GetAllPathViewContextMenuExtenders();

	// FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	// CustomCBMenuDelegate.BindRaw(this,&FSuperManagerModule::CustomCBMenuExtender);//绑定
	// ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate); //添加委托

	//添加委托
	ContentBrowserModuleMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

//定义插入菜单项的位置
TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"), //扩展挂钩，插入的位置
		                               EExtensionHook::After, //后面
		                               TSharedPtr<FUICommandList>(), //自定义热键
		                               FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry));
		//第二个绑定。定义菜单项的详细信息。

		FolderPathsSelected = SelectedPaths;
	}

	return MenuExtender;
}

//定义自定义菜单项的详细信息。
void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("删除未使用的资产")), //菜单项的标题文本
		FText::FromString(TEXT("安全删除文件夹下所有未使用的资产")), //菜单项的提示文本。
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteUnusedAssets"), //自定义图标的地方
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked) //要执行的实际功能函数。
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("删除空文件夹")), //菜单项的标题文本
		FText::FromString(TEXT("安全删除所有空文件夹")), //菜单项的提示文本。
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolders"), //自定义图标的地方
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked) //要执行的实际功能函数。
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("高级删除")), //菜单项的标题文本
		FText::FromString(TEXT("在选项卡中按特定条件列出资产以用于删除")), //菜单项的提示文本。
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"), //自定义图标的地方
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked) //要执行的实际功能函数。
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	//DebugHeader::Print(TEXT("Working"),FColor::Green);
	//DebugHeader::PrintLog(TEXT("HELLO"));

	if(ConstructDockTab.IsValid())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("在执行此操作之前，请关闭高级删除选项卡!!!!"));
		return;
	}
	
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("你只可以对一个文件夹进行该操作！！！"));
		return;
	}

	DebugHeader::PrintLog(TEXT("当前选择的文件夹是 ：") + FolderPathsSelected[0]);

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	//返回在DirectoryPath中找到的所有资源的列表。

	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("在选定的文件夹下没有找到资产"));
		return;
	}

	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,
		                           TEXT("总共找到了") + FString::FromInt(AssetsPathNames.Num()) + TEXT("个资产.\n  你想继续吗？"),
		                           false);
	if (ConfirmResult == EAppReturnType::No) return;

	FixUpRedirectors(); //修复重定向器

	TArray<FAssetData> UnusedAssetsDataArray;
	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetsPathNames.Contains(TEXT("Developers")) ||
			AssetsPathNames.Contains(TEXT("Collections")) ||
			AssetsPathNames.Contains(TEXT("__ExternalActors__")) ||
			AssetsPathNames.Contains(TEXT("__ExternalObjects__"))) //不会碰到这四个文件夹
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue; //资源不在内容浏览器中  跳出

		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		//查找资源的全部引用

		if (AssetReferencers.Num() == 0) //没有引用添加到未使用的资产数组中
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray); //删除
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("在选定的文件夹下没有找到未使用的资产"));
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	//DebugHeader::PrintLog(TEXT("FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked"));

	if(ConstructDockTab.IsValid())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("在执行此操作之前，请关闭高级删除选项卡!!!!"));
		return;
	}
	
	FixUpRedirectors();

	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath : FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__"))) //不会碰到这四个文件夹
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue; //检查目录是否存在于内容浏览器中,不存在跳出

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath)) //检查目录中是否存在资产 ,不存在添加
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("在所选择的文件夹下未找到空文件夹"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
	                                                                TEXT("空文件夹在:\n") + EmptyFolderPathsNames + TEXT(
		                                                                "\n你要全部删除吗？"),
	                                                                false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath)
			? ++Counter
			: DebugHeader::PrintLog(EmptyFolderPath + TEXT("删除失败"));
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("成功删除") + FString::FromInt(Counter) + TEXT("个文件夹"));
	}
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	//DebugHeader::PrintLog(TEXT("FSuperManagerModule::OnAdvanceDeletionButtonClicked"));
	FixUpRedirectors();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("高级删除"));
}

void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry"));

	FARFilter Filter; //过滤器
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const auto& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray); //修复
}
#pragma endregion

#pragma region CustomEditorTab
void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("高级删除"),
	                                                  FOnSpawnTab::CreateRaw(
		                                                  this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab))
	                        .SetDisplayName(FText::FromString(TEXT("高级删除")))
	                        .SetIcon(
		                        FSlateIcon(FSuperManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if(FolderPathsSelected.Num() == 0)
	{
		return SNew(SDockTab).TabRole(NomadTab);
	}
	
	ConstructDockTab = SNew(SDockTab).TabRole(NomadTab)
	[
		SNew(SAdvanceDeletionTab)
		.AssetDataToStore(GetAllAssetDataUnderSelectedFolder())
		.CurrentSelectedFolder(FolderPathsSelected[0])
	];
	
	ConstructDockTab->SetOnTabClosed //选项卡小部件关闭的时候
	(SDockTab::FOnTabClosedCallback::CreateRaw(this,&FSuperManagerModule::OnAdvanceDeletionTabClosed));

	return ConstructDockTab.ToSharedRef();
}

void FSuperManagerModule::OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose)
{
	if(ConstructDockTab.IsValid())
	{
		ConstructDockTab.Reset();
		FolderPathsSelected.Empty();
	}
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaiableAssetsData;
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__"))) //不会碰到这四个文件夹
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue; //检查资产是否存在于内容浏览器中,不存在跳出

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetsData;
}

#pragma endregion

#pragma region LevelEditorMenuExtension

void FSuperManagerModule::InitLevelEditorExtension()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TSharedRef<FUICommandList> ExistingLevelCommands = LevelEditorModule.GetGlobalLevelEditorActions();//所有关卡编辑器都通用的绑定关卡编辑器命令列表
	ExistingLevelCommands->Append(CustomUICommands.ToSharedRef());
	
	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders = LevelEditorModule.
		GetAllLevelViewportContextMenuExtenders();
	
	LevelEditorMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::
		CreateRaw(this, &FSuperManagerModule::CustomLevelEditorMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList,
                                                                         const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	if (SelectedActors.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("ActorOptions"),
		                               EExtensionHook::Before,
		                               UICommandList,
		                               FMenuExtensionDelegate::CreateRaw(
			                               this, &FSuperManagerModule::AddLevelEditorMenuEntry));
	}
	return MenuExtender;
}

void FSuperManagerModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("锁定选中的Actor")),
		FText::FromString(TEXT("防止Actor被选中")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"LevelEditor.LockSelection"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnLockActorsSelectionButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("解锁全部选定的Actor")),
		FText::FromString(TEXT("移除所有参与者的选择约束")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"LevelEditor.UnlockSelection"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnlockActorsSelectionButtonClicked)
	);
	
}

void FSuperManagerModule::OnLockActorsSelectionButtonClicked()
{
	// DebugHeader::PrintLog(TEXT("Locked"));
	if(!GetEditorActorSubSystem()) return;

	TArray<AActor*> SelectedActors = WeakEditorActorsSubsystem->GetSelectedLevelActors();

	if(SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("没有Actor被选择"));
		return;
	}

	FString CurrentLockedActorsName = TEXT("锁定选项 : ");
	for(AActor* SelectedActor:SelectedActors)
	{
		if(!SelectedActor) continue;

		LockActorsSelection(SelectedActor);

		WeakEditorActorsSubsystem->SetActorSelectionState(SelectedActor,false);

		CurrentLockedActorsName.Append(TEXT("\n"));
		CurrentLockedActorsName.Append(SelectedActor->GetActorLabel());
	}
	
	RefreshSceneOutliner();
	DebugHeader::ShowNotifyInfo(CurrentLockedActorsName);
}

void FSuperManagerModule::OnUnlockActorsSelectionButtonClicked()
{
	//DebugHeader::PrintLog(TEXT("Unlocked"));

	if(!GetEditorActorSubSystem()) return;
	
	TArray<AActor*> SelectedActors = WeakEditorActorsSubsystem->GetAllLevelActors();
	TArray<AActor*> AllLockedActors;

	for(AActor* ActorInLevel:SelectedActors)
	{
		if(!ActorInLevel) continue;
		
		if(CheckIsActorsSelectionLocked(ActorInLevel))
		{
			AllLockedActors.Add(ActorInLevel);
		}
	}

	if(AllLockedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("当前没有锁定的Actor"));
		return;
	}

	FString UnlockedActorNames = TEXT("解除选择锁定 ：");
	for(AActor* LockedActor : AllLockedActors)
	{
		UnlockActorsSelection(LockedActor);

		UnlockedActorNames.Append(TEXT("\n"));
		UnlockedActorNames.Append(LockedActor->GetActorLabel());
	}

	RefreshSceneOutliner();
	DebugHeader::ShowNotifyInfo(UnlockedActorNames);
}

#pragma endregion

#pragma  region SelectionLock

void FSuperManagerModule::InitCustomSelectionEvent()
{
	USelection* UserSelection = GEditor->GetSelectedActors(); //返回所选角色的集合。

	UserSelection->SelectObjectEvent.AddRaw(this,&FSuperManagerModule::OnActorSelected); //当对象被选中时调用
}

void FSuperManagerModule::OnActorSelected(UObject* SelectedObject)
{
	if(!GetEditorActorSubSystem()) return;
	
	if(AActor* SelectedActor = Cast<AActor>(SelectedObject))
	{
		//DebugHeader::PrintLog(SelectedActor->GetActorLabel());

		if(CheckIsActorsSelectionLocked(SelectedActor))
		{
			//立即取消演员选择
			WeakEditorActorsSubsystem->SetActorSelectionState(SelectedActor,false);
		}
	}
}

void FSuperManagerModule::LockActorsSelection(AActor* ActorToProcess)
{
	if(!ActorToProcess) return;
	
	if(!ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Add(FName("Locked"));
	}
}

void FSuperManagerModule::UnlockActorsSelection(AActor* ActorToProcess)
{
	if(!ActorToProcess) return;
	
	if(ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Remove(FName("Locked"));
	}
}

void FSuperManagerModule::RefreshSceneOutliner()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<ISceneOutliner> SceneOutliner = LevelEditorModule.GetFirstLevelEditor()->GetMostRecentlyUsedSceneOutliner();
	if(SceneOutliner.IsValid())
	{
		SceneOutliner->FullRefresh();//场景大纲刷新
	}
}

bool FSuperManagerModule::CheckIsActorsSelectionLocked(AActor* ActorToProcess)
{
	if(!ActorToProcess) return false;

	return ActorToProcess->ActorHasTag(FName("Locked"));
}

void FSuperManagerModule::ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLock)
{
	if(!GetEditorActorSubSystem()) return;
	
	if(bShouldLock)
	{
		LockActorsSelection(ActorToProcess);
		WeakEditorActorsSubsystem->SetActorSelectionState(ActorToProcess,false);
		DebugHeader::ShowNotifyInfo(TEXT("Locked selection for(锁定选择): \n") + ActorToProcess->GetActorLabel());
	}
	else
	{
		UnlockActorsSelection(ActorToProcess);
		DebugHeader::ShowNotifyInfo(TEXT("Removed selection Lock for(移除锁定) : \n") + ActorToProcess->GetActorLabel());
	}
	
}
#pragma endregion


#pragma region CustomEditorUICommands

void FSuperManagerModule::InitCustomUICommands()
{
	CustomUICommands = MakeShareable(new FUICommandList());

	CustomUICommands.Get()->MapAction(FSuperManagerUICommands::Get().LockActorSelection
		,FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnSelectionLockHotKeyPressed));

	CustomUICommands.Get()->MapAction(FSuperManagerUICommands::Get().UnlockActorSelection
		,FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnUnlockActorSelectionHotKeyPressed));
}

void FSuperManagerModule::OnSelectionLockHotKeyPressed()
{
	//DebugHeader::PrintLog(TEXT("Hot key working"));
	OnLockActorsSelectionButtonClicked();
	
}

void FSuperManagerModule::OnUnlockActorSelectionHotKeyPressed()
{
	OnUnlockActorsSelectionButtonClicked();
}

#pragma endregion 

#pragma region SceneOutlinerExtension

void FSuperManagerModule::InitSceneOutlinerColumnExtension() 
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	FSceneOutlinerColumnInfo SelectionLockColumnInfo(
	ESceneOutlinerColumnVisibility::Visible,
	1,
	FCreateSceneOutlinerColumn::CreateRaw(this,&FSuperManagerModule::OnCreateSelectionLockColumn) //新的列类型
		);
	
	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerSelectionLockColumn>(SelectionLockColumnInfo);

}

TSharedRef<ISceneOutlinerColumn> FSuperManagerModule::OnCreateSelectionLockColumn(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerSelectionLockColumn(SceneOutliner));
}

void FSuperManagerModule::UnRegisterSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));
	
	SceneOutlinerModule.UnRegisterColumnType<FOutlinerSelectionLockColumn>();
}


#pragma endregion

#pragma region ProcessDataForAdvanceDeletionTab

bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return true;
	}
	return false;
}

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetDataToDelete)
{
	if (ObjectTools::DeleteAssets(AssetDataToDelete) > 0)
	{
		return true;
	}
	return false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
                                                       TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(
			DataSharedPtr->GetObjectPathString());
		if (AssetReferencers.Num() == 0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}

void FSuperManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
                                                         TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();

	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetsData);

		if (OutAssetsData.Num() <= 1) continue;

		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData)
		{
			if (SameNameData.IsValid())
			{
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}
}

void FSuperManagerModule::SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetsPathToSync;
	AssetsPathToSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathToSync);
}

#pragma endregion


bool FSuperManagerModule::GetEditorActorSubSystem()
{
	if(!WeakEditorActorsSubsystem.IsValid())
	{
		WeakEditorActorsSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}

	return WeakEditorActorsSubsystem.IsValid();
}


void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("高级删除"));
	FSuperManagerStyle::ShutDown();
	FSuperManagerUICommands::Unregister();
	UnRegisterSceneOutlinerColumnExtension();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)
