#pragma once

#include <string>
#include <cwchar>

namespace jwgui {

inline void AppendUtf8(std::string& out, unsigned int cp) {
    // Lone UTF-16 surrogates are not characters; encoding them makes invalid UTF-8.
    if (cp > 0x10FFFFu || (cp >= 0xD800u && cp <= 0xDFFFu)) {
        return;
    }
    if (cp < 0x80u) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800u) {
        out.push_back(static_cast<char>(0xC0u | (cp >> 6)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    } else if (cp < 0x10000u) {
        out.push_back(static_cast<char>(0xE0u | (cp >> 12)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    } else {
        out.push_back(static_cast<char>(0xF0u | (cp >> 18)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 12) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | ((cp >> 6) & 0x3Fu)));
        out.push_back(static_cast<char>(0x80u | (cp & 0x3Fu)));
    }
}

inline std::string Utf8FromCodepoint(unsigned int cp) {
    std::string out;
    AppendUtf8(out, cp);
    return out;
}

inline unsigned int DecodeUtf8(const char*& p) {
    if (!p || !*p) {
        return 0;
    }
    const unsigned char c = static_cast<unsigned char>(*p++);
    unsigned int cp = 0;
    int extra = 0;
    if (c < 0x80u) {
        return c;
    }
    if (c < 0xC0u) {
        return 0;
    }
    if (c < 0xE0u) {
        cp = c & 0x1Fu;
        extra = 1;
    } else if (c < 0xF0u) {
        cp = c & 0x0Fu;
        extra = 2;
    } else {
        cp = c & 0x07u;
        extra = 3;
    }
    for (int i = 0; i < extra; ++i) {
        const unsigned char b = static_cast<unsigned char>(*p);
        if ((b & 0xC0u) != 0x80u) {
            return 0;
        }
        ++p;
        cp = (cp << 6) | (b & 0x3Fu);
    }
    return cp;
}

inline std::string Utf8FromWide(const wchar_t* text) {
    std::string out;
    if (!text) {
        return out;
    }
    while (*text) {
        unsigned int cp = static_cast<unsigned int>(*text++);
#if WCHAR_MAX <= 0xFFFF
        if (cp >= 0xD800u && cp <= 0xDBFFu && *text) {
            unsigned int lo = static_cast<unsigned int>(*text);
            if (lo >= 0xDC00u && lo <= 0xDFFFu) {
                ++text;
                cp = 0x10000u + ((cp - 0xD800u) << 10) + (lo - 0xDC00u);
            }
        }
#endif
        AppendUtf8(out, cp);
    }
    return out;
}

inline int Utf8CodepointCount(const std::string& s) {
    int n = 0;
    const char* p = s.c_str();
    while (*p) {
        DecodeUtf8(p);
        ++n;
    }
    return n;
}

inline int Utf8ByteIndexFromCodepoint(const std::string& s, int cpIndex) {
    const char* start = s.c_str();
    const char* p = start;
    int i = 0;
    while (*p && i < cpIndex) {
        DecodeUtf8(p);
        ++i;
    }
    return static_cast<int>(p - start);
}

inline int Utf8CodepointFromByteIndex(const std::string& s, int byteIndex) {
    if (byteIndex <= 0) return 0;
    const char* start = s.c_str();
    const char* p = start;
    const char* end = start + (byteIndex < static_cast<int>(s.size()) ? byteIndex : static_cast<int>(s.size()));
    int i = 0;
    while (p < end && *p) {
        DecodeUtf8(p);
        ++i;
    }
    return i;
}

} // namespace jwgui
