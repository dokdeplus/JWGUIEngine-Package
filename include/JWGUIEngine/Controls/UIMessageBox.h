#pragma once
#include "UIPopup.h"
#include "UIWindowPanel.h"
#include "../Core/Event.h"

#include <map>
#include <string>

namespace jwgui {

enum class MessageBoxType {
    Ok,
    YesNo,
    YesNoCancel
};

class JWGUI_API UIMessageBox : public UIPopup {
public:
    UIMessageBox(float x = 0, float y = 0, float w = 0, float h = 0);
    virtual ~UIMessageBox() = default;

    void SetTitle(const std::string& title);
    void SetMessage(const std::string& message);
    void SetType(MessageBoxType type);
    // Replaces the text of one button ("Yes", "No", ...), e.g. with the action it takes.
    void SetButtonText(MessageBoxResult result, const std::string& text);

    void ShowMessageBox(std::shared_ptr<UIRenderView> parent);
    // The card's rectangle after ShowMessageBox (it grows with the message).
    UIRect DialogRect() const;
    void ClosePopup() override;

    EventHandler<MessageBoxResultEventArgs> ResultEvent;

    // Overrides for modality (blocking events)
    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    void OnGlobalMouseDown(float x, float y) override;

private:
    std::string m_title;
    std::string m_message;
    MessageBoxType m_msgType = MessageBoxType::Ok;
    std::map<MessageBoxResult, std::string> m_buttonText;

    std::shared_ptr<UIWindowPanel> m_dialogPanel;

    void SetupDialog();
    bool OverlayUsesScale() const override { return false; }
};

} // namespace jwgui
