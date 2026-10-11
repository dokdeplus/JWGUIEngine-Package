#pragma once

#include "Export.h"
#include "UIStrings.h"
#include <string>
#include <vector>

namespace jwgui {

// What an app keeps between runs (settings, recent files, a best score): a
// flat JSON object of texts, read once and written by Save.
//   Windows: %APPDATA%\JWGUI\<group>\<name>.json
//   others:  ~/.config/jwgui/<group>/<name>.json
//   browser: localStorage "jwgui.<group>.<name>"
class JWGUI_API UserStorage {
public:
    // group: a folder for a family of programs ("apps", "games").
    UserStorage(const std::string& group, const std::string& name);

    bool Has(const std::string& key) const;
    std::string Get(const std::string& key, const std::string& fallback = std::string()) const;
    int GetInt(const std::string& key, int fallback = 0) const;
    bool GetBool(const std::string& key, bool fallback = false) const;
    void Set(const std::string& key, const std::string& value);
    void SetInt(const std::string& key, int value);
    void SetBool(const std::string& key, bool value);
    // Texts under one key, one per line (recent files).
    std::vector<std::string> GetList(const std::string& key) const;
    void SetList(const std::string& key, const std::vector<std::string>& values);
    void Remove(const std::string& key);
    void Save() const;

private:
    std::string m_group;
    std::string m_name;
    StringTable m_table;
};

} // namespace jwgui
