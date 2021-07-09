 //
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2018
//

#include "optionsTab.h"

#include "helper.h"
#include "clickLabel.h"
#include "pmfdefines.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <gfile.hpp>
#include <gstuff.hpp>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLineEdit>



#define OPTTAB_CFG_VER_NAME "CFG_FILE_VER"
#define OPTTAB_CUR_FILE_VER 1



OptionsTab::OptionsTab(QWidget *parent, GString settingsName) :QDialog(parent)
{
    this->resize(480,180);
    this->setWindowTitle("Additional options");

    m_mainWdgt = new QTabWidget(this);
    QGridLayout * mainGrid = new QGridLayout(this);


    QPushButton * ok = new QPushButton("OK", this);
    QPushButton * cancel = new QPushButton("Cancel", this);
    QPushButton * saveSettings = new QPushButton("Save settings", this);
    QPushButton * clearSettings = new QPushButton("Restore default", this);
    connect(ok, SIGNAL(clicked()), this, SLOT(OKClicked()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(CancelClicked()));
    connect(saveSettings, SIGNAL(clicked()), this, SLOT(saveSettingsClicked()));
    connect(clearSettings, SIGNAL(clicked()), this, SLOT(clearSettingsClicked()));
    mainGrid->addWidget(m_mainWdgt, 0, 0, 1,5);
    mainGrid->addWidget(ok, 2, 0);
    mainGrid->addWidget(cancel, 2, 1);
    saveSettings->hide();
    clearSettings->hide();
    //mainGrid->addWidget(saveSettings, 2, 2);
    //mainGrid->addWidget(clearSettings, 2, 3);
    m_strSettingsFileName = settingsName+"_OptionsTabSettings";
    loadPreviousSettings();
    if( m_iCfgFileVer < OPTTAB_CUR_FILE_VER )
    {
        this->clearRows();        
    }
}

OptionsTab::~OptionsTab()
{
    while(m_rowsSeq.numberOfElements())
    {
        OPTIONS_TAB_ROW *pRow;
        pRow = m_rowsSeq.firstElement();
        m_rowsSeq.removeFirst();
        delete pRow;
    }
}

void OptionsTab::clearRows(GString groupName)
{
    OPTIONS_TAB_ROW *pRow;
    for( int i = 1; i <= m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( groupName.length() && groupName != pRow->GroupName ) continue;
        m_rowsSeq.removeAt(i);
        delete pRow;
        i--;
    }
}

void OptionsTab::saveSettingsClicked()
{
    saveSettingsForAllRows();
}

void OptionsTab::clearSettingsClicked()
{
    setRowsToDefault();
}

int OptionsTab::rowCount(GString groupName)
{
    int count = 0;
    OPTIONS_TAB_ROW * pRow;
    for( int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( groupName != pRow->GroupName ) continue;
        count++;
    }
    return count;
}

QWidget * OptionsTab::createGridWidget(GString groupName)
{

    OPTIONS_TAB_ROW * pRow;
    QWidget * pWdgt = new QWidget(this);
    QGridLayout * grid = new QGridLayout(pWdgt);

    for( int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( groupName != pRow->GroupName ) continue;
        displayRow(m_rowsSeq.elementAtPosition(i), grid, i-1);
        grid->setRowStretch(i-1, 0);
        grid->setColumnStretch(i-1, 0);
    }
    return pWdgt;
}


void OptionsTab::displayRowsInTab(GString groupName,  GString tabTitle)
{
    if( !rowCount(groupName) ) return;
    QWidget * pWdgt = createGridWidget(groupName);
    m_mainWdgt->addTab(pWdgt, tabTitle);
}

void OptionsTab::addRow(GString groupName, GString text)
{
    if( isInSeq( groupName, text) ) return;

    OPTIONS_TAB_ROW * pRow = new OPTIONS_TAB_ROW;
    pRow->GroupName = groupName;
    pRow->WdgType = ROW_COMMENT;
    pRow->Title = text;
    pRow->MaxLen = 0;
    pRow->TheWdgt = 0;
    pRow->Value = "";
    pRow->HelpText = "";
    pRow->OldValue = "";
    m_rowsSeq.add(pRow);
}

void OptionsTab::addRow(GString groupName,  GString title, int type, GString value, GString helpText, int maxLen)
{    
    if( isInSeq( groupName, title) ) return;

    OPTIONS_TAB_ROW * pRow = new OPTIONS_TAB_ROW;
    pRow->GroupName = groupName;
    pRow->WdgType = type;
    pRow->Value = value;
    pRow->HelpText = helpText;
    pRow->Title = title;
    pRow->MaxLen = maxLen;
    if( type == ROW_CHECKBOX )
    {
        QCheckBox * qCB = new QCheckBox();
        if( value.asInt() > 0 ) qCB->setChecked(true);
        else qCB->setChecked(false);
        pRow->OldValue = qCB->isChecked() ? "1" : "0";
        pRow->TheWdgt = qCB;
    }
    else if(type == ROW_LINEEDIT )
    {
        QLineEdit *pLE = new QLineEdit();

        if( maxLen > 0 )
        {
            pLE->setMaxLength(maxLen); //Usually single char
            pLE->setMaximumWidth(30);
        }
        else pLE->setMinimumWidth(200);
        pLE->setText(value);
        pRow->OldValue = value;
        pRow->TheWdgt = pLE;
    }
    else if(type == ROW_TEXTBOX )
    {
        QTextEdit *pTE = new QTextEdit();

        pTE->setMinimumWidth(200);
        pTE->setText(value);
        pRow->OldValue = value;
        pRow->TheWdgt = pTE;
    }

    m_rowsSeq.add(pRow);
}

int OptionsTab::isInSeq(GString groupName, GString title)
{
    OPTIONS_TAB_ROW * pRow = new OPTIONS_TAB_ROW;
    for( int i = 1; i <= m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( pRow->GroupName == groupName && pRow->Title == title ) return 1;
    }
    return 0;
}


void OptionsTab::displayAllRows()
{
    for(int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        displayRow(m_rowsSeq.elementAtPosition(i), m_pMainGrid, i-1);
    }
}

void OptionsTab::displayRow(OPTIONS_TAB_ROW * pRow, QGridLayout *pGrid, int row )
{
    if( pRow->WdgType == ROW_COMMENT )
    {
        pGrid->addWidget(new QLabel(pRow->Title), row, 0, 1, 4);
        return;
    }

    pGrid->addWidget(new QLabel(pRow->Title), row, 0);
    if( pRow->WdgType == ROW_TEXTBOX )
    {
        pGrid->addWidget(pRow->TheWdgt, row, 1, pRow->MaxLen, 1);
    }
    else pGrid->addWidget(pRow->TheWdgt, row, 1);
    if( pRow->MaxLen == 0 ) pGrid->addWidget(new QLabel(pRow->HelpText), row, 2, 1, 4);
    else pGrid->addWidget(new QLabel(pRow->HelpText), row, 2);
}

void OptionsTab::saveSettingsForAllRows()
{
    GString data;
    int checked;
    GSeq <GString> seqAll;
    OPTIONS_TAB_ROW * pRow;

    seqAll.add(GString(OPTTAB_CFG_VER_NAME)+"="+GString(OPTTAB_CUR_FILE_VER));
    for(int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( pRow->WdgType == ROW_LINEEDIT ) data = getFieldValue(pRow->GroupName, pRow->Title);
        else if( pRow->WdgType == ROW_TEXTBOX ) data = getTextBoxValue(pRow->GroupName, pRow->Title);
        else if( pRow->WdgType == ROW_CHECKBOX ) checked = getCheckBoxValue(pRow->GroupName, pRow->Title);
        seqAll.add(optionsrowToString(pRow));
    }
    GFile settingsFile( optionsTabSettingsDir()+m_strSettingsFileName, GF_OVERWRITE );
    settingsFile.overwrite(&seqAll);
}

int OptionsTab::settingsChanged()
{
    int checked;
    OPTIONS_TAB_ROW * pRow;
    for(int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( pRow->WdgType == ROW_COMMENT ) continue;

        if( pRow->WdgType == ROW_LINEEDIT )
        {
            if( pRow->OldValue != GString(((QLineEdit*) pRow->TheWdgt)->text()) )
            {
                return 1;
            }
        }
        else if( pRow->WdgType == ROW_CHECKBOX )
        {
            checked =((QCheckBox*) pRow->TheWdgt)->isChecked() ? 1 : 0;
            if( GString(checked) != pRow->OldValue )
            {
                printf("%s: Old: %i, New: %i\n", (char*) pRow->Title,  checked, (char*) pRow->OldValue);
                return 1;
            }
        }
        if( pRow->WdgType == ROW_TEXTBOX )
        {
            if( pRow->OldValue != GString(((QTextEdit*) pRow->TheWdgt)->toPlainText()) )
            {
                return 1;
            }
        }

    }
    return 0;
}

GString OptionsTab::optionsrowToString(OPTIONS_TAB_ROW * pRow)
{
    GString data;
    if( pRow->WdgType == ROW_LINEEDIT ) data = getFieldValue( pRow->GroupName, pRow->Title);
    else if( pRow->WdgType == ROW_CHECKBOX ) data = getCheckBoxValue( pRow->GroupName, pRow->Title) ? "1" : "0";
    else if( pRow->WdgType == ROW_TEXTBOX ) data = getTextBoxValue(pRow->GroupName, pRow->Title);
    return GString(pRow->GroupName)+"@"+pRow->Title+"@"+GString(pRow->WdgType)+"@"+
            data+"@"+GString(pRow->HelpText)+"@"+GString(pRow->MaxLen);

}

void OptionsTab::loadPreviousSettings()
{
    GFile settingsFile( optionsTabSettingsDir()+m_strSettingsFileName);
    GString input;
    OPTIONS_TAB_ROW * pRow;
    m_iCfgFileVer = 0;
    for(int i = 1; i <= settingsFile.lines(); ++i )
    {
        pRow = new OPTIONS_TAB_ROW;
        input = settingsFile.getLine(i);
        if( input.occurrencesOf(OPTTAB_CFG_VER_NAME) )
        {
            m_iCfgFileVer = input.subString(input.indexOf("=")+1, input.length()).strip().asInt();
        }

        if( input.occurrencesOf("@") < 5 ) continue;

        GSeq <GString> vals = input.split('@');

        addRow(vals.elementAtPosition(1), vals.elementAtPosition(2), vals.elementAtPosition(3).asInt(), vals.elementAtPosition(4),
               vals.elementAtPosition(5), vals.elementAtPosition(6).asInt());
    }
}

void OptionsTab::setRowsToDefault()
{
    OPTIONS_TAB_ROW * pRow;
    for(int j = 1; j <= (int)m_rowsSeq.numberOfElements(); ++j)
    {
        pRow = m_rowsSeq.elementAtPosition(j);
        if( pRow->WdgType == ROW_LINEEDIT )
        {
            ((QLineEdit*) pRow->TheWdgt)->setText(pRow->Value);
        }
        else if( pRow->WdgType == ROW_CHECKBOX )
        {
            ((QCheckBox*) pRow->TheWdgt)->setChecked(pRow->Value.length() > 0);
        }
        else if( pRow->WdgType == ROW_TEXTBOX )
        {
            ((QTextEdit*) pRow->TheWdgt)->setText(pRow->Value);
        }

    }
}

void OptionsTab::OKClicked()
{
    saveSettingsForAllRows();
    close();
}

void OptionsTab::CancelClicked()
{
	close();
}

void OptionsTab::mb(GString msg)
{
    Helper::msgBox(this, "Options", msg);
}

OPTIONS_TAB_ROW* OptionsTab::getOptionsRow(GString groupName, GString fieldName)
{
    OPTIONS_TAB_ROW * pRow;
    for( int i = 1; i <= m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( pRow->GroupName != groupName ) continue;
        if( pRow->Title == fieldName )
        {
            return pRow;
        }
    }
    return NULL;
}

int OptionsTab::setFieldValue(GString groupName, GString fieldName, GString data)
{
    OPTIONS_TAB_ROW * pRow = getOptionsRow(groupName, fieldName);
    if( pRow != NULL )
    {
        ((QLineEdit*) pRow->TheWdgt)->setText(data);
        return 0;
    }
    return 1;
}

GString OptionsTab::getFieldValue( GString groupName, GString fieldName)
{
    OPTIONS_TAB_ROW * pRow = getOptionsRow(groupName, fieldName);
    if( pRow != NULL )
    {
        return ((QLineEdit*) pRow->TheWdgt)->text();
    }
    return "";
}

GString OptionsTab::getTextBoxValue(GString groupName, GString fieldName)
{
    OPTIONS_TAB_ROW * pRow = getOptionsRow(groupName, fieldName);
    if( pRow != NULL )
    {
        return ((QTextEdit*) pRow->TheWdgt)->toPlainText();
    }
    return "";

}


int OptionsTab::getCheckBoxValue(GString groupName, GString fieldName)
{
    OPTIONS_TAB_ROW * pRow = getOptionsRow(groupName, fieldName);
    if( pRow != NULL )
    {
        return ((QCheckBox*) pRow->TheWdgt)->isChecked() ? 1 : 0;
    }
    return 0;
}

int OptionsTab::setCheckBoxValue(GString groupName, GString fieldName, int checked)
{
    OPTIONS_TAB_ROW * pRow = getOptionsRow(groupName, fieldName);
    if( pRow != NULL )
    {
        ((QCheckBox*) pRow->TheWdgt)->setChecked( checked > 0 ? true : false );
    }
    return 0;
}

int OptionsTab::cfgFileVersion()
{
    return m_iCfgFileVer;
}
