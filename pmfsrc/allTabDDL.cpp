//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "allTabDDL.h"
#include "gstuff.hpp"
#include <qlayout.h>
#include <QGroupBox>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QSettings>
#include "pmfdefines.h"


AllTabDDL::AllTabDDL(DSQLPlugin* pDSQL, QWidget *parent, GString currentSchema, int hideSysTabs )
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    this->resize(390, 500);
    this->setWindowTitle("Create DDLs");
    m_pDSQL = pDSQL;
    m_pParent = parent;

	QBoxLayout *topLayout = new QVBoxLayout(this);
	QGridLayout * grid = new QGridLayout();
    topLayout->addLayout(grid, 9);

	ok = new QPushButton(this);

    ok->setDefault(true);

	cancel = new QPushButton(this);
    ok ->setText("Create DDLs");
	ok->setFixedHeight( ok->sizeHint().height());
    cancel->setText("Close");
	cancel->setFixedHeight( cancel->sizeHint().height());
    connect(ok, SIGNAL(clicked()), SLOT(okClicked()));
    connect(cancel, SIGNAL(clicked()), SLOT(cancelClicked()));

    tbSel = new TableSelector(pDSQL, parent, currentSchema, hideSysTabs);

    QButtonGroup * formatGroup = new QButtonGroup(this);

    asTxtRB = new QRadioButton("as Text");
    asXmlRB = new QRadioButton("as XML");
    formatGroup->addButton(asTxtRB);
    formatGroup->addButton(asXmlRB);
    asTxtRB->setChecked(true);


    QGroupBox * btGroupBox = new QGroupBox();
    //btGroupBox->setTitle("Select format");
    QHBoxLayout *radioButtonLayout = new QHBoxLayout;

    radioButtonLayout->addWidget(asTxtRB);
    radioButtonLayout->addWidget(asXmlRB);

    btGroupBox->setLayout(radioButtonLayout);
    radioButtonLayout->setContentsMargins(0, 0, 0, 0);

    optionsBt = new QPushButton("Options", this);
    connect(optionsBt, SIGNAL(clicked()), SLOT(optionsClicked()));


    grid->addWidget(new QLabel("Create DDLs for all selected tables"), 0, 0, 1, 5);
    //grid->addWidget(btGroupBox, 1, 0, 1, 4);
    grid->addWidget(asTxtRB, 1, 0);
    grid->addWidget(asXmlRB, 1, 1);
    grid->addWidget(tbSel, 2, 0, 1, 5);
    grid->addWidget(ok, 3, 0);
    grid->addWidget(cancel, 3, 1);
    grid->addWidget(optionsBt, 3, 4);

    infoLE = new QLineEdit(this);
    infoLE->setEnabled(false);
    grid->addWidget(infoLE, 4, 0, 1, 5);
    infoLE->hide();
}

AllTabDDL::~AllTabDDL()
{
	delete ok;
	delete cancel;
}

int AllTabDDL::askOverwrite(GString fileName)
{
    QMessageBox msgBox;
    msgBox.setText(fileName + " already exists. Overwrite?");
    //msgBox.setInformativeText("");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::YesToAll  );
    msgBox.setDefaultButton(QMessageBox::Cancel);
    return msgBox.exec();

}

int AllTabDDL::exportToFile(GString tabName, long overwrite)
{
    int erc;
    Getclp getClp(m_pDSQL, NULL, tabName.removeAll('\"'));
    GString outFile = GStuff::formatPath(GString(m_qstrPrevDir)+"/"+tabName);

    if( asTxtRB->isChecked() ) erc = getClp.saveAsPlainText(outFile+".DDL", overwrite);
    else erc = getClp.saveAsXml(outFile+".XML", overwrite);

    return erc;
}

void AllTabDDL::optionsClicked()
{
    Getclp getClp(m_pDSQL, NULL);
    getClp.showOptionsDialog(this);
}

void AllTabDDL::okClicked()
{
    QSettings settings(_CFG_DIR, "pmf5");
    m_qstrPrevDir = settings.value("ddlExportPath", "").toString();

    QListWidget *pLB = tbSel->getTableHandle();

    long i, erc, count = 0, created = 0;
    for( i=0; i < pLB->count(); ++i)
	{
        if( pLB->isItemSelected(pLB->item(i)) ) count++;
	}
	if( !count )
	{
        QMessageBox::information(this, "PMF", "Select tables(s)");
		return;
	}
    m_qstrPrevDir = QFileDialog::getExistingDirectory(this, "Select path", m_qstrPrevDir);
    if( m_qstrPrevDir.isNull() || !m_qstrPrevDir.length() ) return ;

    this->repaint();
    GString tabName;    

    if( !overwriteFiles(pLB, m_qstrPrevDir, tbSel->tablePrefix()) ) return;

    QProgressDialog apd("Creating DDLs...", "Cancel", 0, pLB->count(), this);
    apd.setMinimumDuration(1);
    apd.setWindowModality(Qt::WindowModal);


    for( i=0; i < pLB->count(); ++i)
	{        
        if( pLB->isItemSelected(pLB->item(i)) )
		{
            if (apd.wasCanceled()) break;

            tabName = tbSel->tablePrefix()+ GString(pLB->item(i)->text());
            apd.setLabelText(tabName.removeAll('\"'));
            //setInfo("Processing "+tabName.removeAll('\"'));
            erc = exportToFile(tabName, 1);
			if( !erc )
			{
                pLB->item(i)->setSelected(false);
                created++;
            }
            pLB->scrollToItem(pLB->item(i));
            //pLB->repaint();
		}
        apd.setValue(i);
    }    
    apd.setValue(pLB->count());
    settings.setValue("ddlExportPath", m_qstrPrevDir);
    m_qstrPrevDir = settings.value("ddlExportPath", "").toString();
    setInfo("");
    msg("Done. Created "+GString(created)+" files.");

}

int AllTabDDL::overwriteFiles(QListWidget* pLB, GString path, GString schema)
{
    schema = schema.removeAll('\"');
    GString fileName;
    for( int i = 0; i < pLB->count(); ++i)
    {
        if( pLB->isItemSelected(pLB->item(i)) )
        {
            GString tabName = schema + GString(pLB->item(i)->text());
            if( asTxtRB->isChecked() ) fileName = path+"/"+tabName+".DDL";
            else fileName = path+"/"+tabName+".XML";
            QFileInfo check_file(fileName);
            if( check_file.exists() && check_file.isFile() )
            {
                if( askOverwrite(fileName) == QMessageBox::YesToAll ) return 1;
                else return 0;
            }
        }
    }
    return 1;
}

void AllTabDDL::setInfo(GString msg)
{
    if( msg.length() == 0 )
    {
        infoLE->hide();
    }
    else
    {
        infoLE->show();
        infoLE->setText(msg);
        infoLE->repaint();
    }
}

void AllTabDDL::msg(GString message)
{
    QMessageBox::information(this, "PMF", message);
}

void AllTabDDL::cancelClicked()
{
    close();
}

