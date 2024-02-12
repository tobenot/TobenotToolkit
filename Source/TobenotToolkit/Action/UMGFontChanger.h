// Copyright (c) 2024 tobenot
// This code is licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "CoreMinimal.h"
#include "Blutility/Classes/AssetActionUtility.h"
#include "Fonts/SlateFontInfo.h"

#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"

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
    
private:
    // 遍历UMG中的所有有字体的控件并更换字体
    void ChangeFontInWidget(UWidgetBlueprint* WidgetBP, UFont* NewFont);
    
    void ChangeEditableTextFont(UEditableText* EditableText, UFont* NewFont);
    void ChangeEditableTextBoxFont(UEditableTextBox* EditableTextBox, UFont* NewFont);
    void ChangeTextBlockFont(UTextBlock* TextBlock, UFont* NewFont);

    void OnFontSelected(const FAssetData& AssetData);

    TWeakPtr<SWindow> PickerWindowPtr;  // 用来存储字体选择器窗口的弱引用
};