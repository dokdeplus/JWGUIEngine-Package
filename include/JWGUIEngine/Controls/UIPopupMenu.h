#pragma once

#include "UIPopup.h"
#include "UIListBox.h"
#include <string>
#include <vector>

namespace jwgui {

class JWGUI_API UIPopupMenu : public UIPopup {
public:
    UIPopupMenu(float x, float y, float w, float itemHeight = 30.0f);
    virtual ~UIPopupMenu() = default;

    void AddItem(const std::string& text);
    void ClearItems();

    void ShowPopup(std::shared_ptr<UIRenderView> parent, bool autoClose = true) override;
    void ShowAt(std::shared_ptr<UIRenderView> parent, float x, float y, bool autoClose = true) override;

    // Event fired when an item is selected. Arguments: (index, text)
    std::function<void(int, const std::string&)> OnItemSelected;

    void SetStyle(std::shared_ptr<UIStyle> style) override;
    void SetPadding(float px, float py);

    void BuildRenderItems() override;

protected:
    void OnLayout() override;
    void SyncListGeometry();
    
    std::shared_ptr<UIListBox> m_listBox;
    std::shared_ptr<LayoutNode> m_listBoxNode;
    float m_itemHeight;
    float m_paddingX = 0.0f;
    float m_paddingY = 0.0f;
    int m_itemCount = 0;
    
    std::shared_ptr<UIRenderItem> m_bgItem;
};

} // namespace jwgui
