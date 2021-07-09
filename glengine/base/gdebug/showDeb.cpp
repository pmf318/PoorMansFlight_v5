
#include "showDeb.hpp"

///////////////////////////////////////////////////////
///////// Build this only when DEBUG_GUI is defined
/// ///////////////////////////////////////////////////
#ifdef DEBUG_GUI


#include <QPushButton>
#include <QKeyEvent>

#include "gdebug.hpp"
#include "gstuff.hpp"
#include <idsql.hpp>

ShowDeb::ShowDeb(QWidget *parent, GDebug *pGDeb)
{

    m_pGDebug = pGDeb;
    this->resize(600, 400);
    QBoxLayout *topLayout = new QVBoxLayout(this);

    QGridLayout * grid = new QGridLayout();
    topLayout->addLayout(grid, 2);

    ok = new QPushButton(this);
    ok->setText("Close");
    ok->setDefault(true);
    //ok->setGeometry(20, 360, 80, 30);
    connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));


    infoLB = new QTextEdit(this);
    infoLB->setReadOnly(true);


    grid->addWidget(infoLB, 0, 0, 1, 3);
    grid->addWidget(ok, 1, 1);
}
void ShowDeb::OKClicked()
{
    close();
}
void ShowDeb::closeEvent(QCloseEvent * event)
{
    PMF_UNUSED(event);
    m_pGDebug->showDebClosed();
}
void ShowDeb::keyPressEvent(QKeyEvent *event)
{
    if( event->key() == Qt::Key_Escape )  close();
}
short ShowDeb::addToBox(GString txt)
{
    //GString dateStamp = QDate::currentDate().toString("yyyy.MM.dd");
    infoLB->moveCursor (QTextCursor::End);
    infoLB->insertPlainText ("["+GStuff::GetTime()+"] "+ txt+"\n");
    return 0;
    infoLB->moveCursor (QTextCursor::End);
    infoLB->insertPlainText (txt);
    infoLB->moveCursor (QTextCursor::End);

    return 0;
}
void ShowDeb::removeItems()
{
    /*
    QListWidgetItem* item;

    while(infoLB->count() > 100 )
    {
        item = infoLB->takeItem(0);
        delete item;
    }
    */
}
#endif
