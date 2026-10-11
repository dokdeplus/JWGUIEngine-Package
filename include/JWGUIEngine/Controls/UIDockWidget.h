#pragma once

#include "UIPanel.h"
#include "UIStatic.h"
#include "UIButton.h"
#include "../Core/Event.h"
#include "../Layout/LayoutTypes.h"
#include <cstdint>
#include <limits>
#include <string>

namespace jwgui {

class UIDockHost;

enum class DockFeature : uint32_t {
    Closable  = 1 << 0,
    Movable   = 1 << 1,
    Floatable = 1 << 2
};

constexpr uint32_t operator|(DockFeature a, DockFeature b) {
    return static_cast<uint32_t>(a) | static_cast<uint32_t>(b);
}
constexpr uint32_t operator|(uint32_t a, DockFeature b) {
    return a | static_cast<uint32_t>(b);
}

constexpr uint32_t kDockFeatureDefault =
    static_cast<uint32_t>(DockFeature::Closable) |
    static_cast<uint32_t>(DockFeature::Movable) |
    static_cast<uint32_t>(DockFeature::Floatable);

class JWGUI_API UIDockWidget : public UIPanel {
public:
    UIDockWidget(float x, float y, float w, float h);
    virtual ~UIDockWidget();

    void OnLayout() override;
    void SetStyle(std::shared_ptr<UIStyle> style) override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;

    void SetTitle(const std::string& title);
    const std::string& GetTitle() const { return m_title; }

    void SetWidget(std::shared_ptr<UIRenderView> content);
    std::shared_ptr<UIRenderView> GetWidget() const { return m_content; }

    void SetFeatures(uint32_t features);
    uint32_t GetFeatures() const { return m_features; }
    bool HasFeature(DockFeature feature) const;

    void SetDockVisible(bool visible);
    bool IsDockVisible() const { return m_dockVisible; }

    void SetFloating(bool floating);
    bool IsFloating() const { return m_floating; }
    // Opens as a floating window at (x, y) with that size, in screen pixels,
    // or moves it there when it floats already. False when the platform has no
    // owned windows (a host then docks it instead).
    bool ShowFloating(int x, int y, int width, int height);

    void SetShowTitleBar(bool show);
    bool ShowTitleBar() const { return m_showTitleBar; }

    void BeginTearOff(float grabOffsetX, float grabOffsetY);

    void SetHost(UIDockHost* host) { m_host = host; }
    UIDockHost* GetHost() const { return m_host; }

    void SetMinimumSize(float w, float h);
    void SetMaximumSize(float w, float h);
    Size GetMinimumSize() const { return m_minSize; }
    Size GetMaximumSize() const { return m_maxSize; }

    EventHandler<EventArgs> CloseEvent;
    EventHandler<EventArgs> VisibilityChangedEvent;

    static constexpr float kTitleBarHeight = 24.0f;

private:
    friend class UIDockHost;

    std::string m_title;
    uint32_t m_features = kDockFeatureDefault;
    bool m_dockVisible = true;
    bool m_floating = false;
    bool m_showTitleBar = true;
    bool m_dragPending = false;
    bool m_dragging = false;
    float m_dragStartX = 0.0f;
    float m_dragStartY = 0.0f;
    float m_grabOffsetX = 0.0f;
    float m_grabOffsetY = 0.0f;
    void* m_floatHwnd = nullptr;
    UIDockHost* m_host = nullptr;

    std::shared_ptr<UIPanel> m_titleBar;
    std::shared_ptr<UIStatic> m_titleLabel;
    std::shared_ptr<UIButton> m_closeBtn;
    std::shared_ptr<UIRenderView> m_content;
    std::shared_ptr<jwgui::LayoutNode> m_titleNode;
    std::shared_ptr<jwgui::LayoutNode> m_contentNode;

    void RebuildChrome();
    void ApplyTitleChromeStyles();
    void OnCloseClicked();
    void BeginTitleDrag(float x, float y);
    void UpdateTitleDrag(float x, float y);
    void EndTitleDrag(float x, float y);
    void CommitFloatGesture();
    void ApplyFloatWindowPos();
    void ClearFloatWindow();
    bool m_suppressFloatClose = false;
    Size m_minSize;
    Size m_maxSize = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
};

} // namespace jwgui
