#ifndef DEVICEDIALOG_H
#define DEVICEDIALOG_H

#include <QButtonGroup>
#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QVBoxLayout>

#include "util/DmlUtil.h"

namespace FBL {
    class DeviceDialog : public QDialog {
    Q_OBJECT
    public:
        explicit DeviceDialog(const QList<GPUInfo> &items, QWidget *parent = nullptr);

        ~DeviceDialog() = default;

        bool useGpu() const;

        int deviceIndex() const;

    private Q_SLOTS:

        void onItemSelected(int index);
        void onRadioButtonToggled(bool checked);

    private:
        QRadioButton *radioButtonCpu;
        QRadioButton *radioButtonGpu;
        QButtonGroup *radioGroup;
        QComboBox *comboBox;
        QDialogButtonBox *buttonBox;
        int m_deviceIndex;
        bool m_useGpu;
    };
}

#endif