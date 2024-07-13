#include <QLabel>
#include "DeviceDialog.h"

namespace FBL {
    DeviceDialog::DeviceDialog(const QStringList &gpuList, QWidget *parent)
            : QDialog(parent), m_deviceIndex(0), m_useGpu(false) {

        radioButtonCpu = new QRadioButton("CPU", this);
        radioButtonGpu = new QRadioButton("GPU (DirectML)", this);
        radioGroup = new QButtonGroup(this);
        radioGroup->addButton(radioButtonCpu);
        radioGroup->addButton(radioButtonGpu);
        radioButtonCpu->setChecked(true);

        comboBox = new QComboBox(this);
        int currentGpuIndex = 0;
        for (const auto &gpu: std::as_const(gpuList)) {
            comboBox->addItem(QString("[%1] %2").arg(currentGpuIndex).arg(gpu), currentGpuIndex);
            ++currentGpuIndex;
        }
        comboBox->setEnabled(radioButtonGpu->isChecked());
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
        m_deviceIndex = index;
    }

    void DeviceDialog::onRadioButtonToggled(bool checked) {
        bool useGpu = radioButtonGpu->isChecked();
        comboBox->setEnabled(useGpu);
        m_useGpu = useGpu;
    }
}