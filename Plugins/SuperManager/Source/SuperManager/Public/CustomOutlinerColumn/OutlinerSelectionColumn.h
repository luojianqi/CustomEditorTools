// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "ISceneOutlinerColumn.h"

class FOutlinerSelectionLockColumn : public ISceneOutlinerColumn //用于场景大纲轮廓栏的接口
{
public:
	FOutlinerSelectionLockColumn(ISceneOutliner& SceneOutliner){}
	
	virtual FName GetColumnID() override {return GetID();}
	
	static FName GetID() {return FName("SelectionLock");}
	

	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override; //构造标题行列
	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override; //构造行小部件

private:
	void OnRowWidgetCheckStateChanged(ECheckBoxState NewState,TWeakObjectPtr<AActor> CorrespondingActor);
};
