//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <QDragEnterEvent>
#include <QLineEdit>
#ifndef _GSEQ_
#include "gseq.hpp"
#endif
#ifndef _GSTRING_
#include "gstring.hpp"
#endif

#ifndef _PMFDROPZONE_H
#define _PMFDROPZONE_H

class PmfDropZone : public QLineEdit
{
    Q_OBJECT
    public:
        PmfDropZone( QWidget * parent = 0 ) : QLineEdit(parent)
        {
        }

        GSeq<GString> fileList()
        {
            return m_listFiles;
        }
        void clearFileList();


    private:
        GSeq<GString> m_listFiles;

    protected:
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
    signals:
        void fileWasDropped();

};

#endif
