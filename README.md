# Motor de renderizado ligero en Vulkan (C++17)

Repositorio de un **motor 3D** minimalista para **visualización interactiva** sobre **Vulkan 1.3**, escrito en **C++17** y desarrollado principalmente con **Microsoft Visual Studio** (Windows).  
Forma parte de mi **TFM (MUDS · Universidad de Granada, 2025)**. El código está pensado para ser claro y fácil de extender.

---

## Características

- **Vulkan 1.3**: swapchain, render pass, depth, viewport/scissor dinámicos.
- **Pipeline básico (VS/FS)** con shaders GLSL → SPIR-V.
- **UBO global**: matrices `model/view/proj` y parámetros de luz puntual.
- **Carga de modelos OBJ** (posiciones, normales y, si hay, UVs).
- **Dear ImGui**: panel **Performance** (FPS, ms CPU/GPU, %CPU sistema/proceso) y controles.
- **GLFW** (ventana/entrada) y **GLM** (matemáticas).
- Estructura modular: `Renderer`, `SwapChain`, `BasicRenderer`, `PointLightSystem`, `EditorUI`, `Perf`, utilidades de buffers/descriptores.

La **memoria del TFM** documenta la arquitectura, las decisiones de diseño y las pruebas.

---

## Requisitos

- **Windows 10/11**  
- **Visual Studio 2022** (C++17) o **CMake 3.24+**  
- **Vulkan SDK 1.3+** (incluye `glslc`)  
- Drivers de GPU actualizados
