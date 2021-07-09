//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "monExport.h"
#include "gstuff.hpp"
#include "pmfdefines.h"
#include "helper.h"
#include "gfile.hpp"

#include "gfile.hpp"
#include "helper.h"


#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QTime>
#include <QDate>


MonExport::MonExport(DSQLPlugin * pDSQL, GString monitorName, QTableWidget *pTabWdgt, QWidget *parent)
    :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    int buttonWidth = 100;

    this->resize(400, 100);
    QBoxLayout *topLayout = new QVBoxLayout(this);

    QGridLayout * grid = new QGridLayout();
    topLayout->addLayout(grid);

    m_gstrMonitorName = monitorName.upperCase();
    m_pDSQL = pDSQL;
    m_pTabWdgt = pTabWdgt;

    okBt = new QPushButton("OK", this);
    okBt->setDefault(true);
    connect(okBt, SIGNAL(clicked()), SLOT(okClicked()));
    okBt->setMaximumWidth(buttonWidth);
    okBt->setMinimumWidth(buttonWidth);

    escBt = new QPushButton("Cancel", this);
    escBt->setMaximumWidth(buttonWidth);
    escBt->setMinimumWidth(buttonWidth);

    connect(escBt, SIGNAL(clicked()), SLOT(escClicked()));

    asXmlRB = new QRadioButton("As XML file", this);
    asDelRB = new QRadioButton("As DEL file", this);
    toTableRB= new QRadioButton("Insert into table", this);
    tabNameLE = new QLineEdit(this);

    connect(asXmlRB, SIGNAL(clicked()), SLOT(radioBtClicked()));
    connect(asDelRB, SIGNAL(clicked()), SLOT(radioBtClicked()));
    connect(toTableRB, SIGNAL(clicked()), SLOT(radioBtClicked()));

    int row = 0;
    grid->addWidget(new QLabel("Export current monitor data"), row, 0, 1, 2);
    row++;

    grid->addWidget(asDelRB, row, 0);
    row++;
    grid->addWidget(asXmlRB, row, 0);
    row++;
    grid->addWidget(toTableRB, row, 0);
    grid->addWidget(tabNameLE, row, 1);
    row++;


    QGridLayout * buttonGrid = new QGridLayout();
    topLayout->addLayout(buttonGrid);
    buttonGrid->addWidget(okBt, 0, 0);
    buttonGrid->addWidget(escBt, 0, 1);

    QSettings settings(_CFG_DIR, "pmf5");
    QString tabName = settings.value("monTab_"+m_gstrMonitorName, QString((char*)("PMF_MON."+m_gstrMonitorName))).toString();

    tabNameLE->setText(tabName);
    asDelRB->setChecked(true);
    tabNameLE->setEnabled(false);

}

MonExport::~MonExport()
{
}

void MonExport::okClicked()
{
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue("monTab_"+m_gstrMonitorName, tabNameLE->text());

    if( asDelRB->isChecked() ) exportToDel();
    else if( toTableRB->isChecked() ) exportToTable();
    else if( asXmlRB->isChecked() ) exportToXml();
    close();
}

int MonExport::exportToTable()
{
    PmfTable table(m_pDSQL, tabNameLE->text());
    if( table.columnCount() == 0 )
    {
        if( QMessageBox::question(this, "PMF", "Table "+tabNameLE->text()+" does not exist. Create it?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return 1;

        GString err = m_pDSQL->initAll(createStmt());
        if( err.length() )
        {
            msg(err);
            return 1;
        }
        table.reInit();
    }

    for( int i = 0; i < m_pTabWdgt->rowCount(); ++i )
    {

        if( insertIntoTable(i, &table) ) break;
    }

    return 0;
}

int MonExport::insertIntoTable(int row, PmfTable * table)
{
    GString val;
    GString cmd = "Insert into "+tabNameLE->text()+" (";
    for( int i = 1; i < table->columnCount(); ++i ) //Last Col is TIMESTAMP
    {
        cmd += GString(table->column(i)->colName())+",";
    }
    cmd = cmd.stripTrailing(",")+") VALUES (";

    for( int i = 0; i < m_pTabWdgt->columnCount(); ++ i )
    {
        val = m_pTabWdgt->item(row, i)->text();

        if( table->column(i+1)->colType() == "VARCHAR" )
        {
            val = "'"+val+"'";
            m_pDSQL->convToSQL(val);
            cmd += val+",";
        }
        else cmd += val+",";
    }
    cmd = cmd.stripTrailing(",")+")";
    GString err = m_pDSQL->initAll(cmd);
    if( err.length() )
    {
        msg(err);
        return 1;
    }
    return 0;
}

void MonExport::radioBtClicked()
{
    if(toTableRB->isChecked()) tabNameLE->setEnabled(true);
    else tabNameLE->setEnabled(false);
}


GString MonExport::columnDdl(int col)
{
    QTableWidgetItem *pItem = m_pTabWdgt->horizontalHeaderItem(col);
    if( !pItem ) return "";
    GString colDddl = GString(pItem->text()).upperCase();
    colDddl = colDddl.removeAll(' ').removeAll(')').removeAll('(').change("/", "Per");
    if( colDddl == "SQLSTATEMENT") return colDddl + " VARCHAR(3000)";

    GString type = "INTEGER";
    for(int i = 0; i < m_pTabWdgt->rowCount(); ++i )
    {
        if( !GString(m_pTabWdgt->item(i, col)->text()).isDigits() )
        {
            type = "VARCHAR(200)";
            break;
        }
    }
    return colDddl + " "+type;
}

void MonExport::exportAsXml(GString fileName)
{
    remove(fileName);
    QFile *outFile = new QFile(fileName);
    outFile->open(QFile::ReadWrite | QIODevice::Text);
    GString data, col, out;
    writeToUtf8File(outFile, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>", 1);
    writeToUtf8File(outFile, "<Snapshot>");
    for ( int i = 0; i < m_pTabWdgt->rowCount(); ++i)
    {
        out = "\t<"+m_gstrMonitorName+" ";
        for ( int j = 0; j < m_pTabWdgt->columnCount(); ++j)
        {
            col  = m_pTabWdgt->horizontalHeaderItem(j)->text();
            col = col.removeAll(' ').removeAll(')').removeAll('(').change("/", "Per");
            data = m_pTabWdgt->item(i, j)->text();
            out += col+"=\""+GStuff::formatForXml(data).removeButOne()+"\" ";
        }
        out += "/>";
        writeToUtf8File(outFile, out);
    }
    writeToUtf8File(outFile, "</Snapshot>");
    outFile->close();
}

void MonExport::writeToUtf8File(QFile* fileOut, QString txt, int setBOM)
{
    QTextStream streamFileOut(fileOut);
    //This should be called only once, on the first line/entry
    streamFileOut.setGenerateByteOrderMark(setBOM == 1 ? true : false);
    streamFileOut.setCodec("UTF-8");
    streamFileOut << txt <<"\n";
    streamFileOut.flush();
}

void MonExport::exportAsDel(GString fileName)
{
    GFile gf(fileName, 2);
    GString data, out;
    for ( int i = 0; i < m_pTabWdgt->rowCount(); ++i)
    {
        out = "";
        for ( int j = 0; j < m_pTabWdgt->columnCount(); ++j)
        {
            data = m_pTabWdgt->item(i, j)->text();
            out += "\""+data.removeAll('\"')+"\",";
        }
        gf.writeLine(out.stripTrailing(",")+"\n") ;
    }
}

GString MonExport::createStmt(int asDddl)
{
    GString comment;
    if( asDddl ) comment = "-- ";

    GString cmd = comment+"Create table "+GString(tabNameLE->text())+"(\n";
    for( int i = 0; i < m_pTabWdgt->columnCount(); ++i )
    {
        cmd += comment+columnDdl(i)+",\n";
    }
    return cmd+comment+" INS_TIME TIMESTAMP DEFAULT CURRENT TIMESTAMP)";
}

GString MonExport::getFileName(GString extension)
{
    QSettings settings(_CFG_DIR, "pmf5");
    GString prevPath = settings.value("snapExport", "").toString();;
    QString name = QFileDialog::getSaveFileName ( this, "Select file",  prevPath+m_gstrMonitorName+"."+extension, "*",  0 );
    if( name.isNull() || !name.length() ) return "";
    QString path = GStuff::pathFromFullPath(name);
    settings.setValue("snapExport", path);
    return name;
}

int MonExport::exportToDel()
{
    GString fileName = getFileName("DEL");
    if( !fileName.length() ) return 1;

    GFile gf(fileName, 2);
    GString data, out;
    for ( int i = 0; i < m_pTabWdgt->rowCount(); ++i)
    {
        out = "";
        for ( int j = 0; j < m_pTabWdgt->columnCount(); ++j)
        {
            data = m_pTabWdgt->item(i, j)->text();
            out += "\""+data.removeAll('\"')+"\",";
        }
        gf.writeLine(out.stripTrailing(",")) ;
    }
    return 0;
}

int MonExport::exportToXml()
{
    GString fileName = getFileName("XML");
    if( !fileName.length() ) return 1;

    remove(fileName);
    QFile *outFile = new QFile(fileName);
    outFile->open(QFile::ReadWrite | QIODevice::Text);
    GString data, col, out;
    writeToUtf8File(outFile, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>", 1);
    writeToUtf8File(outFile, "<Snapshot>");
    for ( int i = 0; i < m_pTabWdgt->rowCount(); ++i)
    {
        out = "\t<"+m_gstrMonitorName+" ";
        for ( int j = 0; j < m_pTabWdgt->columnCount(); ++j)
        {
            col  = m_pTabWdgt->horizontalHeaderItem(j)->text();
            col = col.removeAll(' ').removeAll(')').removeAll('(').change("/", "Per");
            data = m_pTabWdgt->item(i, j)->text();
            out += col+"=\""+GStuff::formatForXml(data).removeButOne()+"\" ";
        }
        out += "/>";
        writeToUtf8File(outFile, out);
    }
    writeToUtf8File(outFile, "</Snapshot>");
    outFile->close();
    return 0;
}

void MonExport::escClicked()
{
    close();
}


void MonExport::reject()
{
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue(m_gstrMonitorName+"Geometry", this->saveGeometry());
    QDialog::reject();
}
