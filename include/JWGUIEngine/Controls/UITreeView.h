#pragma once
#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

class UITreeView;

class JWGUI_API UITreeViewItem {
public:
    explicit UITreeViewItem(const std::string& text);
    ~UITreeViewItem() = default;

    const std::string& GetText() const { return m_text; }
    void SetText(const std::string& text);

    bool IsExpanded() const { return m_expanded; }
    void SetExpanded(bool expanded);

    void* GetTag() const { return m_tag; }
    void SetTag(void* tag) { m_tag = tag; }

    UITreeViewItem* GetParent() const { return m_parent; }
    UITreeViewItem* AddChild(const std::string& text);
    bool RemoveChild(UITreeViewItem* child);
    const std::vector<std::shared_ptr<UITreeViewItem>>& Children() const { return m_children; }

private:
    friend class UITreeView;

    void AttachTree(UITreeView* tree);

    std::string m_text;
    bool m_expanded = true;
    void* m_tag = nullptr;
    UITreeView* m_tree = nullptr;
    UITreeViewItem* m_parent = nullptr;
    std::vector<std::shared_ptr<UITreeViewItem>> m_children;
};

class JWGUI_API UITreeView : public UIRenderView {
public:
    static constexpr float kItemHeight = 26.0f;
    static constexpr float kIndent = 16.0f;
    static constexpr float kExpanderWidth = 16.0f;

    UITreeView(float x, float y, float w, float h);
    virtual ~UITreeView() = default;

    bool IsTabStop() const override { return !IsDisabled(); }

    void OnLayout() override;
    void BuildRenderItems() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    void RenderContent(IRenderer* renderer, float absX, float absY) override;

    // Drag the selected items onto another row: a line shows Before / After,
    // an outlined row Inside. ItemsDroppedEvent says where (the selection is
    // what moved); the tree itself does not change. canDropInside (optional)
    // says which rows take children; dropping onto a selected row is refused.
    void SetItemDragEnabled(bool enabled) { m_itemDrag = enabled; }
    std::function<bool(UITreeViewItem*)> CanDropInside;
    EventHandler<TreeDropEventArgs> ItemsDroppedEvent;
    void OnMouseLeave() override;
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;

    UITreeViewItem* AddItem(const std::string& text);
    void RemoveItem(UITreeViewItem* item);
    void ClearItems();

    void SetSelectedItem(UITreeViewItem* item);
    // Extended mode: shows these as selected, primary focused. No event.
    void SetSelectedItems(const std::vector<UITreeViewItem*>& items, UITreeViewItem* primary);
    UITreeViewItem* GetSelectedItem() const { return m_selected; }

    void SetSelectionMode(SelectionMode mode);
    SelectionMode GetSelectionMode() const { return m_selectionMode; }
    std::vector<UITreeViewItem*> GetSelectedItems() const;
    bool IsItemSelected(const UITreeViewItem* item) const;
    void ClearSelection();

    void ExpandAll();
    void CollapseAll();

    UITreeViewItem* FindItemByTag(void* tag) const;

    EventHandler<TreeSelectionEventArgs> SelectionChangedEvent;
    EventHandler<TreeExpandEventArgs> ExpandedChangedEvent;
    EventHandler<TreeSelectionEventArgs> ItemActivatedEvent;
    EventHandler<TreeSelectionEventArgs> ContextMenuEvent;

private:
    friend class UITreeViewItem;

    struct VisibleRow {
        UITreeViewItem* item = nullptr;
        int depth = 0;
    };

    std::vector<std::shared_ptr<UITreeViewItem>> m_roots;
    std::vector<VisibleRow> m_visible;
    std::vector<std::shared_ptr<class UIRenderText>> m_rowItems;
    std::shared_ptr<class UIRenderItem> m_bgItem;
    UITreeViewItem* m_selected = nullptr;
    UITreeViewItem* m_hovered = nullptr;
    UITreeViewItem* m_anchor = nullptr;
    UITreeViewItem* m_lastClickItem = nullptr;
    double m_lastClickMs = 0.0;
    SelectionMode m_selectionMode = SelectionMode::Single;
    std::vector<UITreeViewItem*> m_selection;

    bool m_itemDrag = false;
    UITreeViewItem* m_pressItem = nullptr;   // left press on a row
    bool m_pressDeferred = false;            // select only it on release (pressed inside a multi selection)
    bool m_dragging = false;
    float m_pressX = 0.0f;
    float m_pressY = 0.0f;
    UITreeViewItem* m_dropItem = nullptr;
    TreeDropPosition m_dropPos = TreeDropPosition::Inside;
    void UpdateDropTarget(float x, float y);
    bool IsInSelectedSubtree(const UITreeViewItem* item) const;

    void NotifyStructureChanged();
    void NotifyExpandChanged(UITreeViewItem* item, bool expanded);
    void NotifyTextChanged();

    void CollectVisible();
    void CollectVisible(UITreeViewItem* item, int depth);
    void RebuildRows();
    void UpdateContentSize();
    void ApplyRowState(int index, bool animate);
    void ApplyAllRowStates(bool animate);
    UIState RowVisualState(UITreeViewItem* item) const;
    int VisibleIndexOf(const UITreeViewItem* item) const;
    int HitTestRow(float x, float y) const;
    bool HitTestExpander(int rowIndex, float x, float y) const;
    float ContentX(float x) const;
    float ContentY(float y) const;
    bool OwnsItem(const UITreeViewItem* item) const;
    UITreeViewItem* FindItemByTag(const std::vector<std::shared_ptr<UITreeViewItem>>& items, void* tag) const;
    bool ExpandAncestors(UITreeViewItem* item);
    void SetExpandedRecursive(UITreeViewItem* item, bool expanded);
    void ToggleExpanded(UITreeViewItem* item);
    void EnsureVisible(UITreeViewItem* item);
    void PrepareRemove(UITreeViewItem* item);
    void SelectVisibleOffset(int delta);
    void SelectOnly(UITreeViewItem* item, bool notify);
    void ToggleItem(UITreeViewItem* item, bool notify);
    void SelectVisibleRange(UITreeViewItem* from, UITreeViewItem* to, bool notify);
    bool ContainsSelection(const UITreeViewItem* item) const;
    void NotifySelection();
};

} // namespace jwgui
