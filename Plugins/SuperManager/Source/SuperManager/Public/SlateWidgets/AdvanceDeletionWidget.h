// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class SAdvanceDeletionTab: public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}
	
	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetDataToStore)

	SLATE_ARGUMENT(FString,CurrentSelectedFolder)
	
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;

	TArray<TSharedPtr<SCheckBox>> CheckBoxesArray;
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;  //被删除的资产数据数组
	
	
	TSharedRef< SListView< TSharedPtr <FAssetData> > > ConstructAssetListView();
	TSharedPtr< SListView< TSharedPtr <FAssetData> > > ConstructedAssetListView;
	
	void RefreshAssetListView();

#pragma region	ComboBoxForListingCondition

	TSharedRef< SComboBox < TSharedPtr <FString> > > ConstructComboBox();
	
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;

	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption,ESelectInfo::Type InSelectInfo);

	TSharedPtr<STextBlock> ComboDisplayTextBlock;

	TSharedRef<STextBlock> ConstructComboHelpTexts(const FString& TextConten,ETextJustify::Type TextJustify);
	
#pragma endregion 
#pragma region  RowWidgetForAssetListView
	
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,const TSharedRef<STableViewBase>& OwnerTable);//生成列表行

	void OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData);
	
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckBoxStateChanged(ECheckBoxState NewState,TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent,const FSlateFontInfo& FontToUse);

	TSharedRef<SButton>	ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

#pragma endregion

#pragma region TabButtons
	
	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();

	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();

	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

#pragma endregion
	
	FSlateFontInfo GetEmbossedTextFont() const {return FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));}
};
