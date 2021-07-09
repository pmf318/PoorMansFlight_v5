#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QKeyEvent>

#include "gstring.hpp"


#ifndef _GETCODEPAGE_
#define _GETCODEPAGE_


class GetCodepage : public QDialog
{
     Q_OBJECT
public:
    GetCodepage(QWidget *parent=0);
    ~GetCodepage();
    GString getValue();
    void setValue(GString data);

private slots:
    void ok();
    void cancel();

private:
    QLineEdit *codepageLE;
    QPushButton * okBT;
    QPushButton * escBT;
    GString m_strData;
    void keyPressEvent(QKeyEvent * key);

};

#endif
