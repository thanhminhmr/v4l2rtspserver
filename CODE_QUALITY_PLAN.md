# C++ Code Quality Improvement Plan

## Status: IN-PROGRESS

## Scope

- `main.cpp`
- `src/*`
- `inc/*`

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

### Changes Applied:

- Replaced `memcpy` with `std::copy` / `std::copy_n`
- Replaced `memmove` with `std::copy`
- Replaced `memset` with value initialization `{}`
- Replaced `new[]`/`delete[]` with `std::make_unique` / `std::vector`
- Replaced `unsigned char*` with `std::uint8_t*`
- Replaced C-style casts with `reinterpret_cast`
- Added missing includes (`<cstdint>`, `<memory>`, `<algorithm>`)
- Used `clang-format` on all modified files
- Applied clang-tidy `modernize-use-override` suggestions
- Applied clang-tidy `modernize-use-equals-default` suggestions
- Applied clang-tidy `modernize-return-braced-init-list` suggestions
- Consolidated duplicate switch cases in getVideoRtpFormat
- Added default cases to switch statements
- Added `[[nodiscard]]` to getter functions
- Replaced `virtual` with `override` where applicable

## Build Verification

Run `make` in `cmake-build-debug/` folder to verify changes compile correctly.

## Todo list

Fix warning produced by `clang-tidy -p cmake-build-debug <source-file>`. This tool runs quite slow, prefer to cache the
output to a file.