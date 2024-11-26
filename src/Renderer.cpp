// Renderer.cpp
#include "Renderer.h"
#include "Vertex.h"
#include "ModelLoader.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <wrl.h>
#include "d3dx12.h"
#include <memory>
#include <string>
#include <iostream>
#include <d3dcompiler.h>

using namespace Microsoft::WRL;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

void Renderer::Initialize(HWND hwnd)
{
    CreateDevice();
    CreateCommandQueue();
    CreateFence();   
    CreateSwapChain(hwnd);
    CreateDescriptorHeaps();
    LoadShaders();
    CreateRootSignature();
    CreatePipelineState();
    CreateCommandList();

    if (!ModelLoader::LoadOBJ("D:\\Personal Project\\Direct3D12Renderer\\models\\cube.obj", m_vertices, m_indices)) {
        std::cerr << "Failed to load the model!" << std::endl;
    }

    CreateVertexBuffer(m_vertices,m_indices);
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
    swapChainDesc.Width = 1280;
    swapChainDesc.Height = 720;
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

    // 为每个交换链缓冲区创建 RTV 描述符
    for (UINT i = 0; i < m_swapChainBufferCount; i++) {
        hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to get swap chain buffer");
        }

        // 创建渲染目标视图
        m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        // 移动到下一个描述符
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr = rtvHandle.ptr + m_rtvDescriptorSize;
    }
}


void Renderer::LoadShaders()
{
    // 着色器文件的路径
    const std::wstring vertexShaderPath = L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\vertex_shader.hlsl";
    const std::wstring pixelShaderPath = L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\pixel_shader.hlsl";

    // 编译顶点着色器
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderErrorBlob;
    HRESULT hr = D3DCompileFromFile(
        vertexShaderPath.c_str(),
        nullptr,  // 可以传递自定义的宏定义
        nullptr,  // 传递自定义的 include 文件
        "main",   // 着色器的入口函数名
        "vs_5_0", // 指定编译目标模型
        D3DCOMPILE_ENABLE_STRICTNESS, // 启用严格模式
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
        "main",   // 着色器的入口函数名
        "ps_5_0", // 指定编译目标模型
        D3DCOMPILE_ENABLE_STRICTNESS, // 启用严格模式
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
}


void Renderer::CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // 创建根签名
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(hr)) {
        std::cout << "Failed to serialize root signature" << std::endl;
        throw std::runtime_error("Failed to serialize root signature");
    }

    hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    if (FAILED(hr)) {
        std::cout << "Failed to create root signature" << std::endl;
        throw std::runtime_error("Failed to create root signature");
    }
}

void Renderer::CreatePipelineState()
{
    // 创建输入布局
    D3D12_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // 定义根参数，绑定常量缓冲区
    D3D12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 常量缓冲区
    rootParameters[0].Descriptor.ShaderRegister = 0;   // 在着色器中的寄存器位置
    rootParameters[0].Descriptor.RegisterSpace = 0;     // 注册空间，通常为 0
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 顶点着色器可见

    // 根签名描述符
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // 生成根签名
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    // 序列化根签名
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, &errorBlob);
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

    // 创建管线状态对象描述
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { layout, ARRAYSIZE(layout) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(m_vertexShader->GetBufferPointer()), m_vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(m_pixelShader->GetBufferPointer()), m_pixelShader->GetBufferSize() };

    // 配置光栅化状态
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; // 背面剔除
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

    // 配置混合状态
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // 配置深度和模板状态
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // 配置样本
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // 配置渲染目标格式
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA 8bit 格式
    psoDesc.SampleDesc.Count = 1;

    // 创建图形管线状态对象
    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(hr)) {
        std::cout << "Failed to create pipeline state. HRESULT: " << hr << std::endl;
        throw std::runtime_error("Failed to create pipeline state");
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

    // Set root signature
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

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

    // Set the primitive topology based on the model's data
    if (m_indices.size() % 3 == 0) {
        // If the model has a multiple of 3 indices, assume it is a triangle list
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    } else {
        // Otherwise, set it based on the model type (you can customize this check)
        std::cerr << "Unknown topology, defaulting to triangle list" << std::endl;
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
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
    UINT backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

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
    const FLOAT clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f }; // 深蓝色
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    if (m_dsvHeap) {
        m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }


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
