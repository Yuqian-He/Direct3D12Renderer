# Real-Time Global Illumination Renderer Based on Direct3D 12 (基于 Direct3D 12 的实时全局光照渲染器)

## **Goals**
Develop a real-time global illumination rendering engine capable of computing indirect lighting in dynamic scenes. 
开发一个实时全局光照渲染引擎，支持动态场景中的间接光照计算。

## **Key Features**
- Implement the core rendering pipeline using Direct3D 12. 使用 Direct3D 12 编写核心渲染管线。
- Write HLSL shaders to achieve Screen Space Reflections (SSR) and Screen Space Global Illumination (SSGI). 编写 HLSL 着色器实现屏幕空间反射 (SSR) 和屏幕空间全局光照 (SSGI)。
- Support shadow mapping for light sources and dynamic reflections. 支持光源阴影映射和动态反射。
- Use multithreading to handle scene geometry frustum culling and lighting calculations. 使用多线程处理场景几何体剔除（Frustum Culling）和光照计算。

## **Project Setup and Environment**

- Build System and Compiler Information
    - Build System Generator:  [CMake](https://cmake.org/)
    - Build System: [MSBuild](https://learn.microsoft.com/en-us/visualstudio/msbuild/msbuild?view=vs-2022)
    - Compiler: [MSVC (Microsoft Visual C++)](https://learn.microsoft.com/en-us/cpp/?view=msvc-170)
    - Integrated Development Environment (IDE): [Visual Studio 2022](https://visualstudio.microsoft.com/)
- Libraries
    - Graphics and Rendering: [DirectX 12](https://learn.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-environment) 
    - Windows Platform Interface: [Windows API](https://learn.microsoft.com/en-us/windows/win32/api/) 
    - COM Programming: [Microsoft WRL (Windows Runtime C++ Template Library)](https://learn.microsoft.com/en-us/cpp/cppcx/wrl/using-the-windows-runtime-cpp-template-library-wrl?view=msvc-170) 
    - Model Loading: [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- Development and Debugging Tools
    - Package Manager: [vcpkg](https://github.com/microsoft/vcpkg)
    - Version Control: [Git](https://github.com/)
    - Debugging: [PIX](https://devblogs.microsoft.com/pix/)

## **Project Roadmap**

### **Phase 1: Project Foundation and Setup**
**Objective:** Set up the development environment and understand Direct3D 12 basics.  
**Tasks:**
- ✅ Install **Visual Studio 2022** and **Windows SDK**.
- ✅ Learn Direct3D 12 pipeline: Root Signature, Command List, Descriptor Heap.  
- ✅ Initialize project: Render a simple triangle.  

**Result:**
[Direct3D 12 Triangle Rendering](https://github.com/Yuqian-He/Direct3DTriangleRenderer)

**Timeframe:** 2024-11-15 to 2024-11-29  

---

### **Phase 2: Basic Rendering Engine**
**Objective:** Implement the rendering pipeline and basic lighting with shadows.  

**Tasks:**
- ✅ Implement Root Signature and PSO.  
- ✅ Add camera movement.
- ✅ Load simple models.  
- ❌ Implement dynamic lighting and shadow mapping.  
<sub><span style="color:gray;">After reconstructing my code, I still couldn’t get it to work properly. So, I’ve decided to start reading and practicing with the book ["Introduce to 3D Game Programming with DirectX12"](https://www.google.co.uk/books/edition/Introduction_to_3D_Game_Programming_with/gj6TDgAAQBAJ?hl=en&gbpv=0). I’m also actively updating my code and notes in my repository [D3D12book_code](https://github.com/Yuqian-He/D3D12book_code). I’ll continue sharing updates once I feel more confident in my understanding. 😊

**Timeframe:** 2024-11-30 to  xxx

---

### **Phase 3: Screen-Space Effects**
**Objective:** Add screen-space global illumination (SSGI) and reflections (SSR).  

**Tasks:**
- [ ] Implement SSGI for indirect lighting.  
- [ ] Add SSR using depth buffer ray tracing.  
- [ ] Optimize post-processing performance.  

**Timeframe:** xxx to xxx  

---

### **Phase 4: Multithreading and Culling**
**Objective:** Optimize rendering performance with multithreading and culling.  

**Tasks:**
- [ ] Use command lists for parallel rendering.  
- [ ] Implement frustum culling.  
- [ ] Optimize memory usage and resource management.  


**Timeframe:** xxx to xxx  

---

### **Phase 5: Advanced Features**
**Objective:** Enhance visual quality with PBR materials and volumetric lighting.  

**Tasks:**
- [ ] Add PBR materials with metalness and roughness.  
- [ ] Implement volumetric lighting for atmospheric effects.  
- [ ] Create a demo showcasing all features.  

**Timeframe:** xxx to xxx  


