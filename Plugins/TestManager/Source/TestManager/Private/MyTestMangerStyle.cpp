// Vince Petrelli All Rights Reserverd


#include "MyTestMangerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
FName FTestMangerStyle::StyleSetName = FName("TestManagerStyle");
TSharedPtr<FSlateStyleSet>FTestMangerStyle::CreatedSlateStyleSet = nullptr;
void FTestMangerStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet) {
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);//зЂВс
	}
}



TSharedRef<FSlateStyleSet> FTestMangerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));
	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("TestManager"))->GetBaseDir() / "Resources";
	CustomStyleSet->SetContentRoot(IconDirectory);
	const FVector2D Icon16x16(16.f, 16.f);
	CustomStyleSet->Set("ContentBrowser.MyDeleteUnusedAssets", new FSlateImageBrush(IconDirectory / "DeleteUnused.png", Icon16x16));
	CustomStyleSet->Set("ContentBrowser.DeleteEmptyFloders", new FSlateImageBrush(IconDirectory / "MyDeleteEmptyAsset.png", Icon16x16));
	CustomStyleSet->Set("ContentBrowser.MyAdvanenceDeletion", new FSlateImageBrush(IconDirectory / "MyAdvabceDelete.png", Icon16x16));

	return CustomStyleSet;
}


void FTestMangerStyle::Shutdown()
{
	if (CreatedSlateStyleSet.IsValid()) {
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}