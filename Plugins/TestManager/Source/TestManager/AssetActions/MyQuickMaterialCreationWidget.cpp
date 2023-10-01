// Vince Petrelli All Rights Reserverd


#include "MyQuickMaterialCreationWidget.h"
#include "MyDebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#pragma region MyQuickMaterialCreationCore

void UMyQuickMaterialCreationWidget::CreateMaterialFromSelectedTexture()
{
	if (bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid name"));
			return;
		} 
	}
	TArray<FAssetData>SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*>SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;
	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath))return;//文件夹路径
	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName))return;//验证新资产名称是否与现有资产同名

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath);//创建材质
	if (!CreatedMaterial) {
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Failed to create material"));
	}


	for (UTexture2D* SelectedTexture : SelectedTexturesArray)//每个纹理都会调用一遍
	{
		if (!SelectedTexture) continue;

		Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
	}
	//MyDebugHeader::print(SelectedTextureFolderPath, FColor::Cyan);
	if (PinsConnectedCounter > 0)
	{
		MyDebugHeader::showNotifyInfo(TEXT("Successfully conntected ") + FString::FromInt(PinsConnectedCounter) + TEXT("pins"));
	}
}

//处理选中的数据，将过滤掉纹理，如果未选中则返回false
bool UMyQuickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProccess, TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProccess.Num() == 0)
	{
		MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No textures selected"));
		return false;
	}
	bool bMaterialNameSet = false;
	for (const FAssetData& SelectData : SelectedDataToProccess) 
	{
		UObject* SelectAsset = SelectData.GetAsset();
		if (!SelectAsset)continue;
		UTexture2D* SelectTexture = Cast<UTexture2D>(SelectAsset);
		if (!SelectTexture)
		{
			MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select only textures\n")+SelectAsset->GetName()+TEXT("is not a texture"));
			return false;
		}
		OutSelectedTexturesArray.Add(SelectTexture);
		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectData.PackagePath.ToString();//获取存在包的文件夹路径
		}
		if (!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0,TEXT("M_"));
			bMaterialNameSet = true;
		}

	}
	return true;
}
//如果材质名称被指定文件夹下的资源使用，则返回true
bool UMyQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{

	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);

	for (const FString& ExistingAssetPath : ExistingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath); //用于获取指定路径的基本文件名（即去除路径和扩展名后的文件名部分

		if (ExistingAssetName.Equals(MaterialNameToCheck))
		{
			MyDebugHeader::ShowMsgDialog(EAppMsgType::Ok, MaterialNameToCheck +
				TEXT(" is already used by asset"));

			return true;
		}
	}

	return false;
}
UMaterial* UMyQuickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();//实际创建材质的工厂类
	UObject* CreateObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterial, PathToPutMaterial, UMaterial::StaticClass(), MaterialFactory);

	return Cast<UMaterial>(CreateObject);


}
void UMyQuickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);//纹理采集器
	if (!TextureSampleNode)return;
	if (!CreatedMaterial->BaseColor.IsConnected())//BaseColor节点未连接
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))//连接节点
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->Metallic.IsConnected())
	{
		if (TryConnectMetalic(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
}
#pragma endregion


#pragma region MyCreateMaterialNodesConnectPings
bool UMyQuickMaterialCreationWidget::TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))//若选中的材质中包含BaseColor类型
		{
			TextureSampleNode->Texture = SelectedTexture;//
			CreatedMaterial->Expressions.Add(TextureSampleNode);//将纹理采集器添加到材质中去
			CreatedMaterial->BaseColor.Expression = TextureSampleNode;//连接纹理采集器
			CreatedMaterial->PostEditChange();//通知编辑器材质已经发生了改变。
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			return true;
		}

	}
	return false;
}
bool UMyQuickMaterialCreationWidget::TryConnectMetalic(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& MetaliName : MetallicArray)
	{
		if (SelectedTexture->GetName().Contains(MetaliName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;//纹理设置压缩格式
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();
			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;//纹理采集类型
			CreatedMaterial->Expressions.Add(TextureSampleNode);
			CreatedMaterial->Metallic.Expression = TextureSampleNode;
			CreatedMaterial->PostEditChange();
			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240;
		}
	}
	return false;
}

#pragma  endregion