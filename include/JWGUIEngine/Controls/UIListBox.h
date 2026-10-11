#pragma once
#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIListBox : public UIRenderView {
public:
    UIListBox(float x, float y, float w, float h);
    virtual ~UIListBox() = default;

    bool IsTabStop() const override { return !IsDisabled(); }

    void OnLayout() override;
    void BuildRenderItems() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    void OnMouseLeave() override;

    void AddItem(const std::string& item);
    void RemoveItem(int index);
    void ClearItems();

    void SetSelectedIndex(int index);
    int GetSelectedIndex() const;
    std::string GetSelectedItem() const;

    void SetSelectionMode(SelectionMode mode);
    SelectionMode GetSelectionMode() const { return m_selectionMode; }
    std::vector<int> GetSelectedIndices() const;
    bool IsItemSelected(int index) const;
    void ClearSelection();

    void LockItemHoverUntilMouseMove();
    // Scrolls so the row is fully in view.
    void EnsureVisible(int index);

    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;

    EventHandler<SelectionEventArgs> SelectionChangedEvent;

private:
    static constexpr float kHoverUnlockDistance = 3.0f;

    std::vector<std::string> m_items;
    // The frame (view space) and one text item per row (content space);
    // m_renderItems is rebuilt from them by SyncRenderItems.
    std::shared_ptr<class UIRenderItem> m_bgItem;
    std::vector<std::shared_ptr<class UIRenderText>> m_rowItems;
    std::vector<char> m_selectedFlags;
    int m_selectedIndex = -1;
    int m_anchorIndex = -1;
    int m_hoveredIndex = -1;
    SelectionMode m_selectionMode = SelectionMode::Single;
    float m_itemHeight = 30.0f;
    bool m_hoverLocked = false;
    bool m_hoverLockPosValid = false;
    float m_hoverLockX = 0.0f;
    float m_hoverLockY = 0.0f;

    int HitTestRow(float x, float y) const;
    void UpdateContentSize();
    bool ShouldSuppressRowHover(float x, float y);
    void UpdateHoverState(float x, float y);
    void ApplyRowState(int index, bool animate);
    std::shared_ptr<class UIRenderText> CreateRowItem(size_t index);
    void SyncRenderItems();
    bool IsIndexSelected(int index) const;
    void SetFlagsSize();
    void SelectOnly(int index, bool notify);
    void ToggleIndex(int index, bool notify);
    void SelectRange(int from, int to, bool notify);
    void NotifySelection();
};

} // namespace jwgui
