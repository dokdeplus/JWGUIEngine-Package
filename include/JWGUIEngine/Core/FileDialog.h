#pragma once

#include <functional>
#include <string>
#include <vector>

namespace jwgui {

// One line of a file dialog's type list: "Images" + "*.png;*.jpg".
struct FileDialogFilter {
    std::string name;      // shown; the host adds nothing to it
    std::string patterns;  // "*.png;*.jpg", or "*.*"
};

enum class FileDialogMode {
    Open,          // one existing file
    OpenMultiple,  // one or more existing files
    Save,          // a file to write (asks before replacing one)
    Folder         // one existing folder
};

// The OS file dialog (Windows), or the browser's file picker.
//
//   FileDialogOptions options;
//   options.filters = { { Tr("app.images", "Images"), "*.png;*.jpg" } };
//   Application::Get()->GetHost()->ShowFileDialog(options, [](const std::vector<std::string>& paths) {
//       if (!paths.empty()) Open(paths[0]);
//   });
//
// `done` gets UTF-8 paths, or nothing when the user cancelled. On Windows it
// runs before ShowFileDialog returns; in the browser later (the picker is
// asynchronous), so code after the call must not expect the answer.
//
// In the browser a picked file is copied into the page's own file system and
// the path names that copy; Save gives a path there too, and
// IPlatformHost::DeliverSavedFile hands the written file to the user as a
// download. Folder is not available there.
struct FileDialogOptions {
    FileDialogMode mode = FileDialogMode::Open;
    std::string title;                      // empty = the system's
    std::vector<FileDialogFilter> filters;  // empty = every file
    int filterIndex = 0;                    // the filter shown first
    std::string defaultExtension;           // Save: added when the name has none ("png")
    std::string fileName;                   // Save: the name offered
    std::string initialDir;                 // empty = where the system left off
};

using FileDialogDone = std::function<void(const std::vector<std::string>& paths)>;

} // namespace jwgui
