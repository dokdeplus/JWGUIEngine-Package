#pragma once

#include "Export.h"
#include <cstdint>
#include <string>
#include <vector>

namespace jwgui {

// ---------------------------------------------------------
// Geometry (UIRect)
// ---------------------------------------------------------
struct JWGUI_API UIRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    UIRect() = default;
    UIRect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}

    float Right() const { return x + width; }
    float Bottom() const { return y + height; }
    bool Contains(float px, float py) const { return px >= x && px <= Right() && py >= y && py <= Bottom(); }
};

// ---------------------------------------------------------
// State (bit flags; combine with | )
// ---------------------------------------------------------
enum class UIState : uint32_t {
    Normal   = 0,
    Hovered  = 1 << 0,
    Pressed  = 1 << 1,
    Selected = 1 << 2,
    Focused  = 1 << 3,
    Disabled = 1 << 4,
};

constexpr uint32_t ToU32(UIState state) {
    return static_cast<uint32_t>(state);
}

constexpr UIState operator|(UIState a, UIState b) {
    return static_cast<UIState>(ToU32(a) | ToU32(b));
}
constexpr UIState operator&(UIState a, UIState b) {
    return static_cast<UIState>(ToU32(a) & ToU32(b));
}
constexpr UIState operator~(UIState a) {
    return static_cast<UIState>(~ToU32(a));
}
constexpr UIState& operator|=(UIState& a, UIState b) {
    a = a | b;
    return a;
}
constexpr UIState& operator&=(UIState& a, UIState b) {
    a = a & b;
    return a;
}

constexpr bool Has(UIState state, UIState flag) {
    if (flag == UIState::Normal) return state == UIState::Normal;
    return (state & flag) != UIState::Normal;
}

// ---------------------------------------------------------
// Thickness
// ---------------------------------------------------------
/// Edge insets (margin, padding, text insets). Layout margin/padding is set via JWLayout API.
struct JWGUI_API Thickness {
    float left = 0, top = 0, right = 0, bottom = 0;

    Thickness() = default;
    Thickness(float all);
    Thickness(float h, float v);
    Thickness(float l, float t, float r, float b);

    static Thickness All(float v);
    static Thickness Horizontal(float v);
    static Thickness Vertical(float v);
};

// ---------------------------------------------------------
// CornerRadius (WPF order: TL, TR, BR, BL)
// ---------------------------------------------------------
struct JWGUI_API CornerRadius {
    float topLeft = 0.0f;
    float topRight = 0.0f;
    float bottomRight = 0.0f;
    float bottomLeft = 0.0f;

    CornerRadius() = default;
    CornerRadius(float all);
    CornerRadius(float tl, float tr, float br, float bl);

    bool IsUniform() const {
        return topLeft == topRight && topLeft == bottomRight && topLeft == bottomLeft;
    }
    bool IsZero() const {
        return topLeft <= 0.0f && topRight <= 0.0f && bottomRight <= 0.0f && bottomLeft <= 0.0f;
    }
};

// ---------------------------------------------------------
// Alignment
// ---------------------------------------------------------
enum class HorizontalAlignment { Left, Center, Right };
enum class VerticalAlignment   { Top,  Center, Bottom };

enum class CursorType {
    Arrow,
    IBeam,
    Hand,
    SizeWE,
    SizeNS,
    SizeAll,
    Cross,
    SizeNWSE,
    SizeNESW
};

enum class MouseButton {
    Left,
    Right,
    Middle
};

enum class TextAlignH {
    Left,
    Center,
    Right
};

enum class TextAlignV {
    Top,
    Center,
    Bottom
};

// Icon vs text on UIButton (and similar chrome).
// Left/Right/Top/Bottom sit beside the caption.
// Center overlays a centered icon (icon-only bars).
// Absolute uses SetImagePosition local coordinates.
enum class ImagePlacement {
    Left,
    Right,
    Top,
    Bottom,
    Center,
    Absolute
};

class IRenderer;

// ---------------------------------------------------------
// KeyCode & KeyModifiers
// ---------------------------------------------------------
enum class KeyCode {
    Unknown = 0,
    Left, Right, Up, Down,
    Home, End, PageUp, PageDown,
    Enter, Escape, Space, Tab, Backspace, DeleteKey,
    Add, Subtract,
    F11,
    Digit1, Digit2,
    A, C, L, V, X,
    // Appended: the web host passes these values as integers.
    F5, S, Z, Y, D,
    N, P, R,
    // The rest of the keyboard (app shortcuts: Ctrl+F, F2, Ctrl+0, ...).
    B, E, F, G, H, I, J, K, M, O, Q, T, U, W,
    Digit0, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9,
    F1, F2, F3, F4, F6, F7, F8, F9, F10, F12,
    Insert, Multiply, Divide,
    Comma, Period, Slash, BracketLeft, BracketRight
};

struct KeyModifiers {
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

enum class SelectionMode {
    Single,
    Multiple,
    Extended
};

enum class ImageStretch {
    Stretch,
    Uniform,
    UniformToFill,
    None
};

KeyCode KeyCodeFromLegacyVirtualKey(int vk);
int KeyCodeToLegacyVirtualKey(KeyCode key);

// ---------------------------------------------------------
// WindowCreateOptions
// ---------------------------------------------------------
/// Native window chrome mode. Stable values for future C# P/Invoke wrappers.
enum class JWGUI_API WindowChromeStyle : uint32_t {
    Standard = 0,   ///< Standard overlapped window with system title bar.
    Borderless = 1, ///< No system title bar; client area fills the window frame.
};

/// Window creation parameters consumed by WindowApp before Initialize().
/// Intended to map cleanly to a C# wrapper struct (Sequential layout).
struct JWGUI_API WindowCreateOptions {
    /// Layout units: the window comes out larger on a 150 % monitor.
    int width = 800;
    int height = 600;
    const wchar_t* title = L"";

    WindowChromeStyle chromeStyle = WindowChromeStyle::Standard;

    /// Applies to Standard chrome. Borderless windows use thick frame when true.
    bool resizable = true;
    /// Applies to Standard chrome only (caption buttons).
    bool minimizable = true;
    /// Applies to Standard chrome only (caption buttons).
    bool maximizable = true;

    /// Enable transparent layered window mode (WS_EX_LAYERED)
    bool isLayered = false;
    /// Initial opacity if isLayered is true
    float opacity = 1.0f;
    /// Floating tool window (WS_EX_TOOLWINDOW)
    bool toolWindow = false;
    /// width/height are the client area instead of the outer frame (main window).
    bool clientSize = false;
    /// Draw with the GPU (Skia: OpenGL on Win32, WebGL 2 in the browser) when
    /// available, else the CPU. Not for layered windows. JWGUI_RENDERER=cpu|gpu
    /// (environment; ?renderer= on the web) overrides it.
    bool gpu = false;
};

// ---------------------------------------------------------
// IWindowEventHandler
// ---------------------------------------------------------
/// Platform-agnostic interface for window events routed from PlatformHost
/// Sizes and points are device pixels (see IPlatformHost::GetScale).
class JWGUI_API IWindowEventHandler {
public:
    virtual ~IWindowEventHandler() = default;
    virtual void OnPaint() = 0;
    virtual void OnResize(int width, int height) = 0;
    /// The window's scale changed (another monitor's DPI, the OS scale
    /// setting, browser zoom). OnResize follows if the size changes too.
    virtual void OnScaleChanged(float scale) { (void)scale; }
    virtual void OnMouseMove(float x, float y) = 0;
    virtual void OnMouseLeaveWindow() {}
    virtual void OnMouseDown(float x, float y, MouseButton button) = 0;
    virtual void OnMouseUp(float x, float y, MouseButton button) = 0;
    virtual void OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) = 0;
    /// Trackpad pinch: scale factor since the last event (1 = no change).
    virtual void OnMagnify(float scale, float x, float y) { (void)scale; (void)x; (void)y; }
    virtual void OnKeyDown(KeyCode key, const KeyModifiers& modifiers) = 0;
    virtual void OnChar(unsigned int charCode) = 0;
    virtual void OnImeComposition(const std::string& compStr) = 0;
    virtual void OnTimer(unsigned int timerId) = 0;
    virtual void OnFrameTick(double nowMs) { (void)nowMs; }
    virtual void OnDestroy() = 0;
    // Window close button, Alt+F4 or RequestClose. Return false to keep the window open.
    virtual bool OnCloseRequested() { return true; }
    /// Files or folders dropped on the window (UTF-8 paths; in the browser,
    /// copies in the page's own file system). The point is where they fell.
    virtual void OnDropFiles(const std::vector<std::string>& paths, float x, float y) {
        (void)paths; (void)x; (void)y;
    }
    /// The host woke the UI thread for IPlatformHost::RequestPosted.
    virtual void OnPosted() {}
};

} // namespace jwgui
