#pragma once
#include "Export.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace jwgui {

// Small, dependency-free primitives for the .jwpack container. Portable C++
// (Win32, wasm, and the platforms to come); no OS crypto API.

// ChaCha20 (RFC 8439): XORs data with the key stream starting at block
// `counter`. Encrypting and decrypting are the same call.
JWGUI_API void ChaCha20Xor(const uint8_t key[32], const uint8_t nonce[12], uint32_t counter,
                           uint8_t* data, size_t size);

// Bytes from the OS random source (std::random_device: RtlGenRandom on
// Windows, crypto.getRandomValues in the browser).
JWGUI_API void RandomBytes(uint8_t* out, size_t size);

JWGUI_API std::string Base64Encode(const uint8_t* data, size_t size);
// False on characters outside the alphabet.
JWGUI_API bool Base64Decode(const std::string& text, std::vector<uint8_t>& out);

} // namespace jwgui
