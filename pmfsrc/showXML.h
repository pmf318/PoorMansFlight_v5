//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlistwidget.h>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qfont.h>
#include <QCheckBox>
#include <QTextEdit>
#include <QTableWidgetItem>
#include <QDomDocument>
#include <QMenu>

#include "xmlEdit.h"
#include "idsql.hpp"
#include <gdebug.hpp>

#ifndef _showXML_
#define _showXML_

//avoid revcursive include
class TabEdit;

class ShowXML : public QDialog
{
   Q_OBJECT
public:
   ShowXML(GDebug *pGDeb,TabEdit *parent, QTableWidgetItem * pItem, GString srcFile, GString lastSearchString = "");
   ~ShowXML();
   short fillLB();
   void setSrcFile(GString fileName);
   int needsReload();
   GString newXML();
   GString lastSearchString();
   void saveWdgtGeometry();
   void restoreWdgtGeometry();
   GString attributesString(QDomNode node, GString xPath = "");


private slots:
   void OKClicked();
   void saveToFileClicked();
   void slotShowXPath() ;
   void slotShowAttrSql() ;
   void updateClicked();
   void formatChecked();
   void reloadClicked();
   void checkSyntaxClicked();
   void onTextChanged();
   void mainWidgetWasCleared();


protected slots:
    void popUpXmlActions(const QPoint &);    

private:
   void closeEvent(QCloseEvent * event);
   void keyPressEvent(QKeyEvent *event);
   void keyReleaseEvent(QKeyEvent *event);
   void findNextTextOccurrence();
   int findText(int offset = 0);
   void msg(QString txt);
   void deb(GString msg);
   GString getAllPaths();
   GString getXPath(QDomDocument doc, QString nodeName);
   GString getXPathWithConstraints(QDomDocument doc, QString selTxt);
   GString readXmlFile();
   int checkRefresh();
   GString attributeSql();
   GString createXPathText(int checkForAttribute = 0);

   GString iTableName;
   QPushButton * okButton;
   QPushButton * saveToFileButton;
   QPushButton * updateButton;
   QPushButton * reloadButton;
   QPushButton * checkSyntaxButton;
   //QTextEdit *xmlTE;
   XmlEdit *xmlTE;
   QLineEdit * findLE;
   QLineEdit * xpathLE;
   
   int m_iNeedsReload;
   GString m_strFile;
   void closeMe();
   QCheckBox * m_cbFormat;
   QCheckBox * m_cbCaseSensitive;
   void displayData();
   QString m_qstrOrgTxt;
   GString m_gstrNewXML;
   TabEdit * m_Master;
   QRect m_qrGeometry;
   QTableWidgetItem *m_pItem;
   QLabel * m_qlStatus;
   int m_iLastFindPos;
   GString m_strLastSearchString;
   GDebug *m_pGDeb;
   QAction * m_qaShowXPath;
   QAction * m_qaShowAttrSql;
   QMenu actionsMenu;
   long m_iItemAddr;
   
};

#endif
