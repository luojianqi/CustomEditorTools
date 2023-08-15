// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Styling/SlateStyle.h"

/**
 * @brief
 * 设置样式设置名称
 * 创建自定义样式集
 * 从目录加载图标
 * 设置图标名称
 * 设置图标大小
 */
class FSuperManagerStyle
{
public:
	static void InitializeIcons();
	static void ShutDown();
	
private:
	static FName StyleSetName;

	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;

public:
	static FName GetStyleSetName() {return StyleSetName;}

	static TSharedRef<FSlateStyleSet> GetCreatedSlateStyleSet() {return CreatedSlateStyleSet.ToSharedRef();}
};
