// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.

#include "UMGFontChanger.h"
#include "ScopedTransaction.h"
#include "WidgetBlueprint.h"
#include "Blueprint/UserWidget.h"
#include "EditorUtilityLibrary.h"
#include "Engine/Font.h"

#include "Blueprint/WidgetTree.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"
#include "Editor/ContentBrowser/Public/IContentBrowserSingleton.h"
#include "Kismet2/BlueprintEditorUtils.h" // for MarkBlueprintAsModified()

// 导入进度条和消息框相关头文件
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"


void UUMGFontChanger::ChangeFontsInSelectedWidgets()
{
	UE_LOG(LogTemp, Log, TEXT("开始更换选中Widget中的字体。"));

	// 创建字体选择对话框
	TSharedPtr<SWindow> PickerWindow = SNew(SWindow)
		.Title(NSLOCTEXT("UnrealEd", "FontPicker", "选择字体"))
		.ClientSize(FVector2D(350, 550))
		.SupportsMaximize(false)
		.SupportsMinimize(false);
	
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassPaths.Add(UFont::StaticClass()->GetClassPathName());
	AssetPickerConfig.Filter.bRecursiveClasses = true;

	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateUObject(this,&UUMGFontChanger::OnFontSelected);
	
	const TSharedRef<SWidget> AssetPickerWidget = ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);

	PickerWindow->SetContent(AssetPickerWidget);
	PickerWindowPtr = PickerWindow;
	
	// 显示窗口
	FSlateApplication::Get().AddWindow(PickerWindow.ToSharedRef());
	UE_LOG(LogTemp, Log, TEXT("字体选择对话框已显示。"));
}

/*UUMGFontChanger::UUMGFontChanger()
{
	//SupportedClasses.Add(UWidgetBlueprint::StaticClass());
}*/

void UUMGFontChanger::OnFontSelected(const FAssetData& AssetData)
{
	UFont* SelectedFont = Cast<UFont>(AssetData.GetAsset());
	if (!SelectedFont)
	{
		UE_LOG(LogTemp, Warning, TEXT("选中的资源不是字体！"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("用户已选择字体：%s"), *SelectedFont->GetName());

	TArray<UObject*> SelectedWidgets = UEditorUtilityLibrary::GetSelectedAssets();
	// 在选择字体后续操作中，增加进度条显示的相关代码
	// FSlateNotificationManager::Get().AddNotification(FNotificationInfo(NSLOCTEXT("UnrealEd", "FontChangeInProgress", "正在更换字体...")));
    
	for (UObject* Obj : SelectedWidgets)
	{
		UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(Obj);
		if (WidgetBP)
		{
			ChangeFontInWidget(WidgetBP, SelectedFont);
		}
	}
}

void UUMGFontChanger::ChangeFontInWidget(UWidgetBlueprint* WidgetBP, UFont* NewFont)
{
	// 开始一个新事务，所有的字体更换都在这个事务中完成
	FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ChangeFont", "更换字体"));

	UE_LOG(LogTemp, Log, TEXT("开始遍历%s中的控件，更换字体为：%s。"), *WidgetBP->GetName(), *NewFont->GetName());

	TArray<UWidget*> AllWidgets = WidgetBP->GetAllSourceWidgets();
	FScopedSlowTask SlowTask(AllWidgets.Num(), NSLOCTEXT("UnrealEd", "ChangingFonts", "更换字体中..."));
	SlowTask.MakeDialog();
	
	WidgetBP->Modify();
	// 遍历UMG中的所有控件开始更换字体
	for (UWidget* EachWidget : AllWidgets)
	{
		// 更新进度并显示当前处理的控件
		SlowTask.EnterProgressFrame(1, FText::Format(NSLOCTEXT("UnrealEd", "ProcessingWidget", "正在处理: {0}"), FText::FromString(EachWidget->GetName())));
		// 处理文本控件
		UTextBlock* TextBlock = Cast<UTextBlock>(EachWidget);
		if (TextBlock)
		{
			ChangeTextBlockFont(TextBlock, NewFont);
		}
		// 处理可编辑文本控件
		UEditableText* EditableText = Cast<UEditableText>(EachWidget);
		if (EditableText)
		{
			ChangeEditableTextFont(EditableText, NewFont);
		}
		// 处理可编辑文本框控件
		UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(EachWidget);
		if (EditableTextBox)
		{
			ChangeEditableTextBoxFont(EditableTextBox, NewFont);
		}
	}
	// 标记蓝图已修改，完成事务
	FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBP);
	
	FNotificationInfo Info(NSLOCTEXT("UnrealEd", "FontChangeComplete", "控件字体更换完成！"));
	Info.bFireAndForget = true;
	Info.FadeOutDuration = 3.0f;
	Info.ExpireDuration = 2.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	UE_LOG(LogTemp, Log, TEXT("%s中的所有控件字体更改完成。"), *WidgetBP->GetName());
}

void UUMGFontChanger::ChangeEditableTextFont(UEditableText* EditableText, UFont* NewFont)
{
	FSlateFontInfo NewFontInfo = EditableText->GetFont();
	NewFontInfo.FontObject = NewFont;
	EditableText->SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("EditableText %s 的字体已经更改。"), *EditableText->GetName());
}

void UUMGFontChanger::ChangeEditableTextBoxFont(UEditableTextBox* EditableTextBox, UFont* NewFont)
{
	FSlateFontInfo NewFontInfo = EditableTextBox->WidgetStyle.TextStyle.Font;
	NewFontInfo.FontObject = NewFont;
	EditableTextBox->WidgetStyle.SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("EditableTextBox %s 的字体已经更改。"), *EditableTextBox->GetName());
}

void UUMGFontChanger::ChangeTextBlockFont(UTextBlock* TextBlock, UFont* NewFont)
{
    FSlateFontInfo NewFontInfo = TextBlock->GetFont();
    NewFontInfo.FontObject = NewFont;
	TextBlock->SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("TextBlock %s 的字体已经更改。"), *TextBlock->GetName());
}