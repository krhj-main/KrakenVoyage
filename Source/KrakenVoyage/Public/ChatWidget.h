#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UEditableTextBox;
class UButton;

UCLASS()
class KRAKENVOYAGE_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// ================================================================
	// BindWidget
	// ================================================================

	// UScrollBox: 자식 위젯들을 스크롤 가능하게 만드는 컨테이너
	// 메시지가 많아지면 자동으로 스크롤바 생성
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ScrollBox_Messages;

	// 메시지 입력창
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* Input_ChatMessage;

	// 전송 버튼 (Enter키로도 전송 가능)
	UPROPERTY(meta = (BindWidget))
	UButton* Btn_Send;

	// ================================================================
	// 외부에서 호출
	// ================================================================

	// 새 메시지를 채팅 로그에 추가 (수신 시 호출)
	void AddChatMessage(int32 SenderIndex, const FString& Message);

	// 입력창에 포커스 설정 (Enter키로 채팅 열 때)
	void FocusChatInput();

	// 입력창 포커스 해제
	void UnfocusChatInput();

	// 입력창이 포커스 상태인지
	bool IsChatFocused() const;

protected:
	UFUNCTION()
	void OnSendClicked();

	// EditableTextBox의 OnTextCommitted 이벤트 (Enter키)
	// ETextCommit::Type: 텍스트 입력이 어떻게 완료되었는지
	// OnEnter = Enter키, OnCleared = ESC키 등
	UFUNCTION()
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// 실제 메시지 전송 처리
	void SendCurrentMessage();

	// 채팅 메시지 색상 (플레이어별)
	FLinearColor GetPlayerColor(int32 PlayerIndex) const;

	// 최대 표시 메시지 수 (너무 많으면 오래된 것 삭제)
	static constexpr int32 MaxVisibleMessages = 50;
	int32 CurrentMessageCount = 0;
};