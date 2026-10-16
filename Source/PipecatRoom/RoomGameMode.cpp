//
// Copyright (c) 2026, Daily
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "RoomGameMode.h"

#include "RoomHouse.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "UObject/ConstructorHelpers.h"

ARoomGameMode::ARoomGameMode()
{
	// Keep the template's pawn and controller, including Enhanced Input setup.
	static ConstructorHelpers::FClassFinder<AGameModeBase> TemplateClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode"));
	if (TemplateClass.Succeeded())
	{
		const AGameModeBase* Template = TemplateClass.Class->GetDefaultObject<AGameModeBase>();
		DefaultPawnClass = Template->DefaultPawnClass;
		PlayerControllerClass = Template->PlayerControllerClass;
	}
}

void ARoomGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Entry has no PlayerStarts. Create two before login; GameMode chooses an
	// unoccupied one, including for late joins and respawns.
	const FRoomSpot Start = GetDefault<ARoomHouse>()->GetPlayerStart();
	for (float Offset : {-120.0f, 120.0f})
	{
		const FVector Location = Start.Location + FVector(0.0f, Offset, 100.0f);
		GetWorld()->SpawnActor<APlayerStart>(Location, FRotator(0.0f, Start.Yaw, 0.0f));
	}
}

APawn* ARoomGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	APawn* Pawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	if (Pawn)
	{
		Pawn->SetReplicates(true);
		Pawn->SetReplicateMovement(true);
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			// Match the owning client's indoor walking speed for prediction.
			Character->GetCharacterMovement()->MaxWalkSpeed = 380.0f;
		}
	}
	return Pawn;
}
