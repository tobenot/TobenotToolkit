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
	
	// 这里你可以增加Typeface选择和是否应该更改Typeface的控件
	// 例如，添加一个下拉框来选择Typeface
	TSharedRef<SComboBox<FName>> TypefaceComboBox = SNew(SComboBox<FName>)
	.OptionsSource(&TypefaceOptions)
	.OnGenerateWidget_Lambda([](FName Item)
	{
		return SNew(STextBlock).Text(FText::FromName(Item));
	})
	.OnSelectionChanged_Lambda([this](FName Item, ESelectInfo::Type SelectInfo)
	{
		OnTypefaceSelected(Item, SelectInfo);
	})
	.Content()
	[
		SNew(STextBlock)
		.MinDesiredWidth(200)
		.Text_Lambda([this]()
		{
			return FText::FromName(SelectedTypefaceName);
		})
	];
	
	TypefaceComboBoxPtr = TypefaceComboBox;
	
	TSharedRef<SHorizontalBox> CheckboxWithLabel = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(SCheckBox)
		.IsChecked(false)
		.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
		{
			bShouldChangeTypeface = (NewState == ECheckBoxState::Checked);
			UE_LOG(LogTemp, Log, TEXT("应改变Typeface设置为：%s"), bShouldChangeTypeface ? TEXT("true") : TEXT("false"));
		})
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(10, 0, 0, 0)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Change Typeface")))
	];
	
	// 现有的 AssetPickerWidget 设置内容
	// 我们可以将其包含在一个水平或垂直布局中，一起和新的控件
	TSharedRef<SVerticalBox> MainVerticalBox = SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.FillHeight(1.0)
	[
		AssetPickerWidget
	]
	// 这里添加我们的复选框控件
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		CheckboxWithLabel
	]
	// 这里添加我们的Typeface选择器
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		TypefaceComboBox
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(2)
	[
		SNew(SButton)
		.Text(NSLOCTEXT("UnrealEd", "ConfirmButton", "确认"))
		.OnClicked_Lambda([this]() -> FReply
		{
			return OnConfirmButtonClick();
		})
	];

	PickerWindow->SetContent(MainVerticalBox);
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
	SelectedFont = Cast<UFont>(AssetData.GetAsset());
	if (!SelectedFont)
	{
		UE_LOG(LogTemp, Warning, TEXT("选中的资源不是字体！"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("用户已选择字体：%s"), *SelectedFont->GetName());
	PopulateTypefaceOptions();

	TSharedPtr<SComboBox<FName>> TypefaceComboBox = TypefaceComboBoxPtr.Pin();
	if(TypefaceComboBox.IsValid())
	{
		TypefaceComboBox->RefreshOptions(); 
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
	for (UWidget* EachWidget : AllWidgets) {
		SlowTask.EnterProgressFrame(1, FText::Format(NSLOCTEXT("UnrealEd", "ProcessingWidget", "正在处理: {0}"), FText::FromString(EachWidget->GetName())));
    
		if (UTextBlock* TextBlock = Cast<UTextBlock>(EachWidget)) {
			ChangeTextBlockFont(TextBlock, NewFont);
		} else if (UEditableText* EditableText = Cast<UEditableText>(EachWidget)) {
			ChangeEditableTextFont(EditableText, NewFont);
		} else if (UEditableTextBox* EditableTextBox = Cast<UEditableTextBox>(EachWidget)) {
			ChangeEditableTextBoxFont(EditableTextBox, NewFont);
		} else if (UMultiLineEditableText* MultiLineEditableText = Cast<UMultiLineEditableText>(EachWidget)) {
			ChangeMultiLineEditableTextFont(MultiLineEditableText, NewFont);
		} else if (UMultiLineEditableTextBox* MultiLineEditableTextBox = Cast<UMultiLineEditableTextBox>(EachWidget)) {
			ChangeMultiLineEditableTextBoxFont(MultiLineEditableTextBox, NewFont);
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

// Utility function to apply new font settings to a SlateFontInfo structure
FSlateFontInfo UUMGFontChanger::GetUpdatedFontInfo(const FSlateFontInfo& CurrentFontInfo, UFont* NewFont, bool bInShouldChangeTypeface, const FName& InSelectedTypefaceName)
{
	FSlateFontInfo UpdatedFontInfo = CurrentFontInfo;
	UpdatedFontInfo.FontObject = NewFont;

	if (bInShouldChangeTypeface)
	{
		UpdatedFontInfo.TypefaceFontName = InSelectedTypefaceName;
	}

	return UpdatedFontInfo;
}

void UUMGFontChanger::ChangeEditableTextFont(UEditableText* EditableText, UFont* NewFont)
{
	FSlateFontInfo NewFontInfo = GetUpdatedFontInfo(EditableText->GetFont(), NewFont, bShouldChangeTypeface, SelectedTypefaceName);
	EditableText->SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("EditableText %s 的字体已经更改。"), *EditableText->GetName());
}

void UUMGFontChanger::ChangeEditableTextBoxFont(UEditableTextBox* EditableTextBox, UFont* NewFont)
{
	FSlateFontInfo NewFontInfo = GetUpdatedFontInfo(EditableTextBox->WidgetStyle.TextStyle.Font, NewFont, bShouldChangeTypeface, SelectedTypefaceName);
	EditableTextBox->WidgetStyle.SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("EditableTextBox %s 的字体已经更改。"), *EditableTextBox->GetName());
}

void UUMGFontChanger::ChangeTextBlockFont(UTextBlock* TextBlock, UFont* NewFont)
{
	FSlateFontInfo NewFontInfo = GetUpdatedFontInfo(TextBlock->GetFont(), NewFont, bShouldChangeTypeface, SelectedTypefaceName);
	TextBlock->SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("TextBlock %s 的字体已经更改。"), *TextBlock->GetName());
}
void UUMGFontChanger::ChangeMultiLineEditableTextFont(UMultiLineEditableText* MultiLineEditableText, UFont* NewFont)
{
	FSlateFontInfo NewFontInfo = GetUpdatedFontInfo(MultiLineEditableText->GetFont(), NewFont, bShouldChangeTypeface, SelectedTypefaceName);
	MultiLineEditableText->SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("MultiLineEditableText %s 的字体已经更改。"), *MultiLineEditableText->GetName());
}

void UUMGFontChanger::ChangeMultiLineEditableTextBoxFont(UMultiLineEditableTextBox* MultiLineEditableTextBox, UFont* NewFont)
{
	FSlateFontInfo NewFontInfo = GetUpdatedFontInfo(MultiLineEditableTextBox->WidgetStyle.TextStyle.Font, NewFont, bShouldChangeTypeface, SelectedTypefaceName);
	MultiLineEditableTextBox->WidgetStyle.SetFont(NewFontInfo);
	UE_LOG(LogTemp, Log, TEXT("MultiLineEditableTextBox %s 的字体已经更改。"), *MultiLineEditableTextBox->GetName());
}

void UUMGFontChanger::PopulateTypefaceOptions()
{
	if (SelectedFont == nullptr)
	{
		// 错误处理: 字体为空
		UE_LOG(LogTemp, Warning, TEXT("无法加载Typeface选项，因为SelectedFont为null。"));
		return;
	}

	TypefaceOptions.Empty(); // 清空之前的选项

	// 获取字体的CompositeFont属性，这是个FTypedCompositeFont结构体
	FCompositeFont CompositeFont = SelectedFont->CompositeFont;

	// 遍历所有的Typeface，填充TypefaceOptions
	for(const FTypefaceEntry& TypefaceEntry : CompositeFont.DefaultTypeface.Fonts)
	{
		FName TypefaceName = TypefaceEntry.Name;
		TypefaceOptions.Add(TypefaceName);
	}
}

void UUMGFontChanger::OnTypefaceSelected(FName NewValue, ESelectInfo::Type SelectInfo)
{
	if (NewValue.IsValid())
	{
		SelectedTypefaceName = NewValue;
		UE_LOG(LogTemp, Log, TEXT("用户已选择Typeface：%s"), *SelectedTypefaceName.ToString());
	}
}

FReply UUMGFontChanger::OnConfirmButtonClick()
{
	// Close the font picker window
	TSharedPtr<SWindow> PickerWindow = PickerWindowPtr.Pin();
	if (PickerWindow.IsValid())
	{
		PickerWindow->RequestDestroyWindow();
	}

	// First check if SelectedFont is valid before proceeding
	if (!SelectedFont)
	{
		// Log an error if SelectedFont is null
		UE_LOG(LogTemp, Error, TEXT("Font change operation cannot proceed because no font is selected."));
		FNotificationInfo Info(FText::FromString("Font change operation failed: No font is selected."));
		Info.bFireAndForget = true;
		Info.FadeOutDuration = 3.0f;
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);

		// Return FReply::Unhandled() to indicate the operation was not successful
		return FReply::Unhandled();
	}

	if (bShouldChangeTypeface && SelectedTypefaceName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("Font change operation cannot proceed because no typeface is selected."));
		FNotificationInfo Info(FText::FromString("Font change operation cannot proceed because no typeface is selected."));
		Info.bFireAndForget = true;
		Info.FadeOutDuration = 3.0f;
		Info.ExpireDuration = 2.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Unhandled();
	}

	// Perform the font change operation
	TArray<UObject*> SelectedWidgets = UEditorUtilityLibrary::GetSelectedAssets();

	for (UObject* Obj : SelectedWidgets)
	{
		UWidgetBlueprint* WidgetBP = Cast<UWidgetBlueprint>(Obj);
		if (WidgetBP)
		{
			ChangeFontInWidget(WidgetBP, SelectedFont);
		}
	}

	// Return FReply::Handled() to indicate the event has been processed
	return FReply::Handled();
}
