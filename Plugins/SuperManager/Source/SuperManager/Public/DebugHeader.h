// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Framework/Notifications/NotificationManager.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace DebugHeader
{
	static void Print(const FString& Message,const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,8.f,Color,Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp,Warning,TEXT("%s"),*Message);
	}

	//打开ue消息框.(中间的)
	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType,const FString& Message,bool bShowMsgAsWarning = true)
	{
		if(bShowMsgAsWarning)
		{
			FText MsgTitle = FText::FromString(TEXT("Warning"));
			return  FMessageDialog::Open(MsgType,FText::FromString(Message),&MsgTitle);
		}
		else
		{
			return  FMessageDialog::Open(MsgType,FText::FromString(Message));
		}
	}

	//ue 右下角通知
	static void ShowNotifyInfo(const FString& Message)
	{
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseThrobber = true;  //动画
		NotifyInfo.FadeOutDuration = 7.f; //输入几秒
	
		FSlateNotificationManager::Get().AddNotification(NotifyInfo);//增加一个浮动通知
	}
}
