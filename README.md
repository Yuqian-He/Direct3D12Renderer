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
