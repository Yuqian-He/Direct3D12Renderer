// Renderer.cpp
#include "Renderer.h"
#include "Vertex.h"
#include "ModelLoader.h"
#include "Model.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <wrl.h>
#include "d3dx12.h"
#include <memory>
#include <string>
#include <iostream>
#include <Windows.h>
#include <dinput.h>
#include "pipeline.h"
#include "ShaderCompiler.h"

using namespace Microsoft::WRL;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

#if defined(DEBUG) || defined(_DEBUG)  
{
    ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
}
#endif

Renderer::Renderer() : m_camera({0.0f, 0.0f, -5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}) {}
Renderer::~Renderer() {}

void Renderer::Initialize(HWND hwnd)
{
    CreateDevice();
    CreateFence();
    CreateCommandQueue();
    CreateSwapChain(hwnd);
    CreateDescriptorHeaps();
    CreateRenderTargetView();
    CreateDepthStencilBuffer();
    
    LoadShaders();
    CreateConstantBuffer();
    CreateLightBuffer();
    CreateShadowMap();
    InitializePipeline();
    CreateCommandList();

    InitializeModel(myModel, "D:\\Personal Project\\Direct3D12Renderer\\models\\suzanne.obj");
    CreateVertexBuffer(m_vertices, m_indices);

}

void Renderer::CreateConstantBuffer() {
    // 创建常量缓冲区
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = sizeof(CameraBuffer);  // 常量缓冲区大小
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // 创建上传缓冲区
    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_cameraBuffer)
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create constant buffer.");
    }
}

void Renderer::CreateLightBuffer() {
    // 创建光照常量缓冲区
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Alignment = 0;
    bufferDesc.Width = sizeof(LightBuffer);  // 光照常量缓冲区大小
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.SampleDesc.Quality = 0;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,  // 使用常量缓冲区状态
        nullptr,
        IID_PPV_ARGS(&m_lightBuffer)
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create light constant buffer.");
    }
}

void Renderer::CreateDevice()
{
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory))); 
    HRESULT hardwareResult = D3D12CreateDevice(
        nullptr,                    // 默认适配器
        D3D_FEATURE_LEVEL_11_0,      // 最低特性级别
        IID_PPV_ARGS(&m_device)); // 创建的设备

    if (FAILED(hardwareResult)) {
        ComPtr<IDXGIAdapter> pWarpAdapter;
        ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
        ThrowIfFailed(D3D12CreateDevice(
            pWarpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)));
    }
}

void Renderer::CreateFence()
{
    // 创建一个 fence
    ThrowIfFailed(m_device->CreateFence(
        0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

    // 查询描述符大小并缓存
    mRtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    mDsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    mCbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void Renderer::CreateCommandQueue()
{
    //命令队列 (Command Queue)
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
    //命令分配器 (Command Allocator)
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.GetAddressOf()))); 
    //命令列表 (Command List)
    ThrowIfFailed(m_device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(m_commandList.GetAddressOf())));
    //命令列表关闭 (Close Command List)
    m_commandList->Close();
}

void Renderer::CreateSwapChain(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = mClientWidth;
    sd.BufferDesc.Height = mClientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = mBackBufferFormat;
    sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = SwapChainBufferCount;
    sd.OutputWindow = hwnd;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ThrowIfFailed(mdxgiFactory->CreateSwapChain(
        m_commandQueue.Get(),
        &sd,
        m_swapChain.GetAddressOf()));

}

void Renderer::CreateDescriptorHeaps()
{
    // 创建 RTV 堆
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = 2;  // 堆中描述符数量
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;  // 描述符类型是 RTV
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  // 无特殊标志
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

    // 创建 DSV 堆
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;  // 只需要一个深度/模板视图
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;  // 描述符类型是 DSaV
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  // 无特殊标志
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));
}

// 获取当前后备缓冲区的渲染目标视图（RTV）句柄
D3D12_CPU_DESCRIPTOR_HANDLE Renderer::CurrentBackBufferView() const
{
    // 通过 CD3DX12 构造函数计算偏移量，得到当前后备缓冲区的 RTV 描述符句柄
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),  // 堆的起始句柄
        mCurrBackBuffer,  // 当前后备缓冲区的索引
        mRtvDescriptorSize);  // RTV 描述符的字节大小
}

// 获取深度/模板视图（DSV）的句柄
D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Renderer::CreateRenderTargetView()
{
    ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(
        mRtvHeap->GetCPUDescriptorHandleForHeapStart()); // 获取渲染目标视图堆的起始句柄

    for (UINT i = 0; i < SwapChainBufferCount; i++) {
        // 获取交换链中的第 i 个缓冲区
        ThrowIfFailed(mSwapChain->GetBuffer(
            i, IID_PPV_ARGS(&mSwapChainBuffer[i])));

        // 创建一个渲染目标视图 (RTV) 给当前的缓冲区
        m_device->CreateRenderTargetView(
            mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);

        // 移动到堆中的下一个描述符位置
        rtvHeapHandle.Offset(1, mRtvDescriptorSize);
    }
}

void Renderer::CreateDepthStencilBuffer()
{
    // 创建深度/模板缓冲区描述
    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 说明这是一个二维纹理资源
    depthStencilDesc.Alignment = 0;  // 默认对齐方式
    depthStencilDesc.Width = mClientWidth;  // 缓冲区的宽度，通常为窗口的宽度
    depthStencilDesc.Height = mClientHeight;  // 缓冲区的高度，通常为窗口的高度
    depthStencilDesc.DepthOrArraySize = 1;  // 资源的深度或数组大小，这里是 1，因为只有一个层
    depthStencilDesc.MipLevels = 1;  // 使用的 Mipmap 层数，通常深度缓冲区只有一个 Mipmap 层
    depthStencilDesc.Format = mDepthStencilFormat;  // 设置深度/模板缓冲区的格式
    depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;  // 如果开启 4x MSAA，样本数为 4，否则为 1
    depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;  // MSAA 的质量
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;  // 使用默认的纹理布局
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;  // 设置资源标志，表示这是一个深度/模板缓冲区

    // 设置清除值，用于初始化深度/模板缓冲区
    D3D12_CLEAR_VALUE optClear;
    optClear.Format = mDepthStencilFormat;  // 使用与缓冲区相同的格式
    optClear.DepthStencil.Depth = 1.0f;  // 深度值的初始化为 1.0
    optClear.DepthStencil.Stencil = 0;  // 模板值初始化为 0

    // 创建深度/模板缓冲区资源
    ThrowIfFailed(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),  // 默认堆类型
        D3D12_HEAP_FLAG_NONE,  // 无堆标志
        &depthStencilDesc,  // 资源描述
        D3D12_RESOURCE_STATE_COMMON,  // 初始资源状态为 COMMON
        &optClear,  // 清除值
        IID_PPV_ARGS(m_depthStencil.GetAddressOf())));  // 获取深度/模板缓冲区的指针

    // 创建深度/模板视图
    m_device->CreateDepthStencilView(
        m_depthStencil.Get(),  // 深度/模板缓冲区
        nullptr,  // 使用默认格式
        DepthStencilView());  // 深度/模板视图

    // 过渡资源状态，从 COMMON 状态转换为 DEPTH_WRITE 状态
    m_commandList->ResourceBarrier(
        1, 
        &CD3DX12_RESOURCE_BARRIER::Transition(
            m_depthStencil.Get(),  // 深度/模板缓冲区资源
            D3D12_RESOURCE_STATE_COMMON,  // 初始状态
            D3D12_RESOURCE_STATE_DEPTH_WRITE));  // 目标状态
}


void Renderer::CreateShadowMap()
{
    // 创建 Shadow Map 的深度纹理
    D3D12_RESOURCE_DESC shadowMapDesc = {};
    shadowMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    shadowMapDesc.Alignment = 0;
    shadowMapDesc.Width = shadowMapWidth;
    shadowMapDesc.Height = shadowMapHeight;
    shadowMapDesc.DepthOrArraySize = 1;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.Format = DXGI_FORMAT_D32_FLOAT; // 使用无符号深度格式
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    shadowMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    // 深度清除值
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    // 创建深度资源
    D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &shadowMapDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&m_shadowMap)
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create shadow map depth resource");
    }

    // 创建 DSV 描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1; // 阴影贴图只有一个 DSV
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // DSV 堆不需要 ShaderVisible标记
    dsvHeapDesc.NodeMask = 0;

    hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_shadowMapDSVHeap));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create DSV heap");
    }

    // 创建 DSV（深度视图）
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    m_device->CreateDepthStencilView(m_shadowMap.Get(), &dsvDesc, m_shadowMapDSVHeap->GetCPUDescriptorHandleForHeapStart());

    // 创建 SRV 描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1; // 阴影贴图只有一个 SRV
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 必须设置为可见的
    srvHeapDesc.NodeMask = 0;

    hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_shadowMapSRVHeap));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create SRV heap");
    }

    // 创建阴影贴图的 SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT; // 对应深度格式
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_device->CreateShaderResourceView(m_shadowMap.Get(), &srvDesc, m_shadowMapSRVHeap->GetCPUDescriptorHandleForHeapStart());

    // 创建采样器描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 1; // 一个采样器
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 必须设置为可见的
    hr = m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create sampler heap");
    }

    // 定义采样器并存储到描述符堆
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;  // 使用最简单的点过滤
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;  // 边界模式
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
    samplerDesc.BorderColor[0] = 1.0f;  // 白色边界
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

    m_device->CreateSampler(&samplerDesc, m_samplerHeap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::LoadShaders() {
    // 编译并加载阴影 Pass 的着色器
    m_shadowVertexShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\ShadowPass.hlsl", 
        "VSMain",  // 顶点着色器的入口点
        "vs_5_0"
    );
    m_shadowPixelShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\ShadowPass.hlsl", 
        "PSMain",  // 像素着色器的入口点
        "ps_5_0"
    );
/*
    // 编译并加载几何 Pass 的着色器
    m_geometryVertexShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\GeometryPass.hlsl", 
        "VSMain", 
        "vs_5_0"
    );
    m_geometryPixelShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\GeometryPass.hlsl", 
        "PSMain", 
        "ps_5_0"
    );

    // 编译并加载光照 Pass 的着色器
    m_lightingVertexShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\LightingPass.hlsl", 
        "VSMain", 
        "vs_5_0"
    );
    m_lightingPixelShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\LightingPass.hlsl", 
        "PSMain", 
        "ps_5_0"
    );

    // 编译并加载后处理 Pass 的着色器
    m_postProcessingVertexShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\PostProcessingPass.hlsl", 
        "VSMain", 
        "vs_5_0"
    );
    m_postProcessingPixelShader = ShaderCompiler::CompileShaderFromFile(
        L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\PostProcessingPass.hlsl", 
        "PSMain", 
        "ps_5_0"
    );
*/
    // 输出着色器信息
    std::cout << "Shadow shader loaded successfully!" << std::endl;
    std::cout << "Geometry shader loaded successfully!" << std::endl;
    std::cout << "Lighting shader loaded successfully!" << std::endl;
    std::cout << "Post-processing shader loaded successfully!" << std::endl;
}

void Renderer::InitializeModel(Model& model, const std::string& filePath) {
    // 加载模型数据
    model.LoadModel(filePath,m_vertices,m_indices);

    model.vertices = m_vertices;
    model.indices = m_indices;
}

void Renderer::InitializePipeline() {
    if (!m_device) {
        throw std::runtime_error("Device is not initialized.");
    }

    // Initialize the pipeline object
    m_pipeline = std::make_unique<Pipeline>();

    // Assuming you have width and height as member variables
    UINT width = m_width;
    UINT height = m_height;

    try {
        m_pipeline->Initialize(m_device.Get(), width, height);
        m_pipeline->CreateShadowPipeline(m_device.Get(), m_shadowVertexShader, m_shadowPixelShader);

        // Store pipeline states and root signatures for rendering
        m_shadowPipelineState = m_pipeline->GetShadowPipelineState();
        m_shadowRootSignature = m_pipeline->GetShadowRootSignature();
/*
        m_geometryPipelineState = m_pipeline->GetGeometryPipelineState();
        m_geometryRootSignature = m_pipeline->GetGeometryRootSignature();

        m_lightingPipelineState = m_pipeline->GetLightingPipelineState();
        m_lightingRootSignature = m_pipeline->GetLightingRootSignature();

        m_postProcessingPipelineState = m_pipeline->GetPostProcessingPipelineState();
        m_postProcessingRootSignature = m_pipeline->GetPostProcessingRootSignature();
*/
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to initialize pipeline: ") + e.what());
    }
}

void Renderer::CreateVertexBuffer(const std::vector<Vertex>& vertices, const std::vector<UINT>& indices)
{
    // 重置命令列表
    HRESULT hr = m_commandList->Reset(m_commandAllocator.Get(), nullptr);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to reset command list in CreateVertexBuffer");
    }

    UINT vertexBufferSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));

    // 创建 GPU 顶点缓冲区
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        __uuidof(ID3D12Resource),  // 显式指定类型
        reinterpret_cast<void**>(m_vertexBuffer.GetAddressOf())
    );
    if (FAILED(hr)) throw std::runtime_error("Failed to create vertex buffer");

    // 创建上传缓冲区
    ComPtr<ID3D12Resource> vertexBufferUpload;
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBufferUpload)
    );
    if (FAILED(hr)) throw std::runtime_error("Failed to create upload buffer");

    // 复制顶点数据到上传缓冲区
    void* pData;
    vertexBufferUpload->Map(0, nullptr, &pData);
    memcpy(pData, vertices.data(), vertexBufferSize);
    vertexBufferUpload->Unmap(0, nullptr);

    // 将数据从上传缓冲区复制到 GPU 顶点缓冲区
    m_commandList->CopyBufferRegion(m_vertexBuffer.Get(), 0, vertexBufferUpload.Get(), 0, vertexBufferSize);

    // 切换顶点缓冲区资源状态
    D3D12_RESOURCE_BARRIER resourceBarrier = {};
    resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_vertexBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    );
    m_commandList->ResourceBarrier(1, &resourceBarrier);

    // 创建索引缓冲区
    UINT indexBufferSize = static_cast<UINT>(indices.size() * sizeof(UINT));

    // 创建 GPU 索引缓冲区
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        __uuidof(ID3D12Resource),  // 显式指定类型
        reinterpret_cast<void**>(m_indexBuffer.GetAddressOf())
    );
    if (FAILED(hr)) throw std::runtime_error("Failed to create index buffer");

    // 创建上传缓冲区
    ComPtr<ID3D12Resource> indexBufferUpload;
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&indexBufferUpload)
    );
    if (FAILED(hr)) throw std::runtime_error("Failed to create index upload buffer");

    // 复制索引数据到上传缓冲区
    indexBufferUpload->Map(0, nullptr, &pData);
    memcpy(pData, indices.data(), indexBufferSize);
    indexBufferUpload->Unmap(0, nullptr);

    // 将数据从上传缓冲区复制到 GPU 索引缓冲区
    m_commandList->CopyBufferRegion(m_indexBuffer.Get(), 0, indexBufferUpload.Get(), 0, indexBufferSize);

    // 切换索引缓冲区资源状态
    resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_indexBuffer.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_INDEX_BUFFER
    );
    m_commandList->ResourceBarrier(1, &resourceBarrier);

    // 关闭命令列表
    hr = m_commandList->Close();
    if (FAILED(hr)) {
        std::cout << "Failed to close command list" << std::endl;
        throw std::runtime_error("Failed to close command list in CreateVertexBuffer");
    }

    // 执行命令列表
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 等待 GPU 完成
    WaitForGpu();
}

void Renderer::WaitForGpu()
{
    // 向命令队列发送信号
    m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

    // 检查当前完成的值
    if (m_fence->GetCompletedValue() < m_fenceValue) {
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        m_fence->SetEventOnCompletion(m_fenceValue, eventHandle);
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    m_fenceValue++;
}

void Renderer::Render() {
    std::cout << "Render Start" << std::endl;
    UpdateLightBuffer();

    // Shadow Mapping Pass
    std::cout << "Starting Shadow Mapping Pass" << std::endl;
    RenderShadowMap();
    std::cout << "Shadow Mapping Pass Finished" << std::endl;

    // Geometry Pass
    //RenderGeometryPass();

    // Lighting Pass
    //RenderLightingPass();

    // Post-processing Pass (Optional)
    //RenderPostProcessing();

    // Present frame
    std::cout << "Presenting Frame" << std::endl;
    PresentFrame();
    std::cout << "Render End" << std::endl;

}

void Renderer::UpdateLightBuffer() {
    std::cout << "UpdateLightBuffer Start" << std::endl;

    // 设置光源的位置、颜色和其他参数
    LightBuffer lightData;
    lightData.lightPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 2.0f, 1.0f); // 设置光源位置
    lightData.lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 设置光源颜色
    lightData.ambientColor = DirectX::XMFLOAT4(0.3f, 0.25f, 0.2f, 1.0f);   // 设置环境光颜色
    lightData.ambientIntensity = 0.6f;  // 环境光强度
    lightData.viewPosition = m_camera.GetPosition(); // 摄像机位置
    lightData.specularIntensity = 0.3f;  // 镜面反射强度
    lightData.shininess = 500.0f;  // 高光系数

    std::cout << "Light Data Initialized:" << std::endl;
    std::cout << "Light Position: " << lightData.lightPosition.x << ", " << lightData.lightPosition.y << ", " << lightData.lightPosition.z << std::endl;
    std::cout << "Light Color: " << lightData.lightColor.x << ", " << lightData.lightColor.y << ", " << lightData.lightColor.z << std::endl;
    std::cout << "Ambient Color: " << lightData.ambientColor.x << ", " << lightData.ambientColor.y << ", " << lightData.ambientColor.z << std::endl;
    std::cout << "Ambient Intensity: " << lightData.ambientIntensity << std::endl;
    std::cout << "Specular Intensity: " << lightData.specularIntensity << std::endl;
    std::cout << "Shininess: " << lightData.shininess << std::endl;

    // 光源的目标方向和上方向
    DirectX::XMVECTOR lightTarget = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 光源目标
    DirectX::XMVECTOR lightUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // 上方向

    // 计算光源的视图矩阵（LookAt）
    DirectX::XMMATRIX lightViewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat4(&lightData.lightPosition), lightTarget, lightUp
    );
    lightData.viewMatrix = DirectX::XMMatrixTranspose(lightViewMatrix);

    std::cout << "Light View Matrix Computed" << std::endl;

    // 定义投影矩阵（例如正交投影）
    float orthoWidth = 20.0f;  // 正交投影宽度
    float orthoHeight = 20.0f; // 正交投影高度
    float nearPlane = 0.1f;    // 近裁剪面
    float farPlane = 100.0f;   // 远裁剪面
    DirectX::XMMATRIX lightProjectionMatrix = DirectX::XMMatrixOrthographicLH(
        orthoWidth, orthoHeight, nearPlane, farPlane
    );
    lightData.projectionMatrix = DirectX::XMMatrixTranspose(lightProjectionMatrix);

    std::cout << "Light Projection Matrix Computed" << std::endl;

    // 更新光照常量缓冲区
    void* pData;
    HRESULT hr = m_lightBuffer->Map(0, nullptr, &pData);
    if (FAILED(hr)) {
        std::cout << "Failed to map light buffer. HRESULT: " << hr << std::endl;
        throw std::runtime_error("Failed to map light buffer.");
    }

    std::cout << "LightBuffer Mapped Successfully" << std::endl;

    memcpy(pData, &lightData, sizeof(LightBuffer));
    m_lightBuffer->Unmap(0, nullptr);

    std::cout << "Light Data Copied to Buffer" << std::endl;

}

void Renderer::RenderShadowMap() {
    std::cout << "RenderShadowMap Start" << std::endl;
    m_commandList->SetGraphicsRootSignature(m_shadowRootSignature.Get());
    m_commandList->SetPipelineState(m_shadowPipelineState.Get());

    // Configure shadow map viewport and scissor
    SetViewportAndScissor(shadowMapWidth, shadowMapHeight);
    std::cout << "Viewport and Scissor Set" << std::endl;

    // Transition shadow map to depth write state
    CD3DX12_RESOURCE_BARRIER shadowBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_shadowMap.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_commandList->ResourceBarrier(1, &shadowBarrier);
    std::cout << "Resource Barrier Set (to DEPTH_WRITE)" << std::endl;

    // Clear and set shadow map DSV
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_shadowMapDSVHeap->GetCPUDescriptorHandleForHeapStart();
    m_commandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    std::cout << "Shadow Map Cleared" << std::endl;

    // Bind resources for shadow pass
    m_commandList->SetGraphicsRootConstantBufferView(0, m_lightBuffer->GetGPUVirtualAddress());
    //m_commandList->SetDescriptorHeaps(1, m_shadowMapSRVHeap.GetAddressOf());
    //m_commandList->SetGraphicsRootDescriptorTable(1, m_shadowMapSRVHeap->GetGPUDescriptorHandleForHeapStart());
    //m_commandList->SetDescriptorHeaps(1, m_samplerHeap.GetAddressOf());
    //m_commandList->SetGraphicsRootDescriptorTable(2, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DrawScene();

    // Transition shadow map back to shader resource state
    shadowBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_shadowMap.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    m_commandList->ResourceBarrier(1, &shadowBarrier);

    std::cout << "RenderShadowMap End" << std::endl;
}

void Renderer::DrawScene() {
    std::cout << "DrawScene" << std::endl;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = static_cast<UINT>(m_vertices.size() * sizeof(Vertex));
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = static_cast<UINT>(m_indices.size() * sizeof(UINT));
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_commandList->IASetIndexBuffer(&indexBufferView);

    // Draw indexed geometry
    m_commandList->DrawIndexedInstanced(static_cast<UINT>(m_indices.size()), 1, 0, 0, 0);
}

/*
void Renderer::RenderGeometryPass() {
    m_commandList->SetGraphicsRootSignature(m_geometryRootSignature.Get());
    m_commandList->SetPipelineState(m_geometryPipelineState.Get());

    // Configure geometry pass viewport and scissor
    SetViewportAndScissor(m_width, m_height);

    // Transition G-buffer resources to render target state
    for (size_t i = 0; i < m_gBufferRTVs.size(); ++i) {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_gBufferTextures[i].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &barrier);
    }

    // Clear G-buffer render targets
    for (size_t i = 0; i < m_gBufferRTVs.size(); ++i) {
        m_commandList->ClearRenderTargetView(m_gBufferRTVs[i], clearColors[i], 0, nullptr);
    }

    // Set G-buffer render targets
    m_commandList->OMSetRenderTargets(static_cast<UINT>(m_gBufferRTVs.size()), m_gBufferRTVs.data(), FALSE, &m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    // Bind resources for geometry pass
    m_commandList->SetGraphicsRootConstantBufferView(0, m_cameraBuffer->GetGPUVirtualAddress());
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DrawScene();

    // Transition G-buffer resources back to shader resource state
    for (size_t i = 0; i < m_gBufferRTVs.size(); ++i) {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_gBufferTextures[i].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &barrier);
    }
}

void Renderer::RenderLightingPass() {
    m_commandList->SetGraphicsRootSignature(m_lightingRootSignature.Get());
    m_commandList->SetPipelineState(m_lightingPipelineState.Get());

    // Configure back buffer viewport and scissor
    SetViewportAndScissor(m_width, m_height);

    // Transition back buffer to render target
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier);

    // Set render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIndex * m_rtvDescriptorSize;
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear render target
    const FLOAT clearColor[] = { 0.0f, 0.5f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Bind G-buffer and shadow map resources
    m_commandList->SetGraphicsRootDescriptorTable(0, m_gBufferSRVHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->SetGraphicsRootDescriptorTable(1, m_shadowMapSRVHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawInstanced(3, 1, 0, 0); // Full-screen quad for lighting

    // Transition back buffer back to present
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier);
}

void Renderer::RenderPostProcessing() {
    m_commandList->SetGraphicsRootSignature(m_postProcessRootSignature.Get());
    m_commandList->SetPipelineState(m_postProcessPipelineState.Get());

    // Configure full-screen viewport and scissor
    SetViewportAndScissor(m_width, m_height);

    // Bind post-processing resources
    m_commandList->SetGraphicsRootDescriptorTable(0, m_postProcessSRVHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawInstanced(3, 1, 0, 0); // Full-screen triangle
}
*/

void Renderer::PresentFrame() {
    std::cout << "into PresentFrame" << std::endl;
    m_commandList->Close();
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    m_swapChain->Present(1, 0);

    // Reset command allocator and command list
    m_commandAllocator->Reset();
    m_commandList->Reset(m_commandAllocator.Get(), nullptr);
}

void Renderer::SetViewportAndScissor(UINT width, UINT height) {
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(width);
    viewport.Height = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_commandList->RSSetViewports(1, &viewport);

    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = width;
    scissorRect.bottom = height;
    m_commandList->RSSetScissorRects(1, &scissorRect);
}
