#include "DmlUtil.h"

#include <QString>

#ifdef _WIN32
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
#endif

namespace FBL {
    QStringList getDirectXGPUs() {
#ifdef _WIN32
        QStringList gpuList;

        // Create DXGI factory
        ComPtr<IDXGIFactory6> dxgiFactory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
            // "Failed to create DXGI factory."
            return gpuList;
        }

        // Enumerate adapters
        ComPtr<IDXGIAdapter1> adapter;
        for (UINT adapterIndex = 0;
             dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND;
             ++adapterIndex) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Skip software adapters
                continue;
            }

            gpuList.push_back(QString::fromWCharArray(desc.Description));
        }
        return gpuList;
#else
        return {};
#endif
    }
}
