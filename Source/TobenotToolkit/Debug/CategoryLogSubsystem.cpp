// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.

#include "CategoryLogSubsystem.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

void UCategoryLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 初始化默认日志类别名称
	DefaultLogCategory = TEXT("Default");
}

FString UCategoryLogSubsystem::GetOrCreateLogFile(const FName& LogCategory)
{
	// 检查LogCategory的合法性
	FString ValidLogCategory = LogCategory.ToString();
	if (!IsValidLogCategory(ValidLogCategory))
	{
		// 如果不合法，使用默认类别名称
		ValidLogCategory = DefaultLogCategory;
	}
    
	// 检查是否已经有这个类别的日志文件
	FString* LogFileName = LogFiles.Find(*ValidLogCategory);
	if (!LogFileName)
	{
		// 如果没有，则创建一个新的文件名
		FDateTime Now = FDateTime::Now();
		FString FileName = FString::Printf(TEXT("%s_%s.txt"), *ValidLogCategory, *Now.ToString(TEXT("%Y%m%d%H%M%S")));
		FString FullPath = FPaths::Combine(*FPaths::ProjectSavedDir(), TEXT("CLogs"), *ValidLogCategory, FileName);
		LogFiles.Add(*ValidLogCategory, FullPath);
		return FullPath;
	}
	return *LogFileName;
}

bool UCategoryLogSubsystem::IsValidLogCategory(const FString& CategoryName)
{
	// 这里可以实现具体的合法性检查逻辑
	// 例如，检查是否包含非法字符等
	// 此处仅为示例，实际逻辑需要根据具体需求实现
	return CategoryName.Len() > 0 && !CategoryName.Contains(TEXT(" "));
}

void UCategoryLogSubsystem::WriteLog(const FName& LogCategory, const FString& LogText)
{
	FString LogFilePath = GetOrCreateLogFile(LogCategory);

	// 确保日志目录存在
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(LogFilePath));

	// 获取当前时间，格式化为包含毫秒的字符串
	FDateTime Now = FDateTime::Now();
	FString TimeStamp = Now.ToString(TEXT("%Y-%m-%d %H:%M:%S.%s"));

	// 创建带时间戳的日志文本
	FString LogEntry = FString::Printf(TEXT("[%s] %s\n"), *TimeStamp, *LogText);

	// 将日志文本追加到文件
	FFileHelper::SaveStringToFile(LogEntry, *LogFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get(), FILEWRITE_Append);
}