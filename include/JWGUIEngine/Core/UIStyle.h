#pragma once
#include "Export.h"
#include "CoreTypes.h"
#include <cstdint>
#include <map>
#include <unordered_map>
#include <string>
#include <variant>
#include <vector>

namespace jwgui {

    using UIStylePropertyID = uint32_t;

    // FNV-1a 32-bit hash for generating unique style property IDs from strings
    JWGUI_API uint32_t HashStyleProperty(const char* str);

    namespace UIStyleProperty {
        extern JWGUI_API const UIStylePropertyID Background;
        extern JWGUI_API const UIStylePropertyID ForegroundColor;
        extern JWGUI_API const UIStylePropertyID OutlineColor;
        extern JWGUI_API const UIStylePropertyID OutlineThickness;
        extern JWGUI_API const UIStylePropertyID FontSize;
        extern JWGUI_API const UIStylePropertyID FontFamily;
        extern JWGUI_API const UIStylePropertyID BorderRadius;
        extern JWGUI_API const UIStylePropertyID BorderWidth;
        extern JWGUI_API const UIStylePropertyID Padding;
        extern JWGUI_API const UIStylePropertyID AccentColor;
        extern JWGUI_API const UIStylePropertyID SelectionBackgroundColor;
        extern JWGUI_API const UIStylePropertyID CaretColor;
        extern JWGUI_API const UIStylePropertyID BoxShadow;
        extern JWGUI_API const UIStylePropertyID TransitionDuration;
        extern JWGUI_API const UIStylePropertyID TransitionEasing;
        extern JWGUI_API const UIStylePropertyID AnimationEnabled;

        JWGUI_API bool IsInheritable(UIStylePropertyID prop);
    }

    // Built-in control type selectors (m_type). Theme rules use these strings.
    namespace UIStyleType {
        extern JWGUI_API const char* const Window;
        extern JWGUI_API const char* const Button;
        extern JWGUI_API const char* const CheckBox;
        extern JWGUI_API const char* const ComboBox;
        extern JWGUI_API const char* const SpinBox;
        extern JWGUI_API const char* const EditBox;
        extern JWGUI_API const char* const ListBox;
        extern JWGUI_API const char* const Popup;
        extern JWGUI_API const char* const PopupMenu;
        extern JWGUI_API const char* const PopupListBox;
        extern JWGUI_API const char* const RadioButton;
        extern JWGUI_API const char* const Switch;
        extern JWGUI_API const char* const ToolButton;
        extern JWGUI_API const char* const ExpanderHeader; // UIExpander title row
        extern JWGUI_API const char* const Panel;
        extern JWGUI_API const char* const WindowPanel;
        extern JWGUI_API const char* const TitleBar;
        extern JWGUI_API const char* const MessageBox;
        extern JWGUI_API const char* const MenuBar;
        extern JWGUI_API const char* const Menu;
        extern JWGUI_API const char* const TabControl;
        extern JWGUI_API const char* const Tab;
        extern JWGUI_API const char* const TreeView;
        extern JWGUI_API const char* const DockHost;
        extern JWGUI_API const char* const DockWidget;
        extern JWGUI_API const char* const DockTitleBar;
        extern JWGUI_API const char* const DockSplitter;
        extern JWGUI_API const char* const DockHint;
        extern JWGUI_API const char* const DockTabBar;
        extern JWGUI_API const char* const Slider;
        extern JWGUI_API const char* const ProgressBar;
        extern JWGUI_API const char* const ToolTip;
        extern JWGUI_API const char* const ToolBar;
        extern JWGUI_API const char* const StatusBar;
        extern JWGUI_API const char* const Splitter;
        extern JWGUI_API const char* const DateControl;
        extern JWGUI_API const char* const TimeControl;
        extern JWGUI_API const char* const Calendar;
        extern JWGUI_API const char* const ColorPicker;
        extern JWGUI_API const char* const CodeEdit;
        extern JWGUI_API const char* const ListView;
    }

    // Built-in style part class names (UIRenderItem::m_className).
    // Combined selector form: Type.class  e.g. "ListBox.item", "CheckBox.box"
    //
    // Control catalog (what users can target):
    //   Button / EditBox / PopupMenu / Window / Popup : Type only
    //   CheckBox      : .box (check square), .text (label)
    //   ComboBox      : Type (body), .arrow (drop indicator). DropDown style hosts an inner EditBox.
    //   SpinBox       : Type (body), .arrow (up/down glyphs)
    //   ListBox       : .bg, .item, .text, .outline
    //   PopupListBox  : .bg, .item, .outline  (+ .text via ListBox items)
    //   Menu          : .item, .separator
    //   MenuBar       : .item
    //   TabControl    : Type only (strip)
    //   Tab           : .text, .close
    //   DockTitleBar  : .close
    //   TreeView      : .item, .arrow
    //   Slider        : .track, .fill, .handle
    //   ProgressBar   : .track, .fill
    //   ToolTip       : Type only
    //   ToolBar       : .separator
    //   StatusBar     : Type only
    //   Splitter      : Type (sash)
    //   DateControl / TimeControl : Type (segmented bar, EditBox chrome), .arrow (spin / calendar glyph)
    //   Calendar      : Type (month grid), .header, .day, .arrow (prev/next month)
    //   CodeEdit      : Type (field, font), .gutter (line numbers), .gutter-current, .current-line,
    //                   .error / .warning / .info (marks), and the highlighter's parts (.comment,
    //                   .selector, .property, .value, .number, .color, .theme-color, .at-rule, ...)
    //   Gauges / indicators : .zone-<name> (normal, ok, warn, alarm, info, or the
    //                   instance's own names) color the value by zone
    namespace UIStyleClass {
        extern JWGUI_API const char* const Background; // "bg"
        extern JWGUI_API const char* const Item;       // "item"
        extern JWGUI_API const char* const Outline;    // "outline"
        extern JWGUI_API const char* const Text;       // "text"
        extern JWGUI_API const char* const Arrow;      // "arrow"
        extern JWGUI_API const char* const Box;        // "box" (CheckBox)
        extern JWGUI_API const char* const Track;      // "track" (Switch)
        extern JWGUI_API const char* const Handle;     // "handle" (Switch)
        extern JWGUI_API const char* const Radio;      // "radio" (RadioButton)
        extern JWGUI_API const char* const Separator;  // "separator" (Menu)
        extern JWGUI_API const char* const Close;      // "close" (Tab, DockTitleBar)
        extern JWGUI_API const char* const Fill;       // "fill" (Slider, ProgressBar)
        extern JWGUI_API const char* const ScrollBar;  // "scrollbar" (any view's bars)
        extern JWGUI_API const char* const Header;     // "header" (Calendar)
        extern JWGUI_API const char* const Day;        // "day" (Calendar)
        extern JWGUI_API const char* const Image;      // "image" (Button / ToolButton icon)
    }

    // Builds "Type" or "Type.class" selector strings for theme APIs.
    inline std::string StyleSel(const char* type) {
        return type ? std::string(type) : std::string();
    }
    inline std::string StyleSel(const char* type, const char* cls) {
        return std::string(type) + "." + cls;
    }

    struct UIGradient {
        std::vector<uint32_t> colors;
        float angle = 0.0f; // degrees
    };

    struct UIImageFill {
        void* imageHandle = nullptr; // IRenderer::ImageHandle
    };

    struct UISolidColor {
        uint32_t color = 0x00000000;
    };

    using UIBackground = std::variant<UISolidColor, UIGradient, UIImageFill>;

    struct UIBoxShadow {
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float blur = 0.0f;
        float spread = 0.0f;
        uint32_t color = 0x00000000;
    };

    enum class UIStylePropertyType {
        None,
        Color,
        Float,
        Background,
        Thickness,
        String,
        CornerRadius,
        BoxShadow
    };

    struct UIStylePropertyValue {
        UIStylePropertyType type = UIStylePropertyType::None;
        std::variant<std::monostate, uint32_t, float, UIBackground, jwgui::Thickness, std::string, jwgui::CornerRadius, UIBoxShadow> value;

        UIStylePropertyValue() = default;
        UIStylePropertyValue(uint32_t c) : type(UIStylePropertyType::Color), value(c) {}
        UIStylePropertyValue(float f) : type(UIStylePropertyType::Float), value(f) {}
        UIStylePropertyValue(const UIBackground& b) : type(UIStylePropertyType::Background), value(b) {}
        UIStylePropertyValue(const jwgui::Thickness& t) : type(UIStylePropertyType::Thickness), value(t) {}
        UIStylePropertyValue(const std::string& s) : type(UIStylePropertyType::String), value(s) {}
        UIStylePropertyValue(const jwgui::CornerRadius& r) : type(UIStylePropertyType::CornerRadius), value(r) {}
        UIStylePropertyValue(const UIBoxShadow& s) : type(UIStylePropertyType::BoxShadow), value(s) {}
    };

    inline UIBackground MakeSolidBackground(uint32_t color) {
        return UIBackground{ UISolidColor{color} };
    }

    class UIRenderView;

    // Style lookup context. Not an HTML DOM — id/class/type are flat selectors.
    struct UIStyleQuery {
        const UIRenderView* view = nullptr; // parent chain for optional inheritance
        std::string id;
        std::string className;
        std::string type;
        // Types `type` builds on (an app subclass of a built-in): rules for
        // them apply when `type` has none.
        const std::vector<std::string>* baseTypes = nullptr;
        UIState state = UIState::Normal;
        // Only rules that name the part: the view's own .part (UILocalStyle),
        // #id.part, Type.part, Base.part, .part. A part that falls back to
        // another part (DigitalClock.hours -> .segment) asks this first.
        bool partOnly = false;
    };

    // A theme color named in a value (@accent in a .jwss): which color of the
    // value it fills in.
    struct UIThemeColorRef {
        uint8_t slot = 0;  // 0: the color, a solid fill or a shadow; i: gradient stop i
        std::string name;  // palette role ("accent", "text-dim") or a sheet's own color
    };

    // A style value as a sheet wrote it: its theme colors stay names until the
    // value is read (UIStyle::Resolve), so it follows a theme change.
    struct UIStyleEntry {
        UIStylePropertyValue value;
        std::vector<UIThemeColorRef> themeColors;
    };

    // A control's own rules from the scene (UISceneNode::localStyle), per part
    // and state. UIStyle reads them before the sheet; the view's SetLocal*
    // values still come first for the control itself. Part "" is the control
    // itself (the items SetLocal* reaches); any other part is a part item.
    class JWGUI_API UILocalStyle {
    public:
        void Set(const std::string& part, UIStylePropertyID prop, UIState stateMask, UIStyleEntry entry);
        // The entry for that state (the state order of UIStyle), or null.
        const UIStyleEntry* Find(const std::string& part, UIStylePropertyID prop, UIState state) const;
        bool Empty() const { return m_rules.empty(); }

    private:
        std::unordered_map<std::string, std::unordered_map<UIStylePropertyID, std::unordered_map<uint32_t, UIStyleEntry>>> m_rules;
    };

    // Semantic colors for one theme. Every control takes its colors from these
    // roles, so the same kind of thing (a field, a raised control, a selected
    // row) looks the same everywhere and Dark / Light share one rule set.
    struct UIThemePalette {
        // Layers: window (scene background) < surface (cards, bars). Fields
        // sit below the surface (sunken); buttons sit above it (raised).
        uint32_t window;
        uint32_t surface;
        uint32_t popup;          // menus, popups, dialog body
        uint32_t titleBar;       // dialog title, set apart from the body
        uint32_t field;          // edit, combo, spin, date, time, list, tree, calendar, check box
        uint32_t fieldHover;
        uint32_t control;        // buttons
        uint32_t controlHover;
        uint32_t controlPressed;
        uint32_t controlDisabled;
        // Lines
        uint32_t border;         // control outline
        uint32_t borderStrong;   // control outline on hover, check / radio edges
        uint32_t divider;        // card outlines, separators
        // Text
        uint32_t text;
        uint32_t textDim;        // captions, inactive tabs, arrows, status
        uint32_t textDisabled;
        // Accent: focus ring, on-state fills, sliders, progress
        uint32_t accent;
        uint32_t accentHint;     // translucent accent (dock drop hint)
        // Selection
        uint32_t rowHover;       // list / tree / menu row under the pointer
        uint32_t rowSelected;    // selected row (text stays `text`)
        uint32_t textSelection;  // selected text, calendar selected day
        // Small parts
        uint32_t track;          // slider / progress track
        uint32_t thumb;          // slider handle, switch handle when on
        uint32_t thumbHover;
        bool thumbOutline;       // light theme: white handle needs an edge
        uint32_t overlay;        // message box backdrop
        uint32_t shadow;         // popup / dialog shadow
        // Value zones of gauges and indicators (Type.zone-<name>); the
        // "normal" zone uses accent.
        uint32_t ok;
        uint32_t warn;
        uint32_t alarm;
        uint32_t info;
    };

    // Central stylesheet: themes, property storage, and selector resolution.
    class JWGUI_API UIStyle {
    public:
        UIStyle() = default;
        ~UIStyle() = default;

        void SetDarkTheme();
        void SetLightTheme();
        void SetGradientDarkTheme();
        // The font family the themes give `*` and code falls back to: the
        // platform's UI font (Application sets it from the host at start),
        // "sans-serif" until then. A project changes it for every control
        // with a `* { font-family }` rule or the Window's font in the editor;
        // a control's own rule still wins.
        static void SetDefaultFontFamily(const std::string& family);
        static const std::string& DefaultFontFamily();
        // Every rule from this palette: a built-in theme with some roles
        // changed (a sheet's @palette). Clears the rules and named colors.
        void SetTheme(const UIThemePalette& palette);
        // The palette of a built-in theme by name ("Dark", "Light", any case).
        // False for another name.
        static bool BuiltinPalette(const std::string& theme, UIThemePalette& out);

        // The palette of the last theme, or null.
        const UIThemePalette* GetThemePalette() const { return m_hasPalette ? &m_palette : nullptr; }

        // Theme colors by name, as a .jwss writes them (@accent): the palette
        // roles in kebab case ("accent", "text-dim", "row-selected", ...) and
        // the colors a sheet adds with SetNamedColor ("brand").
        static const std::vector<std::string>& PaletteRoleNames();
        static bool GetPaletteRole(const UIThemePalette& palette, const std::string& name, uint32_t& out);
        // False when `name` is not a palette role.
        static bool SetPaletteRole(UIThemePalette& palette, const std::string& name, uint32_t color);
        void SetNamedColor(const std::string& name, uint32_t color);
        const std::map<std::string, uint32_t>& GetNamedColors() const { return m_namedColors; }
        bool FindThemeColor(const std::string& name, uint32_t& out) const;
        // The entry's value with its theme colors filled in from this sheet.
        // False when it names a color this sheet does not have.
        bool Resolve(const UIStyleEntry& entry, UIStylePropertyValue& out) const;
        // Adds rules for controls that are not in the built-in sheet. Called
        // at the end of every theme; the default runs each registered
        // ControlInfo::Theme. Replace it only to take over all control rules.
        using ControlThemeProvider = void (*)(UIStyle& style, const UIThemePalette& palette);
        static void SetControlThemeProvider(ControlThemeProvider provider);

        void SetNone(const std::string& selector, UIStylePropertyID prop, UIState stateMask);
        void SetColor(const std::string& selector, UIStylePropertyID prop, UIState stateMask, uint32_t color);
        void SetFloat(const std::string& selector, UIStylePropertyID prop, UIState stateMask, float value);
        void SetBackground(const std::string& selector, UIStylePropertyID prop, UIState stateMask, const UIBackground& bg);
        void SetThickness(const std::string& selector, UIStylePropertyID prop, UIState stateMask, const jwgui::Thickness& value);
        void SetString(const std::string& selector, UIStylePropertyID prop, UIState stateMask, const std::string& value);
        void SetCornerRadius(const std::string& selector, UIStylePropertyID prop, UIState stateMask, const jwgui::CornerRadius& value);
        void SetBoxShadow(const std::string& selector, UIStylePropertyID prop, UIState stateMask, const UIBoxShadow& value);
        // Any value type (a rule read from a .jwss).
        void SetProperty(const std::string& selector, UIStylePropertyID prop, UIState stateMask, const UIStylePropertyValue& value);

        bool GetProperty(UIStylePropertyID prop, const UIStyleQuery& query, UIStylePropertyValue& outVal) const;
        bool GetColor(UIStylePropertyID prop, const UIStyleQuery& query, uint32_t& outColor) const;
        bool GetFloat(UIStylePropertyID prop, const UIStyleQuery& query, float& outFloat) const;
        bool GetBackground(UIStylePropertyID prop, const UIStyleQuery& query, UIBackground& outBg) const;
        bool GetThickness(UIStylePropertyID prop, const UIStyleQuery& query, jwgui::Thickness& outThickness) const;
        bool GetString(UIStylePropertyID prop, const UIStyleQuery& query, std::string& outString) const;
        bool GetCornerRadius(UIStylePropertyID prop, const UIStyleQuery& query, jwgui::CornerRadius& outRadius) const;
        bool GetBoxShadow(UIStylePropertyID prop, const UIStyleQuery& query, UIBoxShadow& outShadow) const;

        bool Save(const char* path) const;
        bool Load(const char* path);
        // Same as Load, from the text of a .jwstyle (e.g. inside a .jwpack).
        bool LoadFromString(const std::string& xml);
        // Like Load / LoadFromString, but the file goes on top of the current
        // rules: where it sets the same selector, property and state it wins,
        // and every other rule stays (a project sheet over a built-in theme).
        // A file that does not read changes nothing.
        bool Merge(const char* path);
        bool MergeFromString(const std::string& xml);

    private:
        bool LoadDocument(const void* xmlDoc, bool merge);
        // skipUniversal leaves out the "" selector (the universal default):
        // an inheritable property asks the parents before it.
        bool ResolveAt(UIStylePropertyID prop, const std::string& id, const std::string& className,
                       const std::string& type, const std::vector<std::string>* baseTypes,
                       UIState state, bool partOnly, UIStylePropertyValue& outVal,
                       bool skipUniversal = false) const;
        void ApplyPalette(const UIThemePalette& palette);
        bool MatchState(const std::unordered_map<uint32_t, UIStylePropertyValue>& stateMap,
                        UIState state, UIStylePropertyValue& outVal) const;

        std::unordered_map<std::string, std::unordered_map<UIStylePropertyID, std::unordered_map<uint32_t, UIStylePropertyValue>>> m_properties;
        UIThemePalette m_palette{};
        bool m_hasPalette = false;
        std::map<std::string, uint32_t> m_namedColors;
    };

} // namespace jwgui
