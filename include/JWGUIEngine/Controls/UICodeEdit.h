#pragma once

#include "../Core/UIRenderView.h"
#include "../Core/Event.h"
#include "../Core/TimerQueue.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// <windows.h> (commdlg.h) makes ReplaceText a macro for ReplaceTextW; an app
// that includes it before this header would look for that name in the DLL.
#ifdef ReplaceText
#undef ReplaceText
#endif

namespace jwgui {

// A place in a UICodeEdit: 0-based line, and byte offset in that line (UTF-8).
struct CodePosition {
    int line = 0;
    int column = 0;

    bool operator==(const CodePosition& o) const { return line == o.line && column == o.column; }
    bool operator!=(const CodePosition& o) const { return !(*this == o); }
    bool operator<(const CodePosition& o) const { return line < o.line || (line == o.line && column < o.column); }
};

// A colored piece of one line: bytes [begin, end) are drawn with the
// ForegroundColor of the part (CodeEdit.<part>, e.g. CodeEdit.comment).
struct CodeSpan {
    int begin = 0;
    int end = 0;
    std::string part;
};

// Colors one line. `state` is what the line before ended in (0 before the
// first line), e.g. "inside a comment"; returns the state this line ends in.
using CodeHighlighter = std::function<int(const std::string& line, int state, std::vector<CodeSpan>& spans)>;

// A problem shown in the text: a mark in the gutter and a wavy underline.
struct CodeMarker {
    enum class Severity { Error, Warning, Info };
    Severity severity = Severity::Error;
    int line = 0;
    int column = 0;  // bytes
    int length = 0;  // bytes; 0 = the word at column (at least one character)
    std::string message;
};

// A small color square drawn in the text before `column` (the text moves
// right to make room), e.g. before each color value of a style sheet.
// A left or right click fires SwatchClickedEvent with its index.
struct CodeSwatch {
    int line = 0;
    int column = 0;
    uint32_t color = 0;  // ARGB
};

// How FindNext / ReplaceAll / the search highlight match text. Matches stay
// inside one line.
struct CodeFindOptions {
    bool matchCase = false;  // off: ASCII letters match without regard to case
    bool wholeWord = false;  // the match may not touch a letter, digit or '_'
};

struct CodeSwatchEventArgs : public EventArgs {
    int index = -1;    // in GetSwatches()
    MouseButton button = MouseButton::Left;
    UIRect screenRect; // the square, in GetScreenRect coordinates (a popup anchor)
};

// Multi-line code editing drawn by the engine (the same on every platform and
// in the browser): a fixed-width font, line numbers, syntax coloring
// (SetHighlighter), problem marks (SetMarkers), color swatches (SetSwatches),
// its own undo / redo, Tab and automatic indent, find and replace, and word
// wrap (SetWordWrap). Undo keeps the changes, not copies of the text, and
// only the lines an edit touches are colored again, so large files stay
// workable.
//
// Keys: arrows, Home (first non-blank, then column 0), End, Page Up / Down,
// Ctrl+Left / Right (words), Ctrl+Home / End, Shift to select, Ctrl+A,
// Ctrl+C / X / V (the whole line when nothing is selected), Ctrl+Z,
// Ctrl+Y / Ctrl+Shift+Z, Tab / Shift+Tab (indent, or the selected lines),
// Enter keeps the indent and indents after '{'; '}' on a blank line outdents.
// Mouse: click, drag, Shift+click, double-click selects a word, triple-click
// the line; the wheel scrolls (Shift: sideways).
//
// Style parts: CodeEdit (Background, ForegroundColor, FontFamily, FontSize,
// SelectionBackgroundColor, CaretColor, OutlineColor / OutlineThickness),
// .gutter (Background; ForegroundColor = line numbers), .gutter-current
// (ForegroundColor: the caret's line number), .current-line (Background),
// .error / .warning / .info (ForegroundColor: marks), .find-match
// (Background: SetSearchHighlight), and one part per highlighter span name
// (ForegroundColor).
class JWGUI_API UICodeEdit : public UIRenderView {
public:
    UICodeEdit(float x, float y, float w, float h);
    ~UICodeEdit() override;

    // Replaces the text and forgets undo (a file was opened). CR LF and CR
    // become LF.
    void SetText(const std::string& text);
    // Replaces the text as one undoable change and keeps the caret near where
    // it was (the text was changed elsewhere). No-op when equal.
    void ReplaceText(const std::string& text);
    std::string GetText() const;
    int LineCount() const { return static_cast<int>(m_lines.size()); }
    const std::string& Line(int index) const;

    void SetReadOnly(bool readOnly);
    bool IsReadOnly() const { return m_readOnly; }
    // Dim text shown while the document is empty (a hint or an example).
    void SetPlaceholder(const std::string& text);
    // Spaces per indent level (Tab, Enter after '{'); tabs in the text are
    // shown to the next multiple of it. Default 4.
    void SetTabSize(int spaces);
    int GetTabSize() const { return m_tabSize; }
    // Long lines continue on the next row instead of scrolling sideways
    // (breaks after spaces, anywhere in CJK text). Line numbers stay those of
    // the text's lines.
    void SetWordWrap(bool wrap);
    bool IsWordWrap() const { return m_wordWrap; }
    // The text size in px, over the style's FontSize; 0 = the style's again.
    void SetFontSize(float size);
    float GetFontSize() const { return m_fontSize; }

    void SetHighlighter(CodeHighlighter highlighter);
    void SetMarkers(std::vector<CodeMarker> markers);
    const std::vector<CodeMarker>& GetMarkers() const { return m_markers; }
    void SetSwatches(std::vector<CodeSwatch> swatches);
    const std::vector<CodeSwatch>& GetSwatches() const { return m_swatches; }

    CodePosition GetCaret() const { return m_caret; }
    CodePosition GetAnchor() const { return m_anchor; }
    // Moves the caret (select = keep the anchor) and scrolls to it.
    void SetCaret(CodePosition pos, bool select = false);
    void Select(CodePosition anchor, CodePosition caret);
    bool HasSelection() const { return m_anchor != m_caret; }
    std::string GetSelectedText() const;
    // Replaces the selection, or inserts at the caret, as one undo step.
    void InsertText(const std::string& text);
    // The caret's column in characters (tabs count as one), 0-based.
    int CaretCharColumn() const;
    // Shows the line in the middle of the view (when it is not on screen).
    void RevealLine(int line);

    bool CanUndo() const { return !m_undo.empty(); }
    bool CanRedo() const { return !m_redo.empty(); }
    void Undo();
    void Redo();
    // The text differs from what it was at the last MarkSaved (or SetText);
    // undoing back to that point makes it unmodified again.
    bool IsModified() const { return m_stateId != m_savedStateId; }
    void MarkSaved();

    // Selects the next (or previous) match after the selection and scrolls to
    // it. wrapAround continues from the other end of the text. False = none.
    bool FindNext(const std::string& text, const CodeFindOptions& options = {}, bool backwards = false,
                  bool wrapAround = true);
    // True when the selection is exactly one match of `text` (Replace acts on it).
    bool SelectionMatches(const std::string& text, const CodeFindOptions& options = {}) const;
    // Replaces every match as one undo step; returns how many.
    int ReplaceAll(const std::string& text, const std::string& replacement, const CodeFindOptions& options = {});
    int CountMatches(const std::string& text, const CodeFindOptions& options = {}) const;
    // Marks every match with the .find-match background while a search is
    // open; empty text = none.
    void SetSearchHighlight(const std::string& text, const CodeFindOptions& options = {});

    // After each change of the text by the user (typing, paste, undo, ...)
    // and by InsertText / ReplaceText; not by SetText. The whole text is put
    // together for it only when someone listens: an editor for large files
    // listens to EditedEvent instead.
    EventHandler<std::string> TextEditedEvent;
    EventHandler<EventArgs> EditedEvent;
    // Focus left after the text changed (commit), as in UIEditBox.
    EventHandler<std::string> TextChangedEvent;
    // The caret or the selection moved.
    EventHandler<EventArgs> CaretMovedEvent;
    EventHandler<CodeSwatchEventArgs> SwatchClickedEvent;

    void RenderContent(IRenderer* renderer, float absX, float absY) override;
    void OnLayout() override;
    void UpdateCachedStyles() override;
    void OnThemeChanged() override;

    bool OnMouseDown(float x, float y, MouseButton button) override;
    bool OnMouseMove(float x, float y) override;
    bool OnMouseUp(float x, float y, MouseButton button) override;
    bool OnMouseWheel(float delta, float x, float y, const KeyModifiers& modifiers) override;
    void OnGlobalMouseDown(float x, float y) override;
    bool OnKeyDown(KeyCode key, const KeyModifiers& modifiers) override;
    bool OnChar(unsigned int charCode) override;
    bool OnImeComposition(const std::string& compStr) override;

    void OnFocusChanged(bool focused) override;
    bool IsTabStop() const override { return !IsDisabled(); }
    bool AcceptsTab() const override { return !m_readOnly && !IsDisabled(); }

private:
    enum class EditKind { None, Typing, Backspace, Delete, Other };
    // One replacement: `removed` at `at` became `inserted`.
    struct EditOp {
        CodePosition at;
        std::string removed;
        std::string inserted;
    };
    // What one Undo takes back: its replacements in order, and the caret
    // and document state before and after.
    struct UndoStep {
        std::vector<EditOp> ops;
        CodePosition caretBefore;
        CodePosition anchorBefore;
        CodePosition caretAfter;
        CodePosition anchorAfter;
        uint64_t stateBefore = 0;
        uint64_t stateAfter = 0;
    };
    struct LineLayout {
        bool valid = false;
        std::vector<float> xs;  // x of each byte offset (0 .. size), swatches included
        float width = 0.0f;     // the widest row
        std::vector<int> rows;  // byte offset each row starts at; {0} unless wrapped
    };

    std::vector<std::string> m_lines{ std::string() };
    CodePosition m_caret;
    CodePosition m_anchor;
    float m_preferredX = -1.0f;  // Up / Down keep this x
    bool m_readOnly = false;
    int m_tabSize = 4;
    bool m_wordWrap = false;
    float m_wrapWidth = 0.0f;       // 0 = not wrapped
    float m_fontSizeOverride = 0.0f;
    std::string m_imeText;
    std::vector<std::string> m_placeholder;
    uint64_t m_editSerial = 0;   // changes so far
    uint64_t m_focusSerial = 0;  // ... when focus came (TextChangedEvent)

    CodeHighlighter m_highlighter;
    std::vector<std::vector<CodeSpan>> m_spans;
    std::vector<int> m_lineStates;  // the highlighter's state after each line; -1 = not known
    bool m_spansDirty = true;       // color every line again
    int m_colorFrom = -1;           // or only these lines (and on while the state differs)
    int m_colorTo = -1;
    std::string m_searchText;
    CodeFindOptions m_searchOptions;
    std::vector<CodeMarker> m_markers;
    std::vector<CodeSwatch> m_swatches;

    std::vector<UndoStep> m_undo;
    std::vector<UndoStep> m_redo;
    EditKind m_lastEdit = EditKind::None;
    bool m_recording = false;    // Replace adds to m_undo.back()
    uint64_t m_stateId = 0;      // names the text's current content
    uint64_t m_savedStateId = 0;
    uint64_t m_nextStateId = 1;
    // Rows (word wrap): the first row of each line, and the count after the last.
    std::vector<int> m_firstRow;
    bool m_rowsDirty = true;

    // Layout, from the style and font.
    std::string m_fontFamily = "Consolas";
    float m_fontSize = 13.0f;
    float m_lineHeight = 18.0f;
    float m_digitWidth = 8.0f;
    float m_spaceWidth = 7.0f;
    uint32_t m_fg = 0xFFD4D4D4;
    uint32_t m_selectionColor = 0x66264F78;
    uint32_t m_caretColor = 0xFFFFFFFF;
    UIBackground m_background;
    uint32_t m_outlineColor = 0;
    float m_outlineThickness = 0.0f;
    std::vector<LineLayout> m_layout;
    std::unordered_map<unsigned int, float> m_advances;
    bool m_metricsValid = false;

    // Mouse.
    bool m_selecting = false;
    int m_clickCount = 0;
    double m_lastClickMs = 0.0;
    float m_lastClickX = 0.0f;
    float m_lastClickY = 0.0f;
    CodePosition m_wordStart;  // double / triple click: the first word or line
    CodePosition m_wordEnd;

    // Caret blink (an app timer, so it also runs in floating windows).
    TimerId m_blinkTimer = 0;
    bool m_caretOn = true;
    bool m_scrollToCaret = false;

    // Text model.
    std::string TextRange(CodePosition a, CodePosition b) const;
    CodePosition Clamp(CodePosition p) const;
    CodePosition SelectionStart() const;
    CodePosition SelectionEnd() const;
    // Replaces [a, b) with text; returns the position after it.
    CodePosition Replace(CodePosition a, CodePosition b, const std::string& text);
    void DeleteSelection();
    void InvalidateLines(int from, int to);

    // Undo.
    void BeginEdit(EditKind kind);
    void EndEdit();
    void AfterUndoRedo(CodePosition caret, CodePosition anchor, uint64_t state);
    // Lines a..b became `count` lines: the per-line caches follow.
    void LinesReplaced(int a, int b, int count);

    // Search.
    int FindInLine(const std::string& line, const std::string& text, const CodeFindOptions& options, int from) const;
    int FindLastInLine(const std::string& line, const std::string& text, const CodeFindOptions& options, int before) const;

    // Rows.
    void EnsureRows();
    int TotalRows();
    int FirstRow(int line);
    int RowInLine(int line, int column);
    int RowOf(CodePosition p);
    // The line a row belongs to, and which of its rows.
    int LineOfRow(int row, int& rowInLine);
    void RowRange(int line, int rowInLine, int& begin, int& end);
    float RowTop(int row) const;
    void UpdateWrapWidth();

    // Moves.
    CodePosition Left(CodePosition p) const;
    CodePosition Right(CodePosition p) const;
    CodePosition WordLeft(CodePosition p) const;
    CodePosition WordRight(CodePosition p) const;
    void WordAt(CodePosition p, CodePosition& start, CodePosition& end) const;
    void MoveTo(CodePosition p, bool select, bool keepPreferredX = false);

    // Editing helpers.
    std::string LeadingSpace(int line) const;
    void IndentLines(bool outdent);
    void NewLine();
    void TypeText(const std::string& text);
    void CopySelection(bool cut);
    void Paste();

    // Layout.
    void EnsureMetrics();
    float Advance(unsigned int codepoint);
    const LineLayout& Layout(int line);
    // x in the row the position is on (from the row's start).
    float XOf(CodePosition p);
    int ColumnAtX(int line, int rowInLine, float x);
    CodePosition PositionAt(float x, float y);
    float GutterWidth() const;
    float TextLeft() const;
    float VisibleTextWidth() const;
    float VisibleHeight() const;
    int VisibleLineCount() const;
    void UpdateContentSize();
    void ClampScroll();
    void ScrollToCaretNow();
    void Highlight();
    uint32_t PartColor(const std::string& part, UIStylePropertyID prop, uint32_t fallback) const;
    bool PartBackground(const std::string& part, uint32_t& color) const;
    int SwatchAt(float x, float y);

    void RestartBlink();
    void StopBlink();
    void Changed();
    void CaretChanged();
};

} // namespace jwgui
