#pragma once

#include "UIPopup.h"
#include <string>

namespace jwgui {

class JWGUI_API UIToolTip : public UIPopup {
public:
    static constexpr unsigned int kTimerId = UIRenderView::kToolTipTimerId;
    static constexpr unsigned int kDelayMs = 500;

    UIToolTip(float x, float y, float w, float h);
    ~UIToolTip() override = default;

    void SetText(const std::string& text);
    const std::string& GetText() const { return m_text; }

    void MeasureToFit();

    static void HandleViewHook(UIRenderView* view, UIRenderView::ToolTipAction action);
    static void FlushPendingShow();

protected:
    bool OverlayUsesScale() const override { return false; }

private:
    std::string m_text;
    std::shared_ptr<class UIRenderText> m_textItem;

    void BuildRenderItems() override;
};

} // namespace jwgui
