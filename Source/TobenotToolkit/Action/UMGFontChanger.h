// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Blutility/Classes/AssetActionUtility.h"
#include "Fonts/SlateFontInfo.h"

#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableText.h"
#include "Components/MultiLineEditableTextBox.h"

#include "UMGFontChanger.generated.h"

class UWidgetBlueprint;

UCLASS(BlueprintType,Blueprintable)
class TOBENOTTOOLKIT_API UUMGFontChanger : public UAssetActionUtility
{
    GENERATED_BODY()

public:
    //UUMGFontChanger();
    
    // 此函数可以在编辑器的资产操作菜单中调用
    UFUNCTION(CallInEditor)
    void ChangeFontsInSelectedWidgets();
    
protected:
    UPROPERTY()
    UFont* SelectedFont;
    
    UPROPERTY()
    TArray<FName> TypefaceOptions;
private:
    TWeakPtr<SWindow> PickerWindowPtr;

    TWeakPtr<SComboBox<FName>> TypefaceComboBoxPtr;
    
    // 遍历UMG中的所有有字体的控件并更换字体
    void ChangeFontInWidget(UWidgetBlueprint* WidgetBP, UFont* NewFont);
    
    FSlateFontInfo GetUpdatedFontInfo(const FSlateFontInfo& CurrentFontInfo, UFont* NewFont, bool bInShouldChangeTypeface, const FName& InSelectedTypefaceName);
    void ChangeEditableTextFont(UEditableText* EditableText, UFont* NewFont);
    void ChangeEditableTextBoxFont(UEditableTextBox* EditableTextBox, UFont* NewFont);
    void ChangeTextBlockFont(UTextBlock* TextBlock, UFont* NewFont);
    void ChangeMultiLineEditableTextFont(UMultiLineEditableText* MultiLineEditableText, UFont* NewFont);
    void ChangeMultiLineEditableTextBoxFont(UMultiLineEditableTextBox* MultiLineEditableTextBox, UFont* NewFont);
    
    void OnFontSelected(const FAssetData& AssetData);

    
    // 用来指示是否应更改Typeface
    bool bShouldChangeTypeface;
    // 存储用户选择的Typeface
    FName SelectedTypefaceName;

    void PopulateTypefaceOptions();
    // 当用户从Typeface选择器中选择一个Typeface时调用
    void OnTypefaceSelected(FName NewValue, ESelectInfo::Type SelectInfo);

    FReply OnConfirmButtonClick();
};