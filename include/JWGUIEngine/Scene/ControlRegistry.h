#pragma once

#include "../Core/Export.h"
#include "../Core/UIRenderView.h"
#include "../Core/UIStyle.h"
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace jwgui {

class UISceneNode;

// What a scene property holds. The editor picks the Properties row from it
// (Number -> spin box, Enum -> combo box, ...).
enum class PropKind {
    Text,
    Number,
    Int,
    Bool,
    Enum,
    Color,
    Zones,
    Date,
    Time,
    Image  // a Project resource id; Properties shows a thumbnail and the image picker
};

// One property of a control: an XML attribute in the .jwui and a text value
// in UISceneNode::props.
struct PropSpec {
    std::string name;
    PropKind kind = PropKind::Text;
    std::string defaultValue;          // text form; equal values are not written
    std::string label;                 // Properties caption; empty = name
    std::string tip;
    std::string group;
    std::vector<std::string> options;  // Enum
    double minValue = -1.0e9;
    double maxValue = 1.0e9;
    double step = 1.0;
    int decimals = 0;
    // Shown in Properties only while property `visibleWhen` has one of
    // visibleValues (e.g. numerals only for an Analog clock). Empty = always.
    std::string visibleWhen;
    std::vector<std::string> visibleValues;
    // Gives the text value to a live view. The loader calls the setters in the
    // order they were declared (a range before the value it clamps).
    std::function<void(UIRenderView&, const std::string&)> apply;
    // Choosing a value in the editor also sets these properties (a preset
    // filling its options), in the same undo step. Declare the property
    // before the ones it fills, so a scene's own values win when loading.
    std::function<std::vector<std::pair<std::string, std::string>>(const std::string& value)> fills;
};

// A color the editor's Style pane offers for a control (its "main colors").
// It names a style part and property, so a pick is stored as a .jwss rule
// (UISceneNode::localStyle `.hours { color: ... }`, or a Type.part rule in the
// project sheet), never by the slot's own name: slots can be renamed, merged
// or reordered without touching saved files.
// What a Style pane row edits and how it is shown.
enum class StyleSlotKind {
    Color,    // a color field: color, background, border-color, accent-color, ...
    Length,   // one number in a spin box: border-width, font-size
    Corners,  // border-radius: one number in the row (four corners in the style code)
    Edges,    // padding: one to four numbers as text
    Toggle,   // the border on or off (a check box; see ControlInfo::Chrome)
    Text,     // font-family
};

// The Style pane's folding sections, in the order they show.
enum class StyleSlotSection { Fill, Text, Border, States, Parts, More };

// One row of the Style pane for a control type: a style property of the
// control itself or of one of its parts, edited for the selected control (its
// own rules) or for every control of the type (the sheet). ControlsPlan §9.
struct StyleSlotSpec {
    std::string name;   // stable id for the editor and its strings ("hours", "border-width")
    std::string label;  // row caption ("Hours", "Width")
    std::string part;   // style part; "" = the control itself
    UIStylePropertyID property = 0;  // ForegroundColor, Background, OutlineThickness, ...
    std::string tip;
    StyleSlotKind kind = StyleSlotKind::Color;
    StyleSlotSection section = StyleSlotSection::Parts;
    // The state it edits, as the .jwss names it ("selected": a switch that
    // is on); "" = Normal.
    std::string state;
    // Picking it also writes :hovered and :pressed shades of the color (and
    // clearing it clears them), so a Button keeps its hover and press.
    bool deriveStates = false;
    // Shown only while property `visibleWhen` has one of visibleValues.
    std::string visibleWhen;
    std::vector<std::string> visibleValues;
    // Length / Corners rows: the spin box.
    double minValue = 0.0;
    double maxValue = 100.0;
    double step = 1.0;
    int decimals = 0;
    // The rows every control shares (Chrome): one string for all types
    // ("control.slot.<name>") instead of one per type.
    bool sharedText = false;
};
using ColorSlotSpec = StyleSlotSpec;  // the earlier name

// Text <-> value for props. Unreadable text gives the fallback.
namespace PropText {
    JWGUI_API double ToNumber(const std::string& text, double fallback = 0.0);
    JWGUI_API int ToInt(const std::string& text, int fallback = 0);
    JWGUI_API bool ToBool(const std::string& text, bool fallback = false);
    JWGUI_API std::string FromNumber(double value);
    JWGUI_API std::string FromInt(int value);
    JWGUI_API std::string FromBool(bool value);

    template <class V> V Parse(const std::string& text);
    template <> inline double Parse<double>(const std::string& text) { return ToNumber(text); }
    template <> inline float Parse<float>(const std::string& text) { return static_cast<float>(ToNumber(text)); }
    template <> inline int Parse<int>(const std::string& text) { return ToInt(text); }
    template <> inline bool Parse<bool>(const std::string& text) { return ToBool(text); }
    template <> inline std::string Parse<std::string>(const std::string& text) { return text; }
}

// Everything the engine and the editor need to know about one control type:
// how to create it, its default size, where it sits in the Toolbox, and its
// scene properties. One ControlInfo replaces the per-type branches in the
// factory, loader, scene file, Properties and Toolbox.
//
//   ControlInfo::Make<UIRadialGauge>("RadialGauge", "Radial Gauge")
//       .Toolbox("Extended", "GAUGES", 400).Size(200, 200)
//       .Number("min", 0, &UIRadialGauge::SetMinimum)
//       .Number("value", 0, &UIRadialGauge::SetValue).Label("Value")
//
// A subclass in app code starts from its base with Extend and registers it
// with ControlRegistry::Register before loading scenes.
struct JWGUI_API ControlInfo {
    std::string type;          // element name in the .jwui
    std::string displayName;   // Toolbox and status bar
    std::string baseType;      // Extend: the type this one builds on
    std::string group;         // Toolbox group: "Basic", then "Extended" under it, then app
                               // groups ("Custom"); empty = not in the Toolbox
    std::string section;       // Toolbox section caption inside the group
    int order = 0;             // Toolbox order (sections sort by their first row)
    std::string icon;          // Host resource id; empty = "control.<type>"
    float width = 100.0f;      // size of a click placement
    float height = 32.0f;
    std::string defaultText;
    bool container = false;    // holds child controls
    bool ownsLayout = false;   // container that places its children itself
    // Built-in controls written before the registry keep their properties in
    // UISceneNode's typed fields; every other type uses UISceneNode::props.
    bool typedFields = false;
    std::vector<PropSpec> props;
    std::vector<std::string> parts;  // style parts: "Type.part" selectors
    std::vector<StyleSlotSpec> styleSlots;  // Style pane rows, in order inside their sections
    size_t lastSlot = 0;                    // the row the Slot* builders modify
    bool hasText = false;            // Properties shows Text; the loader calls applyText

    std::function<std::shared_ptr<UIRenderView>()> create;
    std::function<void(UISceneNode&)> initNode;  // a newly placed node (e.g. today's date)
    std::function<void(UIRenderView&, const std::string&)> applyText;
    // Style rules from the theme's palette roles, added to every Dark / Light
    // sheet (UIStyle::SetControlThemeProvider), so Extended controls look
    // right in both without editing the built-in sheet.
    std::function<void(UIStyle&, const UIThemePalette&)> theme;

    const PropSpec* FindProp(const std::string& name) const;
    const StyleSlotSpec* FindStyleSlot(const std::string& name) const;
    bool IsStyleSlotVisible(const StyleSlotSpec& slot, const UISceneNode& node) const;
    // The slots of one section, in order.
    std::vector<const StyleSlotSpec*> SlotsIn(StyleSlotSection where) const;
    std::string IconId() const;
    // The node's text for a property, or its default.
    std::string PropValue(const UISceneNode& node, const std::string& name) const;
    bool IsPropVisible(const PropSpec& spec, const UISceneNode& node) const;

    template <class T>
    static ControlInfo Make(const std::string& type, const std::string& displayName) {
        ControlInfo info;
        info.type = type;
        info.displayName = displayName;
        info.create = &CreateView<T>;
        return info;
    }

    // Starts from a base (its properties, parts and flags) for a subclass T.
    template <class T>
    static ControlInfo Extend(const ControlInfo& base, const std::string& type, const std::string& displayName) {
        ControlInfo info = base;
        info.type = type;
        info.displayName = displayName;
        info.baseType = base.type;
        info.group = "Custom";
        info.section.clear();
        info.icon = base.IconId();
        info.typedFields = false;
        info.create = &CreateView<T>;
        return info;
    }

    ControlInfo& Toolbox(const std::string& groupName, const std::string& sectionName, int sortOrder) {
        group = groupName;
        section = sectionName;
        order = sortOrder;
        return *this;
    }
    ControlInfo& Size(float w, float h) { width = w; height = h; return *this; }
    ControlInfo& DefaultText(const std::string& text) { defaultText = text; return *this; }
    ControlInfo& Icon(const std::string& id) { icon = id; return *this; }
    ControlInfo& Container(bool ownsChildLayout = false) { container = true; ownsLayout = ownsChildLayout; return *this; }
    ControlInfo& TypedFields() { typedFields = true; return *this; }
    ControlInfo& Parts(std::vector<std::string> names) { parts = std::move(names); return *this; }
    ControlInfo& InitNode(std::function<void(UISceneNode&)> fn) { initNode = std::move(fn); return *this; }
    ControlInfo& Creator(std::function<std::shared_ptr<UIRenderView>()> fn) { create = std::move(fn); return *this; }
    ControlInfo& Theme(std::function<void(UIStyle&, const UIThemePalette&)> fn) { theme = std::move(fn); return *this; }
    // The node's `text` goes to this setter (a caption under a gauge, ...).
    template <class T, class R, class Arg>
    ControlInfo& Caption(R (T::*setter)(Arg)) {
        hasText = true;
        applyText = [setter](UIRenderView& view, const std::string& text) {
            if (auto* target = dynamic_cast<T*>(&view)) (target->*setter)(text);
        };
        return *this;
    }

    template <class T, class R, class Arg>
    ControlInfo& Number(const std::string& name, double def, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Number, name, PropText::FromNumber(def), setter);
    }
    template <class T, class R, class Arg>
    ControlInfo& Int(const std::string& name, int def, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Int, name, PropText::FromInt(def), setter);
    }
    template <class T, class R, class Arg>
    ControlInfo& Bool(const std::string& name, bool def, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Bool, name, PropText::FromBool(def), setter);
    }
    template <class T, class R, class Arg>
    ControlInfo& Text(const std::string& name, const std::string& def, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Text, name, def, setter);
    }
    template <class T, class R, class Arg>
    ControlInfo& Image(const std::string& name, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Image, name, std::string(), setter);
    }
    template <class T, class R, class Arg>
    ControlInfo& Enum(const std::string& name, std::vector<std::string> options, const std::string& def,
                      R (T::*setter)(Arg)) {
        AddProp(PropKind::Enum, name, def, setter);
        props.back().options = std::move(options);
        return *this;
    }
    template <class T, class R, class Arg>
    ControlInfo& Color(const std::string& name, const std::string& def, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Color, name, def, setter);
    }
    template <class T, class R, class Arg>
    ControlInfo& Zones(const std::string& name, const std::string& def, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Zones, name, def, setter);
    }
    template <class T, class R, class Arg>
    ControlInfo& Time(const std::string& name, const std::string& def, R (T::*setter)(Arg)) {
        return AddProp(PropKind::Time, name, def, setter);
    }
    // Any kind with a hand-written apply.
    ControlInfo& Prop(PropSpec spec) { props.push_back(std::move(spec)); return *this; }
    // Rows of the Style pane: `part` ("" = the control) and its style
    // property. A slot with the same name is replaced in place, so a type can
    // relabel one of Chrome's rows or an Extend can change its base's. The
    // section follows the property unless SlotSection says otherwise: a state
    // -> States, a part -> Parts, background -> Fill, color / font size ->
    // Text, border and radius -> Border, the rest (padding, font family) -> More.
    ControlInfo& ColorSlot(const std::string& name, const std::string& label, const std::string& part,
                           UIStylePropertyID property = UIStyleProperty::ForegroundColor,
                           const std::string& tip = std::string());
    // One number (border-width, font-size); range 0-100, step 1 unless SlotRange says otherwise.
    ControlInfo& LengthSlot(const std::string& name, const std::string& label, const std::string& part,
                            UIStylePropertyID property, const std::string& tip = std::string());
    // The corner radius (one number in the row).
    ControlInfo& CornersSlot(const std::string& name, const std::string& label, const std::string& part,
                             const std::string& tip = std::string());
    // Four edges as text (padding).
    ControlInfo& EdgesSlot(const std::string& name, const std::string& label, const std::string& part,
                           UIStylePropertyID property, const std::string& tip = std::string());
    // The border on or off (OutlineThickness).
    ControlInfo& ToggleSlot(const std::string& name, const std::string& label, const std::string& part,
                            const std::string& tip = std::string());
    // A text value (font-family).
    ControlInfo& TextSlot(const std::string& name, const std::string& label, const std::string& part,
                          UIStylePropertyID property, const std::string& tip = std::string());
    // Any row.
    ControlInfo& StyleSlot(StyleSlotSpec spec);
    ControlInfo& RemoveSlot(const std::string& name);
    // Shows the slot declared last only while `prop` has one of `values`.
    ControlInfo& SlotVisibleWhen(const std::string& prop, std::vector<std::string> values) {
        if (StyleSlotSpec* slot = LastSlot()) {
            slot->visibleWhen = prop;
            slot->visibleValues = std::move(values);
        }
        return *this;
    }
    // The slot declared last edits this state ("selected") and moves to
    // STATES (a SlotSection after it can put it elsewhere).
    ControlInfo& SlotState(const std::string& state) {
        if (StyleSlotSpec* slot = LastSlot()) {
            slot->state = state;
            slot->section = StyleSlotSection::States;
        }
        return *this;
    }
    // The slot declared last writes hover and press shades too.
    ControlInfo& SlotDerivesStates() {
        if (StyleSlotSpec* slot = LastSlot()) slot->deriveStates = true;
        return *this;
    }
    ControlInfo& SlotSection(StyleSlotSection where) {
        if (StyleSlotSpec* slot = LastSlot()) slot->section = where;
        return *this;
    }
    ControlInfo& SlotRange(double minValue, double maxValue, double step = 1.0, int decimals = 0) {
        if (StyleSlotSpec* slot = LastSlot()) {
            slot->minValue = minValue;
            slot->maxValue = maxValue;
            slot->step = step;
            slot->decimals = decimals;
        }
        return *this;
    }
    StyleSlotSpec* LastSlot() { return lastSlot < styleSlots.size() ? &styleSlots[lastSlot] : nullptr; }

    // The rows of a control that draws a box of its own: Fill (background)
    // and Text (text color, font size) as plain rows at the top, then the
    // folding sections BORDER (show, color, width, radius), STATES (hovered,
    // pressed, focus ring, disabled), MORE (padding, font). Pick the groups
    // a control has; a type can then relabel a row by declaring it again
    // under the same name.
    enum ChromeRows : unsigned {
        ChromeFill = 1,
        ChromeText = 2,
        ChromeBorder = 4,
        ChromeStates = 8,
        ChromeMore = 16,
        ChromeBox = ChromeFill | ChromeBorder,
        ChromeAll = ChromeFill | ChromeText | ChromeBorder | ChromeStates | ChromeMore,
    };
    ControlInfo& Chrome(unsigned rows = ChromeAll);
    // The same for one part in PARTS, labeled "<label>", "<label> border",
    // "<label> border width", "<label> radius"; names "<part>", "<part>-border", ...
    ControlInfo& PartChrome(const std::string& part, const std::string& label, unsigned rows = ChromeBox);
    // Drops an inherited property (Extend) so Properties no longer shows it.
    ControlInfo& Remove(const std::string& name);

    // Modifiers for the property declared last.
    ControlInfo& Label(const std::string& text) { if (!props.empty()) props.back().label = text; return *this; }
    ControlInfo& Tip(const std::string& text) { if (!props.empty()) props.back().tip = text; return *this; }
    // The property declared last fills others when picked (PropSpec::fills).
    ControlInfo& Fills(std::function<std::vector<std::pair<std::string, std::string>>(const std::string&)> fn) {
        if (!props.empty()) props.back().fills = std::move(fn);
        return *this;
    }
    ControlInfo& Group(const std::string& name) { if (!props.empty()) props.back().group = name; return *this; }
    ControlInfo& VisibleWhen(const std::string& prop, std::vector<std::string> values) {
        if (!props.empty()) {
            props.back().visibleWhen = prop;
            props.back().visibleValues = std::move(values);
        }
        return *this;
    }
    ControlInfo& Range(double minV, double maxV, double stepV = 1.0, int decimalPlaces = 0) {
        if (!props.empty()) {
            props.back().minValue = minV;
            props.back().maxValue = maxV;
            props.back().step = stepV;
            props.back().decimals = decimalPlaces;
        }
        return *this;
    }

private:
    template <class T>
    static std::shared_ptr<UIRenderView> CreateView() {
        if constexpr (std::is_constructible_v<T, float, float, float, float>) {
            return std::make_shared<T>(0.0f, 0.0f, 0.0f, 0.0f);
        } else {
            return std::make_shared<T>();
        }
    }

    template <class T, class R, class Arg>
    ControlInfo& AddProp(PropKind kind, const std::string& name, const std::string& def, R (T::*setter)(Arg)) {
        PropSpec spec;
        spec.name = name;
        spec.kind = kind;
        spec.defaultValue = def;
        if (setter) {
            spec.apply = [setter](UIRenderView& view, const std::string& text) {
                if (auto* target = dynamic_cast<T*>(&view)) {
                    (target->*setter)(PropText::Parse<std::decay_t<Arg>>(text));
                }
            };
        }
        props.push_back(std::move(spec));
        return *this;
    }
};

// Process-wide list of control types. The engine's own controls are listed in
// BuiltinControls.cpp and registered on first use; app code adds its own
// (or replaces a built-in) with Register. UI thread only.
class JWGUI_API ControlRegistry {
public:
    // Adds the type, or replaces the entry with the same type name.
    static void Register(const ControlInfo& info);
    static const ControlInfo* Find(const std::string& type);
    // Registration order: built-ins first, then app types.
    static std::vector<const ControlInfo*> All();
    // The engine's own controls only.
    static std::vector<const ControlInfo*> BuiltIn();

    // New live view, Initialize()d. A type made with Extend gets its own
    // style type and its bases as style fallbacks.
    static std::shared_ptr<UIRenderView> Create(const std::string& type);
    // Base types of an Extend'ed type, nearest first.
    static std::vector<std::string> BaseChain(const std::string& type);
    // False for built-ins with typed UISceneNode fields; true for the rest,
    // unknown types included (their attributes are kept in props).
    static bool UsesProps(const std::string& type);

    // Gives each declared property of the node's type to the view: the
    // node's value, or the default when the node has none.
    static void ApplyProps(UIRenderView& view, const UISceneNode& node);
};

// The registry's words in the UI language (UIStrings), for the editor and
// the Controls reference. The English given at registration is the default;
// a type made with Extend falls back to its base types' ids.
//   control.<Type>                              Toolbox name
//   control.<Type>.text                         text of a newly placed control
//   control.<Type>.prop.<name>, ....tip         Properties caption and tip
//   control.<Type>.prop.<name>.option.<value>   an Enum value as shown (the value stays)
//   control.<Type>.color.<slot>, ....tip        Style pane color
//   toolbox.group.<Group>, toolbox.section.<SECTION>
struct JWGUI_API ControlText {
    static std::string Name(const ControlInfo& info);
    static std::string DefaultText(const ControlInfo& info);
    static std::string PropLabel(const ControlInfo& info, const PropSpec& spec);
    static std::string PropTip(const ControlInfo& info, const PropSpec& spec);
    static std::string OptionLabel(const ControlInfo& info, const PropSpec& spec, const std::string& option);
    static std::string SlotLabel(const ControlInfo& info, const StyleSlotSpec& slot);
    static std::string SlotTip(const ControlInfo& info, const StyleSlotSpec& slot);
    // The strings of a slot: "control.<Type>.color.<name>" or, for the shared
    // rows, "control.slot.<name>" (+ ".tip").
    static std::string SlotKey(const ControlInfo& info, const StyleSlotSpec& slot);
    static std::string ToolboxGroup(const std::string& group);
    static std::string ToolboxSection(const std::string& section);

    // Every id of the registered controls (or the engine's own) with its
    // English text: the source of the tables and of the reference documents.
    static std::vector<std::pair<std::string, std::string>> AllEnglish(bool builtInOnly = false);
    // An id piece for a value or caption ("DATE & TIME" -> "DATE_&_TIME").
    static std::string IdPart(const std::string& text);
};

} // namespace jwgui
