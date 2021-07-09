#ifndef PMFTABLEWDGT_H
#define PMFTABLEWDGT_H


#include <QComboBox>
#include <QTableWidget>
#include <QMenuBar>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolTip>
#include <QGridLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QClipboard>
#include <QWidgetAction>
#include <QLineEdit>


#include <QTableWidget>


class TabEdit;
/******************* Subclassed QTableWidget******************/
class PmfTableWdgt : public QTableWidget
{
    public:
        //PMF_QTableWidget( QWidget * parent = 0 ) : QTableWidget(parent){myTabEdit = (tabEdit*)parent;}

    public:
    explicit PmfTableWdgt(QWidget* parent = 0) : QTableWidget(parent)
    {
        myTabEdit = (TabEdit*)parent;
        //connect(this->itemDelegate(), SIGNAL(closeEditor(QWidget*)), this, SLOT(editMultipleItems(QWidget*)));
        //connect(this,  SIGNAL(itemChanged(QTableWidgetItem*)), this,  SLOT(editMultipleItems(QTableWidgetItem*)));


    }
    private:
        TabEdit * myTabEdit;

    protected:
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void mousePressEvent(QMouseEvent *event);


    public slots:

    //void  editMultipleItems(QTableWidgetItem* item);
};

#endif // PMFTABLEWDGT_H
