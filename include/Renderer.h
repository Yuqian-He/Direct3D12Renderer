#pragma once
#include <d3d12.h>
#include <vector>
#include "Vertex.h"
#include "Camera.h"
#include "Model.h"
#include <wrl.h>
#include <DirectXMath.h>
#include <stdexcept>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgi1_5.h>
#include <wrl.h>
#include <wrl/client.h>
#include <stdexcept>
#include <iostream>
#include <d3dcompiler.h>
#include "pipeline.h"

// 定义一个通用的对齐宏（256 字节对齐）
constexpr UINT ALIGNMENT = 256;

template<typename T>
constexpr UINT AlignTo256() {
    return (sizeof(T) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

class Renderer {
public:
    Renderer();
    ~Renderer();

    struct CameraBuffer {
        DirectX::XMMATRIX worldMatrix;
        DirectX::XMMATRIX viewMatrix;
        DirectX::XMMATRIX projectionMatrix;
    };

    struct alignas(ALIGNMENT) LightBuffer {
        DirectX::XMFLOAT4 lightPosition;     // 光源位置 (16 bytes)
        DirectX::XMFLOAT4 lightColor;        // 光源颜色 (16 bytes)
        DirectX::XMFLOAT4 ambientColor;      // 环境光颜色 (16 bytes)
        float ambientIntensity;             // 环境光强度 (4 bytes)
        float shininess;                    // 高光系数 (4 bytes)
        float specularIntensity;            // 高光强度 (4 bytes)
        DirectX::XMFLOAT3 viewPosition;     // 摄像机位置 (12 bytes)
        DirectX::XMMATRIX viewMatrix;       // 光源视角矩阵 (64 bytes)
        DirectX::XMMATRIX projectionMatrix; // 光源投影矩阵 (64 bytes)
        float padding2[2];                  // 对齐用
    };


    std::vector<Vertex> m_vertices;  // 或其他类型
    std::vector<UINT> m_indices;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;

    void Initialize(HWND hwnd);
    void Render();
    bool RenderOBJModel();

    // 使这些函数可以在外部调用
    void LoadShaders();
    void CreateDescriptorHeaps();
    void CreateVertexBuffer(const std::vector<Vertex>& vertices, const std::vector<UINT>& indices);
    void CreateCommandList();
    void ExecuteCommandList();
    void WaitForGpu();
    void CreateFence();
    void CreateConstantBuffer();
    void CreateLightBuffer();
    void CreateDepthStencilBuffer();
    void CreateShadowMap();
    void InitializeModel(Model& model, const std::string& filePath);
    void InitializePipeline();
    void CreateRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
    Camera m_camera;
    Model myModel;
    UINT m_width = 800;  // 窗口宽度
    UINT m_height = 600; // 窗口高度
    UINT m_swapChainBufferCount = 2; // 默认使用双缓冲
    UINT shadowMapWidth = 800;
    UINT shadowMapHeight = 600;
    UINT backBufferIndex;
    UINT mRtvDescriptorSize = 0;     // RTV 描述符大小
    UINT mDsvDescriptorSize = 0;     // DSV 描述符大小
    UINT mCbvSrvDescriptorSize = 0;  // CBV/SRV/UAV 描述符大小
    int mCurrBackBuffer = 0;

    void ProcessInput();
    void CreateDevice();
    void CreateCommandQueue();
    void CreateSwapChain(HWND hwnd);
    void ReleaseResources(); // Clean up resources when no longer needed
    void UpdateLightBuffer();
    void DrawSceneToShadowMap();
    void RenderShadowMap();
    void DrawScene();
    void RenderGeometryPass();
    void RenderLightingPass();
    void RenderPostProcessing();
    void PresentFrame();
    void SetViewportAndScissor(UINT width, UINT height);

    HRESULT hr;
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_cameraBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_lightBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_shadowMap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_shadowMapDSVHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_shadowMapSRVHeap; 
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
    uint64_t m_fenceValue = 1;
    std::vector<Model> m_models; 
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_gBufferTextures; // 存储 G-buffer 资源
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_gBufferRTVs;                // 存储 G-buffer 的 RTV 句柄
    std::vector<const float*> clearColors;  

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_shadowPipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_shadowRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_geometryPipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_geometryRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_lightingPipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_lightingRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_postProcessingPipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_postProcessingRootSignature;
    std::unique_ptr<Pipeline> m_pipeline;

    //shaders
    Microsoft::WRL::ComPtr<ID3DBlob> m_shadowVertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_shadowPixelShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_geometryVertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_geometryPixelShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_lightingVertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_lightingPixelShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_postProcessingVertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> m_postProcessingPixelShader;

    static const UINT FRAME_COUNT = 2; // 假设交换链有两个后台缓冲区
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FRAME_COUNT]; // 后台缓冲区数组
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap; // RTV 堆
    UINT m_rtvDescriptorSize = 0; // RTV 描述符大小
};

