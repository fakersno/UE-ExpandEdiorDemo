// Vince Petrelli All Rights Reserverd

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "MyQuickMaterialCreationWidget.generated.h"

/**
 * 
 */
UCLASS()
class TESTMANAGER_API UMyQuickMaterialCreationWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:

#pragma region MyQuickMaterialCreationCore

	UFUNCTION(BlueprintCallable)
	void CreateMaterialFromSelectedTexture();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCustomMaterialName=true;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="CreateMaterialFromSelectedTextures",meta=(EditCondition="bCustomMaterialName"))
	FString MaterialName=TEXT("M_");
#pragma region SupportedTextureNames

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
		TArray<FString> BaseColorArray = {
			TEXT("_BaseColor"),
			TEXT("_Albedo"),
			TEXT("_Diffuse"),
			TEXT("_diff")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
		TArray<FString> MetallicArray = {
			TEXT("_Metallic"),
			TEXT("_metal")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
		TArray<FString> RoughnessArray = {
			TEXT("_Roughness"),
			TEXT("_RoughnessMap"),
			TEXT("_rough")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
		TArray<FString> NormalArray = {
			TEXT("_Normal"),
			TEXT("_NormalMap"),
			TEXT("_nor")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
		TArray<FString> AmbientOcclusionArray = {
			TEXT("_AmbientOcclusion"),
			TEXT("_AmbientOcclusionMap"),
			TEXT("_AO")
	};


#pragma  endregion
#pragma endregion
#pragma  region MyQuickMaterialCreation
	bool ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProccess, TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath);
	bool CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck);
	UMaterial* CreateMaterialAsset(const FString& NameOfTheMaterial, const FString& PathToPutMaterial);
	void Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter);

#pragma  endregion

#pragma region MyCreateMaterialNodesConnectPings

	bool TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
	bool TryConnectMetalic(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial);
#pragma  endregion

};
