#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <stdexcept>
#include <dxgi1_6.h>
#include <iostream>
#include <d3dcompiler.h>

#include "MathHelper.h"
#include "d3dUtil.h"
#include "UploadBuffer.h"
#include "Camera.h"
#include "Pipeline.h"
#include "Vertex.h"

struct ObjectConstants{
    DirectX::XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class Renderer {
public:
/*
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
*/

    Renderer();
    ~Renderer();

    
    void Initialize(HWND hwnd);
    void Render();
    void Update();
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
    ID3D12Resource* CurrentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:

    //初始化
    UINT m_width = 1280;  
    UINT m_height = 720; 
    UINT mRtvDescriptorSize = 0;     
    UINT mDsvDescriptorSize = 0;    
    UINT mCbvSrvDescriptorSize = 0; 
    UINT mCurrentFence = 0;
    int mCurrBackBuffer = 0;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    bool m4xMsaaState = false; 
    UINT m4xMsaaQuality = 0;  
    static const UINT SwapChainBufferCount = 2; 
    std::vector<Vertex> m_vertices;
    std::vector<UINT> m_indices;
    HRESULT hr;
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencil;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap; 
    Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainBuffer[SwapChainBufferCount];

    void CreateDevice();
    void CreateCommandQueue();
    void CreateSwapChain(HWND hwnd);
    void CreateFence();
    void CreateDescriptorHeaps();
    void CreateRenderTargetView();
    void CreateDepthStencilBuffer();
    void SetViewportAndScissor(UINT width, UINT height);
    void FlushCommandQueue();

    //相机的参数
    Camera m_camera;
    DirectX::XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;
    void ProcessInput();

    //shader参数
    Microsoft::WRL::ComPtr<ID3DBlob> mvsByteCode = nullptr; 
    Microsoft::WRL::ComPtr<ID3DBlob> mpsByteCode = nullptr;

    //其他参数
    std::unique_ptr<Pipeline> m_pipeline;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_geometryRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_geometryPipelineState;
    std::vector<D3D12_INPUT_ELEMENT_DESC> m_GeometryInputLayout;
    std::unique_ptr<MeshGeometry> mImportGeo = nullptr;
    void InitialObject();
    void BuildDescriptorHeaps();
    void BuildConstantBuffers();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildGeometry();
    void BuildPSO();






/*
    Model myModel;
    UINT shadowMapWidth = 800;
    UINT shadowMapHeight = 600;
    UINT backBufferIndex;
    void ReleaseResources(); // Clean up resources when no longer needed
    void UpdateLightBuffer();
    void DrawSceneToShadowMap();
    void RenderShadowMap();
    void DrawScene();
    void RenderGeometryPass();
    void RenderLightingPass();
    void RenderPostProcessing();
    void PresentFrame();
    
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
    
    
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_lightingPipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_lightingRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_postProcessingPipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_postProcessingRootSignature;
    

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
*/
};

