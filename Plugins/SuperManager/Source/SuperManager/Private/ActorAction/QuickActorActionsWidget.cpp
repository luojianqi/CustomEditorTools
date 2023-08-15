// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorAction/QuickActorActionsWidget.h"

#include "DebugHeader.h"
#include "Subsystems/EditorActorSubsystem.h"

void UQuickActorActionsWidget::SelectAllActorsWithSimilarName()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("没有Actor被选择"));
		return;
	}

	if (SelectedActors.Num() > 1)
	{
		DebugHeader::ShowNotifyInfo(TEXT("你仅仅只能选择一个Actor"));
		return;
	}

	FString SelectedActorName = SelectedActors[0]->GetActorLabel();
	const FString NameToSearch = SelectedActorName.LeftChop(4);

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	for (AActor* ActorInLevel : AllLevelActors)
	{
		if (!ActorInLevel) continue;
		if (ActorInLevel->GetActorLabel().Contains(NameToSearch, SearchCase))
		{
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel, true); //为选定的参与者设置选择状态
			SelectionCounter++;
		}
	}

	if (SelectionCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("成功选择") + FString::FromInt(SelectionCounter) + TEXT("actors"));
	}
	else
	{
		DebugHeader::ShowNotifyInfo(TEXT("没有找到相似名字的Actor"));
	}
}

#pragma region ActorBatchDuplication  //批量复制Actor
void UQuickActorActionsWidget::DuplicateActors()
{
	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("没有Actor被选择"));
		return;
	}

	// if(SelectedActors.Num() > 1)
	// {
	// 	DebugHeader::ShowNotifyInfo(TEXT("你仅仅只能选择一个Actor"));
	// 	return;
	// }

	if (NumberOfDuplicates <= 0 || offsetDist == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("未指定复制次数或偏移距离"));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) continue;

		for (int32 i = 0; i < NumberOfDuplicates; i++)
		{
			AActor* DuplicateActor = EditorActorSubsystem->DuplicateActor(SelectedActor, SelectedActor->GetWorld());

			if (!DuplicateActor) continue;

			const float DuplicationOffsetDist = (i + 1) * offsetDist;

			switch (AxisForDuplication)
			{
			case E_DuplicationAxis::EDA_XAxis:
				DuplicateActor->AddActorWorldOffset(FVector(DuplicationOffsetDist, 0.f, 0.f));
				break;
			case E_DuplicationAxis::EDA_YAxis:
				DuplicateActor->AddActorWorldOffset(FVector(0.f, DuplicationOffsetDist, 0.f));
				break;
			case E_DuplicationAxis::EDA_ZAxis:
				DuplicateActor->AddActorWorldOffset(FVector(0.f, 0.f, DuplicationOffsetDist));
				break;
			case E_DuplicationAxis::EDA_DefaultMAX:
				break;
			default:
				break;
			}

			EditorActorSubsystem->SetActorSelectionState(DuplicateActor, true);
			Counter++;
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("成功复制") + FString::FromInt(Counter) + TEXT("个Actors"));
	}
}
#pragma endregion

void UQuickActorActionsWidget::RandomizeActorTransform()
{
	const bool bConditionNotSet = !RandomActorRotation.bRandomizeRotYaw
		&& !RandomActorRotation.bRandomizeRotPitch
		&& !RandomActorRotation.bRandomizeRotRoll
		&& !bRandomizeScale
		&& !bRandomizeOffset;

	if (bConditionNotSet)
	{
		DebugHeader::ShowNotifyInfo(TEXT("没有指定属性条件"));
		return;
	}

	if (!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("没有Actor被选择"));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor) continue;

		if (RandomActorRotation.bRandomizeRotYaw)
		{
			const float RandomRotYawValue = FMath::RandRange(RandomActorRotation.RotYawMin,
			                                                 RandomActorRotation.RotYawMax);

			SelectedActor->AddActorWorldRotation(FRotator(0.f, RandomRotYawValue, 0.f));
		}

		if (RandomActorRotation.bRandomizeRotPitch)
		{
			const float RandomRotPitchValue = FMath::RandRange(RandomActorRotation.RotPitchMin,
			                                                   RandomActorRotation.RotPitchMax);

			SelectedActor->AddActorWorldRotation(FRotator(RandomRotPitchValue, 0.f, 0.f));
		}

		if (RandomActorRotation.bRandomizeRotRoll)
		{
			const float RandomRotRollValue = FMath::RandRange(RandomActorRotation.RotRollMin,
			                                                  RandomActorRotation.RotRollMax);

			SelectedActor->AddActorWorldRotation(FRotator(0.f, 0.f, RandomRotRollValue));
		}

		if(bRandomizeScale)
		{
			SelectedActor->SetActorScale3D(FVector(FMath::RandRange(ScaleMin,ScaleMax)));
		}

		if(bRandomizeOffset)
		{
			const float RandomOffsetValue = FMath::RandRange(OffsetMin,OffsetMax);
			
			SelectedActor->AddActorWorldOffset(FVector(RandomOffsetValue,RandomOffsetValue,0.f));
		}
		Counter++;
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("成功设置") + FString::FromInt(Counter) + TEXT("个Actors"));
	}
}

bool UQuickActorActionsWidget::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}

	return EditorActorSubsystem != nullptr;
}
