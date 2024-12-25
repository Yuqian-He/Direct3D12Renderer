// pipeline.cpp
#include "pipeline.h"
#include <d3dx12.h>
#include <stdexcept>

Pipeline::Pipeline() {}

Pipeline::~Pipeline() {}

void Pipeline::Initialize(ID3D12Device* device, UINT width, UINT height) {
    CreateShadowRootSignature(device);
    //CreateGeometryRootSignature(device);
    //CreateLightingRootSignature(device);
    //CreatePostProcessingRootSignature(device);
}

void Pipeline::CreateShadowRootSignature(ID3D12Device* device) {
    CD3DX12_ROOT_PARAMETER1 rootParameters[3];

    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 插槽
    rootParameters[1].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_DESCRIPTOR_RANGE1 samplerRange;
    samplerRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); // s0 插槽
    rootParameters[2].InitAsDescriptorTable(1, &samplerRange, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    if (FAILED(hr)) {throw std::runtime_error("Failed to serialize shadow root signature.");}

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_shadowRootSignature));
    if (FAILED(hr)) {throw std::runtime_error("Failed to create shadow root signature.");}
}

/*
void Pipeline::CreateGeometryRootSignature(ID3D12Device* device) {
    CD3DX12_ROOT_PARAMETER1 rootParameters[2];
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsDescriptorTable(1, &CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0), D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to serialize geometry root signature.");
    }

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_geometryRootSignature));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create geometry root signature.");
    }
}

void Pipeline::CreateLightingRootSignature(ID3D12Device* device) {
    CD3DX12_ROOT_PARAMETER1 rootParameters[2];
    rootParameters[0].InitAsDescriptorTable(1, &CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0), D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to serialize lighting root signature.");
    }

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_lightingRootSignature));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create lighting root signature.");
    }
}

void Pipeline::CreatePostProcessingRootSignature(ID3D12Device* device) {
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &CD3DX12_DESCRIPTOR_RANGE1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0), D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to serialize post-processing root signature.");
    }

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_postProcessingRootSignature));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create post-processing root signature.");
    }
}
*/

void Pipeline::CreateShadowPipeline(ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob) 
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_shadowRootSignature.Get();

    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize());

    // Rasterizer and depth stencil settings
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // Input layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 0; // Shadow pass does not use color render targets
    psoDesc.SampleDesc.Count = 1;

    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_shadowPipelineState));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create shadow pipeline state.");
    }
}

/*
void Pipeline::CreateGeometryPipeline(ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();

    // Fill shaders
    psoDesc.VS = CD3DX12_SHADER_BYTECODE();
    psoDesc.PS = CD3DX12_SHADER_BYTECODE();

    // Rasterizer and depth stencil settings
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

    // Input layout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 3;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_geometryPipelineState));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create geometry pipeline state.");
    }
}

void Pipeline::CreateLightingPipeline(ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();

    // Fill shaders
    psoDesc.VS = CD3DX12_SHADER_BYTECODE();
    psoDesc.PS = CD3DX12_SHADER_BYTECODE();

    // Rasterizer and blend settings
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // Input layout
    psoDesc.InputLayout = { nullptr, 0 }; // No input layout required for full-screen quad

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_lightingPipelineState));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create lighting pipeline state.");
    }
}

void Pipeline::CreatePostProcessingPipeline(ID3D12Device* device,Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob, Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_rootSignature.Get();

    // Fill shaders
    psoDesc.VS = CD3DX12_SHADER_BYTECODE();
    psoDesc.PS = CD3DX12_SHADER_BYTECODE();

    // Rasterizer and blend settings
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

    // Input layout
    psoDesc.InputLayout = { nullptr, 0 }; // No input layout required for full-screen quad

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_postProcessingPipelineState));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create post-processing pipeline state.");
    }
}

*/