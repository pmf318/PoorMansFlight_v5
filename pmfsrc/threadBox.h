//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2004
//

#include <qapplication.h>
#include <qpushbutton.h>
#include <qmainwindow.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qstring.h>
#include <gthread.hpp>

#include <qmessagebox.h>
#include <qwidget.h>
#include <qtimer.h>
#include <qdatetime.h>

#include <gstring.hpp>

#ifndef _threadBox_
#define _threadBox_

class ThreadBox : public QDialog
{
   Q_OBJECT
  public:
     ThreadBox(QWidget *parent, const char * Name, const char * procName, const char* dbTypeName);
     ~ThreadBox();
     void enableClose();
     void setThread(GThread * pThread);

  private:
     QLabel * infoText; 
     int seconds;
     QTime *time;
     QPushButton * closeB;
     GThread * m_pThread;
	 QTimer *m_timer;
     GString m_dbTypeName;
//     void tm(GString message){QMessageBox::information(this, "Process", (char*)message);}

  public slots:
     void timerEvent();
     void callExit();
   

  protected:
   void closeEvent( QCloseEvent *);
   void keyPressEvent(QKeyEvent * key);
};

#endif
