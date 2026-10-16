//
// Copyright (c) 2026, Daily
//
// SPDX-License-Identifier: BSD-2-Clause
//

#include "RoomGameInstance.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "IPAddress.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "SocketSubsystem.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRoomNetwork, Log, All);

namespace
{
// Only travel to an endpoint, never interpret pasted map/options or console commands.
bool ParseEndpoint(FString Link, FString& Endpoint)
{
	Link.TrimStartAndEndInline();
	Link.RemoveFromStart(TEXT("unreal://"), ESearchCase::IgnoreCase);
	FString Host;
	FString Port;
	if (!Link.Split(TEXT(":"), &Host, &Port) || Host.IsEmpty() || Port.IsEmpty() || Port.Len() > 5)
	{
		return false;
	}
	for (TCHAR Character : Host)
	{
		if (!FChar::IsAlnum(Character) && Character != TEXT('.') && Character != TEXT('-'))
		{
			return false;
		}
	}
	for (TCHAR Character : Port)
	{
		if (Character < TEXT('0') || Character > TEXT('9'))
		{
			return false;
		}
	}
	const int32 Number = FCString::Atoi(*Port);
	if (Number < 1 || Number > 65535)
	{
		return false;
	}
	Endpoint = FString::Printf(TEXT("%s:%d"), *Host, Number);
	return true;
}

FAutoConsoleCommandWithWorldAndArgs JoinCommand(
	TEXT("Room.Join"),
	TEXT("Join a host: Room.Join unreal://host:7777 (or host:7777)"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic([](const TArray<FString>& Args, UWorld* World) {
		if (URoomGameInstance* Game = World ? Cast<URoomGameInstance>(World->GetGameInstance()) : nullptr)
		{
			if (Args.Num() == 1)
			{
				Game->Join(Args[0]);
			}
			else
			{
				UE_LOG(LogRoomNetwork, Warning, TEXT("Usage: Room.Join unreal://host:7777"));
			}
		}
	}));

FAutoConsoleCommandWithWorldAndArgs JoinLinkCommand(
	TEXT("Room.JoinLink"),
	TEXT("Print this listen server's join link again; use -AdvertiseHost=PUBLIC_IP_OR_DNS for remote guests"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic([](const TArray<FString>&, UWorld* World) {
		if (URoomGameInstance* Game = World ? Cast<URoomGameInstance>(World->GetGameInstance()) : nullptr)
		{
			Game->ShowJoinLink();
		}
	}));
} // namespace

void URoomGameInstance::Init()
{
	Super::Init();
	FParse::Value(FCommandLine::Get(), TEXT("ConnectUrl="), ConnectUrl);
}

void URoomGameInstance::OnStart()
{
	Super::OnStart();
	if (!ConnectUrl.IsEmpty() && GetWorld())
	{
		// Wait for the startup world's controller and BeginPlay. GameInstance
		// survives travel, so consume the command line only once.
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() {
			const FString Link = MoveTemp(ConnectUrl);
			ConnectUrl.Reset();
			Join(Link);
		}));
	}
}

void URoomGameInstance::Join(const FString& Link)
{
	FString Endpoint;
	if (!ParseEndpoint(Link, Endpoint))
	{
		UE_LOG(LogRoomNetwork, Warning, TEXT("Invalid join link. Expected unreal://host:port or host:port (IPv4 or DNS)."));
		return;
	}
	UWorld* World = GetWorld();
	APlayerController* Player = GetFirstLocalPlayerController();
	if (!World || !Player || World->GetNetMode() == NM_ListenServer)
	{
		UE_LOG(LogRoomNetwork, Warning, TEXT("Join from a standalone game or client, not an active listen server."));
		return;
	}
	UE_LOG(LogRoomNetwork, Log, TEXT("Connecting to %s"), *Endpoint);
	Player->ClientTravel(Endpoint, TRAVEL_Absolute);
}

void URoomGameInstance::ShowJoinLink()
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() != NM_ListenServer)
	{
		UE_LOG(LogRoomNetwork, Warning, TEXT("Host with /Engine/Maps/Entry?listen to get a join link."));
		return;
	}
	FString Host;
	if (!FParse::Value(FCommandLine::Get(), TEXT("AdvertiseHost="), Host))
	{
		bool bCanBindAll = false;
		if (ISocketSubsystem* Sockets = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
		{
			const TSharedRef<FInternetAddr> Address = Sockets->GetLocalHostAddr(*GLog, bCanBindAll);
			Host = Address->ToString(false);
		}
		UE_LOG(LogRoomNetwork, Log, TEXT("Detected a local address; Internet guests need -AdvertiseHost=PUBLIC_IP_OR_DNS and UDP port forwarding."));
	}
	FString Endpoint;
	if (ParseEndpoint(FString::Printf(TEXT("%s:%d"), *Host, World->URL.Port), Endpoint))
	{
		UE_LOG(LogRoomNetwork, Display, TEXT("Join link: unreal://%s"), *Endpoint);
	}
	else
	{
		UE_LOG(LogRoomNetwork, Warning, TEXT("Cannot advertise this address; set -AdvertiseHost=IPV4_OR_DNS."));
	}
}
