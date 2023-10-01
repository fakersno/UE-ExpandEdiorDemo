#pragma once
#include "Misc/MessageDialog.h"
#include"Widgets/Notifications/SNotificationList.h"//通知列表
#include"Framework/Notifications/NotificationManager.h"//通知管理器库




namespace MyDebugHeader {
	static void print(const FString& Message, const FColor& Color) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}
	static void pringLog(const FString& Message) {
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowMsgAsWarning = true) {
		if (bShowMsgAsWarning) {
			FText Msgtitle = FText::FromString(TEXT("Warning"));
			return FMessageDialog::Open(MsgType, FText::FromString(Message), &Msgtitle);//带标头
		}
		else {
			return FMessageDialog::Open(MsgType, FText::FromString(Message));

		}
	}

	static void showNotifyInfo(const FString& Message) {//显示到右下角的通知信息
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.0f;
		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}
