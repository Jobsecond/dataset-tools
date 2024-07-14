#include <QLabel>
#include <QLocale>
#include "DeviceDialog.h"

namespace FBL {
    DeviceDialog::DeviceDialog(const QList<GPUInfo> &gpuList, QWidget *parent)
            : QDialog(parent), m_deviceIndex(0), m_useGpu(false) {

        radioButtonCpu = new QRadioButton("CPU", this);
        radioButtonGpu = new QRadioButton("GPU (DirectML)", this);
        radioGroup = new QButtonGroup(this);
        radioGroup->addButton(radioButtonCpu);
        radioGroup->addButton(radioButtonGpu);

        comboBox = new QComboBox(this);

        if (!gpuList.isEmpty()) {
            int defaultComboBoxIndex = 0, currentComboBoxIndex = 0;
            size_t maxMemory = 0;
            auto locale = QLocale::system();
            for (const auto &gpu: std::as_const(gpuList)) {
                auto gpuMemoryInMiB = locale.toString(static_cast<double>(gpu.memory) / (1024.0 * 1024.0), 'f', 0);
                comboBox->addItem(QString("[%1] %2 (%3 MiB)").arg(gpu.index).arg(gpu.name).
                        arg(gpuMemoryInMiB), gpu.index);
                if (gpu.memory > maxMemory) {
                    defaultComboBoxIndex = currentComboBoxIndex;
                    maxMemory = gpu.memory;
                }
                ++currentComboBoxIndex;
            }
            radioButtonGpu->setChecked(true);
            comboBox->setCurrentIndex(defaultComboBoxIndex);
            onItemSelected(defaultComboBoxIndex);
            onRadioButtonToggled(true);
        } else {
            radioButtonGpu->setEnabled(false);
            radioButtonCpu->setChecked(true);
            onRadioButtonToggled(false);
        }
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);

        connect(buttonBox, &QDialogButtonBox::accepted, this, &DeviceDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &DeviceDialog::reject);
        connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, &DeviceDialog::onItemSelected);
        connect(radioButtonCpu, &QRadioButton::toggled, this, &DeviceDialog::onRadioButtonToggled);
        connect(radioButtonGpu, &QRadioButton::toggled, this, &DeviceDialog::onRadioButtonToggled);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel("Which device do you want to run the model on?"));
        layout->addWidget(radioButtonCpu);
        layout->addWidget(radioButtonGpu);
        layout->addWidget(new QLabel("Choose GPU from list:"));
        layout->addWidget(comboBox);
        layout->addWidget(buttonBox);

        setLayout(layout);
    }

    bool DeviceDialog::useGpu() const {
        return m_useGpu;
    }

    int DeviceDialog::deviceIndex() const {
        return m_deviceIndex;
    }

    void DeviceDialog::onItemSelected(int index) {
        m_deviceIndex = comboBox->itemData(index).toInt();
    }

    void DeviceDialog::onRadioButtonToggled(bool checked) {
        bool useGpu = radioButtonGpu->isChecked();
        comboBox->setEnabled(useGpu);
        m_useGpu = useGpu;
    }
}