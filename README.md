# Graph Visual Document Development Kit (Graph VDDK)

Game-engine-style ECS (EnTT) + precise page-sized canvas (A4 mm) + TikZ export + OpenGL option + WASM browser target.

## Features

- EnTT registry as single source of truth (nodes/edges as entities)
- Qt Quick page canvas (210×297 mm, drag & drop nodes)
- OpenGL accelerated rendering (FramebufferObject)
- TikZ/LaTeX exporter (1:1 coordinate match)
- Ready for SVG/PNG/JSON/CSV (add later)
- Desktop + WebAssembly (same codebase)

## Build (Desktop)

1. Use Python to fetch a standalone pre-compiled Qt 6.8 build into your local sandbox:
   ```bash
   python3 -m venv .venv && source .venv/bin/activate
   pip install aqtinstall
   aqt install-qt linux desktop 6.8.0 linux_gcc_64 -O ./local_qt
   ```
2. Generate the Makefile and compile:
   ```bash
   ./local_qt/6.8.0/gcc_64/bin/qmake -r GraphDocumentDevKit.pro
   make -j$(nproc)
   ```

Run: `./graph-constructor`

## WASM Browser Build (Qt 6.11+)

1. Install Emscripten SDK (emsdk) and activate latest (3.1.x+)
2. Source the Qt WASM kit environment:
   ```bash
   source /path/to/Qt/6.11.0/wasm_64/bin/qt-wasm-env.sh   # or equivalent
   ```

## Project Overview

**Name**: Graph Constructor Minimal  
**Goal**: A fast, game-engine-inspired visual graph editor for research documents.

- Precise **page-sized canvas** (A4: 210×297 mm) with drag-and-drop nodes for WYSIWYG LaTeX preview (eliminates page-breaking issues).
- Core data model in **EnTT ECS** (entities = nodes/edges, components for Position/Label/Style/Ports).
- Direct **TikZ/LaTeX export** with 1:1 coordinate mapping (cm/mm).
- Additional exports: SVG, PNG, JSON, CSV (extensible).
- **WASM browser** support for easy sharing + native desktop.
- Future: Lean 4 native FFI (desktop), force-directed layouts, edges/ports, snapping, mathematical property exploration bridged with agentic chat.

**Development Style (Agentic)**:

- Single source of truth = EnTT registry.
- UI layer (Qt Quick) is thin presentation only.
- Systems run on registry for layouts, exports, validation.
- Iterative: Extend components → add systems → expose via QML/C++ slots/signals.
- Zero CMake: Pure **qmake + Makefile** for simplicity and WASM compatibility.

### Stack

- **Language**: C++20
- **Core / Game Engine**: EnTT (header-only ECS via git submodule)
- **UI / Canvas**: Qt Quick (QML + custom `QQuickPaintedItem` with `FramebufferObject` for hardware acceleration)
- **Rendering**: QPainter (default) + **OpenGL/WebGL** (via `setRenderTarget(FramebufferObject)`) — works seamlessly on desktop and WASM
- **Export**: Manual string generation (TikZ) + Qt built-ins (SVG/PNG)
- **Build**: qmake (generates Makefile) — no CMake
- **Dependencies**: Qt 6.8+ (Quick + SVG modules), EnTT (submodule)
- **Targets**: Native desktop + WebAssembly (single codebase)
- **Optional future**: spdlog, glm, Lean 4 C ABI

### Core Features (Current Minimal + Planned)

**Implemented in Starter**:

- Page-sized canvas (fixed 210×297 mm bounds with visible border)
- EnTT registry with `Position` and `Label` components
- Draggable nodes (mouse hit-test + live update of EnTT positions)
- Real-time repaint on drag
- OpenGL-accelerated rendering
- Basic TikZ exporter (nodes with cm coordinates)
- Demo graph (3 nodes)

**High-Priority Extensions**:

- Edge entities + bezier/straight drawing
- Node ports / connection handling
- Grid snapping + page margin simulation
- Force-directed / custom layout systems (EnTT systems)
- Full exports: TikZ (copy to clipboard/file), SVG, PNG, JSON, CSV
- Zoom / pan on canvas
- Style components (color, shape, size)
- Lean 4 FFI stub (desktop only: `extern "C"` for planarity, etc.)

**Agentic Extension Points**:

- Add new component → register in GraphCanvas paint / mouse handlers
- Add new system → run on registry::view / each in update loop or button slot
- Expose via QML: properties, signals, or `Q_INVOKABLE` methods

### UI Schema

- **Main Window** (QML): `Window` containing full-screen `GraphCanvas`
- **GraphCanvas** (`QQuickPaintedItem` subclass):
  - Fixed logical size mapping to 210×297 mm (scale for zoom)
  - Paint: Page border + all nodes/edges from EnTT `view<>`
  - Input: `mousePress/Move/Release` for drag (updates EnTT `Position`)
  - OpenGL: `setRenderTarget(QQuickPaintedItem::FramebufferObject)`
- **Future QML Layer**:
  - Toolbar: Export TikZ, Add Node, Layout, Zoom, Load/Save JSON
  - Overlay: Rulers, grid, page margins (for LaTeX preview)
  - Delegates: Reusable QML items for advanced node rendering (optional)

Example `main.qml` structure:

```qml
Window {
    GraphCanvas { anchors.fill: parent }
    // Toolbar Row { Button { text: "Export TikZ" } ... }
}
```

### Compilation Instructions

#### Desktop (Native)

1. Ensure **Qt 6.8+** (or latest 6.11 as of 2026) with Qt Quick and SVG modules installed.
2. Clone repo with submodule:  
   `git clone --recursive <repo-url>`
3. `cd graph-constructor-minimal`
4. Generate Makefile: `qmake -r`
5. Build: `make -j$(nproc)` (or `make` on Windows via MinGW/MSVC)
6. Run: `./graph-constructor` (or `graph-constructor.exe`)

#### WASM / Browser

1. Install **Emscripten SDK** (emsdk) and activate a version compatible with your Qt (e.g., for Qt 6.11 use recent 4.x; check Qt docs for exact match).
2. Source the **Qt WASM environment** (critical):  
   `source /path/to/Qt/6.xx/wasm_64/bin/qt-wasm-env.sh`  
   (or equivalent on your platform; this sets correct qmake and flags).
3. `cd graph-constructor-minimal`
4. `qmake -r` (uses the wasm qmake automatically)
5. `make -j$(nproc)`
6. Output files: `graph-constructor.html`, `.js`, `.wasm`
7. Serve locally (e.g., `python3 -m http.server`) and open `graph-constructor.html` in a modern browser.

**Notes**:

- OpenGL/WebGL is automatic in WASM (Qt maps to WebGL 2).
- File downloads (exports) work via Qt + browser JS bridging.
- Lean 4 FFI is desktop-only in this setup (full C ABI); WASM version can use a Lean Emscripten build for limited checks.
- No threading in WASM (Qt configures `-no-feature-thread`).
