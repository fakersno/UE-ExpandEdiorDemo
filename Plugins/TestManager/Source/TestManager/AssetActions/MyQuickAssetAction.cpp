// Vince Petrelli All Rights Reserverd
#include "MyQuickAssetAction.h"
#include "MyDebugHeader.h"//自定义通知头文件
#include "EditorUtilityLibrary.h"//编辑器功能工具库
#include "EditorAssetLibrary.h"//资源操作库
#include "Misc/MessageDialog.h"//消息通知库
#include "ObjectTools.h"//对象工具
#include "AssetRegistryModule.h"//资产重定向模块
#include "AssetToolsModule.h"//资产工具模块

void UMyQuickAssetAction::MyTestFunc()
{
	MyDebugHeader::print(TEXT("working"), FColor::Cyan);
	MyDebugHeader::pringLog(TEXT("Working"));
}

void UMyQuickAssetAction::MyduplicateAssets(int32 NumOfDuplicates)//资产批量复制
{
	if (NumOfDuplicates <= 0) {
		//MyDebugHeader::print(TEXT("Plaese enter a VALID number"), FColor::Red);
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Plase enter a VALID number"));
		return;
	}
	TArray<FAssetData>SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();//获取编辑器中当前选中的资产（Asset）的信息
	uint32 counter = 0;
	for (const FAssetData& SelectedAssetData : SelectedAssetsData) {
		for (int32 i = 0; i < NumOfDuplicates; i++) {
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);
			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName)) {
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				counter++;
			}
		}

	}
	if (counter > 0) {
		//MyDebugHeader::print(TEXT("Successfully duplicated" + FString::FromInt(counter) + "files"), FColor::Green);
		MyDebugHeader::showNotifyInfo(TEXT("Successfully duplicated" + FString::FromInt(counter) + "files")); 
	}
}


void UMyQuickAssetAction::MyAddPrefixes()//添加前缀
{
	TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 counter = 0;
	for (UObject* SelectedObject : SelectedObjects) {
		if (!SelectedObject)continue;
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());
		if (!PrefixFound || PrefixFound->IsEmpty()) {
			MyDebugHeader::print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}
		FString OldName = SelectedObject->GetName();
		if (OldName.StartsWith(* PrefixFound)) {//检查是否已有相应前缀
			MyDebugHeader::print(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));//移除一些材质实例之前带的前缀
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);

		++counter;
	}

		if (counter > 0)
		{
			MyDebugHeader::showNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(counter) + " assets"));
		}

}


void UMyQuickAssetAction::MyRemoveUnusedAssets()//移除资源
{
	TArray<FAssetData>SelcetAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData>UnusedAssetsData;
	MyFixUpRedirectors();
	for (const FAssetData& SelectAssetData : SelcetAssetsData) {
		TArray<FString>AssetRefrence = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectAssetData.ObjectPath.ToString());
		if (AssetRefrence.Num() == 0) {//没有引用的资源
			UnusedAssetsData.Add(SelectAssetData);
		}

	}
	if (UnusedAssetsData.Num() == 0) {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
	}
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);//删除资产
	if (NumOfAssetsDeleted == 0)return;
	MyDebugHeader::showNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));
}


void UMyQuickAssetAction::MyFixUpRedirectors()
{
	TArray<UObjectRedirector*>RedirectorsToFixArray;
	FAssetRegistryModule& AssetRegistryMoudle = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));//用于加载 AssetRegistry 模块并获取其实例的代码
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");//在Game文件夹下搜索
	Filter.ClassNames.Emplace("ObjectRedirector");//只能重定向UObject的资源，若要重定向其他资源要入入Texture2D等元素
	TArray<FAssetData>OutRedirectors;
	AssetRegistryMoudle.Get().GetAssets(Filter, OutRedirectors);//根据搜索条件寻找对象存储道数组中
	for (const FAssetData& RedirectorData : OutRedirectors) {
		//UObjectRedirector是UObject的子类
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset())) {
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}
	FAssetToolsModule& AssetToolsMoude = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsMoude.Get().FixupReferencers(RedirectorsToFixArray);//重定向修复
} 