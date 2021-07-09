//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)
//


#include "pmfDropZone.h"
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>

void PmfDropZone::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void PmfDropZone::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void PmfDropZone::dropEvent(QDropEvent *event)
{
    const QMimeData* pMimeData = event->mimeData();

    //m_listFiles = pMimeData->urls();

    m_listFiles.removeAll();
    if ( !pMimeData->hasUrls() )
    {
        QMessageBox::information(this, "pmf", "This is not a file.");
        return;
    }
    for( int i = 0; i < pMimeData->urls().count(); ++ i )
    {
        m_listFiles.add(pMimeData->urls().at(i).toLocalFile());
    }
    event->acceptProposedAction();
//    if( pMimeData->urls().size() > 1 )
//    {
//        QMessageBox::information(this, "pmf", "Cannot process more than one file at a time.");
//        return;
//    }
    this->setText(pMimeData->urls().at(0).toLocalFile());
    emit fileWasDropped();
}

void PmfDropZone::clearFileList()
{
    m_listFiles.removeAll();
    this->setText("");
}
