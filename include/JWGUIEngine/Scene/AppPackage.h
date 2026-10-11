#pragma once

#include "../Core/Export.h"
#include "UIScenePack.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace jwgui {

// A .jwpack ships a project as one file: a ZIP (stored entries, see
// ZipArchive) read in memory, never extracted.
//
//   mimetype        "application/x-jwgui-pack" (first entry)
//   project.jwui    the scene pack (UIScenePack)
//   res/app.jwres   resource policy; ids come from file names as on disk
//   res/...         images, fonts, .jwstyle, ... from the project res folder
//
// SceneApplication (JWGUIPlayer) opens a .jwpack like a .jwui.
//
// A protected package wraps that ZIP, encrypted with the project's key
// (ChaCha20), so neither the files nor their names can be read without it:
//
//   "JWPK" | version 1 | flags (1 = encrypted) | 2 reserved | nonce[12]
//   | check[16] | encrypted ZIP
//
// check is the first 16 bytes of key stream block 0 and tells a wrong key
// apart; the ZIP is encrypted from block 1. The key lives in the author's
// .jwui (<App packageKey>), never in the package. A shipped app carries a
// masked copy (exe trailer, web page) so it can run: that keeps the package
// closed to the editor and to casual tools, not to someone who reverse
// engineers the player.
class JWGUI_API AppPackage {
public:
    static constexpr size_t kKeySize = 32;
    using Key = std::array<uint8_t, kKeySize>;
    enum class OpenResult { Ok, NotPackage, NeedsKey, WrongKey };

    static constexpr const char* kMimeType = "application/x-jwgui-pack";
    static constexpr const char* kProjectEntry = "project.jwui";
    static constexpr const char* kResourceEntry = "res/app.jwres";
    static constexpr const char* kResourcePrefix = "res/";

    // The policy a new project res folder starts with: every image, font and
    // style file, ids from the relative path ("icons/save.png" -> "icons.save").
    static const char* DefaultResourcePolicy();

    // pack + every file under resDir (hidden files and folders skipped). A res
    // folder without app.jwres gets DefaultResourcePolicy. resDir may be empty.
    static bool Build(const UIScenePack& pack, const std::string& resDir,
                      std::vector<uint8_t>& out, std::string* error = nullptr);
    static bool Write(const UIScenePack& pack, const std::string& resDir,
                      const std::string& path, std::string* error = nullptr);

    // Same, then encrypted with key when it is given.
    static bool Write(const UIScenePack& pack, const std::string& resDir, const std::string& path,
                      const Key* key, std::string* error);

    // Path ends with .jwpack (any case).
    static bool IsPackagePath(const std::string& path);

    static Key NewKey();
    static std::string KeyToText(const Key& key);
    static bool KeyFromText(const std::string& text, Key& key);
    // Short, stable id of a key (hex of its check bytes) to show people.
    static std::string KeyFingerprint(const Key& key);

    static std::vector<uint8_t> Protect(const std::vector<uint8_t>& zip, const Key& key);
    static bool IsProtected(const std::vector<uint8_t>& bytes);
    // The ZIP of a package file: as is when plain, decrypted when protected
    // (key needed and checked).
    static OpenResult Unwrap(const std::vector<uint8_t>& bytes, const Key* key, std::vector<uint8_t>& zip);

    // Key as a shipped app stores it: XOR with a fixed pattern (the same call
    // unmasks), so the file does not hold the key itself.
    static Key MaskKey(const Key& key);
    // [masked key]["JWGUIKEY"] at the end of an exe (found within its last
    // 64 KB, so a later code signature does not hide it).
    static bool AppendKeyToExe(const std::string& exePath, const Key& key);
    static bool ReadKeyFromExe(const std::string& exePath, Key& key);

    // A package inside the program itself (Windows): RCDATA resources
    // JWGUIPACK (the package bytes, protected when key is given) and
    // JWGUIKEY (the masked key). A program shipped as its .exe alone carries
    // its screens and resources this way; nothing next to it can be opened.
    // Written after linking (the release build, scripts/package.ps1).
    static bool EmbedInExe(const std::string& exePath, const std::vector<uint8_t>& packageBytes,
                           const Key* key, std::string* error = nullptr);
    // Only the key (RCDATA JWGUIKEY): for a program that opens a protected
    // .jwpack shipped next to it.
    static bool EmbedKeyInExe(const std::string& exePath, const Key& key, std::string* error = nullptr);
    // The running program's own package: true when it has one.
    static bool HasEmbedded();
    // Its ZIP, decrypted with the embedded key when protected (else with
    // key, when given).
    static bool ReadEmbedded(std::vector<uint8_t>& zip, std::string* error = nullptr, const Key* key = nullptr);
    // The key the running program carries: RCDATA JWGUIKEY, else a key
    // appended to the exe (AppendKeyToExe, the exported player).
    static bool ReadOwnKey(Key& key);
    // SceneApplication's project path for the program's own package.
    static constexpr const char* kEmbeddedPath = ":embedded:";
};

} // namespace jwgui
