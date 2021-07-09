#ifndef showXPath_H
#define showXPath_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDialog>
#include <QMessageBox>
#include "showXML.h"
#include "txtEdit.h"
#include "tabEdit.h"

class ShowXPath : public QDialog
{
    Q_OBJECT

private:
    QPushButton * saveBt;
    QPushButton * exitBt;
    TxtEdit * m_pTxtEdit;

    void keyPressEvent(QKeyEvent *event);

    GDebug * m_pGDeb;
    TabEdit * m_pTabEdit;
    ShowXML * m_Master;
    void msg(GString message){QMessageBox::information(this, "Editor", (char*)message);}


public:
    explicit ShowXPath(GDebug * pGDeb, ShowXML *parent=0, TabEdit *pTabEdit = 0);
    void setText(GString text);

signals:

public slots:
    void saveClicked();
    void exitClicked();
};

#endif // showXPath_H
