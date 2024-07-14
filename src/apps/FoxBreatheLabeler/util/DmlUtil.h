#ifndef DMLUTIL_H
#define DMLUTIL_H

#include <QStringList>

namespace FBL {
    struct GPUInfo {
        int index;
        QString name;
        size_t memory;
    };

    QList<GPUInfo> getDirectXGPUs();
}
#endif