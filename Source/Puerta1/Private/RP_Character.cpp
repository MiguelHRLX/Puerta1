// Fill out your copyright notice in the Description page of Project Settings.


#include "RP_Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "RP_Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Puerta1/Puerta1.h"
#include "Components/RP_HealtComponent.h"


// Sets default values
ARP_Character::ARP_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseFirstPersonView = true;
	bIsDoingMelee = false;
	bCanUseWeapon = true;
	FPSCameraSocketName = "SCK_Camera";
	MeleeSocketName = "SCK_Melee";
	MeleeDamage = 10.0f;
	MaxComboMultiplier = 4.0f;
	CurrentComboMultiplier = 1.0f;

	FPSCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FPS_CameraComponent"));
	FPSCameraComponent->bUsePawnControlRotation = true;//rota siguiendo al mouse
	//FPSCameraComponent->SetupAttachment(RootComponent);
	FPSCameraComponent->SetupAttachment(GetMesh(), FPSCameraSocketName);



	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(RootComponent);


	TPSCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TPS_CameraComponent"));
	//TPSCameraComponent->bUsePawnControlRotation = true;//rota siguiendo al mouse
	//TPSCameraComponent->SetupAttachment(RootComponent);
	TPSCameraComponent->SetupAttachment(SpringArmComponent);

	MeleeDetectorComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MelleDetectorComponent"));
	MeleeDetectorComponent->SetupAttachment(GetMesh(),MeleeSocketName);
	MeleeDetectorComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	//MeleeDetectorComponent->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	MeleeDetectorComponent->SetCollisionResponseToChannel(COLLISION_ENEMY, ECR_Overlap);
	MeleeDetectorComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealtComponent = CreateDefaultSubobject<URP_HealtComponent>(TEXT("HealthComponent"));
	
}

FVector ARP_Character::GetPawnViewLocation() const
{
	//
	if(IsValid(FPSCameraComponent)&&bUseFirstPersonView)
	{
		return FPSCameraComponent->GetComponentLocation();

	}
	if(IsValid(TPSCameraComponent) && !bUseFirstPersonView)
	{
		return TPSCameraComponent->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

// Called when the game starts or when spawned
void ARP_Character::BeginPlay()
{
	Super::BeginPlay();
	InitializeReferences();
	CreateInitialWeapon();
	MeleeDetectorComponent->OnComponentBeginOverlap.AddDynamic(this,&ARP_Character::MakeMeleeDamage);
}

void ARP_Character::InitializeReferences()
{
	if(IsValid(GetMesh()))
	{
		MyAnimInstance = GetMesh()->GetAnimInstance();
	}

}

// Called every frame
void ARP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

// Called to bind functionality to input
void ARP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward",this ,&ARP_Character::MoveForward );
	PlayerInputComponent->BindAxis("MoveRight", this, &ARP_Character::MoveRight);

	PlayerInputComponent->BindAxis("LookUp",this,&ARP_Character::AddControllerPitchInput );
	PlayerInputComponent->BindAxis("LookRight", this, &ACharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ARP_Character::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ARP_Character::StopJumping);

	//PlayerInputComponent->BindAction("changeView", IE_Pressed, this, &ARP_Character::ChangeView);

	PlayerInputComponent->BindAction("WeaponAction", IE_Pressed, this, &ARP_Character::StartWeaponAction);
	PlayerInputComponent->BindAction("WeaponAction", IE_Released, this, &ARP_Character::StopWeaponAction);

	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &ARP_Character::StartMelee);
	PlayerInputComponent->BindAction("Melee", IE_Released, this, &ARP_Character::StopMelee);
	
}


void ARP_Character::ChangeView()
{
	bUseFirstPersonView = !bUseFirstPersonView;
	
}

void ARP_Character::AddControllerPitchInput(float value)
{
	Super::AddControllerPitchInput(bIsLookInversion? -value:value);
}

void ARP_Character::AddKey(FName NewKey)
{
	DoorKeys.Add(NewKey);
}

bool ARP_Character::HasKey(FName KeyTag)
{
	return DoorKeys.Contains(KeyTag);
}


void ARP_Character::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() * value);
}

void ARP_Character::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector()*value);
}

void ARP_Character::Jump()
{
	Super::Jump();
}

void ARP_Character::StopJumping()
{
	Super::StopJumping();
}

void ARP_Character::StartWeaponAction()
{
	if(!bCanUseWeapon)
	{
		return;
	}

	if(IsValid(CurrentWeapon))
	{
		CurrentWeapon->StartWeaponAction();
	}
	
}

void ARP_Character::StopWeaponAction()
{
	if (!bCanUseWeapon)
	{
		return;
	}
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->StopWeaponAction();
	}
}

void ARP_Character::StartMelee()
{
	if(bIsDoingMelee&&!bCanMakeCombos)
	{
		return;
	}

	if(bCanMakeCombos)
	{
		if(bIsDoingMelee)
		{
			if(bIsComboEnable)
			{
				if(CurrentComboMultiplier<MaxComboMultiplier)
				{
					CurrentComboMultiplier++;
					SetComboEnable(false);
				}
				else
				{
					return;
				}
			}else
			{
				return;
			}
			
		}
	}


	//UE_LOG(LogTemp, Log,TEXT( "melee start"));
   if(IsValid(MyAnimInstance)&&IsValid(MeleeMontage))
   {
	   MyAnimInstance->Montage_Play(MeleeMontage);
   }
   SetMeleeState(true);
}


void ARP_Character::StopMelee()
{
	//UE_LOG(LogTemp, Log, TEXT("melee stop"));
}

void ARP_Character::CreateInitialWeapon()
{
	if (IsValid(InitialWeaponClass))
	{
		CurrentWeapon = GetWorld()->SpawnActor<ARP_Weapon>(InitialWeaponClass, GetActorLocation(), GetActorRotation());
		if (IsValid(CurrentWeapon))
		{
			//CurrentWeapon->SetOwner(this);//el padre del arma es el jugador
			CurrentWeapon->SetCharacterOwner(this);//hce los mismo de arriba 
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

	}
}

void ARP_Character::MakeMeleeDamage(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(IsValid(OtherActor)){
	UGameplayStatics::ApplyPointDamage(OtherActor,MeleeDamage*CurrentComboMultiplier,SweepResult.Location,SweepResult,GetInstigatorController(),this,nullptr);
     }
}

void ARP_Character::SetMeleeDetectorCollision(ECollisionEnabled::Type NewCollisionState)
{
	MeleeDetectorComponent->SetCollisionEnabled(NewCollisionState);
}

void ARP_Character::SetMeleeState(bool NewState)
{
	bIsDoingMelee = NewState;
	bCanUseWeapon = !NewState;
}

void ARP_Character::SetComboEnable(bool NewState)
{
	bIsComboEnable = NewState;
}

void ARP_Character::ResetCombo()
{
	SetComboEnable(false);
	CurrentComboMultiplier = 1;

}
