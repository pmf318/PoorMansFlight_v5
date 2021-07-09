//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#ifndef _ODBCMDF_
#define _ODBCMDF_

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGridLayout>

#include <gstring.hpp>
#include <dsqlplugin.hpp>

#include <gdebug.hpp>

class OdbcMdf : public QDialog
{
    Q_OBJECT
    GString dbName, userName, passWord, hostName, port;

public:
   OdbcMdf( GDebug *pGDeb, QWidget* parent = NULL );
   ~OdbcMdf();
   void tm(QString message){QMessageBox::information(this, "db-Connect", message);}

protected slots:
    virtual void cancelClicked();
    virtual void okClicked();
   virtual void helpClicked();
   void closeEvent( QCloseEvent*);
//   void keyPressEvent(QKeyEvent *event);


protected:
    QComboBox* driverCB;
    QLineEdit* dataSourceNameLE;
    QLineEdit* fileNameLE;
    QLineEdit* descriptionLE;
    QLineEdit* portLE;
    QPushButton* okB;
    QPushButton* cancelB;
    QPushButton* helpB;

private:    
    DSQLPlugin *  m_pIDSQL;
    GSeq <CON_SET*> m_seqDBList;
    void deb(GString msg);
    QGridLayout * m_pMainGrid;
    GDebug * m_pGDeb;
    void InstallerError();

};

#endif
