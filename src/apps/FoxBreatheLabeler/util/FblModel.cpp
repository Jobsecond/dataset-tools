#include "FblModel.h"
#include "ExecutionProvider.h"

#include <syscmdline/system.h>

namespace FBL {
    FblModel::FblModel()
        : m_env(Ort::Env(ORT_LOGGING_LEVEL_WARNING, "FblModel")),
          m_session(nullptr), m_isLoaded(false) {

        m_input_name = "waveform";
        m_output_name = "ap_probability";
    }

    FblModel::~FblModel() = default;

    bool FblModel::load(const std::string &model_path, ExecutionProvider ep, int deviceIndex, std::string &msg) {
        try {
            Ort::SessionOptions sessionOptions;
            std::string errorMessage;
            switch (ep) {
                case EP_CUDA:
                    if (!initCUDA(sessionOptions, deviceIndex, &errorMessage)) {
                        msg = "Could not load model using CUDA execution provider: " + errorMessage;
                        return false;
                    }
                    break;
                case EP_DirectML:
                    if (!initDirectML(sessionOptions, deviceIndex, &errorMessage)) {
                        msg = "Could not load model using DirectML execution provider: " + errorMessage;
                        return false;
                    }
                    break;
                default:
                    break;
            }
#ifdef _WIN32
            const std::wstring wstrPath = SysCmdLine::utf8ToWide(model_path);
            m_session = Ort::Session(m_env, wstrPath.c_str(), sessionOptions);
#else
            m_session = Ort::Session(m_env, model_path.c_str(), m_session_options);
#endif
            m_isLoaded = true;
            return true;
        } catch (const Ort::Exception &e) {
            msg = std::string("Could not load model: ") + e.what();
        }
        return false;
    }

    void FblModel::free() {
        m_session = Ort::Session(nullptr);
        m_isLoaded = false;
    }

    bool FblModel::forward(const std::vector<std::vector<float>> &input_data, std::vector<float> &result,
                           std::string &msg) {
        const size_t batch_size = input_data.size();
        if (batch_size == 0) {
            msg = "输入数据不能为空。";
            return false;
        }

        // 确定输入数据中最大的长度
        size_t max_height = 0;
        for (const auto &channel_data : input_data) {
            max_height = std::max(max_height, channel_data.size());
        }

        // 创建一个用于存放扁平化后的输入数据的向量
        std::vector<float> flattened_input(batch_size * max_height, 0.0f);

        // 将输入数据扁平化并填充到flattened_input中
        for (size_t i = 0; i < batch_size; ++i) {
            std::copy(input_data[i].begin(), input_data[i].end(), flattened_input.begin() + i * max_height);
        }

        // 定义输入张量的形状
        const std::array<int64_t, 2> input_shape_{static_cast<int64_t>(batch_size), static_cast<int64_t>(max_height)};

        // 创建输入张量
        const Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            m_memoryInfo, flattened_input.data(), flattened_input.size(), input_shape_.data(), input_shape_.size());

        try {
            auto output_tensors =
                m_session.Run(Ort::RunOptions{nullptr}, &m_input_name, &input_tensor, 1, &m_output_name, 1);

            const float *float_array = output_tensors.front().GetTensorMutableData<float>();
            result = std::vector<float>(
                float_array, float_array + output_tensors.front().GetTensorTypeAndShapeInfo().GetElementCount());
            return true;
        } catch (const Ort::Exception &e) {
            msg = "Error during model inference: " + std::string(e.what());
            return false;
        }
    }

    bool FblModel::isLoaded() const {
        return m_isLoaded;
    }
} // FBL