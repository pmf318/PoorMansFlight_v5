#ifndef _SHOWDEB_
#define _SHOWDEB_


///////////////////////////////////////////////////////
///////// Build this only when DEBUG_GUI is defined
/// ///////////////////////////////////////////////////
#ifdef DEBUG_GUI

#include <QApplication>
#include <QDialog>
#include <QTextEdit>
#include <QBoxLayout>
#include <gstring.hpp>

class GDebug;

class ShowDeb : public QDialog
{
    Q_OBJECT
public:
    ShowDeb(QWidget *parent, GDebug *pGDeb);
    ~ShowDeb(){}
    short addToBox(GString txt);
    void removeItems();

private slots:
    void OKClicked();
private:
    QPushButton * ok;
    QTextEdit* infoLB;
    GDebug * m_pGDebug;


protected:
    void closeEvent(QCloseEvent* );
    void keyPressEvent(QKeyEvent *event);

};
#endif //define DEBUG_GUI
#endif //class

