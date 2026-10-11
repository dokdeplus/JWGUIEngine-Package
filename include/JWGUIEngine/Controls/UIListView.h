#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include <functional>
#include <string>
#include <vector>

namespace jwgui {

// One column of a UIListView.
struct ListViewColumn {
    std::string title;
    float width = 120.0f;
    TextAlignH align = TextAlignH::Left;
    // Takes the room the other columns leave (the name column of a file list).
    bool stretch = false;
};

struct ListViewRowArgs : public EventArgs {
    int row = -1;    // -1: the empty space under the rows
    float x = 0.0f;  // in the window's root view (a menu's place)
    float y = 0.0f;
};

struct ListViewColumnArgs : public EventArgs {
    int column = -1;
};

// A list with columns and a header, for many rows: the owner gives a row
// count and the list asks for the text of the cells it draws (rows out of
// view cost nothing). File lists, tables of records, logs.
//
//   list->SetColumns({ { "Name", 0, TextAlignH::Left, true }, { "Size", 90, TextAlignH::Right } });
//   list->SetCellText([&](int row, int column) { return column == 0 ? files[row].name : files[row].size; });
//   list->SetRowCount(static_cast<int>(files.size()));
//
// One row is the cursor (the band; it moves with the arrow keys and the
// mouse). Rows can also be marked, apart from the cursor, the way two-pane
// file managers do it: Insert or Space marks the cursor's row and steps
// down, Shift+arrows mark while moving, Ctrl+click marks one, Shift+click a
// range, Ctrl+A all, * (keypad) turns the marks around. TargetRows gives the
// marked rows, or the cursor's row when none is marked: what a command acts on.
//
// Keys: Up / Down, Page Up / Down, Home / End, Enter (ActivatedEvent), and
// typing letters goes to the next row that starts with them.
// Mouse: click, double click (ActivatedEvent), right click (ContextMenuEvent),
// a click on a header (HeaderClickedEvent: sort), dragging a header's edge
// changes the column's width.
//
// Style parts: ListView (Background, ForegroundColor, FontFamily, FontSize,
// outline), .header (Background, ForegroundColor), .cursor (Background: the
// band while the list has the focus), .cursor-inactive (Background),
// .marked (ForegroundColor), .separator (ForegroundColor: header lines).
class JWGUI_API UIListView : public UIRenderView {
public:
    using CellText = std::function<std::string(int row, int column)>;
    // Draws a row's icon in the square (x, y, size); the text moves right for it.
    using RowIcon = std::function<void(IRenderer* renderer, int row, float x, float y, float size)>;
    // A row's text color (ARGB); 0 = the style's.
    using RowColor = std::function<uint32_t(int row)>;

    UIListView(float x, float y, float w, float h);
    ~UIListView() override = default;

    void SetColumns(std::vector<ListViewColumn> columns);
    const std::vector<ListViewColumn>& GetColumns() const { return m_columns; }
    void SetCellText(CellText fn);
    void SetRowIcon(RowIcon fn);
    void SetRowColor(RowColor fn);
    // The rows changed: the marks are forgotten, the cursor stays in range.
    void SetRowCount(int count);
    int RowCount() const { return m_rowCount; }
    void SetRowHeight(float height);
    float GetRowHeight() const { return m_rowHeight; }
    void SetShowHeader(bool show);
    // An arrow on a header (the sorted column); -1 = none.
    void SetSortIndicator(int column, bool ascending);

    int GetCursor() const { return m_cursorRow; }
    // Moves the cursor (clamped) and scrolls it into view.
    void SetCursor(int row);
    bool IsMarked(int row) const;
    void SetMarked(int row, bool marked);
    void ClearMarks();
    void MarkAll();
    void InvertMarks();
    int MarkedCount() const { return m_markedCount; }
    std::vector<int> MarkedRows() const;
    // The marked rows, or the cursor's row when none is marked.
    std::vector<int> TargetRows() const;
    void EnsureVisible(int row);

    // Enter or a double click on a row.
    EventHandler<ListViewRowArgs> ActivatedEvent;
    EventHandler<EventArgs> CursorChangedEvent;
    EventHandler<EventArgs> MarksChangedEvent;
    EventHandler<ListViewColumnArgs> HeaderClickedEvent;
    EventHandler<ListViewRowArgs> ContextMenuEvent;
    // The list took the focus (a two-pane app notes the active pane).
    EventHandler<EventArgs> FocusedEvent;

    void RenderContent(IRenderer* renderer, float absX, float absY) override;
    void UpdateCachedStyles() override;
    void OnThemeChanged() override;
    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    void OnGlobalMouseDown(float x, float y) override;
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;
    bool OnChar(unsigned int charCode) override;
    void OnFocusChanged(bool focused) override;
    bool IsTabStop() const override { return !IsDisabled(); }

private:
    float HeaderHeight() const { return m_showHeader ? m_headerHeight : 0.0f; }
    float ViewWidth() const;
    float VisibleRowsHeight() const;
    int VisibleRows() const;
    // The columns' left edges and widths as drawn (the stretch column filled in).
    void ColumnLayout(std::vector<float>& lefts, std::vector<float>& widths) const;
    int RowAt(float y) const;
    int ColumnAt(float x) const;
    // The column whose right edge is at x (a header drag), or -1.
    int DividerAt(float x) const;
    void UpdateContent();
    void ClampScroll();
    void MoveCursor(int row, bool notify = true);
    void ToggleMark(int row);
    void MarkRange(int from, int to);
    uint32_t PartColor(const char* part, UIStylePropertyID prop, uint32_t fallback) const;
    uint32_t PartFill(const char* part, uint32_t fallback) const;

    std::vector<ListViewColumn> m_columns;
    CellText m_cellText;
    RowIcon m_rowIcon;
    RowColor m_rowColor;
    int m_rowCount = 0;
    float m_rowHeight = 24.0f;
    float m_headerHeight = 26.0f;
    bool m_showHeader = true;
    int m_sortColumn = -1;
    bool m_sortAscending = true;

    int m_cursorRow = 0;
    std::vector<char> m_marked;
    int m_markedCount = 0;

    // Mouse.
    bool m_pressed = false;
    int m_dragDivider = -1;
    float m_dragStartX = 0.0f;
    float m_dragStartWidth = 0.0f;
    int m_headerDown = -1;
    double m_lastClickMs = -1.0e9;
    int m_lastClickRow = -1;

    // Typing to find a row.
    std::string m_typed;
    double m_typedAtMs = 0.0;

    // From the style.
    std::string m_fontFamily = UIStyle::DefaultFontFamily();
    float m_fontSize = 13.0f;
    uint32_t m_fg = 0xFFE0E0E0;
    UIBackground m_background;
    uint32_t m_outlineColor = 0;
    float m_outlineThickness = 0.0f;
};

} // namespace jwgui
