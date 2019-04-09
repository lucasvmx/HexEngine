#ifndef HEXINJECTOR_H
#define HEXINJECTOR_H

#include "hexengine.h"

namespace Engine
{
    class HexInjector : public HexEngine
        {
            Q_OBJECT

        public:
            HexInjector();
            ~HexInjector();
            void run();
            void injectFile(QString from, QString to);
            void setIOFiles(QString from, QString to);

        signals:
            void status_updated(QString text);
            void started();
            void stopped();

        private:
            QString from, to;
            bool injectFileToBatchScript(QString fileToInject, QString batchFileName);

    };
}
#endif // HEXINJECTOR_H
