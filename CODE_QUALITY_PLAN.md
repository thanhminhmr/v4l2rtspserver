# C++ Code Quality Improvement Plan

This is a C++ project that has problem with code quality: There are many C-like portion which need to convert to C++ code. Some of them are: Using malloc/free instead of new/delete, using mem* function, using int/unsigned char/etc. instead of size_t/uint8_t/etc., and many more. Your mission is to change them into proper C++ code.

## Status: IN-PROGRESS

## Scope
- `main.cpp`
- `src/*`
- `inc/*`

## Requirements
- Must commit relevant changes in a single commit for human review.
- Prefer to commit frequently with small change set.

## Tools
- Run `make` in `cmake-build-debug/` folder to verify changes compile correctly.
- `clang-tidy` is also installed, so you can run it to check the code directly, for warning/suggestion as a list of problem to fix the code: `clang-tidy <source-file>`
- You can look at the recent `git log` for inspiration.

## Completed Changes

### Commits Made (each change set committed separately for human review):
1. `746c6d3` - Refactor V4L2DeviceSource and HTTPServer to modern C++
2. `a8b3cab` - Refactor MJPEGVideoSource and AddH26xMarkerFilter to modern C++
3. `a73ea61` - Refactor H264/H265 device sources to modern C++
4. `4354297` - Refactor MemoryBufferSink to modern C++
5. `42ef2f3` - Apply clang-tidy modernize-* suggestions
6. `3b1f41a` - Fix more clang-tidy warnings
7. `f484028` - Remove redundant virtual from defaulted destructor
8. `c95e22d` - Fix more switch cases, add [[nodiscard]], override
9. `f14fa2d` - Replace C functions with C++ equivalents
10. `7b3e691` - Replace sscanf with std::stoul in HTTPServer
11. `cae519c` - Replace fprintf with LOG(ERROR) in ALSACapture
12. `3775b77` - Fix clang-tidy warnings: use auto, make getFrameWithMarker static
13. `27f75a7` - Fix clang-tidy warnings: modernize-use-auto, use-equals-default, initialize timeval
14. `dfeb1a4` - Initialize timeval variables to fix cppcoreguidelines warnings
15. `73b96ca` - Fix modernize-use-emplace and performance-faster-string-find warnings
16. `111988a` - Fix readability-container-size-empty and remaining performance-faster-string-find
17. `b3144df` - Fix more clang-tidy: modernize-use-nullptr, modernize-use-emplace

### Changes Applied:
- Replaced `memcpy` with `std::copy` / `std::copy_n`
- Replaced `memmove` with `std::copy`
- Replaced `memset` with value initialization `{}`
- Replaced `new[]`/`delete[]` with `std::make_unique` / `std::vector`
- Replaced `unsigned char*` with `std::uint8_t*`
- Replaced C-style casts with `reinterpret_cast`
- Added missing includes (`<cstdint>`, `<memory>`, `<algorithm>`, `<cstring>`)
- Used `clang-format` on all modified files
- Applied clang-tidy `modernize-use-override` suggestions
- Applied clang-tidy `modernize-use-equals-default` suggestions
- Applied clang-tidy `modernize-return-braced-init-list` suggestions
- Consolidated duplicate switch cases in getVideoRtpFormat
- Added default cases to switch statements
- Added `[[nodiscard]]` to getter functions
- Replaced `virtual` with `override` where applicable
- Added custom `memmem` implementation using `std::search` (portable)
- Replaced `atoi()` with `std::stoi()`
- Replaced `strlen()` with `std::strlen()`
- Replaced `sscanf()` with `std::stoul()`
- Replaced `fprintf()` with `LOG(ERROR)`
- Added custom `memmem` implementation using `std::search` (portable)
- Fixed `timeval` initialization with `{}`
- Used `emplace_back` instead of `push_back`
- Used `std::empty()` instead of `size() != 0`
- Used `rfind()` instead of `find_last_of(".")`
- Replaced C-style casts with `auto` for modernity
- Used `nullptr` instead of `0` for pointers
- Changed `find_first_of(" ")` to `find(' ')` for single char
