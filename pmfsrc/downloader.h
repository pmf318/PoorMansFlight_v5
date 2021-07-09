#ifndef DOWNLOADER_H
#define DOWNLOADER_H
#include <QObject>
#include <QByteArray>
#include "gstring.hpp"
#include "gthread.hpp"
#include "gdebug.hpp"
#include "gsocket.hpp"


class Downloader: public QWidget
{
    Q_OBJECT

    class MyThread : public GThread
    {
        public:
            virtual void run();
            void setOwner(Downloader *pDownLd){ myDownLd = pDownLd; }
        private:
            Downloader * myDownLd;
    };

public:
    Downloader(GDebug *pGDeb = NULL, QWidget* parent = 0);
    virtual ~Downloader();
    int httpGetFile(GString srv, GString srcFile, GString trgFile);
    int getPmfSetup();
    int downloadedSize();
    void cancelDownload();
    void deb(GString msg);


signals:
    void downloadCompleted();
    //void downloadFailed();

private slots:

private:

    MyThread *pThread;
    GDebug * m_pGDeb;
    GSocket * m_pSocket;
    QWidget * m_pParent;


};
#endif // DOWNLOADER_H
