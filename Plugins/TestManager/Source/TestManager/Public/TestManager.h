// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FTestManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
#pragma region ContentBrowserMenuExtention
	void InitCBMenuExtention();
	TArray<FString>MyFolderPathsSelected;
	TSharedRef<FExtender>CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(class FMenuBuilder&MenuBuilder);

	void OnDeleteUnsuedAssetButtonClicked();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvanenceDeletionButtonClicked();
	void FixUpRedirectors();
#pragma endregion

#pragma region CustomEditroTab
	void RegisterAdvanceDeletionTab();
	TSharedRef<SDockTab>OnSpawnAdvanenceDeltionTab(const FSpawnTabArgs& SpawnTabArgs);

	TArray<TSharedPtr <FAssetData>>GetAllAssetDataUnderSelectedFolder();
#pragma endregion
public:
#pragma region ProcessDataForAdvanceDletionTab
	bool MyDeleteSigleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);
	void ListUnusedAssetsForAssetList(const TArray < TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);
	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);
	void MyCBToClickedAssetForAssetList(const FString& AssetPathToSync);
#pragma endregion
};
