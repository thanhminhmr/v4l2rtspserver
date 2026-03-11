# C++ Code Quality Improvement Plan

## Scope
- `main.cpp`
- `src/*`
- `inc/*`

## Issues Found & Required Changes

### 1. mem* functions (C-style memory operations)
Replace with C++ alternatives: `std::copy`, `std::copy_n`, `std::fill`, `std::memmove` (if needed)

| File | Line | Issue |
|------|------|-------|
| src/V4L2DeviceSource.cpp | 143 | memcpy |
| src/HTTPServer.cpp | 47 | memcpy |
| inc/MJPEGVideoSource.h | 59 | memset |
| src/MJPEGVideoSource.cpp | 60, 94 | memcpy, memmove |
| src/TSServerMediaSubsession.cpp | 68 | memcpy |
| inc/AddH26xMarkerFilter.h | 50-51 | memcpy |

### 2. C-style casts
Replace `(unsigned char*)`, `(char*)` with `reinterpret_cast` or better, use `std::uint8_t`/`std::byte`

| File | Lines | Issue |
|------|-------|-------|
| src/V4L2DeviceSource.cpp | 209, 211, 242-245 | `unsigned char*` with C casts |
| src/H264_V4l2DeviceSource.cpp | 48-49, 82 | `unsigned char*` casts from `std::string::c_str()` |
| src/H265_V4l2DeviceSource.cpp | 50-52, 85 | `unsigned char*` casts from `std::string::c_str()` |

### 3. Raw pointers and manual memory management
Consider using `std::vector<std::uint8_t>` or smart pointers instead of `new[]`/`delete[]`

| File | Lines | Issue |
|------|-------|-------|
| src/V4L2DeviceSource.cpp | 174, 177 | `delete[] buffer` |
| src/H264_V4l2DeviceSource.cpp | 79-80 | `delete[] sps_base64`, `delete[] pps_base64` |
| src/H265_V4l2DeviceSource.cpp | 81-83 | `delete[] vps_base64`, etc. |
| src/MemoryBufferSink.cpp | 22, 40 | `delete[] m_buffer` |

### 4. Type usage
- Use `std::uint8_t` instead of `unsigned char`
- Use `std::size_t` for sizes (already mostly done)
- Consider `std::int32_t`, `std::int16_t` for signed integers

### 5. Additional C++ Modernization Opportunities
- Use `std::make_unique` / `std::make_shared` instead of raw `new`
- Use `std::array` for fixed-size buffers
- Use `std::string_view` instead of `(const char*, size_t)` pairs
- Replace `std::pair< unsigned char*, size_t>` with `std::span<std::uint8_t>` (C++20) or custom struct

## Build Verification
Run `make` in `cmake-build-debug/` folder to verify changes compile correctly.

## Version Control
- **Every change set must be committed separately** for human review
- Each commit should contain logically related changes (e.g., "Replace memcpy with std::copy in V4L2DeviceSource.cpp")
- Do not batch multiple files or unrelated changes in a single commit

## Priority
1. **High**: Replace mem* functions with STL algorithms
2. **Medium**: Replace C-style casts with reinterpret_cast or better types
3. **Medium**: Replace manual new/delete with smart pointers or vectors
4. **Low**: Additional modernization based on C++11/14/17 features available
