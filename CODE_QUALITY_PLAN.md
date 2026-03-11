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

### Changes Applied:
- Replaced `memcpy` with `std::copy` / `std::copy_n`
- Replaced `memmove` with `std::copy`
- Replaced `memset` with value initialization `{}`
- Replaced `new[]`/`delete[]` with `std::make_unique` / `std::vector`
- Replaced `unsigned char*` with `std::uint8_t*`
- Replaced C-style casts with `reinterpret_cast`
- Added missing includes (`<cstdint>`, `<memory>`, `<algorithm>`)
- Used `clang-format` on all modified files

## Build Verification
Run `make` in `cmake-build-debug/` folder to verify changes compile correctly.

## Next step
Fix all warning that `clang-tidy` produce.