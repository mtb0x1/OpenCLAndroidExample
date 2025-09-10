## OpenCL Android Example (PoC)

This repo has:
- **UI app (Android)**: Java + native (C/C++) using OpenCL.
- **CLI (Android)**: native binary built with the Android NDK.
- **CLI (Desktop)**: same native code, desktop build to debug outside the phone.

Tested hardware: UI verified on an ARM64 device with a Mali GPU. The code also builds for ARMv7 and x86_64, but those weren’t runtime‑tested here.

### Targets / ABIs
- UI (APK): `arm64-v8a`, `armeabi-v7a`, `x86_64` (tested only on `arm64-v8a`).
- CLI (Android): selectable via `ARCH` (see Makefile).
- CLI (Desktop): host machine.

### Prerequisites
- Android SDK + NDK (r27 in Makefile defaults) and CMake.
- Device/vendor OpenCL drivers (ICD) on the phone/tablet.
- For desktop CLI: a desktop OpenCL runtime/ICD.

### Configuration warnings (read this first)
- **Android SDK location (`local.properties`)**: Gradle expects a `local.properties` file at the repo root with `sdk.dir=/absolute/path/to/Android/Sdk`. This file is typically untracked and machine‑specific. Ensure `sdk.dir` (or environment `ANDROID_HOME`/`ANDROID_SDK_ROOT`) points to a valid SDK. Without it, Gradle builds (e.g., `make ui`) will fail.
- **Makefile toolchain paths**: The `Makefile` provides fallbacks for `CMAKE_PATH` and `TOOLCHAIN_FILE` under `$(HOME)/Android/Sdk/...` (or the Windows equivalent). If your SDK/NDK versions or install paths differ, override via environment variables when invoking make, for example:
  - `CMAKE_PATH=/path/to/cmake CMAKE_TOOLCHAIN_FILE=/path/to/android.toolchain.cmake make cli`
  - Other useful overrides: `ARCH=arm64-v8a|armeabi-v7a|x86_64`, `API=24`, `BUILD_TYPE=Debug|Release`.
- **CMake OpenCL headers and libraries**: The native build (`app/src/main/cpp/CMakeLists.txt`) includes headers from `app/src/main/cpp/OpenCL` and imports an OpenCL library depending on the ABI:
  - Headers: ensure `OpenCL/CL/*.h` (e.g., `CL/cl.h`) are present or point includes to your SDK.
  - Libraries: the imported `.so`/`.dll` are referenced as `OpenCL/libOpenCL64.so`, `OpenCL/libOpenCL32.so`, `OpenCL/libOpenCLx86_64.so` (Android ABIs) and `OpenCL/OpenCL.dll` + `OpenCL/OpenCL.lib` (Windows desktop). You may need to replace these with your vendor ICD/loader or adjust the paths to match your setup.
  - For desktop builds, you can alternatively switch to a system installation via `find_package(OpenCL REQUIRED)` and link the discovered targets instead of the imported ones.
- **ABI/runtime match**: Ensure the OpenCL library you link matches the target ABI (e.g., `arm64-v8a` on device, `x86_64` on desktop). Mismatches will lead to load/link errors at runtime.

### Quick build
Using the provided Makefile (see configurable vars inside):

```bash
# UI APK (Debug/Release)
make ui BUILD_TYPE=Debug   # or Release

# CLI (Android). Choose ABI and API level as needed
make cli ARCH=arm64-v8a API=24 BUILD_TYPE=Release
# ARCH options typically: arm64-v8a | armeabi-v7a | x86_64

# CLI (Desktop) to debug native code on your machine
make cli_desktop BUILD_TYPE=Debug
```

Outputs:
- UI APKs: `app/build/outputs/apk/<variant>/`.
- Android CLI: `build_cli/` (binary and libs).
- Desktop CLI: `build_cli_desktop/`.

### Running
- UI: install the APK (`adb install ...`) and run on device.
- Android CLI: `adb push` the binary to the device and run from shell. Make sure the device has working OpenCL drivers.
- Desktop CLI: run the built binary locally; ensure your OpenCL ICD is installed.

### Notes
- This is a PoC; expect rough edges. Check the `Makefile` for build switches and paths.
- The desktop CLI exists to validate the native OpenCL path when mobile debugging is painful.



### How it works (high level)
- **UI flow**: `MainActivity` collects two matrices (or generates random values), validates shapes, then calls into JNI for `add`, `sub`, `matmul`, or `transpose`. Results (or errors) are shown in `ResultActivity`.
- **JNI bridge**: The app loads the native library `openclandroidexample` and exposes native calls like `processCommand(...)`, `dumpOpenCLDevices()`, and `getGpuName()`.
- **Native/OpenCL**: The native code executes the chosen operation on the GPU (when available) using OpenCL kernels, then returns a printable matrix string back to the UI.

Tip: Use the "GPU Info" button to dump platforms/devices detected by the device's OpenCL ICD.


### Usage (UI)
1. Enter rows/cols for Matrix A and Matrix B. Leave values blank to auto‑generate random matrices.
2. Tap one of the operations (Add/Sub/MatMul/Transpose). For `transpose`, only Matrix A is used.
3. View the result or error details on the next screen.


### Troubleshooting
- If the device has no OpenCL ICD or the vendor library is missing, the app may fall back or fail during device/context creation.
- Ensure the target ABI matches your device (`arm64-v8a` for most modern phones).
- Some vendors restrict OpenCL features; try smaller matrices first and verify with the desktop CLI.


### TODO / Possible improvements
  - **Keep a single context, device, and command queue** for the app lifetime instead of reinitializing per ops.
  - **Create and cache kernels/programs once** (or lazily on first use), then reuse.
  -  **Precompile kernels** (where supported) and cache program binaries with `clGetProgramInfo(CL_PROGRAM_BINARIES)`; reload with `clCreateProgramWithBinary` to avoid rebuilds.
  - Maintain a simple **kernel registry** keyed by op and matrix shape characteristics.
  - **Reuse device buffers** (memory pools) to cut allocations and reduce GC/driver overhead.
  - experimnt with **local work‑group sizes** and tiling; choose per‑device defaults via `clGetDeviceInfo`.
  - Use **event profiling** (`CL_QUEUE_PROFILING_ENABLE`) to measure enqueue, transfer, and kernel times.
  - Add **shape checks** and clearer error messages at the JNI boundary.
  - Provide **sample presets** and a quick benchmark mode in the UI.
  - integrate a BLAS library for CPU matmul to compare.?

