#pragma once

#include "UIPanel.h"
#include <functional>

namespace jwgui {

class JWGUI_API UIPopup : public UIPanel {
public:
    UIPopup(float x, float y, float w, float h);
    virtual ~UIPopup() = default;

    virtual void ShowPopup(std::shared_ptr<UIRenderView> parent, bool autoClose = true);
    // Place the popup's top-left at (x, y) in parent-local coordinates (extends down-right),
    // then attach. Flips left/up only when it would overflow the parent.
    virtual void ShowAt(std::shared_ptr<UIRenderView> parent, float x, float y, bool autoClose = true);
    virtual void ClosePopup();

    void PlaceAt(float x, float y, const UIRenderView* parent);

    void OnGlobalMouseDown(float x, float y) override;
    
    std::function<void()> OnClosed;

protected:
    virtual bool OverlayUsesScale() const { return true; }
    bool m_autoClose = true;
};

} // namespace jwgui
