#ifndef userActions_H
#define userActions_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDialog>
#include <QMessageBox>
#include "tabEdit.h"
#include "txtEdit.h"

class UserActions : public QDialog
{
    Q_OBJECT

private:
    QPushButton * saveBt;
    QPushButton * saveAsBt;
    QPushButton * deleteBt;
    QPushButton * exitBt;
    TxtEdit * editor;
    QComboBox * listCB;
    GString m_curText;
    GString m_curFile;

    int m_iSaved;

    void keyPressEvent(QKeyEvent *event);
    GString currentFile();
    int saveToFile(GString name);
    void loadActions();
    void setHelp();
    int saveCurrentAction();
    void setComboBox(GString val);

    GDebug * m_pGDeb;
    TabEdit * m_Master;
    void msg(GString message){QMessageBox::information(this, "Editor", (char*)message);}


public:
    explicit UserActions(GDebug * pGDeb, TabEdit *parent=0);

signals:

public slots:
    void saveClicked();
    int saveAsClicked();
    void deleteClicked();
    void exitClicked();
    void actionSelected(int pos);


};

#endif // userActions_H
