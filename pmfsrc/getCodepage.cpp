#include "getCodepage.h"
#include <QFont>
#include <QButtonGroup>
#include <QDialogButtonBox>

GetCodepage::GetCodepage(QWidget *parent)  :QDialog(parent)
{
    QGridLayout * grid = new QGridLayout(this);

    QLabel * hint1 = new QLabel("This file has probably not the correct codepage.", this);
    QLabel * hint2 = new QLabel("Suggested codepage", this);

    int j = 0;
    codepageLE = new QLineEdit(this);
    grid->addWidget(hint1, 0, 0, 1, 4);
    grid->addWidget(hint2, 1, 0, 1, 1);
    grid->addWidget(codepageLE, 1, 1, 1, 3);
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));


    grid->addWidget(buttonBox, 2, 0, 1, 4);
}

GetCodepage::~GetCodepage()
{

}

GString GetCodepage::getValue()
{
    return m_strData;
}

void GetCodepage::setValue(GString data)
{
    codepageLE->setText(data);
}


void GetCodepage::ok()
{
    m_strData = codepageLE->text();
    close();
}

void GetCodepage::cancel()
{
    m_strData = "";
    close();
}

void GetCodepage::keyPressEvent(QKeyEvent * key)
{
    if( key->key() == Qt::Key_Escape )m_strData = "";
    if( key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return)m_strData = codepageLE->text();
    close();
}

