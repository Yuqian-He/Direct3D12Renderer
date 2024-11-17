#include "Renderer.h"
#include <stdexcept>


void Renderer::Initialize(HWND hwnd) {
    // 创建 DXGI 工厂
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) throw std::runtime_error("Failed to create DXGI Factory");

    // 创建 D3D12 设备
    hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (FAILED(hr)) throw std::runtime_error("Failed to create D3D12 Device");
}
