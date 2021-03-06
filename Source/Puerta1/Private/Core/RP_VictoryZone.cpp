// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RP_VictoryZone.h"
#include "Components/BoxComponent.h"
#include "RP_Character.h"
#include "Core/RP_GameMod.h"
// Sets default values
ARP_VictoryZone::ARP_VictoryZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VictoryZoneComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("VictoryZoneComponent"));
	RootComponent = VictoryZoneComponent;
	VictoryZoneComponent->SetBoxExtent(FVector(100.0f));


}

// Called when the game starts or when spawned
void ARP_VictoryZone::BeginPlay()
{
	Super::BeginPlay();
	GameModeReference = Cast<ARP_GameMod>(GetWorld()->GetAuthGameMode());
}

// Called every frame
void ARP_VictoryZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARP_VictoryZone::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (IsValid(OtherActor) && IsValid(GameModeReference))
	{
		ARP_Character*RPCharacter = Cast<ARP_Character>(OtherActor);
		if (IsValid(RPCharacter) &&RPCharacter->GetCharacterTpe() == ERP_CharacterType::CharacterType_Player)
		{
			GameModeReference->Victory(RPCharacter);
		}

	}

}

