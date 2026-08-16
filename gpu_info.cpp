#include "gpu_info.h"

#ifdef Q_OS_WIN
#include <dxgi1_4.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

GpuVramInfo queryPrimaryGpuVram()
{
    GpuVramInfo info;

    ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) || !factory)
    {
        return info;
    }

    // Pick the adapter with the most dedicated VRAM, skipping the software
    // (WARP) adapter - on a laptop with an iGPU + dGPU this picks the dGPU,
    // which is the one llama.cpp's -ngl actually offloads to.
    ComPtr<IDXGIAdapter1> bestAdapter;
    SIZE_T bestVram = 0;
    for (UINT i = 0;; ++i)
    {
        ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(i, &adapter) == DXGI_ERROR_NOT_FOUND)
        {
            break;
        }
        DXGI_ADAPTER_DESC1 desc;
        if (SUCCEEDED(adapter->GetDesc1(&desc)) && !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            && desc.DedicatedVideoMemory > bestVram)
        {
            bestVram = desc.DedicatedVideoMemory;
            bestAdapter = adapter;
        }
    }

    if (!bestAdapter)
    {
        return info;
    }

    DXGI_ADAPTER_DESC1 desc;
    if (FAILED(bestAdapter->GetDesc1(&desc)))
    {
        return info;
    }

    // IDXGIAdapter3 (needed for QueryVideoMemoryInfo) needs Windows 10 1607+;
    // older Windows just leaves info invalid and callers fall back.
    ComPtr<IDXGIAdapter3> adapter3;
    if (FAILED(bestAdapter.As(&adapter3)) || !adapter3)
    {
        return info;
    }

    DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
    if (FAILED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo)))
    {
        return info;
    }

    info.adapterName = QString::fromWCharArray(desc.Description);
    info.totalBytes = desc.DedicatedVideoMemory;
    info.freeBytes = memInfo.Budget > memInfo.CurrentUsage ? (memInfo.Budget - memInfo.CurrentUsage) : 0;
    info.valid = true;
    return info;
}

#else

GpuVramInfo queryPrimaryGpuVram()
{
    return {};
}

#endif
