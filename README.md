# Real-Time Global Illumination Renderer Based on Direct3D 12 (基于 Direct3D 12 的实时全局光照渲染器)

## Goals:
Develop a real-time global illumination rendering engine capable of computing indirect lighting in dynamic scenes. 
开发一个实时全局光照渲染引擎，支持动态场景中的间接光照计算。

## Key Features:
- Implement the core rendering pipeline using Direct3D 12. 使用 Direct3D 12 编写核心渲染管线。
- Write HLSL shaders to achieve Screen Space Reflections (SSR) and Screen Space Global Illumination (SSGI). 编写 HLSL 着色器实现屏幕空间反射 (SSR) 和屏幕空间全局光照 (SSGI)。
- Support shadow mapping for light sources and dynamic reflections. 支持光源阴影映射和动态反射。
- Use multithreading to handle scene geometry frustum culling and lighting calculations. 使用多线程处理场景几何体剔除（Frustum Culling）和光照计算。

## Project Setup and Environment

- Build System and Compiler Information
    - Build System Generator:  [CMake](https://cmake.org/)
    - Build System: [MSBuild](https://learn.microsoft.com/en-us/visualstudio/msbuild/msbuild?view=vs-2022)
    - Compiler: [MSVC (Microsoft Visual C++)](https://learn.microsoft.com/en-us/cpp/?view=msvc-170)
    - Integrated Development Environment (IDE): [Visual Studio 2022](https://visualstudio.microsoft.com/)
- Libraries and Tools
    - Graphics and Rendering: [DirectX 12](https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-environment) 
    - Windows Platform Interface: [Windows API](https://learn.microsoft.com/en-us/windows/win32/api/) 
    - COM Programming: [Microsoft WRL (Windows Runtime C++ Template Library)](https://learn.microsoft.com/en-us/cpp/cppcx/wrl/using-the-windows-runtime-cpp-template-library-wrl?view=msvc-170) 
- Version Control
    - Version Control System: Git

## **Project Roadmap**

| **Phase**              | **Objective**                                                                                     | **Tasks**                                                                                                                                                                                                                                                        | **Timeframe**    |
|-------------------------|---------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------|
| **Phase 1:** Project Foundation and Setup  | Set up the development environment and build a basic understanding of Direct3D 12.        | 1. Prepare development environment: Install **Visual Studio 2022** and the latest **Windows SDK**.<br>2. Understand Direct3D 12 rendering pipeline: Root Signature, Command List, Descriptor Heap.<br>3. Initialize a project to render a simple triangle.      | 1–2 weeks       |
| **Phase 2:** Basic Rendering Engine        | Implement the rendering pipeline and basic dynamic lighting with shadow mapping.          | 1. Implement Root Signature and PSO.<br>2. Load simple models and add camera movement.<br>3. Implement dynamic lighting and shadow mapping using depth buffers.                                                                                              | 2–3 weeks       |
| **Phase 3:** Screen-Space Effects          | Add screen-space global illumination (SSGI) and reflections (SSR) for post-processing.    | 1. Implement SSGI using G-buffer data to simulate indirect lighting.<br>2. Add SSR using depth buffer-based ray tracing.<br>3. Optimize post-processing using dynamic resolution and asynchronous computation.                                                  | 3–4 weeks       |
| **Phase 4:** Multithreading and Culling    | Optimize rendering performance with multithreading and geometry culling techniques.        | 1. Use Direct3D 12's command lists for parallel rendering.<br>2. Implement frustum culling with multithreaded computations.<br>3. Optimize memory usage and resource transfers.                                                                                  | 2–3 weeks       |
| **Phase 5:** Advanced Features            | Enhance visual quality with PBR materials and volumetric lighting.                        | 1. Implement PBR materials with metalness and roughness support.<br>2. Add volumetric lighting for atmospheric effects.<br>3. Optimize frame rates and create a demo showcasing the project's features.                                                        | 3–4 weeks       |

