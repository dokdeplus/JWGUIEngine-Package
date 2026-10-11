#pragma once
#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include "UIPopup.h"
#include <string>
#include <vector>

namespace jwgui {

enum class ComboDropDownStyle {
    DropList,
    DropDown
};

class JWGUI_API UIComboBox : public UIRenderView {
public:
    UIComboBox(float x, float y, float w, float h);
    virtual ~UIComboBox() = default;

    void SetFocus() override;
    bool IsTabStop() const override { return !IsDisabled(); }

    void OnLayout() override;
    void BuildRenderItems() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool IsPressed() const override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    void OnGlobalMouseDown(float x, float y) override;

    void AddItem(const std::string& item);
    void RemoveItem(int index);
    void ClearItems();

    void SetSelectedIndex(int index);
    int GetSelectedIndex() const;
    std::string GetSelectedItem() const;

    void SetDropDownStyle(ComboDropDownStyle style);
    ComboDropDownStyle GetDropDownStyle() const;

    void SetText(const std::string& text);
    std::string GetText() const;
    // The hint in the edit while it is empty (DropDown style).
    void SetPlaceholder(const std::string& text);
    // DropDown style: typing narrows the list to the items that contain the
    // text (letter case aside) and opens it; the arrow still shows every item.
    void SetAutoFilter(bool on) { m_autoFilter = on; }
    bool GetAutoFilter() const { return m_autoFilter; }

    void SetStyle(std::shared_ptr<UIStyle> style) override;

    EventHandler<SelectionEventArgs> SelectionChangedEvent;
    // DropDown style: the text the user committed in the edit (focus leaving),
    // one of the items or not.
    EventHandler<std::string> TextChangedEvent;

private:
    std::vector<std::string> m_items;
    int m_selectedIndex = -1;
    ComboDropDownStyle m_dropDownStyle = ComboDropDownStyle::DropList;
    
    bool m_isDroppedDown = false;
    
    std::shared_ptr<UIPopup> m_popupView;
    std::weak_ptr<class UIListBox> m_dropList;  // the list inside the open popup
    std::vector<int> m_shownItems;              // item indexes the open list shows
    bool m_fillingList = false;                 // the list's own selection events are ours then
    bool m_autoFilter = false;
    std::string m_placeholder;
    std::shared_ptr<class UIEditBox> m_edit;

    void ToggleDropdown();
    // Opens the list with the items matching `filter` ("" = every item).
    void OpenDropdown(const std::string& filter);
    void CloseDropdown();
    std::vector<int> MatchingItems(const std::string& filter) const;
    void FillDropList(class UIListBox& list);
    void OnEditTextEdited(const std::string& text);
    void ApplyChromeState(bool pressed, bool hovered);
    void EnsureEdit();
    void RemoveEdit();
    void ApplyEditStyle();
    void SyncEditFromSelection();
    void CommitEditText(bool byUser);
    bool HitArrow(float x, float y) const;

    std::shared_ptr<UIRenderItem> m_bgItem;
    std::shared_ptr<class UIRenderText> m_textItem;
    std::shared_ptr<UIRenderItem> m_arrowItem;
};

} // namespace jwgui
