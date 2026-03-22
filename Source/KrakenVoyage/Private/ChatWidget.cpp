#include "ChatWidget.h"
#include "KrakenPlayerController.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"

void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Send)
	{
		Btn_Send->OnClicked.AddDynamic(this, &UChatWidget::OnSendClicked);
	}

	if (Input_ChatMessage)
	{
		// OnTextCommitted: 텍스트 입력이 "확정"될 때 호출되는 델리게이트
		// Enter키를 누르면 ETextCommit::OnEnter로 호출됨
		Input_ChatMessage->OnTextCommitted.AddDynamic(
			this, &UChatWidget::OnChatTextCommitted);
	}

	UE_LOG(LogTemp, Log, TEXT("[Chat] Widget constructed."));
}

// ============================================================
// 메시지 추가 (수신 시)
// ============================================================
void UChatWidget::AddChatMessage(int32 SenderIndex, const FString& Message)
{
	if (!ScrollBox_Messages) return;

	// 오래된 메시지 제거 (성능)
	if (CurrentMessageCount >= MaxVisibleMessages)
	{
		// GetChildAt(0): ScrollBox의 첫 번째 자식 (가장 오래된 메시지)
		UWidget* OldestChild = ScrollBox_Messages->GetChildAt(0);
		if (OldestChild)
		{
			ScrollBox_Messages->RemoveChild(OldestChild);
			CurrentMessageCount--;
		}
	}

	// 새 TextBlock을 동적으로 생성
	// NewObject: UE5에서 UObject를 동적으로 생성하는 함수
	// WidgetTree: 현재 위젯의 위젯 트리 (자식 위젯 관리)
	UTextBlock* NewMessage = NewObject<UTextBlock>(ScrollBox_Messages);
	if (NewMessage)
	{
		// 형식: "[Player 0] 나 크라켄 아님"
		const FString FormattedMsg = FString::Printf(
			TEXT("[Player %d] %s"), SenderIndex, *Message);

		NewMessage->SetText(FText::FromString(FormattedMsg));

		// 플레이어별 색상 (구분 용이)
		NewMessage->SetColorAndOpacity(FSlateColor(GetPlayerColor(SenderIndex)));

		// 폰트 크기 설정
		// FSlateFontInfo: UE5의 폰트 정보 구조체
		FSlateFontInfo FontInfo = NewMessage->GetFont();
		FontInfo.Size = 14;
		NewMessage->SetFont(FontInfo);

		//자동 줄바꿈
		NewMessage->SetAutoWrapText(true);

		// ScrollBox에 추가
		ScrollBox_Messages->AddChild(NewMessage);
		CurrentMessageCount++;

		// 자동 스크롤 (최신 메시지가 보이도록)
		// ScrollToEnd: ScrollBox를 맨 아래로 스크롤
		ScrollBox_Messages->ScrollToEnd();
	}
}

// ============================================================
// 전송
// ============================================================
void UChatWidget::OnSendClicked()
{
	SendCurrentMessage();
}

void UChatWidget::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Enter키로 전송
	if (CommitMethod == ETextCommit::OnEnter)
	{
		SendCurrentMessage();
	}
}

void UChatWidget::SendCurrentMessage()
{
	if (!Input_ChatMessage) return;

	const FString Message = Input_ChatMessage->GetText().ToString().TrimStartAndEnd();

	// 빈 메시지 무시
	if (Message.IsEmpty())
	{
		// 빈 메시지면 채팅 닫기만
		AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
		if (KPC) KPC->ToggleChat();
		return;
	}

	// PlayerController를 통해 서버에 전송
	AKrakenPlayerController* KPC = Cast<AKrakenPlayerController>(GetOwningPlayer());
	if (KPC)
	{
		KPC->ServerSendChatMessage(Message);
	}

	// 입력창 비우기
	Input_ChatMessage->SetText(FText::GetEmpty());

	// ★ 전송 후 자동으로 채팅 닫기 + 게임 입력 복원
	if (KPC) KPC->ToggleChat();
}

// ============================================================
// 포커스 관리
// ============================================================
void UChatWidget::FocusChatInput()
{
	if (Input_ChatMessage)
	{
		// SetKeyboardFocus: 이 위젯에 키보드 포커스를 줌
		// → 타이핑하면 이 입력창에 글자가 들어감
		// 위젯을 포커스 가능하게 설정 후 포커스
		Input_ChatMessage->SetUserFocus(GetOwningPlayer());
	}
}

void UChatWidget::UnfocusChatInput()
{
	if (Input_ChatMessage)
	{
		Input_ChatMessage->SetText(FText::GetEmpty());
	}

	// 게임에 포커스 돌려줌
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeGameOnly GameMode;
		PC->SetInputMode(GameMode);
	}
}

bool UChatWidget::IsChatFocused() const
{
	if (Input_ChatMessage)
	{
		return Input_ChatMessage->HasKeyboardFocus();
	}
	return false;
}

// ============================================================
// 플레이어별 색상
// ============================================================
FLinearColor UChatWidget::GetPlayerColor(int32 PlayerIndex) const
{
	// 8명까지 다른 색상
	// 공포 게임 분위기에 맞는 차가운/어두운 톤
	switch (PlayerIndex % 8)
	{
	case 0: return FLinearColor(0.3f, 0.8f, 1.0f);   // 하늘색
	case 1: return FLinearColor(0.3f, 1.0f, 0.5f);   // 연두
	case 2: return FLinearColor(1.0f, 0.8f, 0.3f);   // 노란
	case 3: return FLinearColor(1.0f, 0.5f, 0.3f);   // 주황
	case 4: return FLinearColor(0.8f, 0.4f, 1.0f);   // 보라
	case 5: return FLinearColor(1.0f, 0.4f, 0.6f);   // 분홍
	case 6: return FLinearColor(0.6f, 0.8f, 0.8f);   // 민트
	case 7: return FLinearColor(0.9f, 0.9f, 0.9f);   // 흰색
	default: return FLinearColor::White;
	}
}