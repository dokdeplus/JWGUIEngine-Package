#pragma once

#include "UIPanel.h"
#include "UIPopup.h"
#include "UIEditBox.h"
#include "UIButton.h"
#include "../Core/Event.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

// One change in a UIColorPicker (live: the dialog is still open).
struct ColorPickEventArgs : public EventArgs {
    uint32_t color = 0;      // ARGB (a theme color: its current value)
    std::string themeColor;  // a theme swatch's name ("accent"); empty = a free color
};

// How a picker opened with UIColorPicker::Show ended.
struct ColorPickerResult {
    bool accepted = false;   // OK / Enter; false = Cancel, Esc, a click outside
    uint32_t color = 0;      // accepted: the new color; else the starting one
    std::string themeColor;  // as above
};

enum class ColorPickerLayout {
    EditorOnly,  // square, hue, alpha, value, OK / Cancel
    Full,        // a palette column first: theme colors, recent colors, the honeycomb
};

enum class ColorPickerPlacement {
    Popup,  // under an anchor (above it when there is no room), no dimming
    Modal,  // centered over a dimmed window
};

struct ColorPickerOptions {
    std::string title;  // "Hours"
    uint32_t color = 0xFF000000;
    std::string themeColor;  // the starting color is this theme swatch
    bool showAlpha = true;
    ColorPickerLayout layout = ColorPickerLayout::EditorOnly;
    ColorPickerPlacement placement = ColorPickerPlacement::Popup;
    UIRect anchor;  // Popup: the rect it opens under (GetScreenRect of the anchor view)
    // The window it shows in: the anchor view's GetWindowRoot(), so a picker
    // opened in a floating pane stays in that window. Null = the main window.
    std::shared_ptr<UIRenderView> window;
    // Full layout: named colors (a theme's roles, a sheet's own colors).
    struct Swatch {
        std::string name;
        uint32_t color = 0;
    };
    std::vector<Swatch> themeSwatches;
};

// The color dialog's card (after the earlier JWGUIEngine picker): a title, an
// optional palette column (theme colors, recent colors, a honeycomb of hues),
// a saturation / value square with a hue bar, an alpha bar, the value as text
// (#RRGGBB, #RRGGBBAA, rgb(), @name) and Cancel / OK. Enter = OK, Esc = Cancel.
// Drawn by the engine (no OS dialog), the same on every platform and in the
// browser. Open one with Show; the card can also sit inside a panel.
//
// Style parts: ColorPicker (the card: Popup look), .caption (labels,
// ForegroundColor), .swatch (edges, OutlineColor), .ring (the current swatch,
// ForegroundColor).
class JWGUI_API UIColorPicker : public UIPanel {
public:
    using Swatch = ColorPickerOptions::Swatch;

    UIColorPicker(float x, float y, float w, float h);

    void SetTitle(const std::string& title);
    void SetLayout(ColorPickerLayout layout);
    void SetShowAlpha(bool show);
    void SetThemeSwatches(std::vector<Swatch> swatches);
    // Shows a color without an event. `themeColor` names it when it is a swatch.
    void SetColor(uint32_t argb, const std::string& themeColor = std::string());
    uint32_t GetColor() const { return m_color; }
    const std::string& GetThemeColor() const { return m_themeColor; }
    // The card's size for its layout and sections.
    float PreferredWidth() const;
    float PreferredHeight() const;

    // Every change while the card is open (drag, swatch, text).
    EventHandler<ColorPickEventArgs> ColorChangingEvent;
    // OK / Enter (true) or Cancel / Esc (false).
    std::function<void(bool accepted)> OnFinish;

    // Opens a picker over the app's root view. `onChanging` fires on every
    // change (for a live preview), `onClosed` once. A pick with OK goes to the
    // recent colors.
    static std::shared_ptr<UIColorPicker> Show(const ColorPickerOptions& options,
                                               std::function<void(const ColorPickEventArgs&)> onChanging,
                                               std::function<void(const ColorPickerResult&)> onClosed);

    // Free colors picked with OK, newest first (process-wide, like the earlier picker).
    static const std::vector<uint32_t>& RecentColors();
    static void AddRecentColor(uint32_t argb);

    // h in degrees [0, 360), s and v in [0, 1].
    static void ArgbToHsv(uint32_t argb, float& h, float& s, float& v);
    static uint32_t HsvToArgb(float h, float s, float v, uint32_t alpha);
    // The honeycomb's cells (axial q, r) and colors: white in the middle, hue
    // by angle (red at the top), deeper toward the rim.
    struct HoneycombCell {
        int q = 0;
        int r = 0;
        uint32_t color = 0;
    };
    static std::vector<HoneycombCell> Honeycomb(int radius);

    void SetStyle(std::shared_ptr<UIStyle> style) override;
    void OnLayout() override;
    void BuildRenderItems() override;
    bool IsTabStop() const override { return true; }
    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    void OnMouseLeave() override;
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;

private:
    class PaintItem;
    enum class Drag { None, Square, Hue, Alpha };
    struct Cell {
        std::string name;  // theme swatch name; empty for others
        uint32_t color = 0;
        float cx = 0.0f;
        float cy = 0.0f;
    };

    std::string m_title;
    ColorPickerLayout m_layout = ColorPickerLayout::EditorOnly;
    bool m_showAlpha = true;
    std::vector<Swatch> m_theme;
    uint32_t m_color = 0xFF000000;
    std::string m_themeColor;
    float m_hue = 0.0f;
    float m_sat = 0.0f;
    float m_val = 0.0f;
    float m_alpha = 1.0f;
    Drag m_drag = Drag::None;
    int m_hoverCell = -1;

    std::shared_ptr<PaintItem> m_paint;
    std::shared_ptr<UIEditBox> m_text;
    std::shared_ptr<UIButton> m_eyedropper;  // where the host can pick from the screen
    std::shared_ptr<UIButton> m_cancel;
    std::shared_ptr<UIButton> m_ok;

    // Geometry (view space).
    std::vector<Cell> m_cells;  // theme, recent, honeycomb
    float m_hexRadius = 9.0f;
    float m_themeCaptionY = 0.0f;
    float m_recentCaptionY = 0.0f;
    float m_colorsCaptionY = 0.0f;
    UIRect m_square;
    UIRect m_hueBar;
    UIRect m_alphaBar;
    UIRect m_preview;

    void Arrange();
    int CellAt(float x, float y) const;
    void DragTo(float x, float y);
    void FromHsv();
    void ShowText();
    void CommitText();
    void Changed();
    void PickFromScreen();
    void Finish(bool accepted);
    void Draw(IRenderer* renderer, float x, float y);
    uint32_t PartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
};

} // namespace jwgui
