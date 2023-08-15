// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Framework/Commands/Commands.h"

class FSuperManagerUICommands : public TCommands<FSuperManagerUICommands>
{
public:
	FSuperManagerUICommands() : TCommands<FSuperManagerUICommands>(
		TEXT("SuperManager")
		,FText::FromString(TEXT("超级管理器UI命令"))
		,NAME_None,
		TEXT("SuperManager")
		){}
	

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> LockActorSelection;
	TSharedPtr<FUICommandInfo> UnlockActorSelection;
	
};

