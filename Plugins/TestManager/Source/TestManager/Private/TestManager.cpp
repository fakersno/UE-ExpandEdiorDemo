// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestManager.h"
#include "ContentBrowserModule.h"
#include "MyDebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistryModule.h"//资产重定向模块
#include "AssetToolsModule.h"//资产工具模块
#include "TestManager/MySlates/MyAdvanceDeletionWidget.h"
#include <UnrealEdSharedPCH.h>
#include <Widgets/Docking/SDockTab.h>
#include "MyTestMangerStyle.h"

#define LOCTEXT_NAMESPACE "FTestManagerModule"

void FTestManagerModule::StartupModule()
{
	FTestMangerStyle::InitializeIcons();

	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

#pragma region ContentBrowserMenuExtention


void FTestManagerModule::InitCBMenuExtention()//初始化自定义菜单
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	//用于获取所有的路径视图（Path View）的菜单扩展器数据结构。
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	/*第一种绑定
	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FTestManagerModule::CustomCBMenuExtender);
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);*/
	//第二种
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FTestManagerModule::CustomCBMenuExtender));//右键点击资源或文件夹时被调用
}
TSharedRef<FExtender> FTestManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender>MenuExtender(new FExtender());//生成菜单扩展器对象
	if (SelectedPaths.Num() > 0) 
	{
		MenuExtender->AddMenuExtension(FName("Delete"),//扩展项的名称
			EExtensionHook::After,//表示在上面的拓展项之后添加新的该扩展项
			TSharedPtr<FUICommandList>(),//可以用于创建自定义快捷键
			FMenuExtensionDelegate::CreateRaw(this, &FTestManagerModule::AddCBMenuEntry));//基本就是自动触发了，当点击右键点击资源或文件夹时被调用
		MyFolderPathsSelected = SelectedPaths;//获取当前所有选择的文件路径

	}
	return MenuExtender;
}
//添加若干定义的菜单项的细节
void FTestManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("My  Delete Unused Assets")),//标题
		FText::FromString(TEXT("My  Safely delete all unused assets under folder")),//工具说明
		FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.MyDeleteUnusedAssets"),//定制图标
		FExecuteAction::CreateRaw(this, &FTestManagerModule::OnDeleteUnsuedAssetButtonClicked));//点击时触发

	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all empty folders")),
		FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFloders"),
		FExecuteAction::CreateRaw(this, &FTestManagerModule::OnDeleteEmptyFoldersButtonClicked));
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("MyAdvanence Deletion")),
		FText::FromString(TEXT("Tips:List assets by specific condition int a tab for dleting ")),
		FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.MyAdvanenceDeletion"),
		FExecuteAction::CreateRaw(this, &FTestManagerModule::OnAdvanenceDeletionButtonClicked));
}
void FTestManagerModule::OnDeleteUnsuedAssetButtonClicked()
{
	MyDebugHeader::print(TEXT("WorkingMyManager"), FColor::Green);
	if (MyFolderPathsSelected.Num() > 1) {//不能大于一个文件
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok,TEXT("You can only do this to one folder"));
		return;
	}
	TArray<FString>AssetsPathNames = UEditorAssetLibrary::ListAssets(MyFolderPathsSelected[0]);//获取文件下所有资源的路径
	if (AssetsPathNames.Num() == 0) {//没有资源
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"),false);
		return;
	}
	EAppReturnType::Type ConfigResult = MyDebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of" )+ FString::FromInt(AssetsPathNames.Num())
		+ TEXT(" assets need to be checked.\nWould you like to procceed?"), false);
	if (ConfigResult == EAppReturnType::No)return;
	FixUpRedirectors();
	TArray<FAssetData>UnusedAssetsDataArray;
	for (const FString& AssetPathName : AssetsPathNames) {//在以下文件夹中的资源跳过
		if (AssetPathName.Contains(TEXT("Developers"))||
			AssetPathName.Contains(TEXT("Collections"))||
			AssetPathName.Contains(TEXT("__ExternalActors__"))||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))continue;
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))continue;//检查指定的资源路径是否存在有效的资源
		TArray<FString>AssetReferencers=UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);//查对于路径的引用
		if (AssetReferencers.Num() == 0) {//引用为0
			const FAssetData UnusedAssetData=UEditorAssetLibrary::FindAssetData(AssetPathName);
				UnusedAssetsDataArray.Add(UnusedAssetData);
		}

	}
	if (UnusedAssetsDataArray.Num() > 0) {
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"), false);
	}
}
void FTestManagerModule::OnDeleteEmptyFoldersButtonClicked()//删空文件夹
{
	//MyDebugHeader::print(TEXT("haha"), FColor::Green);
	FixUpRedirectors();
	//第一个 true 参数表示包括子文件夹，第二个 true 参数表示包括子文件夹中的资源。
	TArray<FString>FolderPathArray = UEditorAssetLibrary::ListAssets(MyFolderPathsSelected[0], true, true);
	uint32 Counter = 0;
	FString EmptyFolderNames;
	TArray<FString>EmptyFoldersPathArray;
	for (const FString& FolderPath : FolderPathArray) {
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath))continue;//检查指定的文件夹路径是否存在
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath)) {
			EmptyFolderNames.Append(FolderPath);
			EmptyFolderNames.Append("\n");
			EmptyFoldersPathArray.Add(EmptyFolderNames);
		}
	}
	if (EmptyFoldersPathArray.Num() == 0) {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty found under selected folder"), false);
		return;
	}
	EAppReturnType::Type ConfirmResult = MyDebugHeader::ShowMsgDialog(EAppMsgType::OkCancel, TEXT("Empty folders found int :\n") + EmptyFolderNames + TEXT("\nWould you like to delete all?"),false);
	if (ConfirmResult == EAppReturnType::Cancel)return;
	for (const FString& EmpthyFolderPath : EmptyFoldersPathArray) {
		UEditorAssetLibrary::DeleteDirectory(EmpthyFolderPath)?
			Counter++ : MyDebugHeader::print(TEXT("Failed to delete") + EmpthyFolderPath, FColor::Red);
	}
	if (Counter > 0) {
		MyDebugHeader::showNotifyInfo(TEXT("Successfully deleted") + FString::FromInt(Counter) + TEXT("folders"));
	}

}
void FTestManagerModule::OnAdvanenceDeletionButtonClicked()//点击时触发
{
	FixUpRedirectors();
	//MyDebugHeader::print(TEXT("Working "), FColor::Green);
	FGlobalTabmanager::Get()->TryInvokeTab(FName("MyAdvanceDeletion"));//根据名称生成选项卡
}
void FTestManagerModule::FixUpRedirectors()//文件下所有指定类型重定向
{
	TArray<UObjectRedirector*>RedirectorsToFixArray;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");
	TArray<FAssetData>OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);
	for (const FAssetData& RedirectroData : OutRedirectors) {
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectroData.GetAsset())) {
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
	//MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Fix OK"), false);
}
#pragma endregion

#pragma region CustomEditroTab
void FTestManagerModule::RegisterAdvanceDeletionTab()//资源管理器里的选项卡注册初始化
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("MyAdvanceDeletion"),//注册定义匹配的名称
		FOnSpawnTab::CreateRaw(this, &FTestManagerModule::OnSpawnAdvanenceDeltionTab))//*当一个选项卡实例化生成时调用。
		.SetDisplayName(FText::FromString(TEXT("MyAdvance Deletion")))//设置选项卡显示的名字
		.SetIcon(FSlateIcon(FTestMangerStyle::GetStyleSetName(), "ContentBrowser.MyAdvanenceDeletion"));//图标
}
//生成选项卡
TSharedRef<SDockTab> FTestManagerModule::OnSpawnAdvanenceDeltionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SMyAdvanceDeletionTab)
			.MyAssetsDataToStore(GetAllAssetDataUnderSelectedFolder())//传参设置Slate中的变量内容
			.CurrentSelectedFloder(MyFolderPathsSelected.Num() <= 0?FString(): MyFolderPathsSelected[0])
		];
}
TArray<TSharedPtr<FAssetData>> FTestManagerModule::GetAllAssetDataUnderSelectedFolder()//获取资源
{
	TArray<TSharedPtr<FAssetData>>MyAvaiableAssetsData;
	if (MyFolderPathsSelected.Num() <= 0)return MyAvaiableAssetsData;//若之前在编辑器中打开选项卡不关闭，下次启动编辑器会自动打开而未选择文件夹导致数组越界而崩溃
	TArray<FString>MyAssetsPathNames = UEditorAssetLibrary::ListAssets(MyFolderPathsSelected[0]);//获取指定文件目录下的所有资源路径
	for (const FString& AssetPathName : MyAssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))continue;
		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		MyAvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
		
	}
	return MyAvaiableAssetsData;
}
#pragma endregion

#pragma region ProcessDataForAdvanceDletionTab
bool FTestManagerModule::MyDeleteSigleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);
	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0) {
		return true;
	}
	return false;
}
bool FTestManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}
	return false;
}
void FTestManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter) 
	{
		TArray<FString>AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());
		if (AssetReferencers.Num() == 0) //没有引用的资源
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}
void FTestManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();
	TMultiMap<FString, TSharedPtr<FAssetData>>AssetsInfoMultiMap;
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter) {
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter) {
		TArray<TSharedPtr<FAssetData>>OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetsData);//通过资源名称查找相同的资源
		if (OutAssetsData.Num() <= 1)continue;
		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData) {
			if (SameNameData.IsValid()) {
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}
}
void FTestManagerModule::MyCBToClickedAssetForAssetList(const FString& AssetPathToSync)//跳转资源管理器到指定资源
{
	TArray<FString>AssetsPathSync;
	AssetsPathSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathSync);
}
#pragma endregion
void FTestManagerModule::ShutdownModule()//关闭模块
{
	//这个函数可能在关机时被调用来清理你的模块。对于支持动态重载的模块，
//我们在卸载模块之前调用这个函数。
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	//手动关闭模块
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("MyAdvanceDeletion"));
	FTestMangerStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTestManagerModule, TestManager)