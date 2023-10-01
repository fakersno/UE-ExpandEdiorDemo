// Vince Petrelli All Rights Reserverd

#pragma once
#include "Styling/SlateStyle.h"


class FTestMangerStyle
{
public:
	static void InitializeIcons();
	static void Shutdown();
private:
	static FName StyleSetName;
	static TSharedRef<FSlateStyleSet>CreateSlateStyleSet();
	static TSharedPtr<FSlateStyleSet>CreatedSlateStyleSet;

public:
	static FName GetStyleSetName() { return StyleSetName; }
};
