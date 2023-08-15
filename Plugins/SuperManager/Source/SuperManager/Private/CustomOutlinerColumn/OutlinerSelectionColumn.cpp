// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"

#include "ActorTreeItem.h"
#include "ISceneOutlinerTreeItem.h"
#include "SuperManager.h"
#include "CustomStyle/SuperManagerStyle.h"

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments ConstructedHeaderRow =
		SHeaderRow::Column(GetColumnID()) //当列表和树显示多列时，出现在它们上面的标题。  //创建指定ColumnId的列
		.FixedWidth(24.f)
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultTooltip(FText::FromString(TEXT("Actor选择锁定-按下图示锁定选择的Actor”)")))
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(FSuperManagerStyle::GetCreatedSlateStyleSet()->GetBrush(FName("LevelEditor.LockSelection")))
		];
	return ConstructedHeaderRow;
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem,
                                                                           const STableRow<FSceneOutlinerTreeItemPtr>&
                                                                           Row)
{

	FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();  //表示世界中的参与者的树项
	if(!ActorTreeItem || !ActorTreeItem->IsValid()) return SNullWidget::NullWidget;

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bIsActorSelectionLocked = SuperManagerModule.CheckIsActorsSelectionLocked(ActorTreeItem->Actor.Get());

	const FCheckBoxStyle& ToggleButtonStyle = FSuperManagerStyle::GetCreatedSlateStyleSet()->GetWidgetStyle<FCheckBoxStyle>(FName("SceneOutliner.SelectionLock"));
	
	TSharedRef<SCheckBox> ConstructedRowWidgetCheckBox =
	SNew(SCheckBox)
	.Visibility(EVisibility::Visible)
	.Type(ESlateCheckBoxType::ToggleButton)
	.Style(&ToggleButtonStyle)
	.HAlign(HAlign_Center)
	.IsChecked(bIsActorSelectionLocked? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
	.OnCheckStateChanged(this,&FOutlinerSelectionLockColumn::OnRowWidgetCheckStateChanged,ActorTreeItem->Actor);
	
	return  ConstructedRowWidgetCheckBox;
}

void FOutlinerSelectionLockColumn::OnRowWidgetCheckStateChanged(ECheckBoxState NewState,
	TWeakObjectPtr<AActor> CorrespondingActor)
{
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		SuperManagerModule.ProcessLockingForOutliner(CorrespondingActor.Get(),false);
		break;
		
	case ECheckBoxState::Checked:
		SuperManagerModule.ProcessLockingForOutliner(CorrespondingActor.Get(),true);
		break;
		
	case ECheckBoxState::Undetermined:
		
		break;
	default:
		break;
	}
}
