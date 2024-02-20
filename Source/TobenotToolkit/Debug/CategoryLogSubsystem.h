// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "CategoryLogSubsystem.generated.h"

UCLASS()
class TOBENOTTOOLKIT_API UCategoryLogSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 初始化函数，在WorldSubsystem启动时调用
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 用于写日志的接口，传入日志类别和内容
	void WriteLog(const FName& LogCategory, const FString& LogText);

private:
	// 维护一个TMap，存储日志类别和对应的文件名
	TMap<FName, FString> LogFiles;

	// 辅助函数，用于获取或创建日志文件名
	FString GetOrCreateLogFile(const FName& LogCategory);

private:
	FString DefaultLogCategory;
	bool IsValidLogCategory(const FString& CategoryName);
};