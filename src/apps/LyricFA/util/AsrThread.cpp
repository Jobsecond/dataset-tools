#include "AsrThread.h"

#include <QApplication>
#include <QMSystem.h>
#include <QMessageBox>

namespace LyricFA {
    AsrThread::AsrThread(Asr *asr, QString filename, QString wavPath, QString labPath,
                         const QSharedPointer<IKg2p::MandarinG2p> &g2p)
        : m_asr(asr), m_filename(std::move(filename)), m_wavPath(std::move(wavPath)), m_labPath(std::move(labPath)),
          m_g2p(g2p) {
    }

    void AsrThread::run() {
        QString asrMsg;
        const auto asrRes = m_asr->recognize(m_wavPath, asrMsg);

        if (!asrRes) {
            Q_EMIT this->oneFailed(m_filename, asrMsg);
            return;
        }

        QFile labFile(m_labPath);
        if (!labFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr, QApplication::applicationName(),
                                  QString("Failed to write to file %1").arg(QMFs::PathFindFileName(m_labPath)));
            return;
        }

        QTextStream labIn(&labFile);
        labIn.setCodec(QTextCodec::codecForName("UTF-8"));
        if (m_g2p) {
            const auto g2pRes = m_g2p->hanziToPinyin(asrMsg, false, false);
            asrMsg = m_g2p->resToStringList(g2pRes).join(" ");
        }

        labIn << asrMsg;
        labFile.close();
        Q_EMIT this->oneFinished(m_filename, asrMsg);
    }
}