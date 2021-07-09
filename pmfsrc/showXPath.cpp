#include "showXPath.h"
#include "pmfdefines.h"
#include "userInput.h"

#include <QLabel>


ShowXPath::ShowXPath(GDebug* pGDeb, ShowXML *parent, TabEdit *pTabEdit)  :QDialog(parent)
{
    m_pGDeb = pGDeb;
    m_Master = parent;
    m_pTabEdit = pTabEdit;

    QBoxLayout *topLayout = new QVBoxLayout(this);
    QGridLayout * grid = new QGridLayout( );
    topLayout->addLayout(grid, 2);
    this->resize(840, 480);
    this->setWindowTitle("Show XPath");

    QLabel * pLB = new QLabel(this);
    //pLB->setText("XPath");

    saveBt = new QPushButton(this);
    saveBt->setText("Save");
    connect(saveBt, SIGNAL(clicked()), SLOT(saveClicked()));

    exitBt = new QPushButton(this);
    exitBt->setText("Exit");
    connect(exitBt, SIGNAL(clicked()), SLOT(exitClicked()));

    m_pTxtEdit = new TxtEdit(pGDeb, this);
    m_pTabEdit->initTxtEditor(m_pTxtEdit);


    m_pTxtEdit ->setGeometry( 20, 20, 920, 300);
    /*** Add to grid ***/
    grid->addWidget(pLB, 0, 0, 1, 5);

    grid->addWidget(m_pTxtEdit, 3, 0, 1, 5);

    grid->addWidget(saveBt, 4, 0);
    grid->addWidget(exitBt, 4, 1);


}

void ShowXPath::setText(GString text)
{
    m_pTxtEdit->setText(text);
}

void ShowXPath::saveClicked()
{
    QString name = QFileDialog::getSaveFileName(this, "Save", extSqlSaveDir());
    if(GString(name).length() == 0) return;
    QFile f(name);
    if( !f.open(QIODevice::Unbuffered | QIODevice::WriteOnly | QIODevice::Append))
    {
        msg("Could not open File "+GString(name)+", permission denied.");
        return;
    }
    f.write(GString(m_pTxtEdit->toPlainText()));
    f.close();

}

void ShowXPath::exitClicked()
{
    close();
}

void ShowXPath::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            exitClicked();
            break;
        default:
            QWidget::keyPressEvent(event);

    }
}
