
#include "pmfTableWdgt.h"
#include <QMimeData>
#include <QMessageBox>
#include "tabEdit.h"

void PmfTableWdgt::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        QTableWidgetItem * pItem = this->itemAt(event->pos());
        myTabEdit->setActionsMenu(pItem);
        if( pItem )
        {
//            QItemSelectionModel* selectionModel = myTabEdit->mainWdgt->selectionModel();
//            QModelIndexList selected = helper::getSelectedRows(selectionModel);

            //if( selected.count() <= 1 ) myTabEdit->mainWdgt->setCurrentItem(pItem);
        //    myTabEdit->setActionsMenu(pItem);
        }
    }
    else QTableWidget::mousePressEvent(event);
}

void PmfTableWdgt::keyPressEvent(QKeyEvent *event)
{
    //myTabEdit->deb(__FUNCTION__, "evt");

    if(event->modifiers() & Qt::ControlModifier)
    {
        if( event->key() == Qt::Key_End )
        {
            clearSelection();
            if( rowCount() > 0 ) selectRow(rowCount()-1);
        }
        else if( event->key() == Qt::Key_Home )
        {
            clearSelection();
            if( rowCount() > 0 )
            {
                selectRow(0);
                setCurrentItem(item(0, 0), QItemSelectionModel::Select);
            }

        }
    }

    if( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
        if( myTabEdit->isChecked(_ENTERTOSAVE) ) myTabEdit->chgClick();
        return; //Do not propagate return, it will reload the table. Successful ::chgClick will reload anyway.
    }
    //Todo: pmf.cpp should catch this. Works only if mainWdgt has focus.
    if(event->modifiers() & Qt::AltModifier )
    {
        myTabEdit->deb("alt, key: "+GString(event->key()));
        if( event->key() == Qt::Key_Right ) myTabEdit->slotForward();
        else if( event->key() == Qt::Key_Left ) myTabEdit->slotBack();
    }

    QTableWidget::keyPressEvent(event);
}

void PmfTableWdgt::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
void PmfTableWdgt::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}
void PmfTableWdgt::dropEvent(QDropEvent *event)
{
    const QMimeData* pMimeData = event->mimeData();
    QList< QUrl > urlList = pMimeData->urls();
    QTableWidgetItem * pItem = itemAt(event->pos());
    if( !pItem ) return;

    if ( !pMimeData->hasUrls() )
    {
        QMessageBox::information(this, "pmf", "This is not a file.");
        return;
    }
    event->acceptProposedAction();
    if( urlList.size() > 1 )
    {
        QMessageBox::information(this, "pmf", "Cannot drop more than one file.");
        return;
    }
    if( myTabEdit->colType(pItem->column()-1) == 0 )
    {
        QMessageBox::information(this, "pmf", "Column '"+myTabEdit->m_pMainDSQL->hostVariable(pItem->column()-1)+"' is not recognized as LOB or XML column.");
        return;
    }
    pItem->setText(urlList.at(0).toLocalFile());
}
