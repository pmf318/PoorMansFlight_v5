#ifndef tabEdit_H
#define tabEdit_H

//#include <QtGui/QWidget>
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

#include <gstring.hpp>
#include <gseq.hpp>
#include <gseq.hpp>
#include <gthread.hpp>

#include "txtEdit.h"


#include <dsqlplugin.hpp>
#include "pmfSchemaCB.h"
#include "pmfdefines.h"
#include "getclp.h"
#include "showXML.h"
#include "pmfDropZone.h"


#define hintXML 1
#define hintNewVer 2
#define hintMultiCellEdit 3
#define hintColorHint 4


class ExtSQL;
class Pmf;


typedef struct{

    QAction *Action;
    GString Name;
    GString Default;
} CHECKBOX_ACTION;

class TabEdit : public QWidget
{
	class MyThread : public GThread //"public" inheritance: Due to a bug in gcc 2.96 - 3.0.1
	{
		public:
			virtual void run();
            void setOwner(TabEdit *pTabEdit){ myTabEdit = pTabEdit; }
		private:
            TabEdit * myTabEdit;
	};

    class PmfLineEdit: public QLineEdit
    {
    public:
        PmfLineEdit (QWidget * parent = 0) :  QLineEdit (parent){}
    private:
        void keyPressEvent(QKeyEvent *event)
        {

            switch (event->key())
            {
                case Qt::Key_Alt:
                return;
            }
            QLineEdit::keyPressEvent(event);
        }
    };

    class LineEditAction : public QWidgetAction
    {
        private:
        QLabel *pLabel;

        public:
        LineEditAction (QWidget * parent = 0) :  QWidgetAction (parent)
        {
            QWidget* pWidget = new QWidget (parent);
            QHBoxLayout* pLayout = new QHBoxLayout();
            pLabel = new QLabel ("Filter:", parent);
            pLayout->addWidget(pLabel);
            pLineEdit = new PmfLineEdit(parent);
            pLayout->addWidget (pLineEdit);
            pWidget->setLayout (pLayout);
            setDefaultWidget(pWidget);
        }
        void setLabel(QString label)
        {
            pLabel->setText(label);
        }

        QLineEdit * lineEdit ()
        {
            return pLineEdit;
        }

        private:
            QLineEdit * pLineEdit;
    };

public:
    TabEdit(Pmf* pPMF, QWidget* parent=0, int tabIndex=0, GDebug *pGDeb = NULL, int useThread = 0 );
    TabEdit(DSQLPlugin * pDSQL, QTableWidget *tabWdgt, GDebug *pGDeb = NULL );

    ~TabEdit();
    void fillSchemaCB(GString context = "", GString selectSchema = "");
	int canClose(); 
	void setDBName(GString db);
	void initDSQL();
	void setMaxRows(GString maxRows);
	GString getSQL();
    GString getLastSelect();
	GString getDB();
    GString currentTable(int wrapped = 1 );
	void loadBookmark(GString table, GString cmd);
	void reload(GString cmd = "");
	void setCmdText(GString cmd);
    GString getCmdText();
	void setExtSqlCmd(GString cmd); 
    void loadCmdHist(int filter = 1);
    void setCheckBoxValues(GSeq <CHECKBOX_ACTION *> *pMenuActionSeq);
	void lostFocus();
	void gotFocus();	
	void extSQLClosed();
    void getClpClosed();
    void getShowXMLClosed();
    int  colType(int i);
    int colType(GString colName);
    GString firstHistLine();
    void setCmdLine(GString cmd);
    void fillSelectCBs();
    void setHistTableName(GString table);
    void runGetClp();
	void setLastXmlSearchString(GString txt);
    void popupTableCB();
    //GString updateXML(QTableWidgetItem * pItem, GString newXML);
    GString updateLargeXML(QTableWidgetItem * pItem, GString pathToXML);
    GString createSelectForThisItem(QTableWidgetItem* pItem, GSeq <GString> *unqColSeq = NULL, GString castType = "");
    GString createConstraintForThisItem(QTableWidgetItem* pItem);
    GString createFullConstraint(int row, GSeq<GString> *unqCols);
	void setGDebug(GDebug * gd);
    int writeDataToFile(QTableWidgetItem* pItem, GString file, int* outSize);
    int writeDataToFile(GString cmd, GString file, int * outSize);
    void setCellFont(QFont* font);
    void initTxtEditor(TxtEdit * pTxtEdit);
    GString createUniqueColConstraint(QTableWidgetItem * pItem);
    GString createUniqueColConstraintForItem(QTableWidgetItem * pItem, GSeq <GString> * unqCols);
    void showHint(int hint, GString txt = "");
    void setColorScheme(int usePmf);
    int colorScheme();
    int isChecked(GString name);
    GString updateRow(QTableWidgetItem * pItem, DSQLPlugin *pDSQL, GSeq<GString> *unqCols, int lobCol = -1, GString lobTmpFile = "");
    GString updateRowViaUniqueCols(QTableWidgetItem* pItem, DSQLPlugin * pDSQL, GSeq<GString> *unqCols, int lobCol, GString lobTmpFile );
    int isNewRow(QTableWidgetItem *pItem);
    GDebug *getGDeb();



    //int loadContextApp(GString targetFile);
    void setActionsMenu(QTableWidgetItem * pItem);
    void deb(GString txt);
    void deb(GString fnName, GString txt);
    void colorGradeWidget();
    int isSelectStatement(GString in);
    void setNewIndex(int index);
    GString lastSqlSelectCmd();

    DSQLPlugin* m_pMainDSQL;


	
private:
    Q_OBJECT
    QComboBox * contextCB;
    QComboBox* tableCB;
    PmfSchemaCB * schemaCB;
    QPushButton * showBT, *backBT, *forwBT;
    QLineEdit * maxRowsLE, *msgLE, *updLE, *insLE, *dbNameLE;
    TxtEdit *cmdLineLE;
    QLabel * hintLE;
    QCheckBox * singleRowCHK, *filterByTableCB;
    QPushButton *insertB, *saveB, *deleteB, *cancelB, *extSQLB, *exportButton, *importButton, *newLineB, *runB, *refreshB, *clearB;
    QComboBox *hostCB, *likeCB, *orderCB, *filterCB;
    QLabel *cmdText;
    GSeq <GString> m_seqTableNames;
    GSeq <int> m_seqGenerated;
	GSeq<COL_SPEC*> m_colDescSeq;
    GDebug * m_pGDeb;


    int m_iUseColorScheme;
    int m_iUseThread;


    ExtSQL *m_pExtSQL;
    Getclp * m_pGetCLP;
    ShowXML * m_pShowXML;
    QTableWidget * mainWdgt;

    QMenuBar *menu;
    QMenu actionsMenu;
    QTabWidget * m_qTabWDGT;
    int isGeneratedColumn(int i);
    void fillGeneratedColSeq(GString table);

    int m_iTabID;

    QTableWidgetItem *m_pActionItem;
    GString orgCellData(QTableWidgetItem *pItem);

	signed int m_iLastSortedColumn;
	int m_iIsSortedAsc;
	GString m_gstrCurrentSchema;
	
	void createActions();
	
	QAction * fillAct;
	QMenu *fileMenu;
	void addLowerPart(QGridLayout * grid);
	void msg(QString txt);
	int createNewRow(int rowPos);
    int createEmptyRow();
    int checkEmptyRow(int rowPos );
    void setFocusIntoEmptyRow();
	void qdeb(QString s);
    void setVHeader(int full = 1);
	void refreshSort();
    void setHintColorForAllItems();
    GString getFileExtensionFromMime(GString inFile);
    GString setTmpFileExtension(GString inFile, GString ext);

	void setButtonState(bool state);
    void setPendingMsg(int reset = 0);
	void setInfo(GString txt, int blink = 0);
	void clearMainWdgt();
	void addInfoGrid(QGridLayout* pGrid);
    GString deleteByCursor(DSQLPlugin * pDSQL);
    GString changeRowByCursor(DSQLPlugin * pDSQL, GString cmd, GString whr,
                              GSeq <GString> *fileList = NULL,
                              GSeq <long> *lobType = NULL);



	QGridLayout *m_mainGrid;
	GSeq <int> m_seqUpdates;
	GSeq <int> m_seqInserts;
	
	void markForUpd(QTableWidgetItem * pItem);
	void markForIns(QTableWidgetItem * pItem);
	void setCurrentTable(GString table);



	GString readXMLFromTMP(GString xmlSrcFile);
    GString insertRow(QTableWidgetItem * pItem, DSQLPlugin * pDSQL);
    GString deleteThisRow(long pos, DSQLPlugin * pDSQL, GSeq<GString> *unqCols, int yesToAll = 0);
    int     hasUsableColumns();
	int isOrderStmt(GString cmd);
	unsigned int maxRows();
	void addToStoreCB(QString s, int writeToFile = 1);
	void showData();
    
    int setLOBMarkers(QTableWidgetItem* pItem, GSeq <GString> *fileNameSeq, int setQuestionMark = 0);    
    GString setLOBMarker(QTableWidgetItem* pItem, int setQuestionMark = 0);
    int cellDataChanged(QTableWidgetItem * pItem);
    int hasXMLCols();
    int hasLOBCols();
    void setWhatsThisTexts();
    void waitForThread();
    int findInComboBox(QComboBox *pCB, GString txt);
    GString createInsertCmd(QTableWidgetItem * pItem, GString separator = ",");
    int hasTwin(QTableWidgetItem * pItem);
    GString writeLobToTemp(QTableWidgetItem * pItem, GString * fileName, GSeq <GString> *unqColSeq);
    GString formatText(GString text);
    GString formatAsDateTime(GString text);
    void setRowBackGroundColor(QTableWidgetItem * pItem, QColor color, int setBold = 0);
    void setRowBackGroundColor(int row, QColor color);
    void createUserActions(QMenu * menu);
    int addColorizedHints(int col, QTableWidgetItem * pItem, GString data);
    void setItemSelected(QTableWidgetItem* pItem);

    GString m_strHistTableName;
    GString wrap(GString in);
    void setTableNameFromStmt(GString in);
	int isIdentityColumn(int colNr);
    int isIdentityColumn(GString colName);
	void readColumnDescription(GString table);
    void loadDoubleByteEditor(int type = -1);
    QTableWidgetItem * currentItem();
    void filterCurrentCell(int mode);
    int displayAsNumber(int col);
    int cellsInSingleColumn(const QPoint & p);
    GString m_gstrPrevExportPath;
    GString m_gstrPrevImportPath;

    GString createUpdateCmd(GString val);
    void setLastSensibleSqlCmd(GString cmd);
    void disableActions();

    void deleteRows();
    int deleteViaFetch(GString tableName, GString whereClause);
    void enableActions(int hint);
    GSeq <GString> clipBoardToSeq(GString data);
	
	QTimer * m_tmrWaitTimer;
	unsigned long m_ulTimerEvts;
    short  m_iTimerDirections;


	
    MyThread * m_pThread;
    static void *threadedStuff(void * arg);


	GString m_gstrSQLCMD;
	long m_lMaxRows;

	GString m_gstrSqlErr;
	GString m_gstrExtSqlCmd;
	GString histFileName();
	GSeq <GString> m_seqCmdHist;
	GSeq <GString> m_seqCmdHistTable;
	int m_iCmdHistPos;
	void addToCmdHist(GString cmd);
	void setHistButtons();

	int m_iLockAll;
	GString m_gstrLastFilter;
	Pmf* m_pPMF;
	QAction * m_qaCurCell;
	QAction * m_qaAddConst;	
	QAction * m_qaCurRow;	
    QAction * m_qaDistinct;
    QAction * m_qaCount;
    QAction * m_qaGroup;
    QAction * m_qaNotNull;

	
	QAction * m_qaBitHide;
	QAction * m_qaBitAsHex;	
	QAction * m_qaBitAsBin;	
    QAction * m_qaShowXML;
    QAction * m_qaEditLong;
    QAction * m_qaEditText;
    QAction * m_qaEditGraphic;
    QAction * m_qaEditDBClob;
    QAction * m_qaSaveBlob;
    QAction * m_qaOpenBlob;
    QAction * m_qaOpenBlobAs;
    QAction * m_qaLoadBlob;
    QAction * m_qaCopy;
    QAction * m_qaPaste;
    QAction * m_qaUpdate;
    QAction * m_qaDelTabAll;
    LineEditAction * pLineEditFilterAction;    
    LineEditAction * pLineEditUpdateAction;

    QAction *m_qaUserActions[MaxUserActions];

	GString m_strLastXmlSearchString;
    GSeq <CHECKBOX_ACTION *> *m_pCbMenuActionSeq;
    int m_iCurrentIndex;
    PmfDropZone *pPmfDropZone;


public slots:
    GString  okClick();

public slots:
	void itemChg(QTableWidgetItem* cur);
	void test();
	void sortClicked(int pos);	
	void fill();
	void  refreshClicked();
    void  delButtonClick();
	void  insClick();
    int  chgClick();
	void  cancelClick();
	void  extSQLClick();    
    void  setCmdLine(int index);
    void  setHost(int);
    void  setLike(int);
    void  setOrder(int);
    void loadSqlEditor();
    void loadSqlEditor(QDropEvent *event);
	
	void  timerDone();
	void timerCalled();

	void  newLineClick();

	void  getBlobClick();
    short getBlob(long pos, int * written);
	void  putBlobClick();
	void  clearButtonClicked();
	void  mainWdgtDoubleClicked();
	void schemaSelected(int index);
	void tableSelected();
	void timerEvent(QTimerEvent*);
	GString currentSchema();
    GString currentContext();
    int writeHist(GString table, GString action, QTableWidgetItem * pItem, GString sqlCmd = "");
	void slotRightClickActionCell();
	void slotRightClickActionRow();
	void slotRightClickActionAddConstraint();
	
	void slotRightClickActionBitHide();
	void slotRightClickActionBitAsHex();	
	void slotRightClickActionBitAsBin();	

    void slotRightClickActionCopy();
    void slotRightClickActionPaste();

    void slotRightClickActionShowXML();
    void slotRightClickActionEditLong();
    void slotRightClickActionEditText();
    void slotRightClickActionEditGraphic();
    void slotRightClickActionEditDBClob();
    void slotRightClickSaveBlob();
    void slotRightClickOpenBlob();
    void slotRightClickOpenBlobAs();
    void slotRightClickLoadBlob();
    void slotRightClickDistinct();
    void slotRightClickCount();
    void slotRightClickGroup();
    void slotRightClickUpdate();
    void slotRightClickNotNull();
    void slotRightClickDeleteTable();
    void slotFilterHistByTable();
    void slotLineEditFilter();
    void slotLineEditUpdate();
    void slotLabelLinkClicked(QString);
    void itemDoubleClicked(int row, int column);

    void exportData();
    void importData();


	
    void getUserAction();
    GString formatActionText(GString in);
    void openActionEditor();

	void slotForward();
	void slotBack();
    void popUpActions(const QPoint & p);
    void contextSelected(int index);
    GString createDataToPaste(QTableWidgetItem *pItem, GString separator);
    GString  pasteByGuessing(GString line);
    void createPastedRow(GString line);
	int tabID();
	void setMyTabText(GString txt);
    GString myTabText();
    int tableHasUniqueCols(GString tableName);
    void setMiscFont();
    int deleteViaFetchFromCmdLE(GString cmd);

	int handleDeleteError(int sqlcode, GString errTxt, int * ignoreErr);


    //void dropEvent( QDropEvent *event );
    //void dragEnterEvent(QDragEnterEvent *event);


protected:
	void closeEvent(QCloseEvent* );
	void keyPressEvent(QKeyEvent *event);	
    bool eventFilter(QObject*, QEvent *event);

signals:
    void mainWidgetCleared();

};

#endif 
