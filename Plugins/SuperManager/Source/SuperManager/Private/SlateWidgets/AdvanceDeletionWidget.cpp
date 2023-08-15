// Fill out your copyright notice in the Description page of Project Settings.

#include "SlateWidgets/AdvanceDeletionWidget.h"

#include "DebugHeader.h"
#include "SuperManager.h"
#include "Widgets/Layout/SScrollBox.h"

#define ListAll TEXT("列出所有可用资产")
#define ListUnused TEXT("列出未使用的资产")
#define ListSameName TEXT("列出相同名称的资产")

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true; //这个小部件能支持键盘焦点吗

	StoredAssetsData = InArgs._AssetDataToStore;
	DisplayedAssetsData = StoredAssetsData;
	
	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboBoxSourceItems.Empty();
	
	ComboBoxSourceItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListSameName));
	
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;

	ChildSlot
	[ //主垂直框
		SNew(SVerticalBox)

		//标题文本的第一个垂直槽
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("高级删除")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		//第二个垂直槽用于下拉列表的指定的列表条件和帮助文本
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(.2f)
			[
				ConstructComboBox()
			]

			//组合框槽的帮助文本
			+SHorizontalBox::Slot()
			.FillWidth(.6f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				ConstructComboHelpTexts(TEXT("在下拉列表中指定列表条件，鼠标左键点击到资源所在位置"),ETextJustify::Center)
			]

			//帮助文本的文件夹路径
			+SHorizontalBox::Slot()
			.FillWidth(.2f)
			.VAlign(VAlign_Center)
			.Padding(0,0,20,0)
			[
				ConstructComboHelpTexts(TEXT("当前的文件夹:\n") + InArgs._CurrentSelectedFolder,ETextJustify::Right)
			]
		]
		
		//资产列表的第三个槽位
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				ConstructAssetListView() //中间列表
			]
		]
		//第四个插槽可设置三个按钮 下方按钮
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeleteAllButton()
			]

			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructSelectAllButton()
			]

			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeselectAllButton()
			]
		]
		

	];
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView< TSharedPtr <FAssetData> >)
	.ItemHeight(24.f)
	.ListItemsSource(&DisplayedAssetsData)
	.OnGenerateRow(this,&SAdvanceDeletionTab::OnGenerateRowForList)
	.OnMouseButtonClick(this,&SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked);
	
	return ConstructedAssetListView.ToSharedRef();
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	
	if(ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma region	ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox = SNew(SComboBox<TSharedPtr<FString>>)
	.OptionsSource(&ComboBoxSourceItems)
	.OnGenerateWidget(this,&SAdvanceDeletionTab::OnGenerateComboContent)
	.OnSelectionChanged(this,&SAdvanceDeletionTab::OnComboSelectionChanged)
	[
		SAssignNew(ComboDisplayTextBlock,STextBlock)
		.Text(FText::FromString(TEXT("资产列表选项")))
	];
	return ConstructedComboBox;
}

TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText = SNew(STextBlock)
	.Text(FText::FromString(*SourceItem.Get()));

	return ConstructedComboText;
}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	//根据选择的选项传递数据给我们的模块进行筛选
	if(*SelectedOption.Get() == ListAll)
	{
		//列出所有存储的资产数据
		DisplayedAssetsData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if(*SelectedOption.Get() == ListUnused)
	{
		//列出所有未使用的资产
		SuperManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData,DisplayedAssetsData);
		RefreshAssetListView();
	}
	else if(*SelectedOption.Get() == ListSameName)
	{
		//列出相同名称的资产
		SuperManagerModule.ListSameNameAssetsForAssetList(StoredAssetsData,DisplayedAssetsData);
		RefreshAssetListView();
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructComboHelpTexts(const FString& TextConten,
	ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock> ConstructedHelpText = SNew(STextBlock)
	.Text(FText::FromString(TextConten))
	.Justification(TextJustify)
	.AutoWrapText(true);

	return ConstructedHelpText;
}

#pragma endregion



#pragma region  RowWidgetForAssetListView

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,const TSharedRef<STableViewBase>& OwnerTable)
{
	if(!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable);
	
	const FString DisplayAssetClassName = AssetDataToDisplay->GetClass()->GetName();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15;
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
	SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable).Padding(FMargin(5.f))
	[
		SNew(SHorizontalBox)
		//复选框的第一个槽 :
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]

		//第二个槽用于显示资产类名称
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.FillWidth(.5f)
		[
			ConstructTextForRowWidget(DisplayAssetClassName,AssetClassNameFont) //显示的文字
		]
		
		//第三个槽用于显示资产名称
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			ConstructTextForRowWidget(DisplayAssetName,AssetNameFont)  //显示的文字
		]
				
		//第四个槽是一个按钮
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButtonForRowWidget(AssetDataToDisplay)   //按钮
		]
	];

	return ListViewRowWidget;
}

void SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData)
{
	//DebugHeader::PrintLog(ClickedData->AssetName.ToString() + TEXT("被点击"));
	
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->GetObjectPathString());
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this,&SAdvanceDeletionTab::OnCheckBoxStateChanged,AssetDataToDisplay) //单选框状态改变回调
	.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstructedCheckBox);  //添加
	
	return ConstructedCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState,TSharedPtr<FAssetData> AssetData)
{
	
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		//DebugHeader::PrintLog(AssetData->AssetName.ToString() + TEXT(" 不选中"));
		if(AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		
		//DebugHeader::PrintLog(AssetData->AssetName.ToString() + TEXT(" 选中"));
		
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;
	case ECheckBoxState::Undetermined:
		
		break;
	default:
		break;
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent,
	const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
	.Text(FText::FromString(TEXT("删除")))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteButtonClicked,AssetDataToDisplay);
	
	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	//DebugHeader::PrintLog(ClickedAssetData->AssetName.ToString() + TEXT("被点击"));

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if(bAssetDeleted)
	{
		//刷新列表源项
		if(StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}

		if (DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}

		//更新列表
		RefreshAssetListView();
	}
	
	
	return FReply::Handled();
}
#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteAllButtonClicked);
	
	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("删除所有")));
	
	return DeleteAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	//DebugHeader::PrintLog(TEXT("删除所有"));
	if(AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("当前没有选中资产"));

		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataToDelete;
	for(const TSharedPtr<FAssetData>& Data : AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}
	
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	const bool bAssetDeleted = SuperManagerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);

	if(bAssetDeleted)
	{
		for(const TSharedPtr<FAssetData>& DeletedData: AssetsDataToDeleteArray)
		{
			if(StoredAssetsData.Contains(DeletedData))
			{
				StoredAssetsData.Remove(DeletedData);
			}

			if(DisplayedAssetsData.Contains(DeletedData))
			{
				DisplayedAssetsData.Remove(DeletedData);
			}
		}

		RefreshAssetListView();
	}
	
	return FReply::Handled();
	
}


TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnSelectAllButtonClicked);
	
	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("全选")));
	
	return SelectAllButton;
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	//DebugHeader::PrintLog(TEXT("全选"));

	if(CheckBoxesArray.Num() == 0)
	{
		return FReply::Handled();
	}
	for(const TSharedPtr<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if(!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState(); //显示此复选框的选中状态，根据需要触发事件
		}
		
	}
	
	return FReply::Handled();
}



TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeselectAllButtonClicked);
	
	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("取消全选")));
	
	return DeselectAllButton;
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	//DebugHeader::PrintLog(TEXT("取消全选"));

	if(CheckBoxesArray.Num() == 0)
	{
		return FReply::Handled();
	}
	for(const TSharedPtr<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if(CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState(); //显示此复选框的选中状态，根据需要触发事件
		}
		
	}
	
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 15;
	
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(ButtonTextFont)
	.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion 

