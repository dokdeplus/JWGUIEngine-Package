#pragma once

#include "../Core/Export.h"
#include "../Layout/LayoutTypes.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UISceneNode {
public:
    std::string type;
    std::string id;
    std::string className;
    std::string text;

    LayoutType layoutType = LayoutType::Absolute;
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 32.0f;

    FlexDirection flexDirection = FlexDirection::Column;
    float flexGrow = 0.0f;
    float flexShrink = 1.0f;

    int gridColumns = 2;
    int gridRows = 2;
    int gridRow = 0;
    int gridCol = 0;
    int gridRowSpan = 1;
    int gridColSpan = 1;

    // Flex / Grid container. Padding is ignored by Absolute containers.
    float padding = 0.0f;
    float gap = 0.0f;
    JustifyContent justifyContent = JustifyContent::Start;
    AlignItems alignItems = AlignItems::Stretch;
    // "200, 1*, 2*, 30%": pixels, Star weight, percent. Empty or missing = 1*.
    std::string gridColumnSizes;
    std::string gridRowSizes;
    // Child of a Flex / Grid container: space around it inside its slot.
    float margin = 0.0f;
    // Child of an Absolute container: AnchorFlags. Anchored edges keep their
    // distance to the parent's edges when the parent changes size.
    unsigned int anchors = AnchorNone;
    // Panel: false leaves only the children visible (a pure layout box).
    bool background = true;
    // Size from the content (text, icon, children) instead of width / height
    // (SizingMode::Content). A Flex / Grid parent may still stretch it.
    bool fitContent = false;

    std::string navigateTo;

    // The control's own style rules, .jwss declarations (StyleSheetMode::
    // Inline): for the control itself, then per part and state, e.g.
    // "border-color: #3574F0; .hours { color: #E24B4A; } .colon { color: @accent; }".
    // Read before the project sheet; empty = the sheet alone. Stored by part and
    // style property (not by editor slot), so it survives editor changes.
    std::string localStyle;

    bool password = false;
    bool readOnly = false;
    bool multiLine = false;
    bool checked = false;
    double value = 0.0;
    double rangeMin = 0.0;
    double rangeMax = 100.0;
    bool indeterminate = false;
    std::string dropDownStyle;
    // Static picture / Button icon: a Project resource id ("icons.save"),
    // found in res on disk or inside a .jwpack. imagePath is the older file
    // path; it is used only when image is empty.
    std::string image;
    std::string imagePath;
    std::string imagePlacement; // Button: Left (default), Right, Top, Bottom, Center
    std::string imageStretch;
    float ratio = 0.5f;
    std::string orientation;
    std::string format;
    std::string date;
    std::string time;
    std::string accessory;

    // Properties of controls described by ControlRegistry (every type without
    // typed fields above), and attributes this build does not know, so they
    // survive a load / save. Name -> text value, as in the .jwui.
    std::map<std::string, std::string> props;

    // From the type's ControlInfo; false for unknown types.
    bool IsContainer() const;
    bool OwnsLayout() const;

    std::vector<std::shared_ptr<UISceneNode>> children;

    UISceneNode* FindById(const std::string& id);
    const UISceneNode* FindById(const std::string& id) const;

    UISceneNode* FindParentOf(const UISceneNode* child);
    bool RemoveChild(UISceneNode* child);

    void CollectIds(std::vector<std::string>& out) const;
};

} // namespace jwgui
