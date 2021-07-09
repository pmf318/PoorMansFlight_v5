//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "simpleShow.h"
#include "gstuff.hpp"
#include "pmfdefines.h"
#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSettings>


SimpleShow::SimpleShow(QString name, QWidget *parent, bool showFormatCB, bool readOnly )
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{

  this->resize(460, 400);
  QBoxLayout *topLayout = new QVBoxLayout(this);

  QGridLayout * grid = new QGridLayout();
  topLayout->addLayout(grid, 2);
 
  m_gstrSettingsName = name;
  m_cbFormat = new QCheckBox("Format data");
  connect(m_cbFormat, SIGNAL(stateChanged(int)), SLOT(displayData()));
  if( !showFormatCB ) m_cbFormat->hide();



  grid->addWidget(m_cbFormat, 0, 0, 1, 3);
  ok = new QPushButton(this);
  ok->setText("OK");
  ok->setDefault(true);
  ok->setText("OK");
  ok->setGeometry(20, 360, 80, 30);
  connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
  grid->addWidget(ok, 2, 1);

  infoTE = new TxtEdit(NULL, this);
  //infoTE->setGeometry( 20, 20, 420, 300);
  infoTE->setReadOnly(readOnly);

  grid->addWidget(infoTE, 1, 0, 1, 3);

  QSettings settings(_CFG_DIR, "pmf5");
  QString prev = settings.value(m_gstrSettingsName+"Format", m_gstrSettingsName+"Format").toString();
  if( prev == "N") m_cbFormat->setChecked(false);
  else m_cbFormat->setChecked(true);

  if( m_cbFormat->isHidden())m_cbFormat->setChecked(false);
  QByteArray ba = settings.value(m_gstrSettingsName+"Geometry").toByteArray();
  this->restoreGeometry(settings.value(m_gstrSettingsName+"Geometry").toByteArray());
}
SimpleShow::~SimpleShow()
{
	delete ok;
    delete infoTE;
}
void SimpleShow::OKClicked()
{
    close();
}

void SimpleShow::reject()
{
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue(m_gstrSettingsName+"Geometry", this->saveGeometry());
    if( m_cbFormat->isChecked()) settings.setValue(m_gstrSettingsName+"Format", "Y");
    else settings.setValue(m_gstrSettingsName+"Format", "N");
    QDialog::reject();
}

void SimpleShow::setText(GString text)
{
    m_gstrOrgText = text;
    displayData();
}

void SimpleShow::setSqlHighlighter(int colorScheme, GSeq <GString>* list)
{
    SqlHighlighter * sqlHighlighter = new SqlHighlighter(colorScheme, infoTE->document(), list);
    infoTE->setSqlHighlighter(sqlHighlighter);
}

void SimpleShow::displayData()
{
    infoTE->clear();    
    if (m_cbFormat->isChecked() )
    {
        //infoTE->setText(m_gstrOrgText);
        //infoTE->setText(GStuff::breakLongLine(m_gstrOrgText, 70));
        infoTE->setLineWrapMode(QTextEdit::WidgetWidth);
    }
    else infoTE->setLineWrapMode(QTextEdit::NoWrap);
    infoTE->setText(m_gstrOrgText);
}

void SimpleShow::setLineWrapping(QTextEdit::LineWrapMode mode)
{
    infoTE->setLineWrapMode(mode);
}
