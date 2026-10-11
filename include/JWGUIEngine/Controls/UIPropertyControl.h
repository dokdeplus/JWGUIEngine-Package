#pragma once

#include "UIPanel.h"
#include "UIEditBox.h"
#include "UIComboBox.h"
#include "UICheckBox.h"
#include "UISpinBox.h"
#include "UIStatic.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace jwgui {

class JWGUI_API UIPropertyControl : public UIPanel {
public:
    UIPropertyControl(float x, float y, float w, float h);
    virtual ~UIPropertyControl() = default;

    void OnLayout() override;
    void SetStyle(std::shared_ptr<UIStyle> style) override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    void OnMouseLeave() override;

    void SetTitle(const std::string& text);

    std::shared_ptr<UIEditBox> AddEdit(const std::string& label, const std::string& description = "");
    std::shared_ptr<UISpinBox> AddSpin(const std::string& label, const std::string& description = "");
    std::shared_ptr<UIComboBox> AddCombo(const std::string& label, const std::string& description = "");
    std::shared_ptr<UICheckBox> AddCheck(const std::string& label, const std::string& description = "");
    std::shared_ptr<UIComboBox> AddBool(const std::string& label, const std::string& description = "");
    // Any view as the field of a labeled row (e.g. a picker made of a few views).
    void AddField(const std::string& label, std::shared_ptr<UIRenderView> field, const std::string& description = "");
    void Add(std::shared_ptr<UIRenderView> child, float height = 32.0f, const std::string& description = "");
    // The same, placed before the row of `before` (a field or an Add child);
    // at the end when `before` is not a row here.
    void InsertField(UIRenderView* before, const std::string& label, std::shared_ptr<UIRenderView> field,
                     const std::string& description = "");
    void Insert(UIRenderView* before, std::shared_ptr<UIRenderView> child, float height = 32.0f,
                const std::string& description = "");
    // Takes the row of `field` (a field or an Add child) out of the grid.
    void Remove(UIRenderView* field);

    void Show(UIRenderView* field, bool visible);
    // Rows reused for different things (the Style pane's color rows).
    void SetFieldLabel(UIRenderView* field, const std::string& label);
    void SetFieldDescription(UIRenderView* field, const std::string& description);
    // Marks a row whose value differs between several edited objects: the
    // label gets " *". The value shown is one of them; a change sets all.
    void SetFieldMarked(UIRenderView* field, bool marked);
    void Select(UIRenderView* field);
    void ClearSelect();
    void ResetBodyScroll();

    void SetLabelWidth(float width);
    float GetLabelWidth() const { return m_labelWidth; }

private:
    struct Slot {
        std::shared_ptr<UIRenderView> row;
        std::shared_ptr<LayoutNode> layout;
        std::shared_ptr<LayoutNode> labelLayout;
        float height = 30.0f;
        float gap = 6.0f;
        std::string description;
        std::shared_ptr<UIStatic> label;
        std::string labelText;
    };

    std::shared_ptr<UIStatic> m_title;
    std::shared_ptr<UIPanel> m_body;
    std::shared_ptr<UIPanel> m_descSeparator;
    std::shared_ptr<UIStatic> m_description;
    std::unordered_map<UIRenderView*, Slot> m_slots;
    UIRenderView* m_selectedField = nullptr;

    float m_labelWidth = 88.0f;
    bool m_splitterDragging = false;
    bool m_splitterCursor = false;
    float m_splitterDragStartX = 0.0f;
    float m_splitterDragStartWidth = 88.0f;

    void Attach(const std::string& label, std::shared_ptr<UIRenderView> field, const std::string& description,
                UIRenderView* before = nullptr);
    std::shared_ptr<LayoutNode> AddRow(std::shared_ptr<UIRenderView> row, UIRenderView* before);
    void ApplyLabelWidths();
    void UpdateBodyContentSize();
    void ApplyChromeStyle();
    void HighlightRow(UIRenderView* field, bool on);
    bool HitSplitter(float x, float y) const;
    UIRenderView* HitField(float x, float y) const;
    void SetSplitCursor(bool on);
    static float ClampLabelWidth(float width, float bodyWidth);
};

} // namespace jwgui
