#ifndef FBLMODEL_H
#define FBLMODEL_H

#include <onnxruntime_cxx_api.h>
#include <string>
#include <vector>
namespace FBL {

    enum ExecutionProvider {
        EP_CPU = 1,
        EP_DirectML = 2,
        EP_CUDA = 3,
    };

    class FblModel {
    public:
        FblModel();
        ~FblModel();
        bool load(const std::string &model_path, ExecutionProvider ep, int deviceIndex, std::string &msg);
        void unload();
        bool forward(const std::vector<std::vector<float>> &input_data, std::vector<float> &result,
                     std::string &msg);
        bool isLoaded() const;

    private:
        Ort::Env m_env;
        Ort::Session m_session;
        Ort::AllocatorWithDefaultOptions m_allocator;
        const char *m_input_name;
        const char *m_output_name;

#ifdef _WIN_X86
        Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
#else
        Ort::MemoryInfo m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
#endif
    };

} // FBL

#endif // FBLMODEL_H
