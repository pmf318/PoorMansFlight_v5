//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "threadBox.h"
#include <idsql.hpp>

#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qlayout.h>

//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QKeyEvent>
#include <QCloseEvent>
#include <dbapiplugin.hpp>

//Need TRUE in QDialog to make it fully modal
ThreadBox::ThreadBox(QWidget *parent, const char* name, const char* procName, const char* dbTypeName)
//  :QDialog(parent, name, TRUE )
    :QDialog(parent) /////, "info", TRUE, Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_ContextHelp)
{

    this->setWindowTitle(name);
    m_dbTypeName = dbTypeName;

    QBoxLayout *topLayout = new QVBoxLayout(this);
    QGridLayout * grid = new QGridLayout();;
    topLayout->addLayout(grid, 2);

    seconds = 0;
    time = new QTime(0,0,0);

    this->resize(300,100);
    //this->setCaption("Please wait...");


    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );
    m_timer->start( 1000 );

    GString msg = "Process: "+GString(procName);
    QLabel * lb = new QLabel(msg, this);
    lb->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    lb->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    grid->addWidget(lb, 0,1);


    msg = "You should not try to interrupt this process.\nEven terminating PMF will (sometimes) not stop this process,"
          "\nbecause it is running on the database-server.\n\n"
          "If you absolutely MUST interrupt this process, hit ESC\n"
          "PMF will send an interrupt request and try a graceful exit.";

    QLabel * lb2 = new QLabel(msg, this);
    grid->addWidget(lb2, 1,1);

    infoText = new QLabel("Time expired: "+time->toString(), this);
    infoText->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    infoText->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    grid->addWidget(infoText, 3,1);
    
    m_pThread = NULL;
    

    //    closeB = new QPushButton(bg, "OK");
    /*
    closeB->setText("Exit");
    closeB->setFixedHeight( closeB->sizeHint().height());
    closeB->setFixedWidth( 250 );
    closeB->disable();
    connect(closeB, SIGNAL(clicked()), SLOT(callExit()));
*/


}

void ThreadBox::timerEvent()
{
    QTime t = time->addSecs( ++seconds );
    GString txt = "Time expired: "+GString(t.toString());
    infoText->setText(txt);

    if( m_pThread )
    {
        if( !m_pThread->isAlive() )
        {
            if( m_timer)
            {
                m_timer->stop();
                delete m_timer;
            }
            //Box gets closed by calling application
            //close();
        }
    }
}

ThreadBox::~ThreadBox()
{
    return;
    if( m_timer)
    {
        m_timer->stop();
        delete m_timer;
    }
}

void ThreadBox::callExit()
{
    close();
}

void ThreadBox::enableClose()
{
    //   closeB->enable();

}

void ThreadBox::keyPressEvent(QKeyEvent * key)
{
    //if( key->key() == Key::Key_Escape ) close();
    if( key->key() == Qt::Key_Escape )
    {
        if( m_dbTypeName.length() )
        {
            DBAPIPlugin * pApi = new DBAPIPlugin(m_dbTypeName);
            if( pApi && pApi->isOK() )
            {
                pApi->stopReq();
                delete pApi;
            }
        }
        m_pThread->stop();
        close();
    }
}

void ThreadBox::closeEvent(QCloseEvent * event)
{
    PMF_UNUSED(event);

    /*
  if( GString(name()) == GString("ALIVE") )
  {
     GString txt = "Sending interrupt request.\n"
                   "Continue?";
     if( QMessageBox::information(this, "PMF", txt, "Yes", "No", 0, 0, 1) ) return;
  }
*/
    /*
  * threadBox should be destroyed ONLY by GThread, not by ESC or ALT+F4.
  * Therefore, "ALIVE" is set before GThread starts execution, and GThread sets
  * name() to "DEAD" before trying to kill threadBox.
  */
    /*
  * Changed my mind.
  */
    //We're trying to interrupt the running process...
    /*TODO
  dsqlapi aAPI;
  aAPI.stopReq();
  event->accept();
  */
}

void ThreadBox::setThread(GThread * pThread)
{
    m_pThread = pThread;
}
