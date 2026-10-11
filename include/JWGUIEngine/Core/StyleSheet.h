#pragma once

#include "Export.h"
#include "UIStyle.h"
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

// .jwss: the project style sheet as text (CSS-like), and a control's own rules
// in the scene (UISceneNode::localStyle). One reader for the editor, the
// Player and the web build. Format and version rules: doc/Classes/StyleSheet.md.
//
//   @jwss 1;
//   @palette { accent: #FF6A00; }         theme colors, every theme
//   @palette light { warn: #C77700; }     one theme only
//   DigitalClock.hours { color: #39A0D8; }
//   Button:hovered { background: @control-hover; }

// The syntax version this build reads and writes (`@jwss 1;`). It goes up only
// when an older reader would misread a file. New properties, at-rules,
// functions or selectors keep it: an older reader skips what it does not know
// (with a warning) and keeps it when it writes the text back.
constexpr int kStyleSheetVersion = 1;

enum class StyleSheetMode {
    Sheet,   // a .jwss file: rules with selectors, @palette, @jwss
    Inline,  // a control's own rules: declarations for the control itself,
             // then `.part { }`, `:state { }`, `.part:state { }` rules
};

struct StyleDiagnostic {
    enum class Severity { Warning, Error };
    Severity severity = Severity::Error;
    int line = 0;    // 1-based
    int column = 0;  // 1-based, in characters
    std::string message;
};

struct StyleDeclaration {
    std::string name;   // "color"
    std::string value;  // as written, trimmed: "#E24B4A", "@accent"
    int line = 0;
    int column = 0;
    // Byte offsets in the parsed text, for EditStyleSheetText (not written).
    size_t offset = 0;      // the name
    size_t valueBegin = 0;  // the value as written ...
    size_t valueEnd = 0;    // ... up to its last character (before any !important)
    size_t end = 0;         // after the ';', or at the '}' / end of text without one
    bool terminated = false;
};

// One statement of a sheet, in text order.
struct StyleStatement {
    enum class Kind { Rule, Palette, AtRule };
    Kind kind = Kind::Rule;
    // Rule: the selector list as written ("DigitalClock.hours, .night").
    // Palette: the theme name, lower case ("" = every theme).
    // AtRule: the name and prelude without '@' ("media print").
    std::string prelude;
    std::vector<StyleDeclaration> declarations;  // Rule, Palette
    // AtRule (one this build does not know): its block as written, so a later
    // version's rules survive a rewrite.
    std::string block;
    bool hasBlock = false;
    int line = 0;
    int column = 0;
    // Byte offsets in the parsed text (EditStyleSheetText).
    size_t offset = 0;                     // the selector / '@'
    size_t blockOpen = std::string::npos;  // the '{'
    size_t blockClose = std::string::npos; // the '}' (npos when not closed)
    size_t end = 0;                        // after the '}' or ';'
};

// The parsed text. It keeps what it does not understand (unknown properties
// and at-rules, values that do not read), so writing it back loses nothing
// but comments and layout; what applies is decided when it is applied.
struct JWGUI_API StyleSheetDocument {
    int version = kStyleSheetVersion;  // from @jwss; this version when absent
    bool hasVersion = false;
    size_t versionEnd = 0;             // offset after the @jwss statement
    std::vector<StyleDeclaration> declarations;  // Inline: for the control itself
    std::vector<StyleStatement> statements;

    bool Empty() const { return declarations.empty() && statements.empty(); }

    // Edits for the editor. `selector` "" = the control's own declarations
    // (Inline); otherwise a rule's selector, compared without spaces.
    // Find returns the declaration that wins (the last one).
    const StyleDeclaration* Find(const std::string& selector, const std::string& property) const;
    // Changes the last such declaration, or adds it (and the rule) at the end.
    void Set(const std::string& selector, const std::string& property, const std::string& value);
    // Removes every such declaration and any rule left empty. False if none.
    bool Remove(const std::string& selector, const std::string& property);
};

// Never fails: problems go to `diagnostics` with their line and column, and
// everything else is kept.
JWGUI_API StyleSheetDocument ParseStyleSheet(const std::string& text, StyleSheetMode mode,
                                             std::vector<StyleDiagnostic>* diagnostics = nullptr);
// Canonical text: @jwss first (Sheet), one declaration per line (Inline: one
// line), unknown at-rules as written. Comments are not kept.
JWGUI_API std::string WriteStyleSheet(const StyleSheetDocument& doc, StyleSheetMode mode);

// Changes one declaration in the text and leaves everything else as written
// (layout, comments, rules it does not know). `selector`: a rule's selector
// (compared without spaces; the last such rule), "" = the control's own
// declarations (Inline), "@palette" / "@palette dark" = a palette block.
// An empty `value` removes the declaration, and a rule or palette left empty.
// A missing rule or palette is added (a palette after @jwss, a rule at the
// end); empty Sheet text starts with "@jwss 1;".
JWGUI_API std::string EditStyleSheetText(const std::string& text, StyleSheetMode mode, const std::string& selector,
                                         const std::string& property, const std::string& value);
JWGUI_API bool HasErrors(const std::vector<StyleDiagnostic>& diagnostics);
// "line 3, column 12: Unknown property 'colour'".
JWGUI_API std::string FormatDiagnostic(const StyleDiagnostic& diagnostic);

// Syntax coloring of .jwss text for code views (UICodeEdit), one line at a
// time. `kind` is the part that colors it: "comment", "at-rule", "selector",
// "property", "value", "number", "color", "theme-color", "string",
// "punctuation".
struct StyleSheetToken {
    size_t begin = 0;
    size_t end = 0;
    const char* kind = "";
};
// `state` is what the line before ended in (0 before the first line); returns
// what this line ends in (inside a comment, a block, a value).
JWGUI_API int TokenizeStyleSheetLine(const std::string& line, int state, std::vector<StyleSheetToken>& out);

// Builds `style` from the built-in theme `theme` ("Dark" / "Light"; another
// name uses Dark) with the sheet on top: its @palette blocks for every theme
// and for this one change the palette first, the theme's rules come from that
// palette, then the sheet's rules go over them. Rules that do not read are
// skipped (ParseStyleSheet reported them); `diagnostics` gets what only shows
// here (a theme color this theme does not define).
JWGUI_API void ApplyStyleSheet(UIStyle& style, const std::string& theme, const StyleSheetDocument& sheet,
                               std::vector<StyleDiagnostic>* diagnostics = nullptr);

// A control's own rules (Inline) for UIRenderView::SetLocalStyle. Theme colors
// stay names, read from the view's sheet when drawn. Null when nothing applies.
JWGUI_API std::shared_ptr<const UILocalStyle> CompileLocalStyle(const StyleSheetDocument& doc);

// What a property's value looks like in a .jwss.
enum class StyleValueKind {
    Color,        // #RGB, #RGBA, #RRGGBB, #RRGGBBAA, rgb(), rgba(), transparent, @theme-color
    Number,       // 12, 12px
    Text,         // Malgun Gothic, "Malgun Gothic"
    Background,   // a color, linear-gradient(180deg, c1, c2, ...)
    Edges,        // 1 to 4 numbers, CSS order (top right bottom left)
    Corners,      // 1 to 4 numbers, CSS order (top-left top-right bottom-right bottom-left)
    Shadow,       // x y [blur [spread]] color
    Duration,     // 220ms, 0.2s, 220 (ms)
    Easing,       // linear, ease-out, ease-in-out
    Switch,       // on, off
    BorderWidth,  // one number: the outline; 2 to 4 numbers: widths per side
};
// Any property also takes `none`: no value (the theme's value is cancelled).

// A style property a sheet can set. The engine's own are built in; an app
// control with a style property of its own registers it so sheets can set it.
struct StylePropertyInfo {
    std::string name;  // .jwss name, kebab case: "border-color"
    UIStylePropertyID id = 0;
    StyleValueKind kind = StyleValueKind::Color;
    UIStylePropertyID sidesId = 0;  // BorderWidth: the property of the per-side form
};

// Adds the property, or replaces the one with the same name. UI thread.
JWGUI_API void RegisterStyleProperty(const StylePropertyInfo& info);
JWGUI_API const StylePropertyInfo* FindStyleProperty(const std::string& name);
// By the property it sets (BorderWidth: either of its two); null if none.
JWGUI_API const StylePropertyInfo* FindStyleProperty(UIStylePropertyID id);
JWGUI_API std::vector<StylePropertyInfo> StyleProperties();

// One value of that property. `outId` is the property it sets (BorderWidth
// sets one of two). Theme colors stay names in out.themeColors. False with a
// message for the user when it does not read.
JWGUI_API bool ParseStyleValue(const StylePropertyInfo& info, const std::string& text, UIStyleEntry& out,
                               UIStylePropertyID& outId, std::string& error);
// A color as written in a .jwss. `themeColor` (optional) receives the name of
// "@accent"; without it a theme color is an error.
JWGUI_API bool ParseStyleColor(const std::string& text, uint32_t& argb, std::string* themeColor, std::string& error);
// #RRGGBB, or #RRGGBBAA when not opaque.
JWGUI_API std::string FormatStyleColor(uint32_t argb);
// A state as a .jwss names it ("hovered", "hover", "selected", ...), added to
// `state`; "normal" adds nothing. False for a name it does not know.
JWGUI_API bool ParseStyleState(const std::string& name, UIState& state);

} // namespace jwgui
