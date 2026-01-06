# Camera Control (C++)

A step-by-step learning project to grow from basic C++ into a **professional camera control** stack:
live preview, sensor controls (exposure/gain/white balance), timing, calibration, recording, and telemetry.
Platform: **Windows + Visual Studio (x64)**. Dependencies will be added gradually (e.g., OpenCV via vcpkg).

---

## Purpose
- Learn modern C++ by building practical, testable components.
- Cover essential **sensorics**: exposure, gain, white balance, ROI, frame rate.
- Prepare for **timing & synchronization** (timestamps, triggers) and **calibration** (intrinsics, distortion).
- Keep the design modular and reliable (clear errors, diagnostics, small increments).

---

## Roadmap (high level)
1. Console app skeleton (hello output)
2. Add OpenCV via vcpkg, print `CV_VERSION`
3. Hello Camera: live preview window (quit on `q`/`ESC`)
4. Overlays: FPS, mean brightness; screenshot on `s`
5. Exposure control (auto/manual), gain, white balance (if supported)
6. Recording (MP4/RAW) + metadata (CSV/JSON)
7. Calibration tools (checkerboard, intrinsics), undistort
8. Refactor into modules (drivers/pipeline/io), basic tests

> This repository is educational and vendor-agnostic.

---

## License

This project is licensed under the MIT License.  
See the [LICENSE](LICENSE.txt) file for details.

---

## Project Status
This project is under active development.
Current focus: basic camera control, live preview and modular architecture.

---
