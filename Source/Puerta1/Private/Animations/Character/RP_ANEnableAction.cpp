// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/Character/RP_ANEnableAction.h"

#include "RP_Character.h"

void URP_ANEnableAction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AActor* CharacterActor = MeshComp->GetOwner();
	if (IsValid(CharacterActor))
	{
		ARP_Character* Character = Cast<ARP_Character>(CharacterActor);
		if (IsValid(Character))
		{
			Character->SetMeleeState(false);
		}
	}
}
