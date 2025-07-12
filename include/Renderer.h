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
#include "FrameResource.h"

struct RenderItem
{
	RenderItem() = default;

    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4(); //该几何体的世界矩阵
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
	UINT ObjCBIndex = -1; //该几何体的常量数据在objConstantBuffer中的索引
	MeshGeometry* Geo = nullptr;
    Material* Mat = nullptr;
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
    int NumFramesDirty = gNumFrameResources;
};

class Renderer {
public:

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

    //3缓冲
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;
    void BuildFrameResources();

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
    void BuildMaterials();
    void OnKeyboardInput();
    void UpdateCamera();
    void UpdateMainPassCB();
    void UpdateObjectCBs();
    void UpdateMaterialCB();
    void BuildRenderItem();

    //相机的参数
    Camera m_camera;
    DirectX::XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;
    std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;
    std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    std::unordered_map<std::string,  Microsoft::WRL::ComPtr<ID3DBlob>> mShaders;
    std::unique_ptr<MeshGeometry> geo = nullptr;
    std::vector<RenderItem*> mOpaqueRitems;
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
    void DrawRenderItems(ID3D12GraphicsCommandList* m_commandList,const std::vector<RenderItem*>& ritems);

    //太阳（平行光）位置的球坐标
    float sunTheta = 1.25f * DirectX::XM_PI;
    float sunPhi = DirectX::XM_PIDIV4;

};

