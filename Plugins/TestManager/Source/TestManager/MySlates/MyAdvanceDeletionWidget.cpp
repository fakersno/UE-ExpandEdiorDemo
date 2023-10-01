// Vince Petrelli All Rights Reserverd

//为了创建自定义的Slate小部件，需要先建一个空类
#include "MyAdvanceDeletionWidget.h"
#include "SlateBasics.h"
#include "MyDebugHeader.h"
#include "TestManager.h"

#define ListAll TEXT("List All Available Assets")
#define  ListUnused TEXT("List Unused Assets")
#define ListSameName TEXT("List Assets With Same Name")
void SMyAdvanceDeletionTab::Construct(const FArguments& InArgs)//构造函数
{
	///**小部件是否支持键盘焦点*/
	bCanSupportFocus = true;

	FSlateFontInfo TitleTextFont = GetEmboseedTextFont();//字体设置
	TitleTextFont.Size = 32;
	MyStoredAssetsData = InArgs._MyAssetsDataToStore;
	MyDisplayedAssetsData = MyStoredAssetsData;
	CheckBoxesArray.Empty();
	AssetDataDeleteArray.Empty();
	ComboBoxSoureItems.Empty();
	ComboBoxSoureItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSoureItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSoureItems.Add(MakeShared<FString>(ListSameName));
	ChildSlot
		[
			//InArgs._TestString 获取了 TestString 参数的值
			SNew(SVerticalBox)//生成文本块,//Main vertical box
			+ SVerticalBox::Slot()//加插槽
			.AutoHeight()//根据字体大小自动高度
			[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("My first Advantance Dletion")))
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::Blue)
			.Font(TitleTextFont)
			]
			+SVerticalBox::Slot()//SecondSlot用于下拉指定列表条件和帮助文本
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					ConstructComBox()//创建选择框
				]
				+SHorizontalBox::Slot()
				.FillWidth(.6f)//可用空间按比例分配
				[
					ConstructComboHelpTexts(TEXT("Specify the listing condition in the drop down. Left mouse click to go to where asset is located"),ETextJustify::Center )
				]
				+ SHorizontalBox::Slot()
					.FillWidth(.1f)
				[
						ConstructComboHelpTexts(TEXT("Current Folder:\n") + InArgs._CurrentSelectedFloder, ETextJustify::Right)//根路径
				]
			]
			+SVerticalBox::Slot()//资产列表的第三个槽位
			.VAlign(VAlign_Fill)//让该槽位的内容充满垂直方向的可用空间
			[
				SNew(SScrollBox)//下拉框
				+SScrollBox::Slot()
				[
					ConstructAssetListView()//列表数据
				]
			]
			+ SVerticalBox::Slot()//第四个插槽，可设置3个按钮
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(10.0f)
					.Padding(5.0f)
					[
					ConstructDeleteAllButton()
					]
					+ SHorizontalBox::Slot()
					.FillWidth(10.0f)
					.Padding(5.0f)
					[
					ConstructSelectButton()
					]
					+ SHorizontalBox::Slot()
					.FillWidth(10.0f)//插槽的宽度比例为 10.0
					.Padding(5.0f)//填充值
					[
						ConstructDeselectAllButton()
					]
			]
		];
}
TSharedRef<SListView<TSharedPtr<FAssetData>>> SMyAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView<TSharedPtr<FAssetData>>)//Slate链表
		.ItemHeight(24.f)
		.ListItemsSource(&MyDisplayedAssetsData)
		.OnGenerateRow(this, &SMyAdvanceDeletionTab::OnGenerateRowForList)//一行调用一个，设置生成列表行的回调函数为SMyAdvanceDeletionTab类中的OnGenerateRowForList函数。
		.OnMouseButtonClick(this, &SMyAdvanceDeletionTab::OnRowWidgetMouseButtonClicker);
	return ConstructedAssetListView.ToSharedRef();
}

#pragma region ComboBoxForListingCondition
TSharedRef<SComboBox<TSharedPtr<FString>>> SMyAdvanceDeletionTab::ConstructComBox()
{

	TSharedRef<SComboBox<TSharedPtr<FString>>>ConstrucedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSoureItems)
		.OnGenerateWidget(this, &SMyAdvanceDeletionTab::OnGenerateComboContent)//生成时触发
		.OnSelectionChanged(this, &SMyAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))
		];
	return ConstrucedComboBox;
}
TSharedRef<SWidget> SMyAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)//显示文字内容
{
	TSharedRef<STextBlock>ConstructedComboText = SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));
	return ConstructedComboText;
}
void SMyAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo0)//点击触发事件
{

	//MyDebugHeader::print(*SelectedOption.Get(), FColor::Cyan);

	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));
	FTestManagerModule& TestManagerModule = FModuleManager::LoadModuleChecked<FTestManagerModule>(TEXT("TestManager"));
	if (*SelectedOption.Get() == ListAll) {
		MyDisplayedAssetsData = MyStoredAssetsData;
		RefreshAssetListView();

	} 
	else if (*SelectedOption.Get()==ListUnused) {
		TestManagerModule.ListUnusedAssetsForAssetList(MyStoredAssetsData, MyDisplayedAssetsData);
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListSameName) {
		TestManagerModule.ListSameNameAssetsForAssetList(MyStoredAssetsData, MyDisplayedAssetsData);
		RefreshAssetListView();
	}
}
TSharedRef<STextBlock> SMyAdvanceDeletionTab::ConstructComboHelpTexts(const FString& TestContent, ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock>ConstructedHelpText = SNew(STextBlock)
		.Text(FText::FromString(TestContent))
		.Justification(TextJustify)
		.AutoWrapText(true);//自动换行
		return ConstructedHelpText;
}
#pragma  endregion
#pragma region RowWidgetForAssetListView
TSharedRef<ITableRow> SMyAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase> &OwnerTable)//每行触发一次
{
	if (!AssetDataToDisplay->IsValid())return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);//行列表控件
	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClass.ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	FSlateFontInfo AssetClassNameFont = GetEmboseedTextFont();
	AssetClassNameFont.Size = 10;
	FSlateFontInfo AssetNameFont = GetEmboseedTextFont();
	AssetNameFont.Size = 15;
	TSharedRef<STableRow<TSharedPtr<FAssetData>>>ListViewRowWidet =//针对每行UI
		SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(5.f))//.Padding(FMargin(5.f))：设置表格行的内边距，即在控件边缘和内容之间添加上、下、左、右各 5 个单位长度的空白区域，用作视觉上的间距效果。
		[
			SNew(SHorizontalBox)
			//水平框第一个插槽
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(.05f)
			[
				ConstructCheckBox(AssetDataToDisplay)//生成勾选框
			]
			//Second slot for displaying asset class name
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			.FillWidth(.5f)
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)//生成类名
			]
			//Third slot for displaying asset name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				ConstructTextForRowWidget(DisplayAssetName,AssetNameFont)//生成类对象名
			]
			//Fourth slot for a button
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
				[
					ConstructButtonForRowWidget(AssetDataToDisplay)//生成按钮
 			]

		];
	return ListViewRowWidet;
}

void SMyAdvanceDeletionTab::RefreshAssetListView()//重新构建列表
{
	AssetDataDeleteArray.Empty();
	if (ConstructedAssetListView.IsValid()) {
		ConstructedAssetListView->RebuildList();
	}
}

void SMyAdvanceDeletionTab::OnRowWidgetMouseButtonClicker(TSharedPtr<FAssetData> ClickedData)//转到资源管理器相应资源
{
	FTestManagerModule& TestManagerModule = FModuleManager::LoadModuleChecked<FTestManagerModule>("TestManager");
	TestManagerModule.MyCBToClickedAssetForAssetList(ClickedData->ObjectPath.ToString());
}

TSharedRef<SCheckBox> SMyAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox>ConstrucedCheckBox = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SMyAdvanceDeletionTab::OnCheckBoxStateChangeg, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstrucedCheckBox);
	return ConstrucedCheckBox;
}

void SMyAdvanceDeletionTab::OnCheckBoxStateChangeg(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	//根据是否勾选来为数组添加资源
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		//MyDebugHeader::print(AssetData->AssetName.ToString() + TEXT(" is unchecked"),FColor::Red);
		if (AssetDataDeleteArray.Contains(AssetData)) {
			AssetDataDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		//MyDebugHeader::print(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);
		AssetDataDeleteArray.AddUnique(AssetData);

		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}
#pragma endregion
#pragma region TabButtons
TSharedRef<STextBlock> SMyAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo FontToUse)
{
	TSharedRef<STextBlock>ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);
	return ConstructedTextBlock;
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetData)
{
	TSharedRef<SButton>ConstructedButton = SNew(SButton)
		.Text(FText::FromString(TEXT("MyDelete")))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnDeleteButtonClicked, AssetData);

	return ConstructedButton;
}

FReply SMyAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	//MyDebugHeader::print(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"), FColor::Red);
	FTestManagerModule& MyTestMangerModule = FModuleManager::LoadModuleChecked <FTestManagerModule>(TEXT("TestManager"));
	const bool bAssetDeleted = MyTestMangerModule.MyDeleteSigleAssetForAssetList(*ClickedAssetData.Get());
	if (bAssetDeleted) {
		if (MyStoredAssetsData.Contains(ClickedAssetData)) {
			MyStoredAssetsData.Remove(ClickedAssetData);
		}
		if (MyDisplayedAssetsData.Contains(ClickedAssetData))
		{
			MyDisplayedAssetsData.Remove(ClickedAssetData);
		}
		RefreshAssetListView();
	}

	return FReply::Handled();
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructDeleteAllButton()//删除资源键
{
	TSharedRef<SButton>DeleteAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnDeleteAllButtonClicked);
	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));//设置文本
	return DeleteAllButton;
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructSelectButton()
{
	TSharedRef<SButton>SelectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnSelectedAllButtonClicked);
	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));
	return SelectAllButton;	
}

TSharedRef<SButton> SMyAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton>DeselectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SMyAdvanceDeletionTab::OnDeselectAllButtonClicked);
	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));
	return DeselectAllButton;
}

FReply SMyAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	//MyDebugHeader::print(TEXT("Delete All Button Clicked"), FColor::Cyan);
	if (AssetDataDeleteArray.Num() == 0)
	{
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}
	TArray<FAssetData>AssetDataToDelete;
	for (const TSharedPtr<FAssetData>& Data : AssetDataDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}
	FTestManagerModule& TestMangerModule = FModuleManager::LoadModuleChecked<FTestManagerModule>("TestManager");
	const bool bAssetsDeleted = TestMangerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);//引用模块执行删除函数
	if (bAssetsDeleted)
	{
		for (const TSharedPtr<FAssetData>& DeletedData : AssetDataDeleteArray)//去除已经删除的函数的引用，减少引用计数
		{
			if (MyStoredAssetsData.Contains(DeletedData))
			{
				MyStoredAssetsData.Remove(DeletedData);
			}
			if (MyDisplayedAssetsData.Contains(DeletedData))
			{
				MyDisplayedAssetsData.Remove(DeletedData);
			}
		}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SMyAdvanceDeletionTab::OnSelectedAllButtonClicked()
{
	//MyDebugHeader::print(TEXT("Select All Button Clicked"), FColor::Cyan);
	if (CheckBoxesArray.Num() == 0)return FReply::Handled();
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray) {
		if (!CheckBox->IsChecked()) {//为false
			CheckBox->ToggleCheckedState();//切换true状态
		}
	}

	return FReply::Handled();
}

FReply SMyAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	//MyDebugHeader::print(TEXT("Deselect All Button Clicked"), FColor::Cyan);
	if (CheckBoxesArray.Num() == 0)return FReply::Handled();
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray) {
		if (CheckBox->IsChecked()) {//为true
			CheckBox->ToggleCheckedState();//切为false
		}
	}

	return FReply::Handled();
}


TSharedRef<STextBlock> SMyAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextInfo = GetEmboseedTextFont();
	ButtonTextInfo.Size=15.f;
	TSharedRef<STextBlock>ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextInfo)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion
