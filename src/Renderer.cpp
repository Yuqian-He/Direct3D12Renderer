// Renderer.cpp
#include "Renderer.h"
#include "d3dx12.h"
#include "d3dUtil.h"
#include <initguid.h>
#include "Camera.h"
#include "Vertex.h"
#include "ModelLoader.h"
/*
#include "Renderer.h"
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
*/

using namespace Microsoft::WRL;
using Microsoft::WRL::ComPtr;
using namespace DirectX;
const int gNumFrameResources = 3;

Renderer::Renderer() : m_width(1280), m_height(720), mCurrBackBuffer(0), mCurrentFence(0){}
Renderer::~Renderer()
{
    if (m_device != nullptr)
        FlushCommandQueue();
}

void Renderer::Initialize(HWND hwnd)
{

#if defined(DEBUG) || defined(_DEBUG)  
{
    ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
}
#endif

    CreateDevice();
    CreateFence();
    CreateCommandQueue();
    CreateSwapChain(hwnd);
    CreateDescriptorHeaps(); 
    CreateRenderTargetView();
    CreateDepthStencilBuffer();

    //初始化其他物品
    InitialObject();
    

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
    sd.BufferDesc.Width = m_width;
    sd.BufferDesc.Height = m_height;
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


    ComPtr<IDXGISwapChain> swapChain;

    // 创建 SwapChain
    ThrowIfFailed(mdxgiFactory->CreateSwapChain(
        m_commandQueue.Get(),
        &sd,
        swapChain.GetAddressOf()
    ));

    // 获取 IDXGISwapChain4 接口
    ThrowIfFailed(swapChain.As(&m_swapChain));

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
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;  // 描述符类型是 DSV
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

ID3D12Resource* Renderer::CurrentBackBuffer()const
{
    return m_swapChainBuffer[mCurrBackBuffer].Get();
}

// 获取深度/模板视图（DSV）的句柄
D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Renderer::CreateRenderTargetView()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart()); // 获取渲染目标视图堆的起始句柄

    for (UINT i = 0; i < SwapChainBufferCount; i++) {
        // 获取交换链中的第 i 个缓冲区
        ThrowIfFailed(m_swapChain->GetBuffer(
            i, IID_PPV_ARGS(&m_swapChainBuffer[i])));

        // 创建一个渲染目标视图 (RTV) 给当前的缓冲区
        m_device->CreateRenderTargetView(
            m_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);

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
    depthStencilDesc.Width = m_width;  // 缓冲区的宽度，通常为窗口的宽度
    depthStencilDesc.Height = m_height;  // 缓冲区的高度，通常为窗口的高度
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

void Renderer::InitialObject()
{
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
    mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_device.Get(), 1, true);
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildGeometry();
    BuildMaterials();
    BuildRenderItem();
    BuildFrameResources();
    BuildPSO();

    ThrowIfFailed(m_commandList->Close());

    ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();
}

/*
void Renderer::BuildDescriptorHeaps(){
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc,IID_PPV_ARGS(&mCbvHeap)));
}

void Renderer::BuildConstantBuffers(){
    mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_device.Get(), 1, true);
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

    int boxCBufIndex = 0;
	cbAddress += boxCBufIndex*objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	m_device->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}
*/

void Renderer::BuildRootSignature()
{
    m_pipeline = std::make_unique<Pipeline>();
    m_pipeline->Initialize(m_device.Get(), m_width, m_height);
    m_geometryRootSignature = m_pipeline->GetGeometryRootSignature();
}

void Renderer::BuildShadersAndInputLayout()
{
    mvsByteCode = d3dUtil::CompileShader(L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"D:\\Personal Project\\Direct3D12Renderer\\shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

    m_GeometryInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

}

void Renderer::BuildGeometry(){

    std::string modelPath = "D:\\Personal Project\\Direct3D12Renderer\\models\\suzanne.obj";
    if (!ModelLoader::LoadOBJ(modelPath, m_vertices, m_indices)) {
        std::cerr << "Failed to load model: " << modelPath << std::endl;
        return;
    }

    const UINT vbByteSize = static_cast<UINT>(m_vertices.size() * sizeof(Vertex));
    const UINT ibByteSize = static_cast<UINT>(m_indices.size() * sizeof(UINT));

    mImportGeo = std::make_unique<MeshGeometry>();
    mImportGeo->Name = "ImportGeo";

    ThrowIfFailed(D3DCreateBlob(vbByteSize, &mImportGeo->VertexBufferCPU)); //创建一个大小为 vbByteSize 的内存块用于存储顶点数据
    CopyMemory(mImportGeo->VertexBufferCPU->GetBufferPointer(), m_vertices.data(), vbByteSize); //将 vertices 中的顶点数据拷贝到 VertexBufferCPU 中

    ThrowIfFailed(D3DCreateBlob(ibByteSize, &mImportGeo->IndexBufferCPU));
    CopyMemory(mImportGeo->IndexBufferCPU->GetBufferPointer(), m_indices.data(), ibByteSize);

	mImportGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device.Get(),
		m_commandList.Get(), m_vertices.data(), vbByteSize, mImportGeo->VertexBufferUploader); //创建 GPU 顶点缓冲区 将顶点数据从 CPU 上传到 GPU 的默认缓冲区中

	mImportGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_device.Get(),
		m_commandList.Get(), m_indices.data(), ibByteSize, mImportGeo->IndexBufferUploader); //创建 GPU 顶点缓冲区 将索引数据从 CPU 上传到 GPU 的默认缓冲区中

    // 设置缓冲区属性
	mImportGeo->VertexByteStride = sizeof(Vertex);
	mImportGeo->VertexBufferByteSize = vbByteSize;
    mImportGeo->IndexFormat = DXGI_FORMAT_R32_UINT;
	mImportGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)m_indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mImportGeo->DrawArgs["importGeo"] = submesh;

}

void Renderer::BuildMaterials()
{
    auto defaultMat = std::make_unique<Material>();
    defaultMat->Name = "default";
    defaultMat->MatCBIndex = 0;
    defaultMat->DiffuseSrvHeapIndex = 0; // 默认贴图（可以是 checkerboard）
    defaultMat->DiffuseAlbedo = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f); // 明亮灰
    defaultMat->FresnelR0 = XMFLOAT3(0.1f, 0.1f, 0.1f);
    defaultMat->Roughness = 0.25f;

    mMaterials["default"] = std::move(defaultMat);
}

void Renderer::BuildRenderItem(){

    auto importRitem = std::make_unique<RenderItem>();

    // 设置世界矩阵，比如放在原点，缩放一下
    XMStoreFloat4x4(&importRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f));
    XMStoreFloat4x4(&importRitem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));

    importRitem->ObjCBIndex = 0; // 依次编号
    importRitem->Geo = mImportGeo.get(); // 指向你导入的几何体数据
    importRitem->Mat = mMaterials["default"].get(); // 设置一个材质，你需要提前添加一个材质（名字可自定义）
    importRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    importRitem->IndexCount = mImportGeo->DrawArgs["importGeo"].IndexCount;
    importRitem->StartIndexLocation = mImportGeo->DrawArgs["importGeo"].StartIndexLocation;
    importRitem->BaseVertexLocation = mImportGeo->DrawArgs["importGeo"].BaseVertexLocation;

    mAllRitems.push_back(std::move(importRitem));

    for (auto& e : mAllRitems)
        mOpaqueRitems.push_back(e.get());
}

void Renderer::BuildFrameResources()
{
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(m_device.Get(),
            1, (UINT)mAllRitems.size(),(UINT)mMaterials.size()));
    }
}

void Renderer::BuildPSO()
{
    m_pipeline->CreateGeometryPipeline(m_GeometryInputLayout, m_device.Get(), mvsByteCode, mpsByteCode);
    m_geometryPipelineState = m_pipeline->GetGeometryPipelineState();
}

void Renderer::Update()
{
    //每帧遍历一个帧资源（多帧的话就是环形遍历）
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    //如果GPU端围栏值小于CPU端围栏值，即CPU速度快于GPU，则令CPU等待
    if (mCurrFrameResource->Fence != 0 && m_fence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(m_fence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
    UpdateObjectCBs();
    UpdateMaterialCB();
    UpdateMainPassCB();
    UpdateCamera();

}

void Renderer::UpdateCamera(){
    ProcessInput(); 
    XMMATRIX view = m_camera.GetViewMatrix();
}

void Renderer::UpdateMainPassCB(){
    PassConstants passConstants;
    XMMATRIX view = m_camera.GetViewMatrix();
    float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
    float fov = XMConvertToRadians(60.0f);
    float nearZ = 1.0f;
    float farZ = 1000.0f;
    XMMATRIX proj = XMMatrixPerspectiveFovLH(fov, aspectRatio, nearZ, farZ);

    XMMATRIX viewProj =  view * proj;
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
    XMStoreFloat4x4(&passConstants.ViewProj, XMMatrixTranspose(viewProj));

    passConstants.eyePosW = m_camera.GetPosition();

    //在这里设置光照
    OnKeyboardInput();
    passConstants.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
    passConstants.Lights[0].Strength = { 1.0f,1.0f,0.9f };
    XMVECTOR sunDir = -MathHelper::SphericalToCartesian(1.0f, sunTheta, sunPhi);
    XMStoreFloat3(&passConstants.Lights[0].Direction, sunDir);

    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, passConstants);

}

void Renderer::UpdateObjectCBs(){
    auto currObjectCB = mCurrFrameResource->ObjectCB.get();
    for(auto& e : mAllRitems){
        if(e->NumFramesDirty > 0){
            XMMATRIX world = XMLoadFloat4x4(&e->World);
            XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.world, XMMatrixTranspose(world));
            XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

            currObjectCB->CopyData(e->ObjCBIndex, objConstants);
            e->NumFramesDirty--;
        }
    }
}

void Renderer::UpdateMaterialCB(){
    auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
    for(auto& e : mMaterials){
        Material* mat = e.second.get();
        if(mat->NumFramesDirty > 0){
            XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);
			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
        }
    }
}

void Renderer::DrawRenderItems(ID3D12GraphicsCommandList* m_commandList,const std::vector<RenderItem*>& ritems){
    // 常量缓冲区字节对齐大小（例如 256 字节对齐）
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    // 获取当前帧的材质常量缓冲资源
	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

    //遍历渲染项数组
	for (size_t i = 0; i < ritems.size(); i++)
	{
		auto ritem = ritems[i];

        // 设置顶点/索引缓冲区和图元拓扑
		m_commandList->IASetVertexBuffers(0, 1, &ritem->Geo->VertexBufferView());
		m_commandList->IASetIndexBuffer(&ritem->Geo->IndexBufferView());
		m_commandList->IASetPrimitiveTopology(ritem->PrimitiveType);

		// 设置 ObjectCB 的根描述符表（槽位 0）
        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ritem->ObjCBIndex*objCBByteSize;
        D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ritem->Mat->MatCBIndex*matCBByteSize;
        m_commandList->SetGraphicsRootConstantBufferView(0, objCBAddress);
        m_commandList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		//绘制顶点（通过索引缓冲区绘制）
		m_commandList->DrawIndexedInstanced(ritem->IndexCount, //每个实例要绘制的索引数
			1,	//实例化个数
			ritem->StartIndexLocation,	//起始索引位置
			ritem->BaseVertexLocation,	//子物体起始索引在全局索引中的位置
			0);	//实例化的高级技术，暂时设置为0
	}
}

void Renderer::Render()
{
    ThrowIfFailed(m_commandAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_geometryPipelineState.Get()));

    SetViewportAndScissor(m_width,m_height);

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET));

    m_commandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), TRUE, &DepthStencilView()); //RTV
    m_commandList->SetGraphicsRootSignature(m_geometryRootSignature.Get()); //RootSignature

    auto passCB = mCurrFrameResource->PassCB->Resource();
    m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

    DrawRenderItems(m_commandList.Get(),mOpaqueRitems);

    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
        CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	ThrowIfFailed(m_swapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    FlushCommandQueue();
}

void Renderer::FlushCommandQueue()
{
    mCurrentFence++;

    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), mCurrentFence));

    if (m_fence->GetCompletedValue() < mCurrentFence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
        ThrowIfFailed(m_fence->SetEventOnCompletion(mCurrentFence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
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

void Renderer::OnKeyboardInput()
{
    const float dt = 0.001f;
	//左右键改变平行光的Theta角，上下键改变平行光的Phi角
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		sunTheta -= 1.0f * dt;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		sunTheta += 1.0f * dt;
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		sunPhi -= 1.0f * dt;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		sunPhi += 1.0f * dt;

	//将Phi约束在[0, PI/2]之间
	sunPhi = MathHelper::Clamp(sunPhi, 0.1f, XM_PIDIV2);
}
