#pragma once

#include "Export.h"
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace jwgui {

// One string table: an i18next JSON file (lang/ko.json in a resource pack).
//
//   { "menu.file.open": "열기...", "status.selection": "{{count}}개 선택됨",
//     "files.count_one": "{{count}} file", "files.count_other": "{{count}} files" }
//
// Nested objects are read as dotted keys ({"menu": {"open": ...}} = "menu.open").
// Keys starting with '$' are notes for tools ("$version"), never looked up.
// Values that are not text (numbers, arrays) are kept as written and saved again.
struct JWGUI_API StringTable {
    std::map<std::string, std::string> texts;   // key -> text
    std::map<std::string, std::string> others;  // key -> JSON value as written

    // False on a syntax error; `error` gets "line 3, column 7: ..." and `out`
    // keeps what was read before it.
    static bool Parse(const std::string& json, StringTable& out, std::string* error = nullptr);
    // Flat keys in sorted order, two-space indent, UTF-8 as is.
    std::string ToJson() const;
};

using StringArgs = std::vector<std::pair<std::string, std::string>>;

// Words of the UI in the user's language. UI thread only.
//
//   Tr("menu.file.open", "Open...")                       // id + English default
//   TrFormat("status.selection", "{{count}} selected", { { "count", "3" } })
//
// Tables come from the resource packs (lang/<language>.json, Project packs over
// Host packs), from code (AddTable) and the engine's own words. A lookup tries
// the language chain ("ko-KR" -> "ko-kr", "ko"; the system's: every preferred
// language), then gives the default from the code.
class JWGUI_API UIStrings {
public:
    // --- Language ---
    // A name ("ko", "en-US") or "" for the system's (the OS display language,
    // the browser's languages). Repaints and tells the listeners.
    static void SetLanguage(const std::string& name);
    static const std::string& GetLanguage();
    // Lowercase names tried in turn: { "ko-kr", "ko" }.
    static const std::vector<std::string>& LanguageChain();
    // The system's preferred UI languages, best first ("ko-KR", "en-US").
    static std::vector<std::string> SystemLanguages();
    // "ko-KR" -> { "ko-kr", "ko" }; "zh-Hant-TW" -> { "zh-hant-tw", "zh-hant", "zh" }.
    static std::vector<std::string> ChainFor(const std::string& name);
    // The language's name in itself for language menus ("한국어", "Deutsch");
    // the name as given when unknown.
    static std::string NativeName(const std::string& language);
    // Common UI languages, for language menus: "en", "ko", "ja", ...
    static const std::vector<std::string>& CommonLanguages();

    // --- Lookups ---
    static std::string Get(const std::string& id, const std::string& fallback);
    // False when no table of the chain has it.
    static bool Find(const std::string& id, std::string& out);
    // {{name}} from args; a "count" argument also picks the plural form
    // (id_one, id_other, ... for the language, then id_other, then id).
    static std::string Format(const std::string& id, const std::string& fallback, const StringArgs& args);
    // Fills {{name}} placeholders; unknown ones stay as written.
    static std::string Interpolate(const std::string& text, const StringArgs& args);
    // CLDR plural category of a count: "one", "few", "many" or "other".
    static std::string PluralCategory(const std::string& language, double count);

    // --- Scene texts (.jwui) ---
    // "@login.title" names a string; "@@" is a literal '@'. True for a name,
    // with the id in `id`.
    static bool IsReference(const std::string& text, std::string* id = nullptr);
    // The text a view shows: a named string in the scene language (then the
    // default language, then the id itself), "@@x" as "@x", anything else as is.
    static std::string ResolveText(const std::string& text);
    // The language the app's own texts are written in: scene lookups fall back
    // to it before showing the id. "en" by default.
    static void SetDefaultLanguage(const std::string& name);
    static const std::string& GetDefaultLanguage();
    // Scenes follow the UI language unless set (the editor's canvas shows the
    // project's language while the editor keeps its own). "" = the system's.
    static void SetSceneLanguage(const std::string& name);
    static void FollowUiLanguage();
    static const std::vector<std::string>& SceneLanguageChain();

    // --- Tables ---
    // Tables from code: over the engine's words, under the packs' tables.
    static void AddTable(const std::string& language, const StringTable& table);
    static bool AddTableJson(const std::string& language, const std::string& json, std::string* error = nullptr);
    static void ClearTables();
    // A table file that did not read: "lang/ko.json: line 3, column 7: ...".
    static std::vector<std::string> Problems();

    // Where the system languages and the packs' tables come from
    // (Application sets its host's and its ResourceManager's).
    struct TableText {
        std::string language;  // "ko"
        std::string json;
        std::string origin;    // for Problems ("res/lang/ko.json")
    };
    struct Source {
        std::function<std::vector<std::string>()> systemLanguages;
        // Goes up when the tables may have changed; read on the next lookup.
        std::function<unsigned int()> tablesGeneration;
        // Every table, lowest priority first.
        std::function<void(std::vector<TableText>&)> tables;
    };
    static void SetSource(Source source);
    // The OS / browser languages changed.
    static void SystemChanged();

    // --- Changes ---
    // Goes up whenever a lookup may give something else (language, tables).
    static unsigned int Generation();
    // Called after the language (or the system's) changes. Returns an id for
    // RemoveListener.
    static int AddListener(std::function<void()> listener);
    static void RemoveListener(int id);
};

inline std::string Tr(const std::string& id, const std::string& fallback) {
    return UIStrings::Get(id, fallback);
}

inline std::string TrFormat(const std::string& id, const std::string& fallback, const StringArgs& args) {
    return UIStrings::Format(id, fallback, args);
}

// An id and its English text kept for a later lookup (tables of labels), so
// the string tools still find them:
//   const TrText kSizes[] = { TrKey("size.small", "Small"), TrKey("size.large", "Large") };
//   combo->AddItem(Tr(kSizes[i]));
struct TrText {
    const char* id;
    const char* text;
};

constexpr TrText TrKey(const char* id, const char* text) {
    return { id, text };
}

inline std::string Tr(const TrText& text) {
    return UIStrings::Get(text.id, text.text);
}

} // namespace jwgui
