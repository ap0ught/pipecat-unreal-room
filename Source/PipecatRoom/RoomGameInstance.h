//
// Copyright (c) 2026, Daily
//
// SPDX-License-Identifier: BSD-2-Clause
//

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "RoomGameInstance.generated.h"

UCLASS()
class URoomGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void OnStart() override;

	bool HasStartupConnection() const { return !ConnectUrl.IsEmpty(); }
	void Join(const FString& Link);
	void ShowJoinLink();

private:
	FString ConnectUrl;
};
