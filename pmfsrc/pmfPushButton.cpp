//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)
//


#include "pmfPushButton.h"
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>

void PmfPushButton::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
void PmfPushButton::dragMoveEvent(QDragMoveEvent *event)
{    
    event->accept();
}
void PmfPushButton::dropEvent(QDropEvent *event)
{
    const QMimeData* pMimeData = event->mimeData();
    QList< QUrl > urlList = pMimeData->urls();

    if ( !pMimeData->hasUrls() )
    {
        QMessageBox::information(this, "pmf", "This is not a file.");
        return;
    }
    event->acceptProposedAction();
//    if( urlList.size() > 1 )
//    {
//        QMessageBox::information(this, "pmf", "Cannot process more than one file at a time.");
//        return;
//    }
    emit fileListWasDropped(event);
}
