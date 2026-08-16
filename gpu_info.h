#ifndef GPU_INFO_H
#define GPU_INFO_H
#include <QString>
#include <cstdint>

struct GpuVramInfo
{
    bool valid = false;
    QString adapterName;
    quint64 totalBytes = 0;
    // Budget minus what's already in use right now - what we could actually
    // claim, not just the card's rated capacity.
    quint64 freeBytes = 0;
};

// Queries the VRAM budget of the most capable local GPU (largest dedicated
// VRAM, skipping software/WARP adapters) via DXGI - the same unprivileged,
// vendor-neutral API Task Manager's GPU tab uses. Works without local admin
// rights. Returns valid=false if there's no GPU, the OS is too old to
// support IDXGIAdapter3 (pre-Windows 10), or (on non-Windows builds) always.
GpuVramInfo queryPrimaryGpuVram();

#endif // GPU_INFO_H
