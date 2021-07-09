

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

#include "dsqlplugin.hpp"
#include "txtEdit.h"

#ifndef _editLong_
#define _editLong_


//avoid revcursive include
class TabEdit;

class EditDoubleByte : public QDialog
{
   Q_OBJECT
public:
    EditDoubleByte(GDebug *pGDeb, TabEdit *parent, QTableWidgetItem * pItem, DSQLPlugin * pIDSQL, int type);
    ~EditDoubleByte();
    void setSrcFile(GString file, int isBinary = 0);
    void setSrcData(GString data, int isBinary = 0);
    void setInfo(GString text);
    int runRefresh();
    GString data();
    int loadData();

private slots:
   void exitClicked();
   void saveToFileClicked();
   void updateDB();
   void displayChecked();
   void onTextChanged();

private:
   QLabel *info;
   QPushButton * exitB;
   QPushButton * saveToFile;
   QPushButton * updateToDB;
   TxtEdit *dataTE;
   TabEdit * m_Master;
   QTableWidgetItem *m_pItem;
   QRect m_qrGeometry;
   QCheckBox * m_cbDisplay;
   GString m_strFile;
   int m_iRunRefresh;
   DSQLPlugin * m_pIDSQL;
   GString m_strColName;
   int m_iType;
   QCheckBox * m_cbCaseSensitive;
   QLineEdit * findLE;
   GString m_strLastSearchString;
   int m_iLastFindPos;
   GDebug *m_pGDeb;
   QString m_qstrOrigTxt;

   void saveWdgtGeometry();
   void restoreWdgtGeometry();
   void closeEvent(QCloseEvent * event);
   void keyPressEvent(QKeyEvent *event);
   void closeMe();
   int loadVarcharData();
   int loadGraphicData();
   int loadFromFile();
   int loadFromItem();
   int loadDoubleByteData();
   void saveDataToFile(GString fileName);
   void msg(QString txt);
   void findClicked();
   int findText(int offset = 0);
   void keyReleaseEvent(QKeyEvent *event);
   void highlightLine(int line);
   void setCursorPosition( unsigned int line, unsigned int pos  = 0);
   void findNextTextOccurrence();

};

#endif
