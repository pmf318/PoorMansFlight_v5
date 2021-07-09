//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <QDragEnterEvent>
#include <QPushButton>
#include "gstring.hpp"


#ifndef _PMFPUSHBUTTON_H
#define _PMFPUSHBUTTON_H

class PmfPushButton : public QPushButton
{
    Q_OBJECT
    public:
        PmfPushButton( QWidget * parent = 0 ) : QPushButton(parent)
        {
            setAcceptDrops(true);
        }

    private:

    protected:
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
    signals:
        void fileListWasDropped(QDropEvent *event);

};

#endif
