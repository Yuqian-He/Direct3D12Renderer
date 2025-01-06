// pipeline.h
#ifndef PIPELINE_H
#define PIPELINE_H

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "d3dUtil.h"

class Pipeline {
public:
    Pipeline();
    ~Pipeline();

    bool m4xMsaaState = false; 
    UINT m4xMsaaQuality = 0;  
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    void Initialize(ID3D12Device* device, UINT width, UINT height);
    
    //void CreateShadowPipeline(ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob);
    void CreateGeometryPipeline(std::vector<D3D12_INPUT_ELEMENT_DESC> m_GeometryInputLayout,ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob);
    //void CreateLightingPipeline(ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob);
    //void CreatePostProcessingPipeline(ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob);


    Microsoft::WRL::ComPtr<ID3D12RootSignature> GetShadowRootSignature() const { return m_shadowRootSignature; }
    Microsoft::WRL::ComPtr<ID3D12RootSignature> GetGeometryRootSignature() const { return m_geometryRootSignature; }
    Microsoft::WRL::ComPtr<ID3D12RootSignature> GetLightingRootSignature() const { return m_lightingRootSignature; }
    Microsoft::WRL::ComPtr<ID3D12RootSignature> GetPostProcessingRootSignature() const { return m_postProcessingRootSignature; }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> GetShadowPipelineState() const { return m_shadowPipelineState; }
    Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGeometryPipelineState() const { return m_geometryPipelineState; }
    Microsoft::WRL::ComPtr<ID3D12PipelineState> GetLightingPipelineState() const { return m_lightingPipelineState; }
    Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPostProcessingPipelineState() const { return m_postProcessingPipelineState; }

private:



    //void CreateShadowRootSignature(ID3D12Device* device);
    void CreateGeometryRootSignature(ID3D12Device* device);
    //void CreateLightingRootSignature(ID3D12Device* device);
    //void CreatePostProcessingRootSignature(ID3D12Device* device);

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_shadowRootSignature;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_geometryRootSignature;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_lightingRootSignature;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_postProcessingRootSignature;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_shadowPipelineState;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_geometryPipelineState;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_lightingPipelineState;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_postProcessingPipelineState;

};

#endif // PIPELINE_H