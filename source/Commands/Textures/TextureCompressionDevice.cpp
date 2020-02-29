/* Copyright (C) 2019 G'k
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "TextureCompressionDevice.hpp"

namespace CAO {
TextureCompressionDevice::TextureCompressionDevice()
{
    PLOG_WARNING_IF(!createDevice(0, _pDevice.GetAddressOf()))
        << "DirectCompute is not available, using BC6H / BC7 CPU codec."
           " Textures compression will be slower";
}

ID3D11Device *TextureCompressionDevice::getDevice() const
{
    return _pDevice.Get();
}

bool TextureCompressionDevice::isValid() const
{
    return static_cast<bool>(_pDevice);
}

TextureCompressionDevice::operator bool() const
{
    return isValid();
}

bool TextureCompressionDevice::getDXGIFactory(IDXGIFactory1 **pFactory) const
{
    if (!pFactory)
        return false;

    *pFactory = nullptr;

    typedef HRESULT(WINAPI * pfn_CreateDXGIFactory1)(REFIID riid, _Out_ void **ppFactory);

    static pfn_CreateDXGIFactory1 sCreateDXGIFactory1 = nullptr;

    if (!sCreateDXGIFactory1)
    {
        const HMODULE hModDXGI = LoadLibraryW(L"dxgi.dll");
        if (!hModDXGI)
            return false;

        sCreateDXGIFactory1 = reinterpret_cast<pfn_CreateDXGIFactory1>(
            reinterpret_cast<void *>(GetProcAddress(hModDXGI, "CreateDXGIFactory1")));
        if (!sCreateDXGIFactory1)
            return false;
    }

    return SUCCEEDED(sCreateDXGIFactory1(IID_PPV_ARGS(pFactory)));
}

bool TextureCompressionDevice::createDevice(const int adapter, ID3D11Device **pDevice) const
{
    if (!pDevice)
        return false;

    *pDevice = nullptr;

    static PFN_D3D11_CREATE_DEVICE s_DynamicD3D11CreateDevice = nullptr;

    if (!s_DynamicD3D11CreateDevice)
    {
        const HMODULE hModD3D11 = LoadLibraryW(L"d3d11.dll");
        if (!hModD3D11)
            return false;

        s_DynamicD3D11CreateDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(
            reinterpret_cast<void *>(GetProcAddress(hModD3D11, "D3D11CreateDevice")));
        if (!s_DynamicD3D11CreateDevice)
            return false;
    }

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    const UINT createDeviceFlags = 0;

    Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
    if (adapter >= 0)
    {
        Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
        if (getDXGIFactory(dxgiFactory.GetAddressOf()))
        {
            if (FAILED(dxgiFactory->EnumAdapters(static_cast<uint>(adapter), pAdapter.GetAddressOf())))
            {
                PLOG_ERROR << "ERROR: Invalid GPU adapter index: " << adapter;
                return false;
            }
        }
    }

    D3D_FEATURE_LEVEL fl;
    HRESULT hr = s_DynamicD3D11CreateDevice(pAdapter.Get(),
                                            (pAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                                            nullptr,
                                            createDeviceFlags,
                                            featureLevels,
                                            _countof(featureLevels),
                                            D3D11_SDK_VERSION,
                                            pDevice,
                                            &fl,
                                            nullptr);
    if (SUCCEEDED(hr))
    {
        if (fl < D3D_FEATURE_LEVEL_11_0)
        {
            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts;
            hr = (*pDevice)->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
            if (FAILED(hr))
                memset(&hwopts, 0, sizeof(hwopts));

            if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
            {
                if (*pDevice)
                {
                    (*pDevice)->Release();
                    *pDevice = nullptr;
                }
                hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
        hr = (*pDevice)->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));

        return SUCCEEDED(hr);
    }
    return false;
}
} // namespace CAO