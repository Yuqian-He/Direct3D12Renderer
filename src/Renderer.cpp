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

using namespace Microsoft::WRL;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

Renderer::Renderer() : m_camera({0.0f, 0.0f, -5.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}) {}
Renderer::~Renderer() {}

void Renderer::Initialize(HWND hwnd)
{
    CreateDevice();
    CreateCommandQueue();
    CreateFence();   
    CreateSwapChain(hwnd);
    CreateDescriptorHeaps();
    CreateDepthStencilBuffer();
    LoadShaders();
    CreateConstantBuffer();
    CreateLightBuffer();
    CreateShadowMap();
    CreateRootSignature();
    CreatePipelineState();
    CreateCommandList();

    InitializeModel(myModel, "D:\\Personal Project\\Direct3D12Renderer\\models\\suzanne.obj");
    CreateVertexBuffer(m_vertices, m_indices);

}

void Renderer::CreateDepthStencilBuffer()
{
    // Create depth buffer properties
    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    // Create the depth/stencil buffer resource
    HRESULT hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // Default heap
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,  // Initial state
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_depthStencil)
    );

    m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

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

    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_lightBuffer)
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create light constant buffer.");
    }
}

void Renderer::CreateFence()
{
    // 创建一个 fence
    HRESULT hr = m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr)) {
        std::cout << "Failed to create fence" << std::endl;
        throw std::runtime_error("Failed to create fence");
    }

    m_fenceValue = 1;  // 初始化为 1
}

void Renderer::CreateDevice()
{
    // 创建D3D12设备
    hr = D3D12CreateDevice(
        nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create D3D12 device");
    }
}

void Renderer::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command queue");
    }
}

void Renderer::CreateSwapChain(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    ComPtr<IDXGIFactory4> dxgiFactory;
    CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

    hr = dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),    // 指向 ID3D12CommandQueue 的指针
        hwnd,                    // 窗口句柄
        &swapChainDesc,          // 交换链描述
        nullptr,                 // 全屏描述（不需要可以传 nullptr）
        nullptr,                 // 输出限制（不需要可以传 nullptr）
        reinterpret_cast<IDXGISwapChain1**>(m_swapChain.GetAddressOf()) // 双指针形式
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create swap chain");
    }

}

void Renderer::CreateDescriptorHeaps()
{
    // 创建 RTV 描述符堆
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = m_swapChainBufferCount;  // 后台缓冲区数量
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;     // 渲染目标视图类型
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create RTV descriptor heap");
    }

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    
    // Create frame resources.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    // Create a RTV for each frame.
    for (UINT n = 0; n < m_swapChainBufferCount; n++)
    {
        m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }

    // Create DSV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1; // Only one depth/stencil buffer is needed
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // Depth-Stencil View type
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create DSV descriptor heap.");
    }
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

void Renderer::LoadShaders()
{
    // 着色器文件的路径
    const std::wstring vertexShaderPath = L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\vertex_shader.hlsl";
    const std::wstring pixelShaderPath = L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\pixel_shader.hlsl";

// 编译标志，动态支持 Debug 和 Release 模式
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    // 编译顶点着色器
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderErrorBlob;
    HRESULT hr = D3DCompileFromFile(
        vertexShaderPath.c_str(),
        nullptr,  // 可以传递自定义的宏定义
        nullptr,  // 传递自定义的 include 文件
        "VSMain",   // 着色器的入口函数名
        "vs_5_0", // 指定编译目标模型
        compileFlags, // 启用严格模式
        0, // 默认编译标志
        &vertexShaderBlob,
        &vertexShaderErrorBlob
    );

    if (FAILED(hr)) {
        if (vertexShaderErrorBlob) {
            OutputDebugStringA((char*)vertexShaderErrorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile vertex shader");
    }

    // 编译像素着色器
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderErrorBlob;
    hr = D3DCompileFromFile(
        pixelShaderPath.c_str(),
        nullptr,  // 可以传递自定义的宏定义
        nullptr,  // 传递自定义的 include 文件
        "PSMain",   // 着色器的入口函数名
        "ps_5_0", // 指定编译目标模型
        compileFlags, // 启用严格模式
        0, // 默认编译标志
        &pixelShaderBlob,
        &pixelShaderErrorBlob
    );

    if (FAILED(hr)) {
        if (pixelShaderErrorBlob) {
            OutputDebugStringA((char*)pixelShaderErrorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile pixel shader");
    }

    // 将编译后的着色器保存到成员变量中
    m_vertexShader = vertexShaderBlob;
    m_pixelShader = pixelShaderBlob;

    std::cout << "Vertex shader size: " << vertexShaderBlob->GetBufferSize() << std::endl;
    std::cout << "Pixel shader size: " << pixelShaderBlob->GetBufferSize() << std::endl;

}

void Renderer::CreateRootSignature()
{
    // 定义根参数
    D3D12_ROOT_PARAMETER rootParameters[4] = {};

    // 绑定矩阵常量缓冲区
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].Descriptor.ShaderRegister = 0; // 绑定到寄存器 0
    rootParameters[0].Descriptor.RegisterSpace = 0;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // 绑定光源常量缓冲区
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].Descriptor.ShaderRegister = 1; // 绑定到寄存器 1
    rootParameters[1].Descriptor.RegisterSpace = 0;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // 绑定阴影贴图（纹理）
    CD3DX12_DESCRIPTOR_RANGE shadowSRVRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // 绑定到 t0
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &shadowSRVRange;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // 绑定阴影贴图的采样器
    CD3DX12_DESCRIPTOR_RANGE samplerRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); // 绑定到 s0
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &samplerRange;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // 定义根签名描述符
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // 序列化根签名
    ComPtr<ID3DBlob> signatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cout << "Root signature error: " << static_cast<char*>(errorBlob->GetBufferPointer()) << std::endl;
        }
        throw std::runtime_error("Failed to serialize root signature");
    }

    // 创建根签名
    hr = m_device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    if (FAILED(hr)) {
        std::cout << "Failed to create root signature. HRESULT: " << hr << std::endl;
        throw std::runtime_error("Failed to create root signature");
    }
}

void Renderer::CreatePipelineState()
{
    // 定义输入布局
    static const D3D12_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 配置图形管线状态对象描述符
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { layout, ARRAYSIZE(layout) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(m_vertexShader->GetBufferPointer()), m_vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(m_pixelShader->GetBufferPointer()), m_pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // 设置深度模板状态
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.DepthStencilState.StencilEnable = TRUE; 

    // 渲染目标和深度缓冲区格式
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;

    // 创建图形管线状态对象
    HRESULT hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(hr)) {
        std::cout << "Failed to create pipeline state. HRESULT: " << hr << std::endl;
        throw std::runtime_error("Failed to create pipeline state");
    }

    std::cout << "Pipeline state created successfully!" << std::endl;
}

void Renderer::InitializeModel(Model& model, const std::string& filePath) {
    // 加载模型数据
    model.LoadModel(filePath,m_vertices,m_indices);

    model.vertices = m_vertices;
    model.indices = m_indices;
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

void Renderer::CreateCommandList()
{

    hr = m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, 
        IID_PPV_ARGS(&m_commandAllocator)
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command allocator");
    }

    // 创建命令列表
    HRESULT hr = m_device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(),
        m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create command list");
    }
    m_commandList->Close();
}


void Renderer::ExecuteCommandList()
{
    // Ensure the command allocator is reset before resetting the command list.
    try {
        m_commandAllocator->Reset();
        m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());
    } catch (const std::exception& e) {
        std::cout << "Error during command list reset: " << e.what() << std::endl;
        return;
    }

    // Bind vertex buffer
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = static_cast<UINT>(m_vertices.size() * sizeof(Vertex));
    vertexBufferView.StrideInBytes = sizeof(Vertex);

    // Bind index buffer (assuming m_indexBuffer is already created in Initialize or elsewhere)
    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    indexBufferView.SizeInBytes = static_cast<UINT>(m_indices.size() * sizeof(UINT));
    indexBufferView.Format = DXGI_FORMAT_R32_UINT;  // Assuming indices are 32-bit unsigned integers

    if (m_indices.size() % 3 == 0) {
        // If the model has a multiple of 3 indices, assume it is a triangle list
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    } else {
        // Otherwise, set it based on the model type (you can customize this check)
        std::cerr << "Unknown topology, defaulting to triangle list" << std::endl;
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // Bind vertex and index buffers
    m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    m_commandList->IASetIndexBuffer(&indexBufferView);

    // Draw indexed model
    m_commandList->DrawIndexedInstanced(m_indices.size(), 1, 0, 0, 0);

    // Close the command list
    try {
        m_commandList->Close();
    } catch (const std::exception& e) {
        std::cout << "Error closing command list: " << e.what() << std::endl;
    }

    // Execute the command list
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(ARRAYSIZE(ppCommandLists), ppCommandLists);
}

void Renderer::Render()
{
    // 获取当前后台缓冲区索引
    backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->SetPipelineState(m_pipelineState.Get());

    UpdateLightBuffer();
    DrawSceneToShadowMap();

    // 设置资源屏障，将后台缓冲区从 PRESENT 转换为 RENDER_TARGET
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier);

    // 获取渲染目标视图（RTV）
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr = rtvHandle.ptr + backBufferIndex * m_rtvDescriptorSize;

    // 获取深度目标视图（DSV） - 如果你有深度缓冲区
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
    if (m_dsvHeap) {
        dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    // 设置渲染目标视图（RTV）和深度目标视图（DSV）
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, m_dsvHeap ? &dsvHandle : nullptr);

    // 设置视口
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_commandList->RSSetViewports(1, &viewport);

    // 设置裁剪矩形
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = m_width;
    scissorRect.bottom = m_height;
    m_commandList->RSSetScissorRects(1, &scissorRect);

    // 清除渲染目标视图和深度目标视图
    const FLOAT clearColor[] = { 0.0f, 0.5f, 0.4f, 1.0f }; // 深蓝色
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    if (m_dsvHeap) {
        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    // 更新摄像机视图矩阵：处理输入并更新摄像机
    ProcessInput();  // 处理输入来更新相机状态

    DirectX::XMMATRIX viewMatrix = m_camera.GetViewMatrix();  // 获取视图矩阵

    // 更新投影矩阵（通常不会随相机更新而改变）
    float fov = DirectX::XMConvertToRadians(60.0f);
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    float nearZ = 0.1f;
    float farZ = 200.0f;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);

    // 创建相机常量缓冲区数据
    CameraBuffer cameraData;
    cameraData.worldMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity());
    cameraData.viewMatrix = DirectX::XMMatrixTranspose(viewMatrix);
    cameraData.projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);

    // 将相机常量缓冲区数据映射到 GPU
    D3D12_RANGE readRange = { 0, 0 }; // 不进行读取
    void* pData;
    HRESULT hr = m_cameraBuffer->Map(0, &readRange, &pData);
    if (SUCCEEDED(hr)) {
        memcpy(pData, &cameraData, sizeof(cameraData));
        m_cameraBuffer->Unmap(0, nullptr);
    }

    // 将常量缓冲区绑定到顶点着色器
    m_commandList->SetGraphicsRootConstantBufferView(0, m_cameraBuffer->GetGPUVirtualAddress());    

    // Execute the command list (draw the triangle)
    ExecuteCommandList();

    // 设置资源屏障，将后台缓冲区从 RENDER_TARGET 转换为 PRESENT
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[backBufferIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier);

    // 关闭命令列表并提交
    m_commandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 呈现交换链
    try {
        m_swapChain->Present(1, 0);
    } catch (const std::exception& e) {
        std::cout << "Error during swap chain present: " << e.what() << std::endl;
    }

    // 重置命令分配器和命令列表，为下一帧准备
    try {
        m_commandAllocator->Reset();
        m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());
    } catch (const std::exception& e) {
        std::cout << "Error during command allocator reset: " << e.what() << std::endl;
    }
}

/*
void Renderer::UpdateLightBuffer() {
    LightBuffer lightData;
    lightData.lightPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 2.0f, 1.0f); // 设置光源位置
    lightData.lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 设置环境光颜色
    lightData.ambientColor = DirectX::XMFLOAT4(0.3f, 0.25f, 0.2f, 1.0f);   // 设置环境光强度
    lightData.ambientIntensity = 0.6f;
    lightData.viewPosition = m_camera.GetPosition();   
    lightData.specularIntensity = 0.3f;    // 镜面反射强度
    lightData.shininess = 500.0f;  // 高光系数

    // 更新光照常量缓冲区
    void* pData;
    HRESULT hr = m_lightBuffer->Map(0, nullptr, &pData);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to map light buffer.");
    }

    memcpy(pData, &lightData, sizeof(LightBuffer));
    m_lightBuffer->Unmap(0, nullptr);

    m_commandList->SetGraphicsRootConstantBufferView(1, m_lightBuffer->GetGPUVirtualAddress());
}
*/


void Renderer::UpdateLightBuffer() {

    LightBuffer lightData;
    lightData.lightPosition = DirectX::XMFLOAT4(30.0f, 0.0f, 2.0f, 1.0f);
    lightData.lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    lightData.ambientColor = DirectX::XMFLOAT4(0.3f, 0.25f, 0.2f, 1.0f);
    lightData.ambientIntensity = 0.6f;
    lightData.viewPosition = m_camera.GetPosition();
    lightData.specularIntensity = 0.3f;
    lightData.shininess = 500.0f;
    DirectX::XMVECTOR _worldLightDir = XMVector3Normalize(XMVectorSet(1.0f, 0.6f, 1.0f, 0.0f));
    lightData.viewMatrix = DirectX::XMMatrixLookAtRH(5.f * _worldLightDir, XMVectorZero(), XMVectorSet(0.f, 1.0f, 0.f, 0.f));
    lightData.projectionMatrix = DirectX::XMMatrixOrthographicRH(10.f, 10.f, 0.1f, 10.f);

    // 更新常量缓冲区
    void* pData;
    m_lightBuffer->Map(0, nullptr, &pData);
    memcpy(pData, &lightData, sizeof(LightBuffer));
    m_lightBuffer->Unmap(0, nullptr);

    std::cout << "Light buffer unmapped and updated." << std::endl;

    m_commandList->SetGraphicsRootConstantBufferView(1, m_lightBuffer->GetGPUVirtualAddress());
    std::cout << "Light buffer bound to command list at root parameter 1." << std::endl;
}

void Renderer::DrawSceneToShadowMap()
{
    std::cout << "Entering DrawSceneToShadowMap" << std::endl;
    
    // 设置视口和裁剪矩形
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(shadowMapWidth);  // 阴影图宽度
    viewport.Height = static_cast<float>(shadowMapHeight); // 阴影图高度
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_commandList->RSSetViewports(1, &viewport);

    D3D12_RECT scissorRect = {};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = shadowMapWidth;
    scissorRect.bottom = shadowMapHeight;
    m_commandList->RSSetScissorRects(1, &scissorRect);

    // 切换 shadow map 到 DepthWrite 状态
    if (m_shadowMap) {
        CD3DX12_RESOURCE_BARRIER shadowBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_shadowMap.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        m_commandList->ResourceBarrier(1, &shadowBarrier);
    }

    // 获取并清空深度目标视图（DSV）
    D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle = m_shadowMapDSVHeap->GetCPUDescriptorHandleForHeapStart();
    m_commandList->ClearDepthStencilView(DSVHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // 设置深度目标视图（DSV）
    m_commandList->OMSetRenderTargets(0, nullptr, false, &DSVHandle);

    ID3D12DescriptorHeap* descriptorHeaps[] = { m_shadowMapSRVHeap.Get(), m_samplerHeap.Get()};
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    m_commandList->SetGraphicsRootConstantBufferView(0, m_cameraBuffer->GetGPUVirtualAddress());   
    m_commandList->SetGraphicsRootConstantBufferView(1, m_lightBuffer->GetGPUVirtualAddress());
    m_commandList->SetGraphicsRootDescriptorTable(2, m_shadowMapSRVHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetGPUDescriptorHandleForHeapStart());

    // 绘制阴影
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->DrawIndexedInstanced(static_cast<UINT>(myModel.indices.size()), 1, 0, 0, 0);

    // 绘制 shadow map 完成后，切换回像素着色器资源状态
    if (m_shadowMap) {
        CD3DX12_RESOURCE_BARRIER shadowBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_shadowMap.Get(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_commandList->ResourceBarrier(1, &shadowBarrier);

        std::cout << "Shadow map resource barrier: Depth Write to Pixel Shader." << std::endl;
    }
}


void Renderer::ProcessInput()
{
    // 获取当前的键盘和鼠标输入
    if (GetAsyncKeyState('W') & 0x8000) {  // W 键（前进）
        m_camera.MoveForward(0.1f);
    }
    if (GetAsyncKeyState('S') & 0x8000) {  // S 键（后退）
        m_camera.MoveForward(-0.1f);
    }
    if (GetAsyncKeyState('A') & 0x8000) {  // A 键（左移）
        m_camera.MoveRight(-0.1f);
    }
    if (GetAsyncKeyState('D') & 0x8000) {  // D 键（右移）
        m_camera.MoveRight(0.1f);
    }
    if (GetAsyncKeyState('Q') & 0x8000) {  // Q 键（上移）
        m_camera.MoveUp(0.1f);
    }
    if (GetAsyncKeyState('E') & 0x8000) {  // E 键（下移）
        m_camera.MoveUp(-0.1f);
    }

    // 获取鼠标移动，控制相机旋转
    POINT cursorPos;
    GetCursorPos(&cursorPos);  // 获取鼠标相对屏幕的位置

    // 计算鼠标的偏移量
    static POINT lastCursorPos = cursorPos;  // 记录上一帧的鼠标位置
    int deltaX = cursorPos.x - lastCursorPos.x;
    int deltaY = cursorPos.y - lastCursorPos.y;

    // 设置一个灵敏度因子来调节鼠标移动的幅度
    float sensitivity = 0.1f;

    // 调用相机的旋转函数
    m_camera.Rotate(deltaX * sensitivity, -deltaY * sensitivity);  // 鼠标水平移动影响yaw，垂直移动影响pitch

    // 更新上一帧的鼠标位置
    lastCursorPos = cursorPos;
}

