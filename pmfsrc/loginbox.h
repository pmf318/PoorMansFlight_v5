//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#ifndef _LOGINBOX_
#define _LOGINBOX_

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGridLayout>
#include <gstring.hpp>
#include <dsqlplugin.hpp>

#include <gdebug.hpp>

class LoginBox : public QDialog
{
    Q_OBJECT
    GString dbName, userName, passWord, hostName, port;

public:
    LoginBox( GDebug *pGDeb, QWidget* parent = NULL );
    ~LoginBox();
    GString DBName();
    GString HostName();
    GString UserName();
    GString PassWord();
    GString Port();
    void tm(QString message){QMessageBox::information(this, "db-Connect", message);}
    void setDefaultCon();
    DSQLPlugin * getConnection();
    int checkPlugins(GString pluginPath);
    void msg(GString txt);

    void initBox();
protected slots:
    virtual void cancelClicked();
    virtual void okClicked();
    virtual void helpClicked();
    void closeEvent( QCloseEvent*);
    void keyPressEvent(QKeyEvent *event);
    void getConnData(int pos);
    void getAllDatabases(int pos);


protected:
    QComboBox* dbNameCB;
    QComboBox* dbTypeCB;
    QLineEdit* userNameLE;
    QLineEdit* passWordLE;
    QLineEdit* hostNameLE;
    QLineEdit* portLE;
    QPushButton* okB;
    QPushButton* cancelB;
    QPushButton* helpB;

private:
    void loadConnnections(GString dbType);
    int bindAndConnect(DSQLPlugin *pDSQL, GString db, GString uid, GString pwd, GString node, GString port);
    GString checkBindFile(GString bndFile);
    void runAutoCatalog();
    int haveStartupFile();



    void fillCB();
    void runPluginCheck();
    void createPluginInfo();
    int nodeNameHasChanged(GString alias, GString hostName);
    DSQLPlugin *  m_pIDSQL;
    GSeq <CON_SET*> m_seqDBList;
    void deb(GString msg);
    QGridLayout * m_pMainGrid;
    GDebug * m_pGDeb;
    QLabel * pInfoLBL;

};

#endif
