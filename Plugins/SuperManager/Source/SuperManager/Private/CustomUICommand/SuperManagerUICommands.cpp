// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomUICommand/SuperManagerUICommands.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerUICommands::RegisterCommands()
{
	UI_COMMAND(
		LockActorSelection,
		"锁定选择的Actor",
		"在关卡中锁定选择的演员,一旦被触发,Actor将无法再被选中",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W,EModifierKey::Alt)
	);

	UI_COMMAND(
		UnlockActorSelection,
		"解锁选定的Actor",
		"移除所有锁定的Actor",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W,EModifierKey::Alt | EModifierKey::Shift)
	);
}

#undef LOCTEXT_NAMESPACE
