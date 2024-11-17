#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class Renderer {
public:
    void Initialize(HWND hwnd);
private:
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory;
};
