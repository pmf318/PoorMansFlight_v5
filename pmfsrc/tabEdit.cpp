
#include "tabEdit.h"
#include "extSQL.h"
#include <QtGui>
#include <QGraphicsDropShadowEffect>

#ifdef MAKE_VC
#include <windows.h>
#include <process.h>
#endif


#if QT_VERSION >= 0x050000
#else
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QMenuItem>
#endif
#include <QGridLayout>


# ifdef UNICODE
# define FUNC_OPEN_AS_NAME "OpenAs_RunDLLW"
# else
# define FUNC_OPEN_AS_NAME "OpenAs_RunDLL"
# endif

#include <QMessageBox>
#include <QGroupBox>
#include <QTimerEvent>
#include <QPalette>
#include <QHeaderView>
#include <QTextBrowser>
#include <QDesktopServices>
#include <QDomDocument>

#include <gstring.hpp>
#include <gstuff.hpp>
#include <gfile.hpp>
#include <gdebug.hpp>

#include "showXML.h"
#include "editDoubleByte.h"
#include "userActions.h"

#include "pmfCfg.h"
#include "pmf.h"
#include "pmfdefines.h"
#include "helper.h"
#include <QMutex>
#include <QtCore>
#include <QFuture>
#include <QCompleter>

#include "pmfTableWdgt.h"
#include "exportBox.h"
#include "importBox.h"
#include "pmfPushButton.h"
#include "pmfTable.h"


//Colors:
QString _BgYellow = "background-color:yellow;";
QString _BgWhite = "background-color:white;";
QString _BgRed = "background-color: red;";
QString _FgBlue = "color: blue;";
QString _BgBlue = "background: rgb(179, 217, 255)";
QString _FgRed = "color: red;";
QString _NewVersColor = "background: rgb(210,237,184);";
QColor _ColorEven = QColor(250, 247, 212);
QColor _ColorOdd = QColor(255, 255, 255);
QColor _ColorIns = QColor(216, 255, 184);
QColor _ColorUpd = QColor(184, 251, 218);
QColor _ColorApos = QColor(255, 204, 157);
QColor _ColorAposDark = QColor(102, 26, 0);
QColor _ColorBlanks = QColor(185, 237, 178);
QColor _ColorBlanksDark = QColor(128, 32, 0);
QColor _ColorBlanksInside = QColor(230, 255, 255);

QColor _ColorCRLF = QColor(250,177,42);
QColor _ColorCRLFDark = QColor(235,87,121);


QString _qstrColorOdd = "#FAF7D4";
QString _qstrColorEven =  "#FFFFFF";

#ifndef min
#define  min(a,b) (a < b ? a : b)
#endif


//#include <idsql.hpp>

static int _threadRunning;
static IDSQL* _staticDSQL;
static GString _gstrSQLCMD;
static GString _gstrSQLError;
static long _lMaxRows;


TabEdit::TabEdit(DSQLPlugin * pDSQL, QTableWidget *tabWdgt, GDebug *pGDeb)
{
    m_pGDeb = pGDeb;
    mainWdgt = tabWdgt;
    m_pMainDSQL = pDSQL;
    schemaCB = NULL;
    tableCB = NULL;
    m_iUseColorScheme = 0;
}


TabEdit::TabEdit(Pmf* pPMF, QWidget* parent, int tabIndex, GDebug *pGDeb, int useThread) : QWidget(parent)
{
    m_pGDeb = pGDeb;
    m_iUseThread = useThread;
    m_iUseColorScheme = pPMF->getColorScheme();

    if( pPMF->getConnection()->getDBType() == SQLSERVER )
    {
        contextCB = new QComboBox(this);
        pPMF->getConnection()->initAll("SELECT name FROM master.dbo.sysdatabases order by name ");
        contextCB->addItem(_selStringCB); //Add "<Select>" as first entry
        for( int i = 1; i<= (int) pPMF->getConnection()->numberOfRows(); ++i)
        {
            contextCB->addItem(pPMF->getConnection()->rowElement(i,1).strip("'"));
        }
        connect(contextCB, SIGNAL(activated(int)), SLOT(contextSelected(int)));
    }
    else contextCB = NULL;

    deb(__FUNCTION__, "ctor (1)");
    m_qTabWDGT = (QTabWidget*)parent;
    m_pPMF = pPMF;
    m_iTabID = tabIndex;
    m_mainGrid = new QGridLayout(this);


    //setCentralWidget( grid );

    deb(__FUNCTION__, "ctor (2)");
    dbNameLE = new QLineEdit(this);
    dbNameLE->setReadOnly(true);
    if( m_iUseColorScheme ) dbNameLE->setStyleSheet(_BgYellow);

    //schemaComboBox:

    ///schemaCB = new QComboBox(this);
    schemaCB = new PmfSchemaCB(this);
    schemaCB->setEditable(false);
    schemaCB->setToolTip("Select table schema here");
    schemaCB->setMaxVisibleItems(30);

    connect(schemaCB, SIGNAL(activated(int)), SLOT(schemaSelected(int)));
    //TableComboBox
    tableCB = new QComboBox(this);
    tableCB->setEditable(false);
    tableCB->setToolTip("Select table here");
    tableCB->setMaxVisibleItems(30);

    connect(tableCB, SIGNAL(activated(int)), SLOT(tableSelected()));
    deb(__FUNCTION__, "ctor (3)");
    showBT   = new QPushButton(this);
    showBT->setText("&Open");
    showBT->setToolTip("Open a table /display its contents");

    deb(__FUNCTION__, "ctor (4)");
    backBT   = new QPushButton(this);
    //backBT->setStyleSheet("background-color:#20b2aa;font:bold;");
    //backBT->setText("<");
    backBT->setToolTip("Show previous result");
    QIcon icoB;
    icoB.addPixmap(QPixmap(":leftarr.png"), QIcon::Normal, QIcon::On);
    backBT->setIcon(icoB);
    backBT->setMaximumWidth(16);
    backBT->setMaximumHeight(16);
    deb(__FUNCTION__, "ctor (5)");
    forwBT   = new QPushButton(this);
    //forwBT->setText(">");
    //forwBT->setMaximumWidth(15);
    forwBT->setMaximumWidth(16);
    forwBT->setMaximumHeight(16);
    forwBT->setToolTip("Show next result");
    QIcon icoF;
    icoF.addPixmap(QPixmap(":rightarr.png"), QIcon::Normal, QIcon::On);
    forwBT->setIcon(icoF);
    deb(__FUNCTION__, "ctor (6)");
    QLabel *aText = new QLabel(this);
    aText->setText("Rows to fetch:");
    maxRowsLE = new QLineEdit(this);
    maxRowsLE->setMaximumWidth(100);
    maxRowsLE->setToolTip("Max. number of rows to display");
    deb(__FUNCTION__, "ctor (7)");
    singleRowCHK = new QCheckBox(this);
    singleRowCHK->setText("Single Row");
    singleRowCHK->setToolTip("Constrain changes (UPD/DEL) to a single row");
    if( pPMF->getConnection()->getDBType() == MARIADB )singleRowCHK->setEnabled(false);

    /* Replaced by GroupBox
    m_mainGrid->addWidget(schemaCB, 0, 0);
    m_mainGrid->addWidget(tableCB,0,1,1,2);
    m_mainGrid->addWidget(showBT, 0, 3);
    m_mainGrid->addWidget(aText, 0, 4);
    m_mainGrid->addWidget(maxRowsLE, 0, 5);
    m_mainGrid->addWidget(singleRowCHK, 0, 6);
    */
    //Row 1
    QGroupBox * upperBox = new QGroupBox();
    //upperBox->setStyleSheet("border:0;");
    QHBoxLayout *upperLayout = new QHBoxLayout;
    schemaCB->setMinimumWidth(120);
    tableCB->setMinimumWidth(150);
    upperLayout->addWidget(backBT);
    upperLayout->addWidget(forwBT);
    upperLayout->addWidget(dbNameLE, 0);
    if( contextCB )upperLayout->addWidget(contextCB);
    upperLayout->addWidget(schemaCB, 10);
    upperLayout->addWidget(tableCB, 20);
    upperLayout->addWidget(showBT);
    upperLayout->addWidget(aText);
    upperLayout->addWidget(maxRowsLE);
    upperLayout->addWidget(singleRowCHK);
    deb(__FUNCTION__, "ctor (8)");
    upperBox->setLayout(upperLayout);
    m_mainGrid->addWidget(upperBox, 0 , 0, 1, 10);
    //grid->addWidget(msgLE, 1, 0, 1, 10);
    addLowerPart(m_mainGrid);
    addInfoGrid(m_mainGrid);
    connect(showBT, SIGNAL(clicked()), SLOT(fill()));
    connect(backBT, SIGNAL(clicked()), SLOT(slotBack()));
    connect(forwBT, SIGNAL(clicked()), SLOT(slotForward()));
    deb(__FUNCTION__, "ctor (9)");

    //mainWdgt = new PMF_QTableWidget(this);
    mainWdgt = new PmfTableWdgt(this);
#if QT_VERSION >= 0x050000
    mainWdgt->horizontalHeader()->setSectionsMovable(true);
#else
    mainWdgt->horizontalHeader()->setMovable(true);
#endif
    mainWdgt->setAlternatingRowColors(true);
    if( !m_iUseColorScheme)
    {
        //mainWdgt->setStyleSheet("alternate-background-color: rgb(80,80,80);background-color: rgb(20,20,20);");
    }
    else
    {
        //mainWdgt->setStyleSheet("alternate-background-color: rgb(250, 247, 212);background-color: rgb(255, 255, 255);");
    }
    mainWdgt->setAcceptDrops(true);
    mainWdgt->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( mainWdgt, SIGNAL( customContextMenuRequested(const QPoint &) ), this, SLOT( popUpActions(const QPoint &) ) );

    m_mainGrid->addWidget(mainWdgt,2,0,1,10);
    hintLE = new QLabel();
    m_mainGrid->addWidget(hintLE, 1, 0, 1, 10);
    hintLE->setMinimumHeight(18);
    hintLE->hide();


    PmfCfg aCFG(m_pGDeb);
    GString mr = aCFG.getValue("MaxRows");
    setMaxRows(mr);
    deb(__FUNCTION__, "ctor (10)");
    m_iLastSortedColumn = -1;
    m_iIsSortedAsc = 0;
    //NOTE: ::clearMain used to delete/create mainWdgt. Not any more.
    //So we need to connect sortClicked() here, instead of ::showData()
    connect((QWidget*)mainWdgt->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));

    connect(mainWdgt, SIGNAL(itemChanged(QTableWidgetItem*) ), this, SLOT(itemChg(QTableWidgetItem*)));

    deb(__FUNCTION__, "ctor (11)");


    if( m_iUseThread )
    {
        m_tmrWaitTimer = new QTimer( this );
        connect( m_tmrWaitTimer, SIGNAL(timeout()), SLOT(timerCalled()) );
        m_tmrWaitTimer->start( 1000 );
    }
    m_iCmdHistPos = 0;
    backBT->setEnabled(false);
    forwBT->setEnabled(false);
    createActions();


    m_iLockAll = 0;
    m_lMaxRows = 2000;
    m_pExtSQL = NULL;
    m_pGetCLP = NULL;
    m_pShowXML = NULL;
    m_pThread = 0;
    _threadRunning = 0;
    m_pActionItem = NULL;


    setWhatsThisTexts();
    deb(__FUNCTION__, "ctor done.");
    m_iCurrentIndex = m_qTabWDGT->currentIndex();

    m_pMainDSQL = new DSQLPlugin(*m_pPMF->getConnection());
    if( !m_pMainDSQL )
    {
        msg("Failed to load plugin");
        return;
    }
    if( m_pMainDSQL->getDBType() != DB2 )
    {
        importButton->hide();
        pPmfDropZone->hide();
    }

    //mainWdgt->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainWdgt->setSelectionBehavior(QAbstractItemView::SelectItems);
    connect(hintLE, SIGNAL(linkActivated(QString)), this, SLOT(slotLabelLinkClicked(QString)));
    connect(mainWdgt, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(itemDoubleClicked(int,int)));
    setMiscFont();

}


void TabEdit::setMiscFont()
{

//    m_qaCurCell->setFont(this->font());
//    m_qaAddConst->setFont(this->font());
//    m_qaCurRow->setFont(this->font());
//    m_qaNotNull->setFont(this->font());
//    m_qaDistinct->setFont(this->font());
//    m_qaCount->setFont(this->font());
//    m_qaGroup->setFont(this->font());
//    m_qaUpdate->setFont(this->font());
//    m_qaDelTabAll->setFont(this->font());

//    m_qaBitHide->setFont(this->font());
//    m_qaBitAsHex->setFont(this->font());
//    m_qaBitAsBin->setFont(this->font());
//    m_qaShowXML->setFont(this->font());
//    m_qaEditLong->setFont(this->font());
//    m_qaEditText->setFont(this->font());
//    m_qaEditGraphic->setFont(this->font());
//    m_qaEditDBClob->setFont(this->font());
//    m_qaSaveBlob->setFont(this->font());
//    m_qaOpenBlob->setFont(this->font());
//    m_qaOpenBlobAs->setFont(this->font());
//    m_qaLoadBlob->setFont(this->font());
//    m_qaCopy->setFont(this->font());
//    m_qaPaste->setFont(this->font());
//    m_qaUpdate->setFont(this->font());
//    m_qaDelTabAll->setFont(this->font());

//    pLineEditFilterAction->setFont(this->font());

}

/*****************************************************
* If there are no unsaved changes, we are ready to close.
*****************************************************/
int TabEdit::canClose()
{
    saveB->setFocus();
    if( GString(updLE->text()).asInt() == 0 && GString(insLE->text()).asInt() == 0 ) return 1;

    //if( QMessageBox::question(this, "PMF", "Unsaved changes, quit?", "Yes", "No", 0, 0, 1) )
    if( QMessageBox::question(this, "PMF", "Unsaved changes, close this tab?", "Yes", "No", 0, 1) == 0 )
    {
        return 1;
    }

    return 0;
}
void TabEdit::closeEvent(QCloseEvent * event)
{
    event->accept();
}

TabEdit::~TabEdit()
{
    //showXML tries to call "tabEdit::setLastXmlSearchString" in its DTor.
    //This should be avoided on close down.
    getShowXMLClosed();

    if( m_pMainDSQL )
    {
        delete  m_pMainDSQL;
        m_pMainDSQL = NULL;
    }
    if( m_pExtSQL )
    {
        m_pExtSQL->close();
        delete m_pExtSQL;
    }
    if( m_pGetCLP )
    {
        m_pGetCLP->close();
        delete m_pGetCLP;
    }
    if( m_pShowXML )
    {
        m_pShowXML->close();
        delete m_pShowXML;
    }
    GString path = Helper::tempPath()+_TMP_DIR;
    Helper::removeDir(path);
    m_colDescSeq.deleteAll();
    delete pLineEditFilterAction;
    delete pLineEditUpdateAction;
}


void TabEdit::addInfoGrid(QGridLayout* pGrid)
{
    /** replaced by GroupBox
    //Panel to hold a grid which holds texts and LineEdits
    //Grid-in-grid does not work.
    QWidget * pane = new QWidget(this);
    QGridLayout *infoGrid = new QGridLayout(pane);
    **/



    updLE = new QLineEdit(this);
    updLE->setMaximumWidth(40);
    if( m_iUseColorScheme ) updLE->setStyleSheet(_BgWhite);
    insLE = new QLineEdit(this);
    insLE->setMaximumWidth(40);
    if( m_iUseColorScheme ) insLE->setStyleSheet(_BgWhite);
    msgLE = new QLineEdit(this);

    if( m_iUseColorScheme ) msgLE->setStyleSheet("background: white;");
    /* White background, grey foreground
    updLE->setEnabled(false);
    insLE->setEnabled(false);
    msgLE->setEnabled(false);
    */
    updLE->setReadOnly(true);
    insLE->setReadOnly(true);
    msgLE->setReadOnly(true);
    if( m_iUseColorScheme ) msgLE->setStyleSheet(_FgBlue);


    QLabel *updText = new QLabel("Pending Updates:", this);
    QLabel *insText = new QLabel("Inserts:", this);
    QLabel *msgText = new QLabel("Info:", this);

    QGroupBox * lowerBox = new QGroupBox();
    QHBoxLayout *lowerLayout = new QHBoxLayout;
    lowerLayout->addWidget(updText);
    lowerLayout->addWidget(updLE);
    lowerLayout->addWidget(insText);
    lowerLayout->addWidget(insLE);
    lowerLayout->addWidget(msgText);
    lowerLayout->addWidget(msgLE);

    lowerBox->setLayout(lowerLayout);

    pGrid->addWidget(lowerBox, 7,0,1,10);

}
/*****************************************************
*
*****************************************************/
void TabEdit::fill()
{

    if( !currentTable().length() ) return;
    if( currentTable() == _newTabString ) return;
    //set name on tab
    setMyTabText(currentTable(0));

    ////m_iLastSortedColumn = -1; //Could be a fresh table, don't sort
    if( m_pMainDSQL->getDBType() == MARIADB ) reload("SELECT * FROM "+currentTable(0));
    else reload("SELECT * FROM "+currentTable());

    if( m_iUseThread ) return;

    cmdLineLE->setText("");
    addToCmdHist("SELECT * FROM "+currentTable());
    fillSelectCBs();
    return;

}

void TabEdit::setHintColorForAllItems()
{
    if( !m_iUseColorScheme ) return;
    QTableWidgetItem *pItem;
    for (int i = 0; i < mainWdgt->rowCount(); ++i)
    {
        for (int j = 2; j < mainWdgt->columnCount() ; ++j)
        {
            pItem = mainWdgt->item(i, j);
            addColorizedHints(j-1, pItem, pItem->text());
        }
    }
}

/*****************************************************
*
*****************************************************/
void TabEdit::setVHeader(int full)
{
    deb(__FUNCTION__, "start, rows in mainWdgt: "+GString(mainWdgt->rowCount()));
    //The first two columns are hidden and contain an index (col0) and the action (col1, INS or UPD)
    QTableWidgetItem *newItem, *pItem;
    GString txt;
    disconnect(mainWdgt, SIGNAL(itemChanged(QTableWidgetItem*) ), this, SLOT(itemChg(QTableWidgetItem*)));
    mainWdgt->setUpdatesEnabled(false);

//    mainWdgt->setAlternatingRowColors(true);
//    QString styleOddEven = "alternate-background-color: %1; background-color: %2;";
//    mainWdgt->setStyleSheet(styleOddEven.arg(_qstrColorOdd).arg(_qstrColorEven));

    if( full )
    {
        for (int i = 0; i < mainWdgt->rowCount(); ++i )
        {
            pItem = mainWdgt->item(i, 1);
            if( !pItem ) continue;
            if( m_iUseColorScheme && full)
            {
                if( i % 2  ) setRowBackGroundColor(i, _ColorEven);
                else setRowBackGroundColor(i, _ColorOdd);
            }
            txt = pItem->text();
            if( txt == _INSMark )
            {
                txt = _INSRow;
                setRowBackGroundColor(i, _ColorIns);
            }
            else if( txt == _UPDMark) txt = _UPDRow;
            else if( txt == _NEWMark) txt = _NEWRow;
            else txt = mainWdgt->item(i, 0)->text();
            newItem = new QTableWidgetItem((char*) txt);
    #ifdef MAKE_VC
            mainWdgt->setRowHeight(i, QFontMetrics( mainWdgt->font()).height()+5);
    #else
            mainWdgt->setRowHeight(i, QFontMetrics( mainWdgt->font()).height()+5);
    #endif            
            mainWdgt->setVerticalHeaderItem(i, newItem);

        }
    }
    if( full ) setHintColorForAllItems();

    mainWdgt->setColumnWidth(0, 1);
    mainWdgt->setColumnWidth(1, 1);
    #if QT_VERSION >= 0x050000
    mainWdgt->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    mainWdgt->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    #else
    mainWdgt->setColumnHidden(0, true);
    #endif    
    mainWdgt->setColumnHidden(0, true);
    mainWdgt->setColumnHidden(1, true);
    mainWdgt->setUpdatesEnabled(true);
    mainWdgt->blockSignals( false );
    //mainWdgt->setSortingEnabled(false);

//    for(int j = 2; j < mainWdgt->columnCount(); ++j)
//    {
//        mainWdgt->resizeColumnToContents ( j );
//    }
//    for(int j = 2; j < mainWdgt->columnCount(); ++j)
//    {
//        if( m_pMainDSQL->isXMLCol(j-1)) mainWdgt->setColumnWidth(j, 200);
//        else mainWdgt->setColumnWidth(j, mainWdgt->columnWidth(j) > 400 ? 400 : mainWdgt->columnWidth(j)+20);
//    }
//    setHintColorForAllItems();
    mainWdgt->setUpdatesEnabled(true);
    connect(mainWdgt, SIGNAL(itemChanged(QTableWidgetItem*) ), this, SLOT(itemChg(QTableWidgetItem*)));
    //mainWdgt->setSortingEnabled(false);
    deb(__FUNCTION__, "done");
}

/*****************************************************
*
*****************************************************/
void TabEdit::test()
{
    msg("clickety");
}
/*****************************************************
* Default sort order is ascending.
* Clicking the same vertical header twice reverses the sort order
*****************************************************/
void TabEdit::sortClicked(int pos)
{
    mainWdgt->setSortingEnabled(true);
    if( pos == m_iLastSortedColumn )
    {
        m_iIsSortedAsc ? m_iIsSortedAsc = 0 : m_iIsSortedAsc = 1;
    }
    else
    {
        m_iLastSortedColumn = pos;
        m_iIsSortedAsc = 1;
    }
    //mainWdgt->blockSignals( true );
    setVHeader();
    //mainWdgt->blockSignals( false );
}
void TabEdit::refreshSort()
{
    deb(__FUNCTION__, "start");
    deb(__FUNCTION__, "lastSorted: "+GString(m_iLastSortedColumn));
    if( m_iLastSortedColumn == -1 || m_iLastSortedColumn > mainWdgt->columnCount() ) return;
    if( !m_iIsSortedAsc ) mainWdgt->sortItems(m_iLastSortedColumn, Qt::DescendingOrder);
    else mainWdgt->sortItems(m_iLastSortedColumn, Qt::AscendingOrder);
    deb("::refreshSort settting sortInd");
    mainWdgt->horizontalHeader()->setSortIndicator(m_iLastSortedColumn, Qt::AscendingOrder);    

    setVHeader();
    deb(__FUNCTION__, "done");

}
/*****************************************************
*
*****************************************************/
void TabEdit::createActions()
{
    deb(__FUNCTION__, "start");
    //RightClick on a ListWidgetItem
    if( !mainWdgt ) return;



    m_qaCurCell  = new QAction( "Filter: This cell's data", mainWdgt);
    m_qaAddConst = new QAction( "Filter: Add this as constraint", mainWdgt);
    m_qaCurRow   = new QAction( "Filter: This row", mainWdgt );
    m_qaNotNull  = new QAction( "Filter: Not NULL", mainWdgt);
    m_qaDistinct = new QAction( "Distinct: This cell's column", mainWdgt );
    m_qaCount    = new QAction( "Count this cell's occurrences", mainWdgt );
    m_qaGroup    = new QAction( "Group by this column", mainWdgt );
    m_qaUpdate   = new QAction( "Update", mainWdgt );
    m_qaDelTabAll   = new QAction( "Delete table contents", mainWdgt );
    pLineEditFilterAction = new LineEditAction(this);
    pLineEditUpdateAction = new LineEditAction(this);

    pLineEditUpdateAction->setLabel("   Quick Upd.:");
    pLineEditUpdateAction->lineEdit()->setPlaceholderText("Type & Enter");

    connect(pLineEditUpdateAction->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotLineEditUpdate()));
    connect(pLineEditFilterAction->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slotLineEditFilter()));
    connect( m_qaCurCell, SIGNAL( triggered() ), this, SLOT( slotRightClickActionCell() ) );
    connect( m_qaAddConst,  SIGNAL( triggered() ), this, SLOT( slotRightClickActionAddConstraint() ) );
    connect( m_qaCurRow,  SIGNAL( triggered() ), this, SLOT( slotRightClickActionRow() ) );
    connect( m_qaDistinct, SIGNAL( triggered() ), this, SLOT( slotRightClickDistinct()));
    connect( m_qaCount, SIGNAL( triggered() ), this, SLOT( slotRightClickCount()));
    connect( m_qaGroup, SIGNAL( triggered() ), this, SLOT( slotRightClickGroup()));
    connect( m_qaUpdate, SIGNAL( triggered() ), this, SLOT( slotRightClickUpdate()));
    connect( m_qaNotNull, SIGNAL( triggered() ), this, SLOT( slotRightClickNotNull()));
    connect( m_qaDelTabAll, SIGNAL( triggered() ), this, SLOT( slotRightClickDeleteTable()));




    m_qaBitHide  = new QAction( "CharForBit: Hide", mainWdgt);
    m_qaBitAsHex = new QAction( "CharForBit: Show as Hex", mainWdgt);
    m_qaBitAsBin = new QAction( "CharForBit: Show as Binary", mainWdgt );

    connect( m_qaBitHide,   SIGNAL( triggered() ), this, SLOT( slotRightClickActionBitHide() ) );
    connect( m_qaBitAsHex,  SIGNAL( triggered() ), this, SLOT( slotRightClickActionBitAsHex() ) );
    connect( m_qaBitAsBin, SIGNAL( triggered() ), this, SLOT( slotRightClickActionBitAsBin() ) );


    m_qaCopy = new QAction("Copy rows (CTRL+C)", mainWdgt);
    m_qaPaste = new QAction("Paste rows (CTRL+V)", mainWdgt);

    mainWdgt->addAction( m_qaCopy );
    mainWdgt->addAction( m_qaPaste );


    m_qaBitHide->setEnabled(false);
    m_qaBitAsHex->setEnabled(false);
    m_qaBitAsBin->setEnabled(false);

    m_qaEditText = new QAction( "Edit cell data", mainWdgt );
    connect( m_qaEditText, SIGNAL( triggered() ), this, SLOT( slotRightClickActionEditText()));
    m_qaEditText->setEnabled(true);


    m_qaEditLong = new QAction( "Edit LongData", mainWdgt );
    connect( m_qaEditLong, SIGNAL( triggered() ), this, SLOT( slotRightClickActionEditLong()));
    m_qaEditLong->setEnabled(true);

    m_qaEditGraphic = new QAction( "Edit GRAPHIC data", mainWdgt );
    connect( m_qaEditGraphic, SIGNAL( triggered() ), this, SLOT( slotRightClickActionEditGraphic()));
    m_qaEditGraphic->setEnabled(true);

    m_qaEditDBClob = new QAction( "Edit (DB)CLOB data", mainWdgt );
    connect( m_qaEditDBClob, SIGNAL( triggered() ), this, SLOT( slotRightClickActionEditDBClob()));
    m_qaEditDBClob->setEnabled(true);

    m_qaShowXML = new QAction( "Edit XML", mainWdgt );
    connect( m_qaShowXML, SIGNAL( triggered() ), this, SLOT( slotRightClickActionShowXML()));
    m_qaShowXML->setEnabled(false);


    m_qaSaveBlob = new QAction( "Save LOB (LargeObject)", mainWdgt );
    connect( m_qaSaveBlob, SIGNAL( triggered() ), this, SLOT( slotRightClickSaveBlob()));
    m_qaSaveBlob->setEnabled(false);


    m_qaOpenBlob = new QAction( "Display LOB (LargeObject)", mainWdgt );
    connect( m_qaOpenBlob, SIGNAL( triggered() ), this, SLOT( slotRightClickOpenBlob()));
    m_qaOpenBlob->setEnabled(false);

    m_qaOpenBlobAs = new QAction( "Open LOB with...", mainWdgt );
    connect( m_qaOpenBlobAs, SIGNAL( triggered() ), this, SLOT( slotRightClickOpenBlobAs()));
    m_qaOpenBlobAs->setEnabled(false);

    m_qaLoadBlob = new QAction( "Load LOB or XML from file", mainWdgt );
    connect( m_qaLoadBlob, SIGNAL( triggered() ), this, SLOT( slotRightClickLoadBlob() ));
    m_qaLoadBlob->setEnabled(false);

#ifndef MAKE_VC
    //On Linux, I don't know what to do here.
    m_qaOpenBlobAs->setVisible(false);
#endif

    connect( m_qaCopy, SIGNAL( triggered() ), this, SLOT( slotRightClickActionCopy()));
    m_qaCopy->setEnabled(true);


    connect( m_qaPaste, SIGNAL( triggered() ), this, SLOT( slotRightClickActionPaste()));
    m_qaPaste->setEnabled(true);

    deb(__FUNCTION__, "done");

}

int TabEdit::cellsInSingleColumn(const QPoint & p)
{
    QList<QTableWidgetItem*> list = mainWdgt->selectedItems();
    QTableWidgetItem* pItem;
    int col = -1;
    if( list.count() < 2) return 0;
    pItem = mainWdgt->itemAt(p);

    //If rightClick event is on a different column than currently selctedItems: return
    int currentCol = pItem->column();

    for(int i = 0; i < list.count(); ++i )
    {
        pItem = list.at(i);
        if( col < 0 ) col = pItem->column();
        else if( col != pItem->column() ) return 0;
        if( currentCol != col )
        {
            mainWdgt->clearSelection();
            return 0;
        }
    }
    return 1;
}

void  TabEdit::popUpActions(const QPoint & p)
{
    GString allCmds = GString(QApplication::clipboard()->text());
    actionsMenu.clear();
    //QMenu * userMenu = new QMenu("More...");
    QMenu userMenu("More...", this);
    if( cellsInSingleColumn(p) )
    {
        actionsMenu.addAction(pLineEditUpdateAction);
        pLineEditUpdateAction->lineEdit()->setFocus();
        pLineEditUpdateAction->lineEdit()->selectAll();
        pLineEditUpdateAction->lineEdit()->setPlaceholderText("Type & Enter");
    }
    else
    {
        actionsMenu.addAction(pLineEditFilterAction);
        pLineEditFilterAction->lineEdit()->setFocus();
        pLineEditFilterAction->lineEdit()->selectAll();
        pLineEditFilterAction->lineEdit()->setPlaceholderText("Enter value (or +value)");


        actionsMenu.addAction( m_qaDistinct );
        actionsMenu.addAction( m_qaCurCell );
        actionsMenu.addAction( m_qaAddConst );
        actionsMenu.addAction( m_qaCurRow );
        actionsMenu.addAction( m_qaNotNull );
        actionsMenu.addSeparator();
        actionsMenu.addAction( m_qaDistinct );
        actionsMenu.addAction( m_qaCount );
        actionsMenu.addAction( m_qaGroup );
        actionsMenu.addSeparator();
        //QMenu updateMenu("Update");
        actionsMenu.addAction( m_qaUpdate );
        actionsMenu.addSeparator();
        actionsMenu.addAction( m_qaDelTabAll );
        actionsMenu.addSeparator();
        //updateMenu.addAction(pLineEditUpdateAction);
        //actionsMenu.addMenu(&updateMenu);
        actionsMenu.addSeparator();
        actionsMenu.addSeparator();
        actionsMenu.addAction( m_qaEditText );
        actionsMenu.addAction( m_qaEditLong );
        actionsMenu.addAction( m_qaEditGraphic );
        actionsMenu.addAction( m_qaEditDBClob );
        actionsMenu.addAction( m_qaShowXML );
        actionsMenu.addAction( m_qaSaveBlob );
        actionsMenu.addAction( m_qaOpenBlob );
        actionsMenu.addAction( m_qaOpenBlobAs );
        actionsMenu.addAction( m_qaLoadBlob );
        actionsMenu.addSeparator();
        actionsMenu.addAction( m_qaCopy );
        actionsMenu.addAction( m_qaPaste );


        deb(__FUNCTION__, "Data in clipboard: "+allCmds);
        if( allCmds.occurrencesOf(_PMF_CMD) == 0 ) m_qaPaste->setEnabled(false);
        else m_qaPaste->setEnabled(true);
        m_qaPaste->setEnabled(true);

        actionsMenu.addSeparator();
        createUserActions(&userMenu);

        actionsMenu.addMenu(&userMenu);

    }
    if(p.isNull()) return;
    if( !m_pActionItem )
    {
        disableActions();
        m_qaCopy->setEnabled(true);
        m_qaPaste->setEnabled(true);
    }

    actionsMenu.exec(mainWdgt->mapToGlobal(p));

}
void TabEdit::getUserAction()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if( !action ) return;
    GString fileName = userActionsDir() + GString(action->data().toString());
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        msg("Cannot read file "+fileName);
        return;
    }
    QTextStream in(&f);
    GString s = in.readAll();
    f.close();    

    s = formatActionText(s);
    if( s.occurrencesOf("@COL@") || s.occurrencesOf("@TABLE@") || s.occurrencesOf("@DATA@"))
    {
        QTableWidgetItem *pItem =  currentItem();

        if( !pItem ) return;
        GString col = m_pMainDSQL->hostVariable(pItem->column()-1);
        s = s.change("@COL@", col).change("@TABLE@", currentTable()).change("@DATA@", pItem->text());
    }

    m_pPMF->createNewTab(s, 1);

    //    m_gstrLastFilter = s;
    //    cmdLineLE->setText(s);
    //    okClick();

}



GString TabEdit::formatActionText(GString in)
{   //Todo: basically the same code in extSQL::runClicked(), maybe move to helper class.
    GString line, out;
    char cr = '\n';
    in = in.stripTrailing(cr);
    while( in.length() )
    {
        if( in.occurrencesOf(cr) == 0 )
        {
            line = in;
            if( line.occurrencesOf("--") > 0 ) line = line.subString(1, line.indexOf("--") - 1);
            break;
        }
        line = in.subString(1, in.indexOf(cr)-1);
        if( line.occurrencesOf("--") > 0 ) line = line.subString(1, line.indexOf("--") - 1);
        in = in.remove(1, in.indexOf(cr));
        if( line.occurrencesOf("--") > 0 ) line = line.subString(1, line.indexOf("--") - 1);
        out += line + " ";
    }
    out += line;
    return out;
}


void TabEdit::openActionEditor()
{

    UserActions * ua = new UserActions(m_pGDeb, this);
    ua->show();
}

void TabEdit::createUserActions(QMenu * menu)
{

    /***************************************************
    * This is rather nice: The menu-items are created during
    * run-time, the items are connected to the same slot.
    * See http://doc.trolltech.com/4.6/mainwindows-recentfiles-mainwindow-cpp.html
    */
    QDir dir(userActionsDir());
    QStringList files = dir.entryList(QDir::Files);
    int i;
    for( i = 0; i < files.size() && i < MaxUserActions; ++i )
    {
        m_qaUserActions[i] = new QAction(this);
        m_qaUserActions[i]->setText(files.at(i));//Name of the file
        m_qaUserActions[i]->setData(files.at(i));
        menu->addAction(m_qaUserActions[i]);
        connect(m_qaUserActions[i],SIGNAL(triggered()), this, SLOT(getUserAction()));

    }
    menu->addSeparator();
    m_qaUserActions[i] = new QAction(this);
    m_qaUserActions[i]->setText("Add/Edit Actions");
    menu->addAction(m_qaUserActions[i]);
    connect(m_qaUserActions[i],SIGNAL(triggered()), this, SLOT(openActionEditor()));

}
/*****************************************************
*
*****************************************************/
void TabEdit::addLowerPart(QGridLayout* grid)
{
    //NewRowButton
    int row3 = 3, row4 = 4, row5 = 5, row6 = 6;

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QGroupBox * buttonsBox = new QGroupBox(this);
    buttonsBox->setFlat(true);
    insertB = new QPushButton(this);
    insertB->setText("Clo&ne Row");

    //QToolTip::add(insertB, "Insert new row");
    connect(insertB, SIGNAL(clicked()), SLOT(insClick()));
    /////insertB->setFixedHeight( insertB->sizeHint().height() );
    insertB->setToolTip("Creates a new row, don't forget to save");
    buttonsLayout->addWidget(insertB);
    //SaveButton
    saveB = new QPushButton(this);
    saveB->setText("&Save");
    saveB->setToolTip("Commits all changed rows");
    //QToolTip::add( changeB, "Update edited row");
    connect(saveB, SIGNAL(clicked()), SLOT(chgClick()));
    buttonsLayout->addWidget(saveB);

    //DeleteButon
    deleteB = new QPushButton(this);
    deleteB->setText("&Delete (DEL)");
    deleteB->setToolTip("Deletes all selected rows");
    //QToolTip::add(deleteB, "Delete selected rows(s)");
    connect(deleteB, SIGNAL(clicked()), SLOT(delButtonClick()));
    buttonsLayout->addWidget(deleteB);

    //CancelButon
    cancelB = new QPushButton(this);
    cancelB->setText("Cance&l");
    //QToolTip::add(deleteB, "Cancel current transactio");
    connect(cancelB, SIGNAL(clicked()), SLOT(cancelClick()));
    buttonsLayout->addWidget(cancelB);
    if( !m_iUseThread ) cancelB->hide();

    //Run Buton
    runB = new QPushButton(this);
    runB->setText("&Run Cmd (F5)");
    runB->setAutoDefault(true);

    //QToolTip::add(runB, "Executes the SQL-Statement in the field below");
    connect(runB, SIGNAL(clicked()), SLOT(okClick()));
    buttonsLayout->addWidget(runB);


    //ExtendedSQLButton
    extSQLB = new PmfPushButton(this);
    extSQLB->setText("SQL &Editor");
    extSQLB->setToolTip("A primitive editor for multi-lined SQL");
    connect(extSQLB, SIGNAL(clicked()), SLOT(extSQLClick()));
    buttonsLayout->addWidget(extSQLB);
    //connect(extSQLB, SIGNAL(fileWasDropped(GString )), SLOT(loadSqlEditor(GString)));
    connect(extSQLB, SIGNAL(fileListWasDropped(QDropEvent* )), SLOT(loadSqlEditor(QDropEvent*)));


    //Export button
    exportButton = new QPushButton(this);
    exportButton->setText("E&xport");
    exportButton->setToolTip("Export table to file");
    connect(exportButton, SIGNAL(clicked()), SLOT(exportData()));
    exportButton->hide();

    //buttonsLayout->addWidget(exportButton);
    //Import button
    importButton = new QPushButton(this);
    importButton->setText("&Import");
    importButton->setToolTip("Import data from file");
    connect(importButton, SIGNAL(clicked()), SLOT(importData()));
    importButton->hide();
    //buttonsLayout->addWidget(importButton);

    //Import drop zone
    pPmfDropZone = new PmfDropZone(this);
    pPmfDropZone->setMaximumWidth(300);
    pPmfDropZone->setPlaceholderText("Import: Drag&drop file(s) here");
    connect(pPmfDropZone, SIGNAL(fileWasDropped()), SLOT(importData()));    
    buttonsLayout->addWidget(pPmfDropZone);

    buttonsBox->setLayout(buttonsLayout);
    m_mainGrid->addWidget(buttonsBox, row3, 0, 1, 10);

    //NewLineButton
    /********
   newLineB = new QPushButton();
   newLineB->setText("New row");
   //QToolTip::add(newLineB, "Create a row with dummy values");
   connect(newLineB, SIGNAL(clicked()), SLOT(newLineClick()));
   grid->addWidget(newLineB, row3, 8);
*********/
    ///commandLine
    cmdText = new QLabel("SQL Cmd (F7):", this);
    //cmdText->setFixedHeight( cmdText->sizeHint().height() );
    grid->addWidget(cmdText, row4, 0);


    cmdLineLE = new TxtEdit(m_pGDeb, this, 1);
    //QToolTip::add(cmdLineLE, "Your SQL-Statement here");
#ifndef MAKE_VC
    cmdLineLE->setFixedHeight( QFontMetrics( m_pPMF->font()).height()+7 );
#endif


    grid->addWidget(cmdLineLE, row4, 1, 1, 9);
    cmdLineLE->setToolTip("Any SQL statement, do *not* prefix 'db2'");
#if QT_VERSION >= 0x050000
    //cmdLineLE->setPlaceholderText("Hit CTRL+E for text completion");
#endif



    //HostCB
    QLabel * createText = new QLabel("Filter", this);
    //createText->setFixedHeight( createText->sizeHint().height() );
    grid->addWidget(createText, row5, 0);

    hostCB = new QComboBox(this);
    hostCB->setFixedHeight( hostCB->sizeHint().height() );
    hostCB->setToolTip("Generates SQL statement");
    //QToolTip::add(hostCB, "HostVariables (i.e. Columnnames) of selected table. Select to create a SQL-Statement.");
    connect(hostCB, SIGNAL(activated(int)), SLOT(setHost(int)));
    grid->addWidget(hostCB, row5, 1, 1, 2);

    /*
 //LikeCB
   likeCB = new QComboBox();
   likeCB->setFixedHeight( likeCB->sizeHint().height() );
   connect(likeCB, SIGNAL(activated(int)), SLOT(setLike(int)));
   grid->addWidget(likeCB, row5, 3);
*/
    QLabel * orderTxt = new QLabel("Order by", this);
    orderTxt->setAlignment(Qt::AlignCenter | Qt::AlignRight);
    grid->addWidget(orderTxt, row5, 3);
    //orderCB
    orderCB = new QComboBox(this);
    connect(orderCB, SIGNAL(activated(int)), SLOT(setOrder(int)));
    //QToolTip::add(orderCB, "Add 'order by' to SQL-Statement");
    orderCB->setFixedHeight( orderCB->sizeHint().height() );
    grid->addWidget(orderCB, row5, 4, 1, 2);
    //clearButton
    clearB = new QPushButton(this);
    clearB->setText("Clear");
    clearB->setToolTip("Clear field above");
    //QToolTip::add(clearB, "Clear SQL-CMD field above");
    connect(clearB, SIGNAL(clicked()), SLOT(clearButtonClicked()));
    grid->addWidget(clearB, row5, 6);

    //FilterComboBox
    QLabel * filterText = new QLabel("Stored Cmds", this);
    //filterText->setFixedHeight( filterText->sizeHint().height() );
    grid->addWidget(filterText, row6, 0);
    filterCB = new QComboBox(this);
    //QToolTip::add(filterCB, "Buffer for issued SQL-Statements");
    connect(filterCB, SIGNAL(activated(int)), SLOT(setCmdLine(int)));
    filterCB->setFixedHeight( filterCB->sizeHint().height() );
    filterCB->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    grid->addWidget(filterCB, row6, 1, 1, 8);

    //FilterByTableCheckBox
    filterByTableCB = new QCheckBox("Filter by table", this);
    grid->addWidget(filterByTableCB, row6, 9);
    filterByTableCB->setChecked(true);
    connect(filterByTableCB, SIGNAL(stateChanged(int)), SLOT(slotFilterHistByTable()));

}
void TabEdit::mainWdgtDoubleClicked()
{
    msg("GotClick");
}


void TabEdit::refreshClicked()
{}
/*****************************************************
*
* Sorting gets disabled. Clicking on the horizontalHeader enables it [see ::sortClicked].
* If sorting is enabled, changing data triggers sorting immediately and
* the whole row gets repositioned - VERY annoying.
*
*****************************************************/
void TabEdit::itemChg(QTableWidgetItem* cur)
{
    //Disable sorting while editing cells.
    mainWdgt->setSortingEnabled(false);

    deb(__FUNCTION__, ": "+GString(cur->row()));
    QTableWidgetItem * pItem = mainWdgt->verticalHeaderItem(cur->row());
    if( NULL == pItem ) return;
    if(GString(pItem->text()) == _NEWRow ) return;
    //Catch "NULL" -> ""
    if( !cellDataChanged(cur) ) return;

    markForUpd(cur);
    if(GString(pItem->text()) != _INSRow )	pItem->setText(_UPDRow);
    setPendingMsg();
}

void TabEdit::markForUpd(QTableWidgetItem * pItem)
{
    QTableWidgetItem * p_Item = mainWdgt->item(pItem->row(), 1);
    if( !p_Item )
    {
        msg("::markForUpd: NULL pointer on item. That's bad. Maybe reloading the table will help.");
        return;
    }
    //If the row was just inserted, do nothing. It will be saved later.
    if( GString(p_Item->text()) == _INSMark ) return;
    mainWdgt->item(pItem->row(), 1)->setText("U");
    //Qcolor colr = QColor(184, 251, 255);
    setRowBackGroundColor(pItem, _ColorUpd, 1);
}

void TabEdit::setRowBackGroundColor(int row, QColor color)
{
    if( !m_iUseColorScheme ) return;
    for(int i = 2; i < mainWdgt->columnCount(); ++i)
    {
        mainWdgt->item(row, i)->setBackgroundColor(color);
    }
}

void TabEdit::setRowBackGroundColor(QTableWidgetItem *pItem, QColor color, int setBold)
{
    if( !m_iUseColorScheme ) return;
    int row = pItem->row();
    if( setBold )
    {
        QFont font = this->font();
        font.setBold(true);
    }
    setRowBackGroundColor(row, color);
}

void TabEdit::markForIns(QTableWidgetItem * pItem)
{
    deb(__FUNCTION__, "start");
    mainWdgt->item(pItem->row(), 1)->setText(_INSMark);
    setRowBackGroundColor(pItem, _ColorIns);
    deb(__FUNCTION__, "done");
}

int TabEdit::deleteViaFetchFromCmdLE(GString cmd)
{
    if( GString(cmd).strip().upperCase().indexOf("DELETE") != 1 ) return 1;
    GString tabName = GStuff::TableNameFromStmt(cmd);
    if( !tabName.length() ) return 1;

    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    PmfTable table(pDSQL, tabName);
    GSeq <GString> colSeq;
    if( table.uniqueColCount() > 0 )
    {
        colSeq = table.uniqueColNames();
    }
    GString whereClause = GStuff::WhereClauseFromStmt(cmd);
    //int rc = pDSQL->deleteViaFetch(tabName, &colSeq, 1000, whereClause );
    int rc = this->deleteViaFetch(tabName, whereClause );
    delete pDSQL;
    return rc;
}

int TabEdit::deleteViaFetch(GString tableName, GString whereClause)
{
    GStuff::dormez(100);
    QCoreApplication::processEvents();
    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);

    PmfTable table(pDSQL, currentTable());
    GSeq <GString> colSeq;
    if( table.uniqueColCount() > 0 )
    {
        colSeq = table.uniqueColNames();
    }

    GString err;

    QProgressDialog qpd("Deleting rows...", "Cancel", 0, 1, this);
    qpd.setWindowModality(Qt::WindowModal);
    qpd.setValue(1);

    GString labelText;
    qpd.setMinimumDuration(0);
    qpd.show();

    qpd.setWindowTitle("Counting rows...");
    GString countCmd = "SELECT COUNT(*) FROM "+tableName;
    if( whereClause.length() ) countCmd += " WHERE "+whereClause;

    err = pDSQL->initAll( countCmd );
    if( err.length() )
    {
        msg(err);
        delete pDSQL;
        return 0;
    }
    int rows = pDSQL->rowElement(1,1).asInt();
    qpd.setRange(0, rows);
    qpd.setWindowTitle("Deleting...");

    int cnt = 0, rc = 0;
    //while( !pDSQL->tableIsEmpty(currentTable()) )
    while( !rc && cnt*1000 < rows)
    {
        rc = pDSQL->deleteViaFetch(tableName, &colSeq, 1000, whereClause );
        if( rc )
        {
            if( rc != 100 ) msg(pDSQL->sqlError());
            break;
        }
        cnt++;
        qpd.setValue(cnt*1000);
        QCoreApplication::processEvents();
        GStuff::dormez(5);
        labelText = "Deleted "+GString(cnt*1000)+" of "+GString(rows);

//        if( cnt % 4 == 0 ) title += "|";
//        else if( cnt % 4 == 1 ) title += "/";
//        else if( cnt % 4 == 2 ) title += "-";
//        else if( cnt % 4 == 3 ) title += "\\";

        qpd.setLabelText(labelText);
        qpd.repaint();
        if( qpd.wasCanceled() )
        {
            break;
        }
    }
    qpd.setValue(rows);
    delete pDSQL;
//    cmdLineLE->setText("");
//    initDSQL();
    return 0;
}


void TabEdit::delButtonClick()
{
    deleteRows();
}


/*****************************************************
*
*****************************************************/
void TabEdit::deleteRows()
{
    deb(__FUNCTION__, "start");

    if( myTabText() == _newTabString || myTabText() == GString(_SQLCMD) )
    {
        msg(GString("Please select a table first."));
        return;
    }

    GString err;
    int ignoreErrors = 0;
    int yesToAll = 0;
    int rowsNotFound = 0;
    int rc = 0;
    QItemSelectionModel* selectionModel = mainWdgt->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);
    deb(__FUNCTION__, "selCount: "+GString(selected.count()));
    if( selected.count() == 0 )
    {
        msg("No rows selected.\nTo select a row, click on the vertical header.");
        return;
    }
    if( QMessageBox::question(this, "PMF", "Delete selected rows(s)?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return ;
    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);



    if( singleRowCHK->isChecked() || !hasUsableColumns() )
    {
        deleteByCursor(pDSQL);
        delete pDSQL;
        return;
    }
    GSeq <GString> unqColSeq;
    pDSQL->getUniqueCols(currentTable(), &unqColSeq);


    mainWdgt->blockSignals( true );
    pDSQL->setAutoCommmit(0);
    for(int i= 0; i< selected.count();i++)
    {
        deb(__FUNCTION__, "In loop, i: "+GString(i));
        QModelIndex index = selected.at(i);
        if( mainWdgt->item(index.row()-i,1) == NULL )
        {
            deb(__FUNCTION__, "Bugger: Item at modelIndex i="+GString(i)+" is NULL");
            continue;
        }
        //The second (hidden) column may contain 'I' or 'U'
        //This row was inserted, but not yet saved. Just remove it from mainWdgt.
        if(mainWdgt->item(index.row()-i, 1)->text() == "I")
        {
            mainWdgt->removeRow(index.row()-i);
            continue;
        }
        if(mainWdgt->item(index.row()-i, 1)->text() == "N")
        {
            mainWdgt->removeRow(index.row()-i);
            continue;
        }
        if(mainWdgt->item(index.row()-i, 1)->text() == "U")
        {
            continue;
        }

        deb(__FUNCTION__, "calling deleteThisRow, yta="+GString(yesToAll)+", item expected at "+GString(index.row()-i));
        err = deleteThisRow(index.row()-i, pDSQL, &unqColSeq, yesToAll);

        if( pDSQL->sqlCode() == 100 && !yesToAll)
        {
            rowsNotFound++;
            i++;
        }
        if( err == "FORCEBREAK" ) break;
        else if( err == "YESTOALL" )
        {
            yesToAll = 1;
            deb(__FUNCTION__, "calling deleteThisRow AGAIN, yta="+GString(yesToAll));
            deleteThisRow(index.row()-i, pDSQL, &unqColSeq, yesToAll);
        }
        else
        {
            deb(__FUNCTION__, "Need to check handleDeleteError...");
            rc=handleDeleteError( pDSQL->sqlCode(), err, &ignoreErrors);
            if( rc ) break;
        }
    }
    mainWdgt->blockSignals( false );
    //msg("end del");
    pDSQL->setAutoCommmit(1);
    setPendingMsg();
    setVHeader();
    delete pDSQL;
    deb(__FUNCTION__, "done");
    if( rowsNotFound && !ignoreErrors )
    {
        msg("Could not delete "+GString(rowsNotFound)+" of "+GString(selected.count())+" row(s).\nThey have been either updated or deleted by another transaction.");
    }
    if( ignoreErrors || rowsNotFound || !rc)
    {
        if( cmdLineLE->toPlainText().length() ) reload(cmdLineLE->toPlainText());
        else
        {
            setMyTabText(currentTable(0));
            reload("SELECT * FROM "+currentTable());
        }

    }
    else createEmptyRow();
}
int TabEdit::handleDeleteError(int sqlcode, GString errTxt, int * ignoreErr)
{
    deb(__FUNCTION__, "handleDeleteError start, sqlcode: "+GString(sqlcode));
    if( sqlcode == -206 )
    {
        GString s = "Hint: PMFs current table is "+currentTable();
        s +="\n You appear to be working on a different table.\n ";
        s += "Open the table you want to operate on.";
        msg(s);
        return 1;
    }
    else if ( errTxt.length() && !(*ignoreErr) && sqlcode != 100 )
    {
        QMessageBox msgBox;
        msgBox.setText(errTxt);
        msgBox.setInformativeText("Continue?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Cancel ) return 2;
        else *ignoreErr = 1;
    }
    return 0;
}
/*************************************************************
 * For UPD and DEL we need at least one column that can be used as constraint.
 * If for example the table has just a single XML column, we cannot use
 * this coulmn to constrain DELETES or UPDATES.
 * A DELETE would then DELETE all rows.
 ************************************************************/
int TabEdit::hasUsableColumns()
{
    int usableCols = 0;
    for( unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        if( m_pMainDSQL->isLOBCol(i)  || m_pMainDSQL->isXMLCol(i) || m_pMainDSQL->isLongTypeCol(i)
                || m_pMainDSQL->simpleColType(i) == CT_GRAPHIC) continue;
        usableCols++;
    }
    return usableCols;
}

/*****************************************************
*
*****************************************************/
GString TabEdit::deleteThisRow(long pos, DSQLPlugin * pDSQL, GSeq<GString> *unqCols,  int yesToAll)
{
    deb(__FUNCTION__, "start");


    if( pos < 0 || pos > mainWdgt->rowCount()) return 1;

    QTableWidgetItem * item;
    GString cmd, col, val;

    item = mainWdgt->item(pos, 0);
    if( !item ) return "Item is NULL, this should not happen. Reload the table, please.";

    if(!yesToAll)
    {
        if( hasTwin(item)  )
        {

            QMessageBox msgBox;
            QString msg = "There are either identical rows in this table\n-OR-\nonly a subset of COLUMNS is currently selected.\n\n";
            msg += "If you continue, more than the currently selected rows will be deleted.\n\n";
            msg += "You can\n-check 'Single row' in the upper-right corner to delete only the first of multiple identical rows";
            msg += "\n-OR-\nchange the current SELECT\n-OR-\nclick OK to continue to delete mutliple rows.";
            msgBox.setText("Caution: More than the currently selected rows will be deleted.\nContinue?");
            msgBox.setDetailedText (msg);
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::YesToAll);
            int ret = msgBox.exec();
            if( ret == QMessageBox::Cancel ) return "FORCEBREAK";
            else if( ret == QMessageBox::YesToAll ) return "YESTOALL";
        }
    }
    ///TODO: use "tablename()" here

    GString unqDel = createUniqueColConstraintForItem(item, unqCols);
    if( unqDel.length() )cmd = "DELETE FROM "+currentTable() + unqDel;
    else
    {
        cmd = "DELETE FROM "+currentTable()+" WHERE ";
        for( unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
        {
            item = mainWdgt->item(pos, i+1);
            if( !item ) break;
            val = item->text();
            pDSQL->convToSQL(val);
            col = wrap(m_pMainDSQL->hostVariable(i));

            if( !val.strip().length() || val.subString(1, 6) == "@DSQL@"  || m_pMainDSQL->isXMLCol(i)
                    || m_pMainDSQL->isTruncated(pos+1, i) ) continue;
            //Assumption: LONG data types cannot be part of UNQ/PRIM keys
            if( m_pMainDSQL->isLongTypeCol(i) && unqCols->numberOfElements()>0 ) continue;
            if( m_pMainDSQL->simpleColType(i) == CT_GRAPHIC && unqCols->numberOfElements()>0 ) continue;

            if( val == "NULL")cmd += col + " IS NULL AND ";
            else if( m_pMainDSQL->isForBitCol(i) >= 3 ) cmd += col + "=x"+ val + " AND ";
            else cmd += col + "="+ val + " AND ";
        }
        cmd = cmd.stripTrailing(" AND ");
    }
    deb(__FUNCTION__, cmd);
    //msg(cmd);
    GString err = pDSQL->initAll(cmd);


    if( pDSQL->sqlCode() == 0 || !err.length() ) mainWdgt->removeRow(pos);
    if( pDSQL->sqlCode() == 100 && yesToAll )
    {
        mainWdgt->removeRow(pos);
        err = "";
    }

    deb(__FUNCTION__, "done");
    return err;
}
/*****************************************************
*
*****************************************************/
GString TabEdit::deleteByCursor(DSQLPlugin  *pDSQL)
{
    deb(__FUNCTION__, "start");
    GString filter, command, error;
    QTableWidgetItem * pItem;
    filter = cmdLineLE->toPlainText();

    if( filter.length() == 0 ) filter = "SELECT * FROM "+currentTable();
    command = "DELETE FROM "+currentTable();

    QItemSelectionModel* selectionModel = mainWdgt->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

    deb(__FUNCTION__, "selCount: "+GString(selected.count()));
    int ignoreErr = 0;
    for(int i=selected.count()-1; i >= 0; i--)
    {
        QModelIndex index = selected.at(i);
        pItem = mainWdgt->item(index.row(), 0);
        if( !pItem ) return ("Nothing selected");
        pDSQL->setGDebug(m_pGDeb);
        error = pDSQL->currentCursor( filter, command, pItem->text().toLong());
        if( handleDeleteError(pDSQL->sqlCode(), error, &ignoreErr) ) break;
    }
    if( cmdLineLE->toPlainText().length() ) reload(cmdLineLE->toPlainText());
    else
    {
        setMyTabText(currentTable(0));
        reload("SELECT * FROM "+currentTable());
    }
    return "";
}
/*****************************************************
*
*****************************************************/
void TabEdit::insClick()
{
    deb(__FUNCTION__, "start. myID: "+GString(m_iTabID)+", tabText: "+myTabText());
    //No rows, because no table was opened:
    if( myTabText() == _newTabString || myTabText() == GString(_SQLCMD) )
    {
        msg(GString("Please select a table first."));
        return;
    }
    //There are rows, but none is selected.
    deb(__FUNCTION__,"rowCount(1): "+GString(mainWdgt->rowCount())+", tab: "+currentTable());
    if( mainWdgt->selectedItems().count() == 0 && mainWdgt->rowCount() )
    {
        msg("Select a row, the cloned row will be created below the selected.");
        return;
    }
    //No rows, create empty row
    deb(__FUNCTION__,"rowCount(2): "+GString(mainWdgt->rowCount())+", tab: "+currentTable());
    if( mainWdgt->rowCount() ==  0 )createNewRow(-1);
    else createNewRow(mainWdgt->currentRow());
    setPendingMsg();
    deb(__FUNCTION__, "done");
}
/*****************************************************
*
* rowPos is negative if an empty table was opened.
*
*****************************************************/
int TabEdit::createNewRow(int rowPos)
{
    deb(__FUNCTION__, "Start, rowPos: "+GString(rowPos));
    QTableWidgetItem *newItem;
    mainWdgt->insertRow( rowPos+1 );


    newItem = new QTableWidgetItem("-");
    mainWdgt->setItem(rowPos+1, 0, newItem);
    newItem = new QTableWidgetItem("I");
    mainWdgt->setItem(rowPos+1, 1, newItem);
    QString newVal, colName;


    mainWdgt->setVerticalHeaderItem(rowPos+1, new QTableWidgetItem((char*) _NEWRow));
    for( int i = 2; i < mainWdgt->columnCount(); ++ i )
    {
        if( rowPos >= 0 )newVal = mainWdgt->item(rowPos, i)->text();
        else newVal = (char*) m_pMainDSQL->realName(m_pMainDSQL->sqlType(i-1)); //Create empty items.

        colName = mainWdgt->horizontalHeaderItem(i)->text();
        if(isIdentityColumn(colName) ) newVal = (char*)_IDENTITY;
        else if( colType(i-1) == 2 ) newVal = "";  //TODO: ColType by name, not idx

        newItem = new QTableWidgetItem( newVal );

        mainWdgt->setItem(rowPos+1, i, newItem);
    }
    mainWdgt->setCurrentItem(mainWdgt->item(rowPos+1, 0), QItemSelectionModel::Select);
    mainWdgt->editItem(mainWdgt->item(rowPos+1, 2));
    mainWdgt->setRowHeight(rowPos+1, QFontMetrics( mainWdgt->font()).height()+5);

    //To set the verticalHeader's text, we need to (re)set rownumbers    
    setVHeader(0);

    //ToDo: SetFocus, SetEdit, ColorNewRow?
    deb(__FUNCTION__, "setting vHeader at "+GString(rowPos+1));
    mainWdgt->verticalHeaderItem(rowPos+1)->setText(_INSRow);
    deb(__FUNCTION__, "setting vHeader at "+GString(rowPos+1)+" done");
    return 0;

}

void TabEdit::setFocusIntoEmptyRow()
{
    deb(__FUNCTION__, "start");
    GString txt;
    for (int i = 0; i < mainWdgt->rowCount(); ++i)
    {
        txt = mainWdgt->item(i, 1)->text();
        if( txt == _NEWMark)
        {
            mainWdgt->editItem(mainWdgt->item(i, 2));
            mainWdgt->setCurrentItem(mainWdgt->item(i, 2));
            if( i == mainWdgt->rowCount()-1 ) mainWdgt->scrollToBottom();
            break;
        }
    }
    deb(__FUNCTION__, "done");
}




int TabEdit::createEmptyRow()
{

    deb(__FUNCTION__, "start");
    QTableWidgetItem *newItem;
    int rowPos = mainWdgt->rowCount();
    deb(__FUNCTION__, "rows: "+GString(rowPos));
    mainWdgt->insertRow( rowPos );


    newItem = new QTableWidgetItem("-");
    mainWdgt->setItem(rowPos, 0, newItem);
    newItem = new QTableWidgetItem((char*)_NEWMark);
    mainWdgt->setItem(rowPos, 1, newItem);
    QString newVal, colName;

    for( int i = 2; i < mainWdgt->columnCount(); ++ i )
    {
        colName = mainWdgt->horizontalHeaderItem(i)->text();
        if(isIdentityColumn(colName) ) newVal = (char*)_IDENTITY;
        else newVal = "";
        newItem = new QTableWidgetItem( newVal );
        mainWdgt->setItem(rowPos, i, newItem);
    }
    enableActions(0);
    //mainWdgt->setCurrentItem(mainWdgt->item(rowPos, 0), QItemSelectionModel::Select);
    //mainWdgt->editItem(mainWdgt->item(rowPos, 0));

    //To set the verticalHeader's text, we need to (re)set rownumbers
    //setVHeader();
    //ToDo: SetFocus, SetEdit, ColorNewRow?
    //mainWdgt->verticalHeaderItem(rowPos)->setText(_NEWRow);
    deb(__FUNCTION__, "end");
    return 0;
}

int TabEdit::checkEmptyRow(int rowPos )
{
    GString data;
    for( int i = 2; i < (int)mainWdgt->columnCount(); ++ i )
    {
        data = GString(mainWdgt->item(rowPos, i)->text());
        if( data == _IDENTITY ) continue;
        if( data.strip().length() ) return 1;
    }
    return 0;
}


/*****************************************************
* clicking "save" triggers this.
*****************************************************/
int TabEdit::chgClick()
{
    deb(__FUNCTION__, "start");
    //This is, well, dirty. If a user edits a cell and uses CTRL+S (Save) [not the mouse!]
    //the data contained in cell->text() is still the original data. The cell has to lose
    //focus to really get updated. So we make it lose focus.
    //See also ::clearMainWdgt
    GString s;
    if( !cmdLineLE->hasFocus() ) saveB->setFocus();
    int setFocus = 0;



    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    GSeq <GString> unqColSeq;
    pDSQL->getUniqueCols(currentTable(), &unqColSeq);

    int count = 0;
    QTableWidgetItem * actItem;
    for( int i = 0; i < mainWdgt->rowCount(); ++i)
    {
        actItem = mainWdgt->item(i, 1); //2nd column contains the action
        if( NULL == actItem ) continue;
        if( _UPDMark  == GString(actItem->text()) )
        {
            s = updateRow(actItem, pDSQL, &unqColSeq);
            count++;
        }
        else if( _INSMark  == GString(actItem->text()) )
        {
            s = insertRow(actItem, pDSQL );
            count++;
        }
        else if( _NEWMark  == GString(actItem->text()) )
        {
            if(checkEmptyRow(i))
            {
                s = insertRow(actItem, pDSQL );
                setFocus = 1;
                count++;
            }
        }
        else continue;
        if( pDSQL->sqlCode() == -206 )
        {

            s +="\nHint: PMFs current table is "+currentTable();
            s +="\nYou appear to be working on a different table.\n ";
            s += "Open the table you want to operate on.";
            msg(s);
            break;
        }
        else if( pDSQL->sqlCode() == -798 )
        {
            msg("Error: "+m_pMainDSQL->sqlError() +"\nSolution: Simply leave the offending field empty!");
            break;
        }
        else if( pDSQL->sqlCode() == -401 )
        {
            msg("Error: "+m_pMainDSQL->sqlError() +"\nTry again with 'Single Row' checked (in the uppper right corner)");
            break;
        }
        else if( s.strip().length() )
        {
            if( mainWdgt->rowCount() > 1 ) msg( s +"\n\nHint: You should probably refresh (press F5) the view.");
            else msg( s );
            break;
        }

        if( pDSQL->sqlCode() && s.length() )
        {
            QMessageBox msgBox;
            msgBox.setText(s);
            msgBox.setInformativeText("Continue with the remaining tasks?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            int ret = msgBox.exec();
            if( ret == QMessageBox::Cancel ) break;

        }

    }
    deb(__FUNCTION__, "sqlCode: "+GString(pDSQL->sqlCode())+", sqlMsg: "+s+"<-");

    //If something went wrong, do not reload.

    if( pDSQL->sqlCode() || s.strip().length() || count == 0 )
    {
        delete pDSQL;
        if( setFocus ) setFocusIntoEmptyRow();
        else
        {
            if( !cmdLineLE->hasFocus() ) mainWdgt->setFocus();
        }
        return -1;
    }

    if( cmdLineLE->toPlainText().length() )
    {
        ///setMyTabText(m_iTabID, _SQLCMD);
        reload(cmdLineLE->toPlainText());
        addToCmdHist(cmdLineLE->toPlainText());
    }
    else
    {
        setMyTabText(currentTable(0));
        reload("SELECT * FROM "+currentTable());
        addToCmdHist("SELECT * FROM "+currentTable());
    }
    delete pDSQL;
    if( setFocus ) setFocusIntoEmptyRow();
    else mainWdgt->setFocus();
    deb(__FUNCTION__, "done");
    return count;
}
/**********************************************************************
* THIS IS JUST A SKETCH, DO NOT USE
**********************************************************************/

void TabEdit::timerCalled()
{

    if( m_ulTimerEvts >=255 ) m_iTimerDirections = -1;
    else if( m_ulTimerEvts <= 0 ) m_iTimerDirections = 1;
    m_ulTimerEvts = m_ulTimerEvts + m_iTimerDirections;
    colorGradeWidget();
    return;
    //This method gets called if m_iUseThread==1
    if( _threadRunning || !m_iUseThread) return;

    //currently not necessary
    //m_mainDSQL = _staticDSQL;
    deb(__FUNCTION__, "rows in statDSQL: "+GString(_staticDSQL->numberOfRows())+", in mainDSQL: "+GString(m_pMainDSQL->numberOfRows()));
    //m_gstrSqlErr = _gstrSQLError;
    showData();
    deb(__FUNCTION__, "reload setting buttons...");
    setButtonState(true);
    deb(__FUNCTION__, "reload nearly done");
    if( !isOrderStmt(_gstrSQLCMD) ) refreshSort();
    deb(__FUNCTION__, "reload sorted done");
    QApplication::restoreOverrideCursor();
    m_tmrWaitTimer->stop();

    return;
}
void TabEdit::timerEvent(QTimerEvent*)
{
    /*
    if( m_ulTimerEvts % 2 ) cancelB->setText("Click...");
    else cancelB->setText("..to cancel");
    printf("timerEvt\n");
    m_ulTimerEvts++;
*/
}

void TabEdit::MyThread::run()
{
    myTabEdit->initDSQL();
}


/******************
 * used on windows:
 */
#ifdef MAKE_VC
static unsigned __stdcall threadedFn(void *p)
{
    TabEdit* pTE = static_cast<TabEdit* >(p);
    pTE->initDSQL();
    _threadRunning = 0;
    return 0;
}
#endif
/****************
 * used on linux
 */
void *TabEdit::threadedStuff(void * arg)
{
    TabEdit* pTE = static_cast<TabEdit* >(arg);
    pTE->initDSQL();
    _threadRunning = 0;
    return NULL;
}

void TabEdit::reload(GString cmd)
{
    deb(__FUNCTION__, "start");
    if( !cmd.length() && cmdLineLE->toPlainText().length() ) cmd = cmdLineLE->toPlainText();
    if( !cmd.length() ) return;

    addToStoreCB(cmd);
    m_gstrSQLCMD = cmd;
    deb(__FUNCTION__, "getting maxRows...");
    m_lMaxRows = maxRows();

    /*******************************
    * Start a timer to set text on cancelButton
    */
    setButtonState(false);
    m_ulTimerEvts = 0;

    deb(__FUNCTION__, "getting maxRows done");

    //QtConcurrent::run(mutexedStuff,this);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if( m_iUseThread )
    {
        deb(__FUNCTION__, "using thread...");
        if( _threadRunning ) return;
        _threadRunning = 1;
        _gstrSQLCMD = cmd;
        _lMaxRows = maxRows();
        m_iTimerDirections = 1;
        m_tmrWaitTimer->start( 1000 );
        //This calls tabEdit::MyThread::run(), i.e., the thread starts.
        deb(__FUNCTION__, "creating thread");

#ifdef MAKE_VC
        _beginthreadex(NULL,0,threadedFn,(void*)this,0,NULL);
#else
        pthread_t t;
        pthread_create(&t, NULL, &threadedStuff, this);
        //pthread_join(t, NULL); //Check: use this to catch thread termination
#endif
        /* Does not work. Yet.
        if( m_pThread ) delete m_pThread;
        m_pThread = new MyThread;
        deb(__FUNCTION__, "calling thread...");
        m_pThread->start();
        deb(__FUNCTION__, "Thread started....");
        */
    }
    else
    {
        deb(__FUNCTION__, "calling initDSQL...");
        try{
            if( GString(cmd).strip().upperCase().indexOf("DELETE") == 1 )
            {
                if( QMessageBox::question(this, "PMF", "Run 'Delete' command?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
                {
                    if( deleteViaFetchFromCmdLE(cmd) )
                    {
                        initDSQL();
                    }
                    else
                    {
                        cmdLineLE->setText("");
                        initDSQL();
                    }
                }
            }
            else initDSQL();
        } catch(...){msg("Reload/initDSQL: Got Exception");}
        deb(__FUNCTION__, "calling showData....");
        readColumnDescription(currentTable());
        showData();
        deb(__FUNCTION__, "setting buttons...");
        setButtonState(true);
        deb(__FUNCTION__, "nearly done");
        if( !isOrderStmt(cmd) ) refreshSort();
        deb(__FUNCTION__, "sorted done");
        deb(__FUNCTION__, "finally done");
    }
    QApplication::restoreOverrideCursor();
}

int TabEdit::isIdentityColumn(int colNr)
{
    for(int i = 1; i <= (int)m_colDescSeq.numberOfElements(); ++i)
    {
        if( i == colNr && m_colDescSeq.elementAtPosition(i)->Misc.occurrencesOf("IDENTITY") ) return 1;
    }
    return 0;
}

int TabEdit::isIdentityColumn(GString colName)
{
    for(int i = 1; i <= (int)m_colDescSeq.numberOfElements(); ++i)
    {
        if( m_colDescSeq.elementAtPosition(i)->ColName == colName && m_colDescSeq.elementAtPosition(i)->Misc.occurrencesOf("IDENTITY") ) return 1;
    }
    return 0;
}

void TabEdit::readColumnDescription(GString table)
{
    deb(__FUNCTION__, "start");
    DSQLPlugin * qDSQL = new DSQLPlugin(*m_pMainDSQL);
    m_colDescSeq.deleteAll();
    deb(__FUNCTION__, " for table "+table);
    qDSQL->getColSpecs(table, &m_colDescSeq);
    delete qDSQL;
    deb(__FUNCTION__, "readColumnDescription for table "+table+", done");
}
void TabEdit::initDSQL()
{
    deb(__FUNCTION__, "start");
    //////////////////////////////////
    ///  m_mainDSQL gets initialised.
    /////////////////////////////////
    deb(__FUNCTION__, "start, setting debug...");
    m_pMainDSQL->setGDebug(m_pGDeb);

    deb(__FUNCTION__, "start, set readUncommitted: "+GString(isChecked(_READUNCOMMITTED)));
    m_pMainDSQL->setReadUncommitted(isChecked(_READUNCOMMITTED));
    m_pMainDSQL->setCharForBit(m_pPMF->charForBit());
    //m_pMainDSQL->setCLOBReader(1);
    int useStaticData = 0;
    if( m_iUseThread && useStaticData)
    {
        //if m_iUseThread is set, this whole method is running in a thread.
        // DO NOT USE ANYTHING ELSE BUT STATIC VARIABLES HERE!
        deb(__FUNCTION__, "threaded init start");
        //_staticDSQL.setDebug(m_iDebug);
        //_gstrSQLError = _staticDSQL.initAll(_gstrSQLCMD, _lMaxRows);
        m_gstrSqlErr = m_pMainDSQL->initAll(m_gstrSQLCMD, m_lMaxRows);
        deb(__FUNCTION__, "threaded init done");
    }
    else
    {
        deb(__FUNCTION__, "initialising...");
        m_gstrSqlErr = m_pMainDSQL->initAll(m_gstrSQLCMD, m_lMaxRows);
        deb(__FUNCTION__, "initialised, err: "+m_gstrSqlErr);
    }

}

/*****************************************************
*
* mainWdgt gets filled with the table's contents.
* The displayed data matches the data in m_mainDSQL (a sequence of sequences),
* m_mainDSQL must not be changed or re-initialised as long as the corresponding data is displayed.
* In fact, ::reload(..) is the only method which should alter m_pMainDSQL->
*
* The first column contains a counter to map mainWdgt to m_pMainDSQL->
* This is necessary because sorting (via the horizontal header) changes the order.
* The second column stores a flag. Possible values are 'I' for INSERT, 'U' for UPDATE, default is 'x'.
* The first two columns are hidden, therefore visible columns start with "2", not "0".
*
* This layout is simple enough to not confuse me.
*****************************************************/
void TabEdit::showData()
{
    deb(__FUNCTION__, "start");
    int hint = 0;
    QApplication::restoreOverrideCursor();
    //Display errors, but try and fill tableWidget anyway. At least set columns.

    if( m_gstrSqlErr.strip().length() ) msg(m_gstrSqlErr);


    deb(__FUNCTION__, "pos1");
    mainWdgt->blockSignals( true );
    clearMainWdgt();
    mainWdgt->setSortingEnabled(false);
    mainWdgt->setUpdatesEnabled(false);
    mainWdgt->setRowCount(m_pMainDSQL->numberOfRows());
    mainWdgt->setWordWrap(false);


    //first two (additional) columns are needed for mapping and action
    mainWdgt->setColumnCount(m_pMainDSQL->numberOfColumns()+2);
    GString s;
    int rc;
    deb(__FUNCTION__, "pos2");
    QTableWidgetItem * pItem;
    //set column names
    for(unsigned int j = 0; j <= m_pMainDSQL->numberOfColumns()+1; ++j)
    {
        if( 0 == j )pItem = new QTableWidgetItem("MAP");
        else if( 1 == j )pItem = new QTableWidgetItem("ACT");
        else pItem = new QTableWidgetItem((char*)m_pMainDSQL->hostVariable(j-1));
        mainWdgt->setHorizontalHeaderItem(j, pItem);
    }
    deb(__FUNCTION__, "pos3");
    //set data
    QProgressDialog * apd = NULL;
    if( m_pMainDSQL->numberOfRows() )
    {
        apd = new QProgressDialog(GString("Retrieving"), "Cancel", 0, m_pMainDSQL->numberOfRows(), this);
        apd->setWindowModality(Qt::WindowModal);
        apd->setValue(1);
    }
    deb(__FUNCTION__, "pos4");
    //CRS GLineHdl *pLine = NULL;

    deb(__FUNCTION__, "pos4a, lines: "+GString(m_pMainDSQL->numberOfRows()) );

    // m_mainDSQL (a sequence of sequences) contains the table's data.
    // To read it, we set a cursor on the sequence.
    //CRS if( m_pMainDSQL->numberOfRows() > 0 ) pLine = m_pMainDSQL->allRowsSeq.initCrs();

    unsigned int i;
    QApplication::restoreOverrideCursor();

    int numberOfColumns = m_pMainDSQL->numberOfColumns()+1;
    unsigned long numberOfRows = m_pMainDSQL->numberOfRows();


    rc = m_pMainDSQL->initRowCrs();
    deb(__FUNCTION__, " rc: "+GString(rc));
    for(i = 1; i <= numberOfRows; ++i)
    {
        for(int j = 0; j <= (int) numberOfColumns; ++j)
        {
            //deb(__FUNCTION__, "col#"+GString(j));
            //j=0: row counter, hidden. Needed to map dsqlobj to mainWdgt
            //j=1: Flags for insert, update. Hidden.
            //j>1: The actual table data.

            if( 0 == j ) pItem = new QTableWidgetItem(tr("%1").arg(i));
            else if( 1 == j ) pItem = new QTableWidgetItem("x");
            else
            {
                //s = pLine->rowElement(j-1);
                s = m_pMainDSQL->dataAtCrs(j-1);
                //deb(__FUNCTION__, "row "+GString(i)+", col "+GString(j)+", data: "+s);
                pItem = new QTableWidgetItem();

                ///Alphabetical sorting of numbers kind of irritates
                //if( s.isDigits()  ) pItem->setData(Qt::DisplayRole, qlonglong(s.asLong()));
                //if( displayAsNumber(j-1) && s.isDigits() ) pItem->setData(Qt::DisplayRole, qlonglong(s.asLong()));

                if( displayAsNumber(j-1) && s.isDigits() )
                {
                    pItem->setData(Qt::DisplayRole, qlonglong(s.asLongLong()));
                    pItem->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );
                }
                else if( displayAsNumber(j-1) && GString(s).removeAll('.').removeAll('E').removeAll('+').isDigits() )
                {
                    pItem->setData(Qt::EditRole, QVariant((char*)s));
                    pItem->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter);
                }
                else
                {
                    pItem->setData(Qt::DisplayRole, QString::fromLocal8Bit(s));
                    //pItem->setData(Qt::DisplayRole, QString::fromUtf8(s));
                    //pItem->setData(Qt::DisplayRole, QString::fromLatin1(s));
                    if( addColorizedHints(j-1, pItem, s) ) hint = 2;
                }
                //deb(__FUNCTION__, "From cell: "+pItem->text());
                //Disable editing: Or rather not: Must be able to set NULL in XML column
                //if( m_pMainDSQL->isXMLCol(j-1) ) pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);

                if( isIdentityColumn(m_pMainDSQL->hostVariable(j-1)) ) pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
            }
            mainWdgt->setItem(i-1, j, pItem);
        }


        //CRS pLine = m_pMainDSQL->allRowsSeq.setCrsToNext();
        rc = m_pMainDSQL->nextRowCrs();
        //deb(__FUNCTION__, "pos5, set NextCRS, rc: "+GString(rc));

        if( apd )
        {
            apd->setValue(i);
            if( apd->wasCanceled() ) break;
        }

        if( rc || i >= numberOfRows ) break;
    }
    //This is getting a bit fugly. dsqlobj and ODBC-based result sets behave differently on returning errCode for ->nextRowCrs()
    //so I changed a few bits here and there. Most notably "mainWdgt->setRowCount(i-1)" was changed to "mainWdgt->setRowCount(i)"
    //However, if the resultset is empty, i is intitalized anyway to "1", which then crashes in setVHeader (trying to read 1 row where no row exists).
    //Maybe I'll rework this some day (i.e. never)
    if( i > numberOfRows ) i = numberOfRows;

    deb(__FUNCTION__, "rowCount: "+GString(i));
    if( apd )
    {
        apd->setValue(i);
        delete apd;
    }
    mainWdgt->setRowCount(i);
    createEmptyRow(); //This will increment rowCount

    //Hide first two columns
    mainWdgt->setColumnWidth(0, 1);
    #if QT_VERSION >= 0x050000
    mainWdgt->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    #else
    mainWdgt->setColumnHidden(0, true);
    #endif
    //mainWdgt->setColumnHidden(0, true);
    mainWdgt->setColumnHidden(1, true);

    setVHeader();

    mainWdgt->setUpdatesEnabled(true);

    deb(__FUNCTION__, "1");
    mainWdgt->blockSignals( false );
    //while editing sorting gets disabled (see ::itemChg).
    ////connect((QWidget*)mainWdgt->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
    mainWdgt->setSortingEnabled(false);


    setPendingMsg(1);
    deb(__FUNCTION__, "2");

    //resize columns, visible columns start at 2.
    for(int j = 2; j < mainWdgt->columnCount(); ++j)
    {
        mainWdgt->resizeColumnToContents ( j );
    }
    for(int j = 2; j < mainWdgt->columnCount(); ++j)
    {
        if( m_pMainDSQL->isXMLCol(j-1)) mainWdgt->setColumnWidth(j, 200);
        else mainWdgt->setColumnWidth(j, mainWdgt->columnWidth(j) > 400 ? 400 : mainWdgt->columnWidth(j)+20);
    }
    deb(__FUNCTION__, "3");
    m_qaShowXML->setEnabled(false);
    m_qaSaveBlob->setEnabled(false);
    m_qaOpenBlob->setEnabled(false);
    m_qaOpenBlobAs->setEnabled(false);

    deb(__FUNCTION__, "4");
    enableActions(hint);


    //This is technically better, but slower:
    //for( int i = 0; i < mainWdgt->rowCount(); ++i ) mainWdgt->resizeRowToContents ( i ) ;
    deb(__FUNCTION__, "5");

    if( i >= maxRows() ) setInfo("Result was cut at "+GString(i)+" rows, Cost: "+GString(m_pMainDSQL->getCost())+" [Timerons]", 1);
    else setInfo("Displayed rows: "+GString(i)+", Cost: "+GString(m_pMainDSQL->getCost())+" [Timerons]");
    deb(__FUNCTION__, "done");

}

void TabEdit::enableActions(int hint)
{
    for(int j = 1; j <= (int) m_pMainDSQL->numberOfColumns(); ++j)
    {
        if( colType(j) == 2 )
        {
            m_qaShowXML->setEnabled(true);
            hint = 1;
        }
        else if( colType(j) == 1 || colType(j) == 5)
        {
            m_qaSaveBlob->setEnabled(true);
            m_qaOpenBlob->setEnabled(true);
            m_qaOpenBlobAs->setEnabled(true);
            m_qaLoadBlob->setEnabled(true);
            hint = 1;
        }
    }
    if( hint == 1 )showHint(hintXML);
    else if( hint == 2 )showHint(hintColorHint);
    else showHint(hintMultiCellEdit);
}

void TabEdit::colorGradeWidget()
{


    QPalette* palette = new QPalette();
    QLinearGradient linearGradient(QPointF(50, 50), QPointF(mainWdgt->size().width(), mainWdgt->size().height()));
    linearGradient.setColorAt(0, Qt::white);
    //QColor(i, j, k)
    //linearGradient.setColorAt(1, Qt::green);
    int r = m_ulTimerEvts % 256;
    if( (m_ulTimerEvts / 256) % 2 ) r = 256 - (m_ulTimerEvts % 256);
    if( r < 1 ) r =1;
    if( r > 255 ) r = 255;
    //printf("evtCOunt: %i, mod: %i, r: %i\n", m_ulTimerEvts, m_ulTimerEvts % 256, r);
    linearGradient.setColorAt(1, QColor(m_ulTimerEvts, 10, 10));
    palette->setBrush(QPalette::Base,*(new QBrush(linearGradient)));

    mainWdgt->setPalette(*palette);
    /*


    QPalette* palette = new QPalette();
    QLinearGradient linearGradient(QPointF(50, 50), QPointF(mainWdgt->size().width(), mainWdgt->size().height()));



    palette->setBrush(QPalette::Base,*(new QBrush(linearGradient)));


    linearGradient.setColorAt(0, Qt::white);
    linearGradient.setColorAt(1, QColor(255, 204, 157));
    QBrush brush(linearGradient);
    palette->setBrush(QPalette::Window, brush);
    mainWdgt->setPalette(*palette);
    return;
//m_ulTimerEvts
    for( int i = 1; i < 25; ++i)
    {
        for( int k = 1; k < 25; ++k)
        {
            for( int j = 1; j < 2; ++j)
            {
                linearGradient.setColorAt(0, Qt::white);
                linearGradient.setColorAt(1, QColor(i, j, k));
                QBrush brush(linearGradient);
                palette->setBrush(QPalette::Window, brush);
                mainWdgt->setPalette(*palette);


                GStuff::dormez(3);
            }

        }
    }
    */

}

int TabEdit::displayAsNumber(int col)
{

    if( m_pMainDSQL->simpleColType(col) == CT_INTEGER ||
            m_pMainDSQL->simpleColType(col) == CT_LONG ||
            m_pMainDSQL->simpleColType(col) == CT_DECIMAL ||
            m_pMainDSQL->simpleColType(col) == CT_FLOAT ) return 1;
    return 0;
}

int TabEdit::addColorizedHints(int col, QTableWidgetItem *pItem, GString data)
{
    int ret = 0;
    //deb( __FUNCTION__, "col: "+GString(col)+", from sqlType: "+GString(m_pMainDSQL->sqlType(col))+", isFixed: "+GString(m_pMainDSQL->isFixedChar(col)));
    //if( !m_iUseColorScheme ) return;
    //    GString type = m_colDescSeq.elementAtPosition(col)->ColType.upperCase();
    //    if( type == "CHAR" || type == "NCHAR" ) return;

    if( data.occurrencesOf("'") < 2 ) return 0;
    if(data[1] == '\'' && data[data.length()] =='\'' ) data = data.subString(2, data.length()-2);
    if( !data.length() ) return 0;

    if( data.occurrencesOf("\n") && !m_pMainDSQL->isLongTypeCol(col))
    {
        pItem->setBackgroundColor(_ColorCRLF);
        pItem->setToolTip("CR/LF in data. Resize height, use right-click->'Edit cell data' to edit");
    }
    else if(data[1] == '\'' || data[data.length()] == '\'')
    {
        if( m_iUseColorScheme )pItem->setBackgroundColor(_ColorApos);
        else pItem->setBackgroundColor(_ColorAposDark);
        pItem->setToolTip("Apostrophe at start/end of data. Intentional?");
        ret = 1;
    }
    else if( (data[1] == ' ' || data[data.length()] == ' ') && !m_pMainDSQL->isFixedChar(col) )
    {
        if( m_iUseColorScheme )pItem->setBackgroundColor(_ColorBlanks);
        else pItem->setBackgroundColor(_ColorBlanksDark);
        pItem->setToolTip("Blank(s) at start/end of data. Intentional?");
        ret = 1;
    }
    else if( data.occurrencesOf(' ') && !m_pMainDSQL->isXMLCol(col) && !m_pMainDSQL->isFixedChar(col) && !m_pMainDSQL->isDateTime(col) )
    {
        if( m_iUseColorScheme )pItem->setBackgroundColor(_ColorBlanksInside);
        else pItem->setBackgroundColor(_ColorBlanksDark);
        pItem->setToolTip("Blank(s) inside data. Intentional?");
        ret = 1;
    }

    if(data.occurrencesOf("\n"))
    {
        int height = QFontMetrics( mainWdgt->font()).height()+5;
        int count = min(data.occurrencesOf("\n")+1, 3);
        mainWdgt->setRowHeight(pItem->row(), count*height);
    }    
    return ret;
}

/*****************************************************
*
*****************************************************/
void TabEdit::addToStoreCB(QString s, int writeToFile)
{
    //writeToFile: 0 when file 'sqlHist' is read.
    if( !writeToFile )
    {
        for( int i = 0; i < filterCB->count() && i < 10; ++i )
        {
            if( s == filterCB->itemText(i) ) return;
        }
        filterCB->insertItem(0, s);
        return;
    }

    GString data = s;
    if( data.occurrencesOf(_selStringCB) || data == _newTabString) return;
    data += _LAST_SQL_TABLE+currentTable()+"]";

    //Only check the first 10 entries in filterCB. If found, do not add 'data' fo filterCB
    for( int i = 0; i < filterCB->count() && i < 10; ++i )
    {
        if( data == GString(filterCB->itemText(i)) ) return;
    }

    filterCB->insertItem(0, data);
    if( !writeToFile ) return;

    QString home = QDir::homePath ();
    if( !home.length() ) return;
    GFile f(histFileName(), GF_APPENDCREATE);
    f.addLine(data);
}

/************************************************
*
* If a file has been dragged in an QTableWidgetItem, we replace the file's name with "?" and add the fileName to the fileSeq.
* If the LOB-column(s) contain "?" (nothing was dragged), we ask for the fileName(s)
*
************************************************/
int TabEdit::setLOBMarkers(QTableWidgetItem* pItem, GSeq <GString> *fileNameSeq, int setQuestionMark)
{
    GString val;
    deb(__FUNCTION__, "Start");
    int row = pItem->row();
    deb(__FUNCTION__, "row: "+GString(row));
    deb(__FUNCTION__, "cols: "+GString(m_pMainDSQL->numberOfColumns()));


    for(unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        pItem = mainWdgt->item(row, i+1);
        val = setLOBMarker(pItem, setQuestionMark);
        if( val.length() ) fileNameSeq->add(val);
    }
    return 0;
    ///////////////////////
    for(unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        pItem = mainWdgt->item(row, i+1);
        val = pItem->text();
        if( !m_pMainDSQL->isLOBCol(i) && !m_pMainDSQL->isXMLCol(i) ) continue;

        if( val.strip().upperCase() == "NULL") continue;
        else if(val.subString(1, 6) == "@DSQL@")
        {
            if( setQuestionMark &&  val.subString(1, 6) == "@DSQL@")
            {
                pItem->setText("?");
                val = pItem->text();
            }
            else continue;
        }

        //check if cell contents have changed, for example by dragging and dropping a file into the cell:
        if( !cellDataChanged(pItem) ) continue;

        //if this is a simple string, we won't use it for an update:
        //(SqlSrv on Linux: With TDS 7.2, XMLs and long varchar are "-10", so we don't know which is which)
        if( !val.length() ) continue;
        //if( GString(val[1]) == GString("'") && GString(val[val.length()]) == GString("'") ) continue;
        //if( val[(signed long)1] == '\''  && (char)val[val.length()] == '\'' ) continue;
        if( val[1UL] == '\''  && (char)val[val.length()] == '\'' ) continue;


        if( val.strip().strip("'").strip() == "?" )
        {
            GString fileName = QFileDialog::getOpenFileName(this, "Table has LOB-columns, select a LOB (*.*)", "");
#ifdef MAKE_VC
            fileName = GString(fileName).translate('/', '\\');
#endif
            if( !fileName.length() ) return 1;
            fileNameSeq->add(fileName);
        }
        else if(val.subString(1, _PMF_IS_LOB.length()) == _PMF_IS_LOB)
        {
            fileNameSeq->add(val.subString(_PMF_IS_LOB.length()+1, val.length()).strip());
            pItem->setText(val);

        }
        else
        {
            pItem->setText("?");
            fileNameSeq->add(val);
        }
    }
    return 0;
}


GString TabEdit::setLOBMarker(QTableWidgetItem* pItem, int setQuestionMark)
{
    GString val;
    deb(__FUNCTION__, "Start");
    int row = pItem->row();
    int col = pItem->column();
    deb(__FUNCTION__, "row: "+GString(row));
    val = pItem->text();
    if( !m_pMainDSQL->isLOBCol(col-1) && !m_pMainDSQL->isXMLCol(col-1) ) return "";
    if( val.strip().upperCase() == "NULL") return "";
    else if(val.subString(1, 6) == "@DSQL@")
    {
        if( setQuestionMark &&  val.subString(1, 6) == "@DSQL@")
        {
            pItem->setText("?");
            val = pItem->text();
        }
        else return "";
    }
    if( !cellDataChanged(pItem) ) return "";
    if( !val.length() ) return "";
    if( val[1UL] == '\''  && (char)val[val.length()] == '\'' ) return "";

    if( val.strip().strip("'").strip() == "?" )
    {
        GString fileName = QFileDialog::getOpenFileName(this, "Table has LOB-columns, select a LOB (*.*)", "");
#ifdef MAKE_VC
        fileName = GString(fileName).translate('/', '\\');
#endif
        if( !fileName.length() ) return "";
        pItem->setText(val);
        return fileName;
    }
    else if(val.subString(1, _PMF_IS_LOB.length()) == _PMF_IS_LOB)
    {        
        pItem->setText(val);
        return val.subString(_PMF_IS_LOB.length()+1, val.length()).strip();
    }
    else
    {
        pItem->setText("?");
        return val;
    }
    return "";
}

GString TabEdit::updateRowViaUniqueCols(QTableWidgetItem* pItem, DSQLPlugin * pDSQL, GSeq<GString> *unqCols, int lobCol, GString lobTmpFile )
{
    //msg(createUniqueColConstraint(pItem));
    deb(__FUNCTION__, "start");
    GSeq <GString> lobFileSeq;
    GSeq <long> lobTypeSeq;
    GString col, cmd, oldVal, newVal, whr, someLobFile;
    int row, itemCol;
    int params = 0, changedCols = 0;
    int tableHasUniqueKey = unqCols->numberOfElements();

    GString errMsg = "Invalid cell item encountered.\nYou are probably working on a sub-select:\nUpdates will (usually) not work here ";
    if( !pItem ) return errMsg;
    row = pItem->row();
    itemCol = pItem->column();
    if( pItem->row() < 0 ) return errMsg;
    if( !mainWdgt->item(pItem->row(), 0 ) ) return errMsg;


    //If a file has been dragged into a QTableWidgetItem, we replace the file's name with "?" and add the fileName to the fileSeq.
    //If the LOB-column(s) contain "?" (nothing was dragged), we ask for the fileName(s).
    //
    //If an XML was updated in ::showXML, we get it as string in xmlAsString
    //and copy the string directly into the UPDATE. This is a bit ugly, but works in non-UTF8 environs.

    deb(__FUNCTION__, "setting LOB Markers");
//    if( !xmlTmpFile.length() )
//    {
//        if( setLOBMarkers(pItem, &fileSeq) ) return "";
//    }

    int pos = GString( mainWdgt->item(pItem->row(), 0 )->text()).asInt();
    deb(__FUNCTION__, "itemPos: "+GString(pos));

    if( currentTable().occurrencesOf(_selStringCB) || currentTable().occurrencesOf(_newTabString) )
    {
        return "Please open a table first (and repeat the update)";
    }

    cmd = "UPDATE "+currentTable()+" SET ";
    whr = " WHERE ";

    for( int i = 1; i <= (int) m_pMainDSQL->numberOfColumns(); ++ i )
    {
        pItem = mainWdgt->item(row, i+1);
        col = wrap(m_pMainDSQL->hostVariable(i)); //HostVariable, i.e. columnName
        oldVal = m_pMainDSQL->rowElement(pos, i);        
        if( !m_pMainDSQL->isXMLCol(i) ) newVal = Helper::formatItemText(m_pMainDSQL, mainWdgt->item(pItem->row(), i+1));
        else newVal = mainWdgt->item(pItem->row(), i+1)->text();

        pDSQL->convToSQL(newVal);
        pDSQL->convToSQL(oldVal);

        if( !newVal.strip().length() ) newVal = "NULL";
        if( newVal.subString(1, 6) == "@DSQL@" ) continue;
        //if( m_pMainDSQL->isXMLCol(i) && !cellDataChanged(pItem) )  continue;
        if( m_pMainDSQL->isTruncated(row, i)) continue;
        //Assumption: LONG data types cannot be part of UNQ/PRIM keys
        if( m_pMainDSQL->isLongTypeCol(i) && !cellDataChanged(pItem) && tableHasUniqueKey ) continue;
        if( m_pMainDSQL->simpleColType(i) == CT_GRAPHIC && !cellDataChanged(pItem) && tableHasUniqueKey ) continue;


        if( lobCol == i && lobTmpFile.length() ) someLobFile =lobTmpFile;
        else someLobFile = setLOBMarker(pItem);

        if( someLobFile.length() )
        {
            params++;
            newVal = "?";
            lobFileSeq.add(someLobFile);
            lobTypeSeq.add(m_pMainDSQL->sqlType(i));
            deb(__FUNCTION__, "adding file to lobFileSeq: "+someLobFile);
        }
        if( newVal != oldVal )
        {
            if( m_pMainDSQL->isForBitCol(i) >= 3 && GString(newVal).upperCase() != "NULL") newVal = Helper::formatForHex(m_pMainDSQL, newVal);
            cmd += col + "=" + newVal +",";
            changedCols++;
        }
        //WHERE CLAUSE
        deb(__FUNCTION__, "col "+GString(i)+": oldVal: "+oldVal+", isXml: "+GString(m_pMainDSQL->isXMLCol(i))+", isLob: "+GString(m_pMainDSQL->isLOBCol(i)+", isTrunc: "+GString(m_pMainDSQL->isTruncated(pos, i))+", isLong: "+GString(m_pMainDSQL->isLongTypeCol(i))+", isGraph: "+GString(m_pMainDSQL->simpleColType(i) == CT_GRAPHIC)));
        if( oldVal.subString(1, 6) == "@DSQL@" || m_pMainDSQL->isXMLCol(i) || m_pMainDSQL->isLOBCol(i)) continue;

        if( GString(oldVal).upperCase() == "NULL" ) oldVal = " IS NULL ";
        else if( m_pMainDSQL->isForBitCol(i) >= 3 && GString(oldVal).upperCase() != "NULL" ) oldVal = "="+Helper::formatForHex(m_pMainDSQL, oldVal);
        else if( m_pMainDSQL->simpleColType(i) == CT_GRAPHIC && GString(oldVal).upperCase() != "NULL" ) oldVal = "=VARGRAPHIC("+oldVal+") ";
        else oldVal = " = " +oldVal;
        //Note to self: No "else" here (stumbled over this twice)
        whr += col + oldVal + " AND ";
    }
    deb(__FUNCTION__, "changedCols: "+GString(changedCols)+", whr: "+whr);
    if( !changedCols ) return "";

    whr = whr.stripTrailing(" AND ");

    cmd = cmd.stripTrailing(",");

    GString unq_whr = createUniqueColConstraintForItem(pItem, unqCols);

    deb(__FUNCTION__, "::updateRow unq_whr: "+unq_whr);
    if( unq_whr.length() ) whr = unq_whr;

    if( singleRowCHK->isChecked() )
    {
        return changeRowByCursor(pDSQL, cmd, whr, &lobFileSeq, &lobTypeSeq);
    }
    //msg("::updateRow cmd: "+cmd+whr);

    deb(__FUNCTION__, " (nearly) done, cmd: "+cmd+whr);
    if( 0 == params )
    {
        GString err = pDSQL->initAll(cmd + whr);
        return err;
    }
    else
    {
        int erc = pDSQL->uploadBlob(cmd + whr, &lobFileSeq, &lobTypeSeq);
        if( erc ) return pDSQL->sqlError();
    }

    return "";
}


/*****************************************************
*
*****************************************************/
GString TabEdit::updateRow(QTableWidgetItem* pItem, DSQLPlugin * pDSQL, GSeq<GString> *unqCols, int lobCol, GString lobTmpFile )
{

    return updateRowViaUniqueCols(pItem, pDSQL, unqCols, lobCol, lobTmpFile);

//    //msg(createUniqueColConstraint(pItem));
//    deb(__FUNCTION__, "start");
//    GSeq <GString> fileSeq;
//    GSeq <long> lobTypeSeq;
//    GString col, cmd, oldVal, newVal, whr;
//    int params = 0, changedCols = 0;
//    int tableHasUniqueKey = tableHasUniqueCols(currentTable(0));


//    if( !pItem ){msg("FATAL (my fault): in ::updateRow, pItem is NULL"); return "FATAL";}
//    if( !mainWdgt->item(pItem->row(), 0 ) ){msg("FATAL (my fault): in ::updateRow, pItem at "+GString(pItem->row())+" is NULL"); return "FATAL";}


//    //If a file has been dragged in an QTableWidgetItem, we replace the file's name with "?" and add the fileName to the fileSeq.
//    //If the LOB-column(s) contain "?" (nothing was dragged), we ask for the fileName(s).
//    //
//    //If an XML was updated in ::showXML, we get it as string in xmlAsString
//    //and copy the string directly into the UPDATE. This is a bit ugly, but works in non-UTF8 environs.

//    if( !xmlAsString.length() && !xmlTmpFile.length() )
//    {
//        if( setLOBMarkers(pItem, &fileSeq) ) return "";
//    }
//    if( xmlTmpFile.length() ) fileSeq.add(xmlTmpFile);


//    int pos = GString( mainWdgt->item(pItem->row(), 0 )->text()).asInt();

//    if( currentTable().occurrencesOf(_selStringCB) || currentTable().occurrencesOf(_newTabString) )
//    {
//        return "Please open a table first (and repeat the update)";
//    }
//    int row = pItem->row();
//    cmd = "UPDATE "+currentTable()+" SET ";
//    whr = " WHERE ";

//    for( int i = 1; i <= (int) m_pMainDSQL->numberOfColumns(); ++ i )
//    {
//        pItem = mainWdgt->item(row, i+1);
//        col = "\""+m_pMainDSQL->hostVariable(i)+"\""; //HostVariable, i.e. columnName
//        oldVal = m_pMainDSQL->rowElement(pos, i);
//        newVal = formatText(mainWdgt->item(pItem->row(), i+1));
//        pDSQL->convToSQL(newVal);
//        pDSQL->convToSQL(oldVal);
//        //if( !newVal.strip().length() || newVal.subString(1, 6) == "@DSQL@" || (m_pMainDSQL->isXMLCol(i) && !cellDataChanged(pItem) && !xmlAsString.length()) ) continue;
//        if( !newVal.strip().length() ) newVal = "NULL";
//        if( newVal.subString(1, 6) == "@DSQL@" ||
//                (m_pMainDSQL->isXMLCol(i) && !cellDataChanged(pItem) && !xmlAsString.length() && !xmlTmpFile.length()) )
//        {
//            continue;
//        }
//        if( m_pMainDSQL->isTruncated(row, i)) continue;
//        //Assumption: LONG data types cannot be part of UNQ/PRIM keys
//        if( m_pMainDSQL->isLongTypeCol(i) && !cellDataChanged(pItem) && tableHasUniqueKey ) continue;
//        if( m_pMainDSQL->simpleColType(i) == CT_GRAPHIC && !cellDataChanged(pItem) && tableHasUniqueKey ) continue;
//        //if( m_pMainDSQL->isLongTypeCol(i) && !cellDataChanged(pItem) && singleRowCHK->isChecked() ) continue;

//        //SET CLAUSE
//        if(m_pMainDSQL->isXMLCol(i) && i == xmlCol  )
//        {
//            if( xmlAsString.length() )
//            {
//                m_pMainDSQL->createXMLCastString(xmlAsString);
//                newVal = xmlAsString;
//            }
//            if( xmlTmpFile.length( ))
//            {
//                params++;
//                newVal = "?";
//            }

//        }
//        if( newVal.strip().strip("'").strip() == "?")
//        {
//            lobTypeSeq.add(m_pMainDSQL->sqlType(i));
//            params++;
//        }

//        if( newVal != "?" && newVal != oldVal )
//        {
//            if( m_pMainDSQL->isForBitCol(i) >= 3 && GString(newVal).upperCase() != "NULL") newVal = formatForHex(m_pMainDSQL, newVal);
//            else if(m_pMainDSQL->simpleColType(i) == CT_GRAPHIC ) newVal = " VARGRAPHIC("+newVal+") ";
//            cmd += col + "=" + newVal +",";
//            changedCols++;
//        }
//        //WHERE CLAUSE
//        //if( oldVal.subString(1, 6) == "@DSQL@" || oldVal == "NULL"  || m_pMainDSQL->isXMLCol(i) || m_pMainDSQL->isLOBCol(i) ) continue;
//        deb(__FUNCTION__, "col "+GString(i)+": oldVal: "+oldVal+", isXml: "+GString(m_pMainDSQL->isXMLCol(i))+", isLob: "+GString(m_pMainDSQL->isLOBCol(i)+", isTrunc: "+GString(m_pMainDSQL->isTruncated(pos, i))+", isLong: "+GString(m_pMainDSQL->isLongTypeCol(i))+", isGraph: "+GString(m_pMainDSQL->simpleColType(i) == CT_GRAPHIC)));
//        if( oldVal.subString(1, 6) == "@DSQL@" || m_pMainDSQL->isXMLCol(i) || m_pMainDSQL->isLOBCol(i)) continue;

//        if( GString(oldVal).upperCase() == "NULL" ) oldVal = " IS NULL ";
//        else if( m_pMainDSQL->isForBitCol(i) >= 3 && GString(oldVal).upperCase() != "NULL" ) oldVal = "="+formatForHex(m_pMainDSQL, oldVal);
//        else if( m_pMainDSQL->simpleColType(i) == CT_GRAPHIC && GString(oldVal).upperCase() != "NULL" ) oldVal = "=VARGRAPHIC("+oldVal+") ";
//        else oldVal = " = " +oldVal;
//        //Note to self: No "else" here (stumbled over this twice)
//        whr += col + oldVal + " AND ";
//    }
//    /* This is now done by ::setLOBMarkers
//    for( int i = 1;  i<= params; ++i ) //Get filenames, i.e. sources for Blobs
//    {
//        GString myFileName = GString(QFileDialog::getOpenFileName(this, "Table has LOB-columns, select a LOB (*.*)", ""));
//                 #ifdef MAKE_VC
//                   myFileName = GString(myFileName).translate('/', '\\');
//                 #endif
//        fileSeq.add(myFileName);
//    }
//    */
//    //Same values were entered, nothing changed:
//    if( !changedCols ) return "";

//    cmd = cmd.stripTrailing(",");
//    whr = whr.stripTrailing(" AND ");

//    msg("::updateRow cmd: "+cmd+whr);

//    if( singleRowCHK->isChecked() )
//    {
//        return changeRowByCursor(pDSQL, cmd, whr, &fileSeq, &lobTypeSeq);
//    }
//    deb(__FUNCTION__, " (nearly) done, cmd: "+cmd+whr);
//    if( 0 == params )
//    {
//        GString err = pDSQL->initAll(cmd + whr);
//        return err;
//    }
//    else
//    {
//        int erc = pDSQL->uploadBlob(cmd + whr, &fileSeq, &lobTypeSeq);
//        if( erc ) return pDSQL->sqlError();
//    }

//    return "";
}


/*
GUID tabEdit::StringToGUID( LPOLESTR szBuf)
{
    GUID *g = (GUID * ) malloc( sizeof(GUID));
    HRESULT h2 = CLSIDFromString(szBuf, g);

    return *g;
}
unsigned char const* tabEdit::GuidToByteArray(GUID const& g)
{
    return reinterpret_cast<unsigned char const*>(&g);

}
*/
GString TabEdit::readXMLFromTMP(GString xmlSrcFile)
{
    QFile xmlFile(xmlSrcFile);
    xmlFile.open(QFile::ReadOnly | QFile::Text);
    QTextStream ReadFile(&xmlFile);
    GString newVal = " XMLPARSE (DOCUMENT('"+ReadFile.readAll()+"')) ";
    xmlFile.close();
    return newVal;

}
/*****************************************************
*
*****************************************************/
GString TabEdit::changeRowByCursor(DSQLPlugin * pDSQL, GString cmd, GString whr, GSeq <GString> *fileList, GSeq <long> *lobType)
{
    GString filter, error;
    filter = cmdLineLE->toPlainText();


    filter = "SELECT * FROM "+currentTable()+" "+whr;
    pDSQL->setGDebug(m_pGDeb);

    if( !fileList->numberOfElements()) error = pDSQL->currentCursor( filter, cmd, 1, 1, NULL, NULL);
    else error = pDSQL->currentCursor( filter, cmd, 1, 1, fileList, lobType);
    return error;

    //DEAD CODE:
    /*************
    //the first (hidden) column (i.e. col #0 ) of the selected item contains the mapping.
    //So even if a colHeader is clicked (i.e. data is sorted), we can still map the selected row to the result set.
    if( filter.length() == 0 ) filter = "SELECT * FROM "+currentTable();  /// <- The WHERE constraint is missing here. Just saying.
    if( fileList->numberOfElements() ) error = pDSQL->currentCursor( filter, cmd, mainWdgt->item(pItem->row(), 0)->text().toLong(), 1, fileList, lobType);
    else error = pDSQL->currentCursorOld(filter, cmd, mainWdgt->item(pItem->row(), 0)->text().toLong(), 1);
    msg("Done.");
    **************/

    return error;
}
/*****************************************************
*
*****************************************************/
GString TabEdit::insertRow(QTableWidgetItem* pItem, DSQLPlugin * pDSQL)
{
    GSeq <GString> fileSeq;
    GSeq <long> lobTypeSeq;
    int params = 0;

    deb(__FUNCTION__, "start");
    //The first (hidden) column contains the index for mapping pItem to m_mainDSQL
    GString cmd, newVal, error, dummy;
    //msg("::tabEdit new DSQL 2");
    //DSQLPlugin * qDSQL = new DSQLPlugin(*m_pMainDSQL);


    //If a file has been dragged in an QTableWidgetItem, we replace the file's name with "?" and add the fileName to the fileSeq.
    //If the LOB-column(s) contain "?" (nothing was dragged), we ask for the fileName(s)
    if( setLOBMarkers(pItem, &fileSeq, 1) )
    {
        //File selection has been canceled
        return "No file selected.\nHint: You can also drag&drop files into LOB cells.";
    }

    cmd = "INSERT INTO "+currentTable()+" (";
    for( unsigned i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        newVal = Helper::formatItemText(m_pMainDSQL, mainWdgt->item(pItem->row(), i+1));
        if( !newVal.length() || newVal == "NULL" ) continue;
        if (newVal == _IDENTITY || isIdentityColumn(m_pMainDSQL->hostVariable(i)) ) newVal = "";
        if( newVal.strip().length()) cmd += wrap(m_pMainDSQL->hostVariable(i)) +",";
    }
    cmd = cmd.stripTrailing(",") + ") VALUES (";
    for( unsigned i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        deb(__FUNCTION__, "Col #"+GString(i)+", SqlType: "+GString(m_pMainDSQL->sqlType(i)));
        newVal = Helper::formatItemText(m_pMainDSQL, mainWdgt->item(pItem->row(), i+1)).strip();
        if( !newVal.length() || newVal == "NULL" ) continue;
        pDSQL->convToSQL(newVal); //Replace "'" with "''"


        if( newVal.strip().strip("'").strip().occurrencesOf("@DSQL@CHAR_AS_BIT") ) newVal = "''";
        else if( newVal.strip().strip("'").strip().occurrencesOf("@DSQL@") ) newVal = "?";
        else if( m_pMainDSQL->isForBitCol(i) >= 3 && GString(newVal).upperCase() != "NULL" && newVal.strip().length() )newVal = Helper::formatForHex(pDSQL, mainWdgt->item(pItem->row(), i+1));

        if( newVal.occurrencesOf(_PMF_IS_LOB) ) newVal="?";
        else if (newVal == _IDENTITY || isIdentityColumn(m_pMainDSQL->hostVariable(i)) ) newVal = "";
        if( newVal.length() ) cmd += newVal +",";
        if( newVal.strip().strip("'").strip() == "?" )
        {
            params++;
            lobTypeSeq.add(m_pMainDSQL->sqlType(i));
            deb(__FUNCTION__, "Type: "+GString(lobTypeSeq.elementAtPosition(1))+", sqlType: "+GString(m_pMainDSQL->sqlType(i)));
        }
    }
    cmd = cmd.stripTrailing(",") + ")";
    deb(__FUNCTION__, "cmd: "+cmd);
    /* now done via setLobMarkers
    for( i = 1;  i<= params; ++ i ) //Get filenames, i.e. sources for Blobs
    {
        myFileName = GString(QFileDialog::getOpenFileName(this, "BlobSrc (*.*)", ""));
        fileSeq.add(myFileName);
    }
    */
    if( params == 0 )
    {
        dummy = pDSQL->initAll(cmd);
        if(pDSQL->sqlCode() && dummy.length())
        {
            if( pDSQL->sqlCode() == -798 ) msg("Error: "+pDSQL->sqlError() +"\nSolution: Simply leave the offending field empty!");
            else if( pDSQL->sqlCode() == -206 ) msg(dummy+"\n\nHint: PMF's current table is "+currentTable()+". You appear to be working on a different table.\nOpen the table you want to operate on.");
            ////else msg("INSError: "+pDSQL->sqlError());
        }
        if( !pDSQL->sqlCode() ) error = "";
        else error = dummy;
    }
    else  //Insert blobs specified in paramList
    {
        pDSQL->setGDebug(m_pGDeb);
        int erc = pDSQL->uploadBlob(cmd, &fileSeq, &lobTypeSeq);
        //sqlError (in SQLCA) does not get reset when SQLCODE==0. So return sqlError only if SQLCODE != 0.
        if( erc ) error = pDSQL->sqlError();
    }

    deb(__FUNCTION__, "(nearly) done");
    ///delete qDSQL;
    return error;
}
/*****************************************************
*
*****************************************************/
void TabEdit::extSQLClick()
{
    this->loadSqlEditor();
}



void TabEdit::runGetClp()
{
    if( m_pGetCLP ) return;
    m_pGetCLP = new Getclp(m_pMainDSQL, this, currentTable());

    m_pGetCLP->installEventFilter(this);
    m_pGetCLP->show();
}

bool TabEdit::eventFilter(QObject* object, QEvent* event)
{
    if( event->type() == QEvent::WindowActivate )
    {
        if( m_pExtSQL && object == m_pExtSQL) m_pExtSQL->setWindowOpacity ( 1.0 );
        if( m_pGetCLP && object == m_pGetCLP ) m_pGetCLP->setWindowOpacity ( 1.0 );
        if( m_pShowXML && object == m_pShowXML) m_pShowXML->setWindowOpacity ( 1.0 );
    }

    if( event->type() == QEvent::WindowDeactivate )
    {
        if( m_pExtSQL && object == m_pExtSQL) m_pExtSQL->setWindowOpacity ( 0.7 );
        if( m_pGetCLP && object == m_pGetCLP) m_pGetCLP->setWindowOpacity ( 0.7 );
        if( m_pShowXML && object == m_pShowXML ) m_pShowXML->setWindowOpacity ( 0.7 );
    }
    return false;
}

/*****************************************************
*
*****************************************************/
void TabEdit::newLineClick()
{
    //    msg("New?");
}
void TabEdit::contextSelected(int index)
{
    PMF_UNUSED(index);
    deb(__FUNCTION__, "start");
    m_pMainDSQL->setDatabaseContext(contextCB->currentText());
    if( m_pMainDSQL->sqlCode() && m_pMainDSQL->sqlCode() != 1000 ) msg(m_pMainDSQL->sqlError());
    deb(__FUNCTION__, "Setting db to "+contextCB->currentText());
    m_pMainDSQL->initAll("select db_name()");
    deb(__FUNCTION__, "DBContext: "+m_pMainDSQL->rowElement(1,1));

    fillSchemaCB();
    deb(__FUNCTION__, "contextSelected done");
    //m_pMainDSQL->initAll("USE "+databaseCB->currentText());
}
/*****************************************************
*
*****************************************************/
void TabEdit::fillSchemaCB(GString context, GString selectSchema )
{
    deb(__FUNCTION__, "start");
    tableCB->clear();
    int pos;
    if( contextCB )
    {
        schemaCB->clear();
        //if( GString(contextCB->currentText()) == _selStringCB ) return;
        //if( !context.length() ) return;
        pos = findInComboBox(contextCB,context);
        if( pos >= 0 )
        {
            contextCB->setCurrentIndex(pos);
            m_pMainDSQL->setDatabaseContext(contextCB->currentText());
        }
    }    
    if( m_pMainDSQL->getDBType() == MARIADB ) m_pMainDSQL->setCurrentDatabase(selectSchema);
    pos = schemaCB->fill(m_pMainDSQL, selectSchema, isChecked(_HIDESYSTABS), 1);
    if( pos >= 0 ) schemaSelected(pos);
    deb(__FUNCTION__, "done");
}

/*****************************************************
*
*****************************************************/
void TabEdit::schemaSelected(int index)
{
    deb(__FUNCTION__, "start");
    tableCB->clear();
    if( index == 0 )
    {
        fillSchemaCB();
        return;
    }

    GString schema, name;
    m_seqTableNames.removeAll();
    //Index > 0...
    
    tableCB->addItem(_selStringCB);
    schema = schemaCB->currentText();
    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    //if( contextCB )pDSQL->initAll("USE "+contextCB->currentText());
    deb(__FUNCTION__, "getTables for schema "+schema);
    pDSQL->getTables(schema);
    deb(__FUNCTION__, "getTables done");

    if( m_pMainDSQL->getDBType() == MARIADB ) m_pMainDSQL->setCurrentDatabase(schema);
    if( contextCB )
    {
        m_pPMF->setLastSelectedSchema(GString(contextCB->currentText()), schema);
    }
    else m_pPMF->setLastSelectedSchema("", schema);

    for( unsigned int i=1; i<=pDSQL->numberOfRows(); ++i)
    {
        name   = pDSQL->rowElement(i,1).strip().strip("'").strip();
        deb(__FUNCTION__, "adding tabName "+name);
        tableCB->addItem(name);
        m_seqTableNames.add(name);
        m_gstrCurrentSchema = schema;
    }
    delete pDSQL;
    //tableCB->model()->sort(0);
    deb(__FUNCTION__, "end");
    //int w = tableCB->minimumSizeHint().width();
    //tableCB->setMinimumWidth(w+50);

}
/*****************************************************
*
*****************************************************/
void TabEdit::tableSelected()
{
    m_iLastSortedColumn = -1;
    cmdLineLE->setText("");
    setMyTabText(currentTable(0));
    reload("SELECT * FROM "+currentTable());
    addToCmdHist("SELECT * FROM "+currentTable());

    waitForThread();
    fillSelectCBs();
    loadCmdHist();
}
void TabEdit::popupTableCB()
{
    tableCB->showPopup();
}

/*****************************************************
*
*****************************************************/
void TabEdit::fillSelectCBs()
{
    hostCB->clear();
    //likeCB->clear();
    orderCB->clear();
    waitForThread();
    for( unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++i )
    {
        hostCB->addItem("\""+m_pMainDSQL->hostVariable(i)+"\"");
        orderCB->addItem("\""+m_pMainDSQL->hostVariable(i)+"\"");
    }
    //    likeCB->addItem("=");
    //    likeCB->addItem(" LIKE ");
    //    likeCB->addItem(" < ");
    //    likeCB->addItem(" > ");
    //    likeCB->addItem(" <> ");
    initTxtEditor(cmdLineLE);
}
void TabEdit::initTxtEditor(TxtEdit *pTxtEdit)
{
    deb(__FUNCTION__, "Start");
    QStringList completerStrings;

    GSeq <GString> hostVarList;

    hostVarList.add(currentTable());
    for( unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++i )
    {
        if( m_pMainDSQL->getDBType() == MARIADB ) hostVarList.add(m_pMainDSQL->hostVariable(i));
        else hostVarList.add("\""+ m_pMainDSQL->hostVariable(i)+"\"");
    }

    //Completer gets all of the above.
    /*
    for( unsigned int i = 1; i <= hostVarList.numberOfElements(); ++i )completerStrings +=hostVarList.elementAtPosition(i);
    for( unsigned int i = 1; i <= sqlCmdList.numberOfElements(); ++i ) completerStrings +=sqlCmdList.elementAtPosition(i);
    */



    m_pPMF->addToHostVarSeq(&hostVarList);
    completerStrings = m_pPMF->completerStringList();
    deb(__FUNCTION__, "ListCount: "+GString(completerStrings.count()));

    if( isChecked(_TEXTCOMPLETER) )
    {
        QCompleter *completer = new QCompleter(completerStrings, this);        
        pTxtEdit->setCompleter(completer);
    }

    deb(__FUNCTION__, "CountSqlCmdList: "+GString(m_pPMF->sqlCmdSeq()->numberOfElements())+", hostVarSeqCOunt: "+GString(m_pPMF->hostVarSeq()->numberOfElements()));
    QTextDocument *qtd = pTxtEdit->document();
    if( !qtd ) msg("isnull");
    GString txt = qtd->toPlainText();
    deb(__FUNCTION__, "TXT: "+txt);

    SqlHighlighter * sqlHighlighter = new SqlHighlighter(m_iUseColorScheme, pTxtEdit->document(), m_pPMF->sqlCmdSeq(), m_pPMF->hostVarSeq());
    pTxtEdit->setSqlHighlighter(sqlHighlighter);
    deb(__FUNCTION__, "Done");
}

/*****************************************************
*
*****************************************************/
void TabEdit::getBlobClick()
{
    /* stress test
while(1)
{
okClick();
}
*/
    if( currentTable().occurrencesOf(_selStringCB) || currentTable().occurrencesOf(_newTabString) )
    {
        msg("Please open a table first");
        return;
    }

    short sel = 0, erc = 0;
    msg("For each selected row, the LOBs will be written to files.\nYou will be prompted for a file name for *each LOB*.");

    QItemSelectionModel* selectionModel = mainWdgt->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);
    deb(__FUNCTION__, "selCount: "+GString(selected.count()));
    if( selected.count() == 0 )
    {
        msg("No rows selected.\nTo select a row, click on the vertical header.");
        return;
    }
    int written;
    for(int i= 0; i< selected.count();i++)
    {
        QModelIndex index = selected.at(i);
        erc = getBlob(index.row(), &written);
        sel++;

    }
    if( !sel ) msg("Select at least one row.");
    else msg("LOB->file done.\n\nRows selected: "+GString(sel)+", written: "+GString(written)+", errors: "+GString(erc));
}
/*****************************************************
*
*
*
*****************************************************/
short TabEdit::getBlob(long pos, int* written)
{
    int erc = 0;
    *written = 0;
    GString message, host, data, cmd, outFile, error;
    GSeq <GString> blobSeq;

    if( pos < 0 || pos > mainWdgt->rowCount()) return 1;
    message = " FROM "+currentTable()+" WHERE ";
    //m_pMainDSQL->setDebug(4);
    for( unsigned i=1; i<=m_pMainDSQL->numberOfColumns(); ++i)
    {
        //if( (m_pMainDSQL->sqlType(i) >= 404 && m_pMainDSQL->sqlType(i) <= 413)  || (m_pMainDSQL->sqlType(i) >= 960 && m_pMainDSQL->sqlType(i) <= 969) )
        if( m_pMainDSQL->isLOBCol(i) )
        {
            if( mainWdgt->item(pos, i+1)->text() != "NULL" ) blobSeq.add(m_pMainDSQL->hostVariable(i));
        }
        else
        {
            host = m_pMainDSQL->hostVariable(i);
            data = mainWdgt->item(pos, i+1)->text();
            if( data == "NULL" ) continue;
            m_pMainDSQL->convToSQL(data);
            message += host+"="+data+" AND ";
        }
    }
    message = message.strip(" AND ");

    if( blobSeq.numberOfElements() == 0 )
    {
        msg("There are no LOBs in this table.");
        return 9999;
    }
    QString prevDir;
    for( unsigned i = 1; i <= blobSeq.numberOfElements(); ++ i )
    {
        cmd = "SELECT "+blobSeq.elementAtPosition(i)+message;

        outFile = QFileDialog::getSaveFileName(this, "*.*", prevDir);
        if( !outFile.length() ) continue;
        QFileInfo fi(outFile);
        prevDir = fi.absolutePath();
#if defined(MAKE_VC) || defined (__MINGW32__)
        outFile = GString(outFile).translate('/', '\\');
#endif
        int outSize;

        error = m_pMainDSQL->descriptorToFile(cmd, outFile, &outSize);
        if( error.length() )
        {
            msg(error);
            erc++;
        }
        else (*written)++;
    }
    return erc;
}


void TabEdit::putBlobClick()
{
    GString s = "How to insert/update a LOB:\n0. Select a table with LOB columns.\n1. Edit an existing row or click 'Create new row'.\n";
    s += "2. Fill in the values for each column.\n3. In a LOB column, set a question mark (?).\n";
    s += "4. Click 'Save'\n";
    s+= "5. You will be prompted for a file name for each LOB to insert/update.";
    msg(s);

}
void TabEdit::cancelClick()
{
    /*TODO
    dsqlapi aAPI;
    aAPI.stopReq();
    m_pMainDSQL->setStopThread(1);
    */

}

void TabEdit::timerDone()
{}

int TabEdit::isSelectStatement(GString in)
{
    if( in.strip().upperCase().subString(1, 6) == "SELECT" ) return 1;
    if( in.strip().upperCase().subString(1, 6) == "XQUERY" ) return 1;
    return 0;
}

/*****************************************************
* run the SQLCmd from cmdLineLE.
*****************************************************/
GString TabEdit::okClick()
{
    deb(__FUNCTION__, "start");
    if( m_iLockAll ) return "";


    if( cmdLineLE->toPlainText().trimmed().length() == 0 )
    {
        fill();
        return "";
    }

    mainWdgt->blockSignals( true );
    m_iLockAll = 1;
    setButtonState(false);
    GString cmd = cmdLineLE->toPlainText();

    deb(__FUNCTION__, "calling reload, cmd: "+cmd);
    setTableNameFromStmt(cmd);

    reload(cmd);
    if( !m_gstrSqlErr.length())
    {
        deb(__FUNCTION__, "adding to cmdHist, cmd: "+cmd);
        addToCmdHist(cmd);
    }
    if( !isSelectStatement(cmd) && !m_gstrSqlErr.length() ) cmdLineLE->setText("");
    m_iLockAll = 0;
    mainWdgt->blockSignals( false );
    deb(__FUNCTION__, "done.");

    return m_gstrSqlErr;
}
void TabEdit::setCmdLine(int index)
{
    GString s = filterCB->itemText(index);
    this->setCmdLine(s);
    cmdLineLE->setFocus();

}
void TabEdit::setCmdLine(GString cmd)
{
    GString table;
    if( cmd.occurrencesOf(_LAST_SQL_TABLE) )
    {
        cmdLineLE->setText(cmd.subString(1, cmd.indexOf(_LAST_SQL_TABLE)-1));
        table = cmd.subString(cmd.indexOf(_LAST_SQL_TABLE)+_LAST_SQL_TABLE.length(), cmd.length()).strip().stripTrailing("]");
        if( !cmd.occurrencesOf("<Select")  ) setCurrentTable(table);
    }
    else cmdLineLE->setText(cmd);
}

GString TabEdit::firstHistLine()
{
    if( !currentTable().length() ) return "";
    if( filterCB->count() ) return filterCB->itemText(0);
    return "";
}

/***********************************************************
*
* Build SQL command from HostVariables
*
***********************************************************/
void TabEdit::setHost(int)
{

    GString toAdd;
    int orderPos = 0;
    GString content = GString(cmdLineLE->toPlainText());
    cmdLineLE->setFocus();
    QTextCursor qtc =  cmdLineLE->textCursor();
    if( content.length() )
    {
        if(GString(content).upperCase().occurrencesOf(" WHERE ") == 0) toAdd = " WHERE "+GString(hostCB->currentText());
        else toAdd = " AND "+GString(hostCB->currentText());
        orderPos = GString(content).upperCase().indexOf(" ORDER ");

        if( orderPos > 0 ) content.insert(toAdd, orderPos);
        else content += toAdd;

        cmdLineLE->setText(content);
        if( orderPos > 0 ) qtc.setPosition(orderPos+toAdd.length()-1);
        else qtc.setPosition(content.length());
        //qtc.movePosition( QTextCursor::Right, QTextCursor::MoveAnchor, orderPos  );
        cmdLineLE->setTextCursor(qtc);
        return;
    }
    else content = "SELECT * FROM "+currentTable()+" WHERE "+GString(hostCB->currentText());
    cmdLineLE->setText(content);
    qtc.setPosition(content.length());
    cmdLineLE->setTextCursor(qtc);
}
/***********************************************************
*
***********************************************************/
void TabEdit::setLike(int){

    GString msg = cmdLineLE->toPlainText();
    msg += GString(likeCB->currentText());
    cmdLineLE->setText(msg);
}
/***********************************************************
*
***********************************************************/
void TabEdit::setOrder(int){
    GString msg = cmdLineLE->toPlainText();
    if( msg.occurrencesOf("ORDER BY") > 0 ) msg += ", "+orderCB->currentText();
    else msg += " ORDER BY "+orderCB->currentText();
    cmdLineLE->setText(msg);
}
/***********************************************************
*
***********************************************************/
void TabEdit::clearButtonClicked()
{
    cmdLineLE->setText("");
}
/*****************************************************
*
*****************************************************/
void TabEdit::msg(QString txt)
{
    Helper::msgBox(this, "pmf", txt);
    //QMessageBox::information(this, "pmf", txt);
}
/*****************************************************
*
*****************************************************/
void TabEdit::deb(GString txt)
{
    if( m_pGDeb ) m_pGDeb->debugMsg("tabEdit", m_iTabID, txt);
}
void TabEdit::deb(GString fnName, GString txt)
{
    if( m_pGDeb ) m_pGDeb->debugMsg("tabEdit", m_iTabID, "::"+fnName+" "+txt);
}
/*****************************************************
*
*****************************************************/
void TabEdit::qdeb(QString s)
{
#if QT_VERSION >= 0x050000
    QByteArray   bytes  = s.toLatin1();
#else
    QByteArray   bytes  = s.toAscii();
#endif
    const char * ptr    = bytes.data();
    printf("pmf/tabEdit: %s\n", ptr);
}
/*****************************************************
*
*****************************************************/
unsigned int TabEdit::maxRows()
{
    if( GString(maxRowsLE->text()).asInt() == 0 ) maxRowsLE->setText("2000");
    GString(maxRowsLE->text()).asLong();
    return GString(maxRowsLE->text()).asLong();
}
/*****************************************************
*
*****************************************************/
void TabEdit::setPendingMsg(int reset)
{

    int updates = 0, inserts = 0;
    QTableWidgetItem *actItem;
    if( !reset )
    {
        for( int i = 0; i < mainWdgt->rowCount(); ++i)
        {
            actItem = mainWdgt->item(i, 1);
            if( NULL != actItem )
            {
                if(GString(actItem->text()) == _INSMark) inserts++;
                else if(GString(actItem->text()) == _UPDMark )updates++;
            }

        }
    }
    updLE->setText(tr("%1").arg(updates));
    insLE->setText(tr("%1").arg(inserts));

    if( !m_iUseColorScheme ) return;
    if( updates > 0  )  updLE->setStyleSheet(_FgRed);
    else updLE->setStyleSheet(_FgBlue);
    if( inserts > 0 )  insLE->setStyleSheet(_FgRed);
    else insLE->setStyleSheet(_FgBlue);


}
/*****************************************************
*
*****************************************************/
/*
bool tabEdit::eventFilter(QObject* obj, QEvent* event)
{
    //THIS IS NOT USED. See ::clearMainWidget()
    //Todo: this gets triggered by arrowKeys et al.
    if( obj == mainWdgt->viewport() && event->type() == QEvent::KeyRelease )
    {
        printf("On MainWdgt\n");
        if( mainWdgt->currentItem() )    itemChg(mainWdgt->currentItem());
        return true;
    }
    return false;
    //return tabEdit::eventFilter(obj, event);
}
*/
/*****************************************************
*
*****************************************************/
void TabEdit::clearMainWdgt()
{

    mainWdgt->clear();
    emit mainWidgetCleared();
    return;

    delete mainWdgt;
    mainWdgt = new QTableWidget();
    m_mainGrid->addWidget(mainWdgt,2,0,1,10);


    connect(mainWdgt, SIGNAL(itemChanged(QTableWidgetItem*) ), this, SLOT(itemChg(QTableWidgetItem*)));
    //mainWdgt->setEditTriggers(QAbstractItemView::AnyKeyPressed);
    //mainWdgt->setSelectionBehavior(QAbstractItemView::SelectRows);

    //This event filter is needed when a user edits a cell and uses CTRL+S to Save *without* having
    //left the cell. The signal itemChanged does not get fired!
    /* mainWdgt->viewport()->installEventFilter(this); */

    ////NoNoNo: Setting the focus on the Save-Button works better. See ::chgClick()

    //createActions();
    return;

    //DEAD CODE AHEAD
    /*******************************
    mainWdgt->blockSignals( true );
    QTableWidgetItem *pItem;
    for(int i = 0; i < mainWdgt->rowCount(); ++i)
    {
        for(int j = 0; j < mainWdgt->columnCount(); ++j)
        {
            pItem = mainWdgt->item(i, j);
            delete pItem;
        }
    }
    mainWdgt->setRowCount(0);
    mainWdgt->setColumnCount(0);
    mainWdgt->blockSignals( false );
    ********************************/
}

/*****************************************************
*
*****************************************************/
void TabEdit::keyPressEvent(QKeyEvent *event)
{

    switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if( isChecked(_ENTERTOSAVE) )
        {
            if( !cmdLineLE->hasFocus() ) chgClick();
            else okClick();
        }
        else
        {
            if( !cmdLineLE->hasFocus() ) mainWdgt->setFocus();
            else okClick();
        }
        break;

    case Qt::Key_F5:
        if( !isSelectStatement(cmdLineLE->toPlainText()) ) cmdLineLE->clear();
        okClick();
        break;

    case Qt::Key_F7:
        if( GString(cmdLineLE->toPlainText()).strip().length() == 0 )
        {
            cmdLineLE->setText(getSQL()+" ");
            cmdLineLE->setFocus();
            cmdLineLE->moveCursor(QTextCursor::End);
        }
        break;

    case Qt::Key_Escape:
        if( isChecked(_USEESCTOCLOSE) )
        {
            if( m_pExtSQL )
            {
                delete m_pExtSQL;
                m_pExtSQL = NULL;
                return;
            }
            if( m_pGetCLP )
            {
                delete m_pGetCLP;
                m_pGetCLP = NULL;
                return;
            }
            if( m_pShowXML )
            {
                delete m_pShowXML;
                m_pShowXML = NULL;
                return;
            }
            m_pPMF->closePMF();
        }
        return;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        if( mainWdgt->hasFocus() )deleteRows();
        break;
    }
    if(event->modifiers() & Qt::AltModifier )
    {
        if( event->key() == Qt::Key_Right ) slotForward();
        else if( event->key() == Qt::Key_Left ) slotBack();
    }
    if(event->modifiers() & Qt::ControlModifier )
    {
        if( event->key() == Qt::Key_S ) chgClick();
        //        if( event->key() == Qt::Key_U )
        //        {
        //            QTableWidgetItem * pItem = currentItem();
        //            GString s = pItem->text();
        //            pItem->setText(s.upperCase());
        //        }
    }

    if( mainWdgt->hasFocus() && event->key() != Qt::Key_Escape)
    {
        if(event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_Insert)
        {
            slotRightClickActionCopy();
        }
        if(event->modifiers() & Qt::ShiftModifier && event->key() == Qt::Key_Insert)
        {
            slotRightClickActionPaste();
        }
        if(event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_C)
        {
            slotRightClickActionCopy();
        }
        if(event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_V)
        {
            slotRightClickActionPaste();
        }

    }

}
/*
void  tabEdit::PMF_QTableWidget::editMultipleItems(QWidget* editor)
{
    QLineEdit* myeditor = qobject_cast<QLineEdit*>(editor);     //recast to whatever widget was actually used
    if(myeditor != 0) {
        foreach(const QModelIndex& index, this->selectionModel()->selectedIndexes()) {
            QVariant v(myeditor->text());
            model()->setData(index, v, Qt::EditRole);
        }
    }
}

void tabEdit::PMF_QTableWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        QTableWidgetItem * pItem = myTabEdit->mainWdgt->itemAt(event->pos());
        if( pItem )
        {
//            QItemSelectionModel* selectionModel = myTabEdit->mainWdgt->selectionModel();
//            QModelIndexList selected = helper::getSelectedRows(selectionModel);

            //if( selected.count() <= 1 ) myTabEdit->mainWdgt->setCurrentItem(pItem);
            myTabEdit->setActionsMenu(pItem);
        }
    }
    else QTableWidget::mousePressEvent(event);
}
*/
/*****************************************************
*
*****************************************************/
void TabEdit::disableActions()
{
    m_qaBitHide->setEnabled(false);
    m_qaBitAsHex->setEnabled(false);
    m_qaBitAsBin->setEnabled(false);
    m_qaShowXML->setEnabled(false);
    m_qaEditLong->setEnabled(false);
    m_qaEditText->setEnabled(false);
    m_qaEditGraphic->setEnabled(false);
    m_qaEditDBClob->setEnabled(false);
    m_qaSaveBlob->setEnabled(false);
    m_qaOpenBlob->setEnabled(false);
    m_qaOpenBlobAs->setEnabled(false);
    m_qaLoadBlob->setEnabled(false);
    m_qaCopy->setEnabled(false);
    m_qaPaste->setEnabled(false);
    m_qaUpdate->setEnabled(false);
    m_qaDistinct->setEnabled(false);
    m_qaGroup->setEnabled(false);

    m_qaCurCell->setEnabled(false);
    m_qaAddConst->setEnabled(false);
    m_qaCurRow->setEnabled(false);
    m_qaNotNull->setEnabled(false);
    m_qaCount->setEnabled(false);
}

void TabEdit::setActionsMenu(QTableWidgetItem *pItem)
{
    m_pActionItem = pItem; //can be NULL
    if( !pItem )
    {
        disableActions();
        return;
    }
    int col = pItem->column() - 1;
    deb(__FUNCTION__, "col: "+GString(col)+", simpleColType: "+GString(m_pMainDSQL->simpleColType(col))+", colType: "+GString(colType(col)));

    if( isNewRow(pItem) )
    {
        disableActions();
        if( m_pMainDSQL->isLOBCol(col) ) m_qaLoadBlob->setEnabled(true);
        return;
    }

    m_qaEditLong->setEnabled(false);
    m_qaEditGraphic->setEnabled(false);
    m_qaShowXML->setEnabled(false);
    m_qaSaveBlob->setEnabled(false);
    m_qaOpenBlob->setEnabled(false);
    m_qaOpenBlobAs->setEnabled(false);
    m_qaLoadBlob->setEnabled(false);
    m_qaEditDBClob->setEnabled(false);
    m_qaEditText->setEnabled(true);
    m_qaCopy->setEnabled(true);
    m_qaCurCell->setEnabled(true);
    m_qaAddConst->setEnabled(true);
    m_qaCurRow->setEnabled(true);
    m_qaNotNull->setEnabled(true);
    m_qaCount->setEnabled(true);
    m_qaUpdate->setEnabled(true);
    m_qaDistinct->setEnabled(true);
    m_qaGroup->setEnabled(true);
    m_qaDelTabAll->setEnabled(true);

    if( m_pMainDSQL->simpleColType(col) == CT_BLOB) m_qaEditText->setEnabled(false);

    if( m_pMainDSQL->isLOBCol(col) ) //lob
    {
        m_qaSaveBlob->setEnabled(true);
        m_qaOpenBlob->setEnabled(true);
        m_qaOpenBlobAs->setEnabled(true);
        m_qaLoadBlob->setEnabled(true);
        m_qaCurCell->setEnabled(false);
        m_qaAddConst->setEnabled(false);
        m_qaCurRow->setEnabled(false);
        m_qaCount->setEnabled(false);
        m_qaDistinct->setEnabled(false);
        m_qaGroup->setEnabled(false);
    }
    else if( m_pMainDSQL->isXMLCol(col) ) //xml
    {
        m_qaShowXML->setEnabled(true);
        m_qaLoadBlob->setEnabled(true);
        m_qaEditText->setEnabled(false);
        m_qaCurCell->setEnabled(false);
        m_qaAddConst->setEnabled(false);
        m_qaCurRow->setEnabled(false);
        m_qaCount->setEnabled(false);
        m_qaDistinct->setEnabled(false);
        m_qaGroup->setEnabled(false);
    }
    else if( m_pMainDSQL->isLongTypeCol(col) ) m_qaEditLong->setEnabled(true);
    else if( m_pMainDSQL->simpleColType(col) == CT_GRAPHIC )
    {
        m_qaEditGraphic->setEnabled(true);
        m_qaEditText->setEnabled(false);
    }


    if( m_pMainDSQL->simpleColType(col) == CT_DBCLOB || m_pMainDSQL->simpleColType(col) == CT_CLOB )
    {
        m_qaEditDBClob->setEnabled(true);
        m_qaSaveBlob->setEnabled(true);
        m_qaEditText->setEnabled(false);
    }
}

/*****************************************************
*
*****************************************************/
void TabEdit::setDBName(GString db)
{
    dbNameLE->setText(db);
}
//#include "tabEdit.moc"

/*****************************************************
*
*****************************************************/
void TabEdit::setButtonState(bool state)
{
    deb(__FUNCTION__, "Start");
    if( state ) cancelB->setEnabled(false);
    else cancelB->setEnabled(true);
    deb(__FUNCTION__, "Start0");
    schemaCB->setEnabled(state);
    tableCB->setEnabled(state);
    deb(__FUNCTION__, "Start1");
    //refreshB->setEnabled(state);
    runB->setEnabled(state);
    insertB->setEnabled(state);
    saveB->setEnabled(state);
    deleteB->setEnabled(state);
    deb(__FUNCTION__, "Start2");
    extSQLB->setEnabled(state);
    exportButton->setEnabled(state);
    importButton->setEnabled(state);
    deb(__FUNCTION__, "Done");
    //newLineB->setEnabled(state);
}
/*****************************************************
*
*****************************************************/
void TabEdit::setInfo(GString txt, int blink)
{
    if( isChecked(_COUNTROWS) )
    {
        DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
        GString err = pDSQL->initAll("SELECT COUNT(*) FROM "+currentTable());
        if( !err.length() ) txt =  "Rows: "+pDSQL->rowElement(1,1)+", "+txt;
        delete pDSQL;
    }
    msgLE->setText(txt);
    if( blink )
    {
        int i = 4;
        while(i--)
        {
            if( m_iUseColorScheme ) msgLE->setStyleSheet(_BgRed);
            msgLE->repaint();
            GStuff::dormez(100);
            if( m_iUseColorScheme ) msgLE->setStyleSheet(_BgWhite);
            msgLE->repaint();
            GStuff::dormez(100);
        }
        if( m_iUseColorScheme ) msgLE->setStyleSheet(_FgRed);
    }
    else if( m_iUseColorScheme )
    {
        msgLE->setStyleSheet(_FgBlue);
    }
    return;
}
/*****************************************************
*
*****************************************************/
void TabEdit::showHint(int hint, GString txt)
{

    if( 0 == hint )
    {
        hintLE->hide();
        return;
    }
    switch(hint)
    {
    case hintXML:
        hintLE->setText("Hint: Drag and drop files into cells to INSERT or UPDATE LOBs/XMLs or right-click a cell to edit/save LOBs and XMLs");
        if( m_iUseColorScheme ) hintLE->setStyleSheet(_BgYellow);
        break;
    case hintNewVer:
        hintLE->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        //#ifdef MAKE_VC
        hintLE->setOpenExternalLinks(false);
        hintLE->setText("Hint: A new version ("+txt+") is available, click <a href='download'>here</a> to download");
        //#else
        //            hintLE->setOpenExternalLinks(true);
        //            hintLE->setText("Hint: A new version ("+txt+") is available at <a href='http://leipelt.org/downld.html'>www.leipelt.org</a>");
        //#endif
        if( m_iUseColorScheme ) hintLE->setStyleSheet(_NewVersColor);
        break;
    case hintMultiCellEdit:
        hintLE->setText("Hint: Select multiple cells vertically and right-click to update");
        if( m_iUseColorScheme ) hintLE->setStyleSheet(_BgYellow);
        break;

    case hintColorHint:
        hintLE->setText("Hint: Cells containing blanks will be marked in different colors. Hover the mouse over a colorized cell to see the tooltip.");
        if( m_iUseColorScheme ) hintLE->setStyleSheet(_BgBlue);
        break;

    default:
        if( m_iUseColorScheme ) hintLE->setStyleSheet(_BgYellow);
        hintLE->setText("Hint: right-click a cell to filter data ");
        hintLE->setText("Hint: open another tab with CTRL+T, use CTRL+W to close");
        hintLE->setText("Hint: to edit XML data, right-click the XML cell");

        hintLE->hide();
        return;
    }
    hintLE->show();
}

void TabEdit::slotLabelLinkClicked(QString s)
{
    //#ifdef MAKE_VC
    msg("ClickName: "+GString(s));
    m_pPMF->getNewVersion();
    //#endif
}

/*****************************************************
*
*****************************************************/
GString TabEdit::currentSchema()
{
    return schemaCB->currentText();
}

GString TabEdit::currentContext()
{
    if( contextCB )
    {
        return contextCB->currentText();
    }
    return "";
}

/*****************************************************
*
*****************************************************/
GString TabEdit::currentTable(int wrapped)
{
    if( !schemaCB || !tableCB ) return "";

    if( schemaCB->currentIndex() <= 0 || tableCB->currentIndex() <= 0 ) return "";
    //SqlServer:
    if( contextCB )
    {
        if( wrapped )  return "\""+contextCB->currentText()+"\".\""+ schemaCB->currentText()+"\".\""+tableCB->currentText()+"\"";
        else return contextCB->currentText()+"."+ schemaCB->currentText()+"."+tableCB->currentText();
    }
    if( m_pMainDSQL->getDBType() == MARIADB ) return schemaCB->currentText()+"."+tableCB->currentText();
    if( m_pMainDSQL->getDBType() == POSTGRES ) return schemaCB->currentText()+"."+tableCB->currentText();
    //DB2
    if( wrapped )  return "\""+ schemaCB->currentText()+"\".\""+tableCB->currentText()+"\"";
    else return schemaCB->currentText()+"."+tableCB->currentText();

}
/*****************************************************
*
*****************************************************/
void TabEdit::setMaxRows(GString maxRows)
{
    if( maxRows.asInt() > 0 ) maxRowsLE->setText(maxRows);
    else maxRowsLE->setText("2000");
}
/*****************************************************
*
*****************************************************/
/*****************************************************
*
*****************************************************/
GString TabEdit::getSQL()
{
    if( cmdLineLE->toPlainText().length() ) return cmdLineLE->toPlainText();
    return "SELECT * FROM "+currentTable();
}
/*****************************************************
*
*****************************************************/
GString TabEdit::getLastSelect()
{
    if( GString(tableCB->currentText()) == "" || GString(tableCB->currentText()) == _selStringCB ) return "";
    return m_pMainDSQL->lastSqlSelectCommand();
}

/*****************************************************
*
*****************************************************/
GString TabEdit::getDB()
{
    return dbNameLE->text();
}
/*****************************************************
*
*****************************************************/
/*****************************************************
*
*****************************************************/
void TabEdit::loadBookmark(GString table, GString cmd)
{
    setCurrentTable(table);
    cmdLineLE->setText(cmd);
    reload(cmd);
    fillSelectCBs();
    addToCmdHist(cmd);
}
/*****************************************************
*
*****************************************************/
void TabEdit::setCmdText(GString cmd)
{
    cmdLineLE->setText(cmd);
}
/*****************************************************
*
*****************************************************/
GString TabEdit::getCmdText()
{
    return cmdLineLE->toPlainText();
}

/*****************************************************
*
*****************************************************/
void TabEdit::setExtSqlCmd(GString cmd)
{
    m_gstrExtSqlCmd = cmd;
}
/**************************************************************************************
* Back/Forward buttons
**************************************************************************************/
void TabEdit::slotBack()
{
    deb("slotBack");
    if( m_seqCmdHist.numberOfElements() <= 1 ) return;
    m_iCmdHistPos--;
    GString cmd = m_seqCmdHist.elementAtPosition(m_iCmdHistPos);
    setCurrentTable(m_seqCmdHistTable.elementAtPosition(m_iCmdHistPos));
    reload(cmd);
    cmdLineLE->setText(cmd);
    cmdLineLE->setFocus();    
    setHistButtons();
}
/**************************************************************************************
* Back/Forward buttons
**************************************************************************************/
void TabEdit::slotForward()
{
    deb("slotForward");

    if( (int)m_seqCmdHist.numberOfElements() == m_iCmdHistPos ) return;    

    m_iCmdHistPos++;
    GString cmd = m_seqCmdHist.elementAtPosition(m_iCmdHistPos);
    setCurrentTable(m_seqCmdHistTable.elementAtPosition(m_iCmdHistPos));
    reload(cmd);
    cmdLineLE->setText(cmd);
    cmdLineLE->setFocus();
    setHistButtons();
}
void TabEdit::setHistButtons()
{
    if( m_iCmdHistPos <= 1 ) backBT->setEnabled(false);
    else backBT->setEnabled(true);
    if( m_iCmdHistPos >= (signed) m_seqCmdHist.numberOfElements() ) forwBT->setEnabled(false);
    else forwBT->setEnabled(true);

}
/**************************************************************************************
* IN: tableName.
* sets the current selection in contextCB, schemaCB and tableCB
* [QComboBox::findText(...) insists on searching caseSensitive, therefore: findInComboBox]
**************************************************************************************/
void TabEdit::setCurrentTable(GString table)
{

    int pos;
    while( table.occurrencesOf("\"")) table = table.remove(table.indexOf("\""),1);
    if( contextCB && table.occurrencesOf('.') == 2 )
    {
        pos = findInComboBox(contextCB,table.subString(1, table.indexOf(".")-1));
        if( pos >= 0 )
        {
            contextCB->setCurrentIndex(pos);
            contextSelected(pos);
        }
        table.remove(1, table.indexOf('.'));
    }
    pos = findInComboBox(schemaCB,table.subString(1, table.indexOf(".")-1));
    if( pos >= 0 )
    {
        schemaCB->setCurrentIndex(pos);
        schemaSelected(pos);
    }
    pos = findInComboBox(tableCB,table.subString(table.indexOf(".")+1, table.length()).strip());
    if( pos >= 0 )
    {
        tableCB->setCurrentIndex(pos);
        setMyTabText(currentTable(0));
    }
}
int TabEdit::findInComboBox(QComboBox *pCB, GString txt)
{
    for( int i = 0; i < pCB->count(); ++i )
    {
        if( GString(pCB->itemText(i)) == txt ) return i;
    }
    for( int i = 0; i < pCB->count(); ++i ) //Again, but case-insensitive
    {
        if( GString(pCB->itemText(i)).upperCase() == txt.upperCase() ) return i;
    }
    return -1;
}

/**************************************************************************************
*
* Actions when a cell is rightClicked
* Either filter for a cell or for a whole row
*
**************************************************************************************/
QTableWidgetItem *TabEdit::currentItem()
{
    if( currentTable().length() < 2 )
    {
        msg("Please open a table first");
        return NULL;
    }
    //Can be NULL
    printf("CurItem: Act: %i, std: %i\n", (long) m_pActionItem, (long) mainWdgt->currentItem());
    if( m_pActionItem ) return m_pActionItem;
    return mainWdgt->currentItem();
}
void TabEdit::filterCurrentCell(int mode)
{
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);
    GString cmd;
    ///Maybe a bit too sharp for myself
    ///GString cmd = cmdLineLE->text();
    ///if( m_gstrLastFilter == cmd && cmd.length() ) cmd += " AND "+m_pMainDSQL->hostVariable(col)+"="+GString(pItem->text());
    ///else cmd = "SELECT * FROM "+currentTable()+" WHERE "+m_pMainDSQL->hostVariable(col)+"="+GString(pItem->text());

    //Remember: There are 2 invisible columns!

    //int col = mainWdgt->currentItem()->column()-1;
    int col = pItem->column()-1;

    GString val = GString(pItem->text());
    if( val.occurrencesOf("@DSQL@") || (m_pMainDSQL->isXMLCol(col) && val != "NULL") )
    {
        msg("Data in this cell cannot be used as constraint.");
        return;
    }
    if( m_pMainDSQL->isForBitCol(col) >= 3  && val != "NULL"  ) val = "=x"+val;
    else if( val == "NULL") val = " IS NULL";
    else if( val == "NOT NULL") val = " IS NOT NULL";
    else
    {
        m_pMainDSQL->convToSQL(val);
        val= "="+val;
    }

    if(mode == 1 )cmd = "SELECT count(*) as \"[Count]\" FROM "+currentTable()+" WHERE "+wrap(m_pMainDSQL->hostVariable(col))+val;
    else cmd = "SELECT * FROM "+currentTable()+" WHERE "+wrap(m_pMainDSQL->hostVariable(col))+val;
    m_gstrLastFilter = cmd;
    reload(cmd);
    addToCmdHist(cmd);
    cmdLineLE->setText(cmd);

}

void TabEdit::slotRightClickActionCell()
{
    filterCurrentCell(0);
}
void TabEdit::slotRightClickCount()
{
    filterCurrentCell(1);
}


void TabEdit::slotRightClickActionAddConstraint()
{
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);
    //Remember: There are 2 invisible columns!
    //int col = mainWdgt->currentItem()->column()-1;
    int col = pItem->column()-1;
    GString cmd = cmdLineLE->toPlainText();
    GString val = GString(pItem->text());

    if( val.occurrencesOf("@DSQL@") || (m_pMainDSQL->isXMLCol(col) && val != "NULL"))
    {
        msg("Data in this cell cannot be used as constraint.");
        return;
    }


    if( m_pMainDSQL->isForBitCol(col) >= 3  && val != "NULL" ) val = "=x"+val;
    else if( val == "NULL") val = " IS NULL";
    else
    {
        m_pMainDSQL->convToSQL(val);
        val= "="+val;
    }
    if( !cmd.length() ) cmd = "SELECT * FROM "+currentTable()+" WHERE "+wrap(m_pMainDSQL->hostVariable(col))+val;
    else cmd += " AND "+wrap(m_pMainDSQL->hostVariable(col))+val;


    m_gstrLastFilter = cmd;
    reload(cmd);
    addToCmdHist(cmd);
    cmdLineLE->setText(cmd);

}

void TabEdit::slotRightClickActionRow()
{

    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);

    int row;
    row = pItem->row();
    GString val, col;
    GString cmd = "SELECT * FROM "+currentTable()+" WHERE ";
    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    for( unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        col = wrap(m_pMainDSQL->hostVariable(i)); //HostVariable, i.e. columnName
        //Remember: There are 2 invisible columns!
        val = mainWdgt->item(row, i+1)->text();
        if( m_pMainDSQL->isForBitCol(i) >= 3 && val != "NULL" ) val =  Helper::formatForHex(m_pMainDSQL, val);
        if( !val.strip().length() || val.subString(1, 6) == "@DSQL@"  || m_pMainDSQL->isXMLCol(i) ) continue;
        if( val == "NULL") cmd += col + " IS NULL AND ";
        else
        {
            m_pMainDSQL->convToSQL(val);
            cmd += col + "=" + val +" AND ";
        }

    }
    cmd = cmd.stripTrailing(" AND ");
    reload(cmd);
    addToCmdHist(cmd);
    cmdLineLE->setText(cmd);
    delete pDSQL;

}
void TabEdit::slotRightClickGroup()
{
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);
    GString colName = wrap(m_pMainDSQL->hostVariable(pItem->column()-1));
    GString cmd = "SELECT "+colName+", COUNT(*) AS \"[Count]\" FROM "+currentTable()+" GROUP BY "+colName+" ORDER BY "+colName;
    reload(cmd);
    addToCmdHist(cmd);
    cmdLineLE->setText(cmd);

}


void TabEdit::slotRightClickDistinct()
{
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);

    GString colName = wrap(m_pMainDSQL->hostVariable(pItem->column()-1));
    GString cmd = "SELECT DISTINCT("+colName+") FROM "+currentTable()+" ORDER BY "+colName;
    reload(cmd);
    addToCmdHist(cmd);
    cmdLineLE->setText(cmd);
}

void TabEdit::slotRightClickActionBitHide()
{
}
void TabEdit::slotRightClickActionBitAsHex()
{
}
void TabEdit::slotRightClickActionBitAsBin()
{
}
GString TabEdit::createUpdateCmd(GString newVal)
{

    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return "";

    GString cellVal = pItem->text();
    int col = m_pActionItem->column() - 1;
    GString constraint = cmdLineLE->toPlainText();
    int index = GString(constraint).upperCase().indexOf(" WHERE ");
    if( index > 0 )
    {
        constraint = constraint.subString(index+7, constraint.length() ).strip();
    }
    else constraint = "";
    GString cmd = "UPDATE "+currentTable(1)+"\nSET "+wrap(m_pMainDSQL->hostVariable(col))+"=";
    if( m_pMainDSQL->isForBitCol(col) >= 3  && newVal != "NULL"  )
    {
        newVal = Helper::formatForHex(m_pMainDSQL, "'"+newVal+"'");
        if( cellVal != "NULL" ) cellVal = Helper::formatForHex(m_pMainDSQL, cellVal);
    }
    else if( !m_pMainDSQL->isNumType(col) && newVal != "NULL" )
    {
        newVal= "'" + newVal + "'";
    }
    cmd += newVal+"\n";

    if(constraint.length())
    {
        cmd += " WHERE "+constraint;
    }
    else
    {
        if( cellVal != "NULL")  cmd += " WHERE "+wrap(m_pMainDSQL->hostVariable(col))+"="+cellVal;
        else cmd += " WHERE "+wrap(m_pMainDSQL->hostVariable(col))+" IS NULL";
    }
    return cmd;
}

void TabEdit::slotRightClickNotNull()
{
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);
    GString colName = wrap(m_pMainDSQL->hostVariable(pItem->column()-1));
    GString cmd = "SELECT * FROM "+currentTable()+" WHERE "+colName+" IS NOT NULL";
    reload(cmd);
    addToCmdHist(cmd);
    cmdLineLE->setText(cmd);
}

void TabEdit::slotRightClickUpdate()
{
    GString placeHolder, toReplace;
    placeHolder = toReplace = "REPLACE_THIS";

    GString orgFilter = cmdLineLE->toPlainText();

    GString cmd = "-- Generated UPDATE. Hint: To create a constraint, right-click cells\n-- and use the Filters, then right-click UPDATE\n\n";
    cmd += createUpdateCmd(placeHolder);
    /*
    extSQL * pExtSQL = new extSQL(m_pGDeb, m_pMainDSQL, this);
    pExtSQL->installEventFilter(this);
    pExtSQL->setCmd(cmd);
    pExtSQL->findText("REPLACE_THIS");
    pExtSQL->exec();
*/

    if( !m_pExtSQL )
    {
        m_pExtSQL = new ExtSQL(m_pGDeb, m_pMainDSQL, this);
        m_pExtSQL->installEventFilter(this);
    }
    m_pExtSQL->clearCmd();
    m_pExtSQL->setCmd(cmd, orgFilter);
    m_pExtSQL->findText(placeHolder);
    m_pExtSQL->show();
    /*
    m_pExtSQL->exec();
    if( orgFilter.length())
    {
        setCmdText(orgFilter);
        okClick();
    }
*/
}

void TabEdit::slotRightClickDeleteTable()
{
    GString whereClause = GStuff::WhereClauseFromStmt(lastSqlSelectCmd());

    GString out = "This will delete all rows from "+currentTable(0);
    if( whereClause.length() ) out += "\n [constraint: WHERE "+whereClause+"]\n";
    if( m_pMainDSQL->getDBType() == DB2 || m_pMainDSQL->getDBType() == DB2ODBC ) out+= "\n\nHint: On large tables you should first do\nREORG+RUNSTATS (Menu->Table->Reorg etc)\nor deleting may take a LONG time.";
    out+= "\n\nContinue?";

    if( QMessageBox::question(this, "PMF", out, "Yes", "No", 0, 1) == 1 ) return;
    if(!whereClause.length() )
    {
        if( QMessageBox::question(this, "PMF", "Really delete ALL ROWS from "+currentTable(0)+"?", "Yes", "No", 0, 1) == 1 ) return;
    }

    if( deleteViaFetch(currentTable(), whereClause) == 0 )
    {
        okClick();
        return;
    }


    return;

    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    GString err;
    if( whereClause.length() )
    {
        err = pDSQL->initAll("Delete from "+currentTable()+ " WHERE "+whereClause);
    }
    else err = pDSQL->initAll("Delete from "+currentTable());
    if( err.length() ) msg(err);
    else okClick();
}

void TabEdit::slotLineEditUpdate()
{
    //mainWdgt->blockSignals( true );
    GString val = pLineEditUpdateAction->lineEdit()->text();
    QList<QTableWidgetItem*> list = mainWdgt->selectedItems();
    QTableWidgetItem* pItem;
    for(int i = 0; i < list.count(); ++i )
    {
        pItem = list.at(i);
        if( GString(mainWdgt->item(pItem->row(), 1)->text()) == _NEWMark ) continue;
        pItem->setText(val);
    }
    actionsMenu.close();
    //mainWdgt->blockSignals( false );


    /* Currently unused
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;

    GString newVal = pLineEditUpdateAction->lineEdit()->text();

    GString cmd = createUpdateCmd(newVal);

    if( QMessageBox::question(this, "PMF", cmd+"\n\nContinue?", "Yes", "No", 0, 1) == 1 ) return;
    GString err = m_pMainDSQL->initAll(cmd.change("\n", ""));
    if( err.length() )msg(err);
    reload();
    */
}

void TabEdit::slotLineEditFilter()
{

    if( !m_pActionItem )
    {
        msg("Right-click on the cell to be filtered.");
        actionsMenu.close();
        return;
    }    
    GString val = pLineEditFilterAction->lineEdit()->text().toUpper();
    if( !val.strip().length() ) return;

    int addToCmd = 0;
    if( val[1] == '+' && cmdLineLE->toPlainText().length() )
    {
        addToCmd = 1;
        val = val.subString(2, val.length());
    }
    val = m_pMainDSQL->cleanString(val);
    int col = m_pActionItem->column() - 1;
    GString modifier1 = "";
    GString modifier2 = "";
    if( val.occurrencesOf("@DSQL@") || (m_pMainDSQL->isXMLCol(col) && val != "NULL") )
    {
        msg("Data in this cell cannot be filtered.");
        return;
    }

    int isUnqCol = 0;
    int isDb2 = 0;
    if( m_pMainDSQL->getDBTypeName() == _DB2 || m_pMainDSQL->getDBTypeName() == _DB2ODBC ) isDb2 = 1;


    if( m_pMainDSQL->getDBTypeName() == _SQLSRV && m_pMainDSQL->isForBitCol(col) ) isUnqCol = 1;
    else if( isDb2 && m_pMainDSQL->isForBitCol(col) >= 3) isUnqCol = 1;
    if( isUnqCol  && val != "NULL"  )
    {
        val = val.strip();
        if( val.length() % 2 && isDb2)
        {
            msg("This is a double-byte column, so the length of the search term must be an even number");
            return;
        }
        if( isDb2 && isChecked(_CONVERTGUID))
        {            
            if( val.length() == 36 && val.occurrencesOf('-') == 4 ) val = Helper::convertGuid(val);
        }
        else if( m_pMainDSQL->getDBTypeName() == _SQLSRV && isChecked(_CONVERTGUID))
        {
            if( val.length() == 32 && val.occurrencesOf('-') == 0 ) val = Helper::convertGuid(val);
        }
        int lng = 0;
        for(int i = 1; i <= (int)m_colDescSeq.numberOfElements(); ++i)
        {
            if( i == col ) lng =  m_colDescSeq.elementAtPosition(i)->Length.asInt();
        }
        if( isDb2 ) modifier1 = "HEX";
        val = " LIKE '%"+val+"%'";

    }
    else if( val == "NULL") val = " IS NULL";
    else if( m_pMainDSQL->isNumType(col) && m_pMainDSQL->getDBType() != SQLSERVER)
    {
        modifier1 = "CHAR";
        val= " LIKE '%"+val.upperCase()+"%'";
    }
    else if( m_pMainDSQL->isNumType(col) && m_pMainDSQL->getDBType() == SQLSERVER)
    {
        val = " LIKE '%"+val.upperCase()+"%'";
        modifier1 = "CAST ";
        modifier2 = "as NVARCHAR";
    }
    else if( GString(val).upperCase() == "CURRENT TIMESTAMP" || GString(val).upperCase() == "CURRENT DATE" )
    {
        val = "=" + val;
    }
    else if( GString(val).upperCase() == "GETDATE()" )
    {
        val = "=" + val;
    }
    else
    {
        modifier1 = "UPPER";
        m_pMainDSQL->convToSQL(val);
        val = " LIKE '%"+val.upperCase()+"%'";
    }

    GString cmd;
    if( addToCmd && cmdLineLE->toPlainText().length() )
    {
        cmd = GString(cmdLineLE->toPlainText()) + " AND "+modifier1+"("+wrap(m_pMainDSQL->hostVariable(col))+modifier2+")"+val;
    }
    else cmd = "SELECT * FROM "+currentTable()+" WHERE "+modifier1+"("+wrap(m_pMainDSQL->hostVariable(col))+modifier2+")"+val;
    m_gstrLastFilter = cmd;
    reload(cmd);
    addToCmdHist(cmd);
    cmdLineLE->setText(cmd);
    actionsMenu.close();
}

void TabEdit::addToCmdHist(GString cmd)
{
    cmd = cmd.strip();

    //don't store if it's the same cmd as last time
    if( m_seqCmdHist.numberOfElements() > 0 )
    {
        if( m_seqCmdHist.elementAtPosition(m_seqCmdHist.numberOfElements()) == cmd ) return;
    }
    m_seqCmdHist.add(cmd);
    m_seqCmdHistTable.add( currentTable() );
    m_iCmdHistPos = m_seqCmdHist.numberOfElements();
    if( m_iCmdHistPos > 1 ) backBT->setEnabled(true);
}


/**************************************************************************************
*
**************************************************************************************/
int TabEdit::writeHist(GString table, GString action, QTableWidgetItem*,  GString line)
{
    /********
  if( !iHistory ) return 0;
  short erc;
  long i;
  GString cmd, tmpUser;
  dsqlobj tmpDSQL;

  if( userName == "" ) tmpUser = "'<NotSet>'";
  else tmpUser = "'"+userName+"'";

  if( !line.length() )
  {
     for( i=1; i<=fldSeq.numberOfElements(); ++i)
     {
         line += fldSeq.elementAtPosition(i) + "|";
     }
  }
  line = "'"+line+"'";
  tmpDSQL.convToSQL(line);
  cmd = "INSERT INTO PMF.HISTORY VALUES ("+tmpUser+", CURRENT TIMESTAMP, '"+table+"', "+line+", '"+action+"')";
  erc = tmpDSQL.initAll(cmd);

  if( erc != 0 )
  {
     if( erc == -433 ) tm("ERC -433:\nCannot write to table PMF.HISTORY: Row too long. Will proceed anyway.");
     else tm("Write History: \n"+tmpDSQL.sqlError()+"\nCommand was:\n"+cmd);
  }
*******/
    return 0;
}
GString TabEdit::histFileName()
{
    QString home = QDir::homePath ();
    if( !home.length() ) return "";


    GString db = GString(dbNameLE->text()).strip();
    GString out;
#ifdef MAKE_VC
    out = GString(home)+"\\"+_CFG_DIR+"\\"+_HST_FILE;
#else
    out = GString(home)+"/."+_CFG_DIR + "/"+_HST_FILE;
#endif
    QFile f(out+"_"+db);
    if( !f.exists() ) QFile::copy(out, out+"_"+db);
    f.close();

    return  out+"_"+db;
}

/*****************************************************
*
*****************************************************/
void TabEdit::loadCmdHist(int filter)
{
    deb(__FUNCTION__, "Start");
    filterCB->clear();
    if( filter && currentTable().length() == 0 ) return;

    GString s, table;
    GFile f(histFileName());
    for( int i = 1; i <= f.lines(); ++i )
    {
        s = f.getLine(i);
        table = "";
        if( s.occurrencesOf(_LAST_SQL_TABLE) )
        {
            table = s;
            table = table.subString(table.indexOf(_LAST_SQL_TABLE)+_LAST_SQL_TABLE.length(), table.length()).strip().stripTrailing("]").strip();
        }
        if( table.length() && table == currentTable() && filter)
        {
            deb(__FUNCTION__, "adding hist for table: "+table+", cur: "+currentTable()+": "+s);
            addToStoreCB(s, 0);
        }
        else if( !filter ) addToStoreCB(s, 0);
    }
    if( filterCB->count() > 0 ) filterCB->setCurrentIndex(0);
    deb(__FUNCTION__, "Done");
}
void TabEdit::slotFilterHistByTable()
{

    if( filterByTableCB->isChecked() ) loadCmdHist(1);
    else loadCmdHist(0);
}

/*****************************************************
*
*****************************************************/
void TabEdit::setCheckBoxValues(GSeq <CHECKBOX_ACTION *> *pMenuActionSeq)
{
    m_pCbMenuActionSeq = pMenuActionSeq;
}

int TabEdit::isChecked(GString name)
{
    for(int i = 1; i <= m_pCbMenuActionSeq->numberOfElements(); ++i )
    {
        if( m_pCbMenuActionSeq->elementAtPosition(i)->Name == name ) return m_pCbMenuActionSeq->elementAtPosition(i)->Action->isChecked();
    }
    return 0;
}

int TabEdit::isOrderStmt(GString cmd)
{
    deb(__FUNCTION__, "checking "+cmd);
    cmd.upperCase();
    if( cmd.occurrencesOf(" ORDER ") && cmd.occurrencesOf(" BY ") ) return 1;
    return 0;
}
void TabEdit::lostFocus()
{
    if( m_pExtSQL )
    {
        m_pExtSQL->saveGeometry();
        m_pExtSQL->hide();
    }
    if( m_pGetCLP )
    {
        m_pGetCLP->saveGeometry();
        m_pGetCLP->hide();
    }
    if( m_pShowXML )
    {
        m_pShowXML->saveWdgtGeometry();
        m_pShowXML->hide();
    }
}


void TabEdit::gotFocus()
{
    if( isChecked(_REFRESHONFOCUS) && !m_pShowXML && !m_pExtSQL)
    {                
        if( isSelectStatement(cmdLineLE->toPlainText()) || GString(cmdLineLE->toPlainText()).strip().length() == 0 )
        {
            //Do not reload while tabs are moved around:
            if( m_qTabWDGT->currentIndex() == m_iCurrentIndex ) okClick();
            else m_pPMF->refreshTabOrder();
        }
        //reload(sqlFromLastSql());
        //if( sqlFromLastSql().length() ) reload(sqlFromLastSql());
    }

    if( m_pExtSQL )
    {
        m_pExtSQL->show();
        m_pExtSQL->restoreGeometry();
    }
    if( m_pGetCLP )
    {
        m_pGetCLP->show();
        m_pGetCLP->restoreGeometry();
    }
    if( m_pShowXML )
    {
        m_pShowXML->show();
        m_pShowXML->restoreWdgtGeometry();
    }
    mainWdgt->setFocus();
}

void TabEdit::extSQLClosed()
{    
    if( !m_pExtSQL ) return;
    delete m_pExtSQL;
    m_pExtSQL = NULL;
    mainWdgt->setFocus();
}
void TabEdit::getClpClosed()
{
    if( !m_pGetCLP ) return;
    delete m_pGetCLP;
    m_pGetCLP = NULL;
}
void TabEdit::getShowXMLClosed()
{
    if( !m_pShowXML ) return;
    delete m_pShowXML;
    m_pShowXML = NULL;
}
/**************
void tabEdit::setCharForBitMode(int mode)
{

    for( int i = 0; i < mainWdgt->rowCount(); ++i)
    {
        actItem = mainWdgt->item(i, 1); //2nd column contains the action
        if( NULL == actItem ) continue;
        if( _UPDMark  == GString(actItem->text()) ) s = updateRow(actItem, &aDSQL);
        else if( _INSMark  == GString(actItem->text()) ) s = insertRow(actItem, &aDSQL );

pItem->setData(Qt::DisplayRole, (char*)s);
}
***********/


/*********************************************
* SQL statement to fetch data for this item from the database
* returns SELECT including the constraint
*********************************************/
GString TabEdit::createSelectForThisItem(QTableWidgetItem* pItem, GSeq <GString> *unqColSeq, GString castType)
{
    //int colPos = mainWdgt->currentItem()->column()-1;
    int colPos = pItem->column()-1;
    GString cmd;
    if( castType.length() ) cmd = "SELECT "+castType+"("+wrap(m_pMainDSQL->hostVariable(colPos))+") FROM "+currentTable();
    else cmd = "SELECT "+wrap(m_pMainDSQL->hostVariable(colPos))+" FROM "+currentTable();

    GString constraint;
    if( unqColSeq == NULL ) constraint = createUniqueColConstraint(pItem);
    else constraint = createUniqueColConstraintForItem(pItem, unqColSeq);
    if( !constraint.length() ) constraint = createConstraintForThisItem(pItem).change("\"", "");

    cmd += constraint;
    return cmd;
}
/*********************************************
* Partial SQL statement to fetch data for this item from the database
* returns the constraint including " WHERE ..."
*********************************************/
GString TabEdit::createConstraintForThisItem(QTableWidgetItem* pItem)
{
    deb(__FUNCTION__, "start");
    int row = pItem->row();
    GString val, col;
    GString cmd = " WHERE ";

    for( unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        col = wrap(m_pMainDSQL->hostVariable(i)); //HostVariable, i.e. columnName
        //Remember: There are 2 invisible columns!
        val = mainWdgt->item(row, i+1)->text();
        if( m_pMainDSQL->isForBitCol(i) >= 3 && val != "NULL" ) val = Helper::formatForHex(m_pMainDSQL, val);
        m_pMainDSQL->convToSQL(val);
        if( !val.strip().length() || val.subString(1, 6) == "@DSQL@"  ||
                m_pMainDSQL->isTruncated(row+1, i) || m_pMainDSQL->isXMLCol(i)
                || m_pMainDSQL->isLongTypeCol(i) ||  m_pMainDSQL->simpleColType(i) == CT_GRAPHIC) continue;
        if( val == "NULL") cmd += col + " IS NULL AND ";
        //else if( m_pMainDSQL->isForBitCol(i) >= 3 ) val = "="+formatForHex(m_pMainDSQL, val);
        else cmd += col + "=" + val +" AND ";
    }
    cmd = cmd.stripTrailing(" AND ");
    deb(__FUNCTION__, "cmd: "+cmd);
    return cmd;
}

GString TabEdit::createFullConstraint(int row, GSeq<GString> *unqCols)
{
    GString cmd = " WHERE ";
    GString val, col;
    for( unsigned int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++ i )
    {
        val = mainWdgt->item(row, i+1)->text();
        m_pMainDSQL->convToSQL(val);
        col = wrap(m_pMainDSQL->hostVariable(i)); //HostVariable, i.e. columnName

        if( !val.strip().length() || val.subString(1, 6) == "@DSQL@"  || m_pMainDSQL->isXMLCol(i) ||
                m_pMainDSQL->isTruncated(row+1, i) ) continue;
        //Assumption: LONG data types cannot be part of UNQ/PRIM keys
        if( unqCols != NULL)
        {
            if( m_pMainDSQL->isLongTypeCol(i) ||  m_pMainDSQL->simpleColType(i) == CT_GRAPHIC) continue;
        }
        if( val == "NULL")cmd += col + " IS NULL AND ";
        else if( m_pMainDSQL->isForBitCol(i) >= 3 ) cmd += col + "="+Helper::formatForHex(m_pMainDSQL, val)+" AND ";
        else cmd += col + "="+ val + " AND ";
    }
    cmd = cmd.stripTrailing(" AND ");
    return cmd;
}


GString TabEdit::createUniqueColConstraint(QTableWidgetItem* pItem)
{
    GSeq <GString> unqColSeq;
    DSQLPlugin aDSQL(*m_pMainDSQL);
    aDSQL.getUniqueCols(currentTable(), &unqColSeq);
    return createUniqueColConstraintForItem(pItem, &unqColSeq);
}


GString TabEdit::createUniqueColConstraintForItem(QTableWidgetItem* pItem, GSeq <GString> * unqCols)
{
    deb(__FUNCTION__, "start");

    if( unqCols->numberOfElements() == 0 ) return "";

    deb(__FUNCTION__, "have UNQ cols.");
    int row = pItem->row(), colNr;
    GString val, col;
    GString cmd = " WHERE ";

    int rowNr = GString( mainWdgt->item(pItem->row(), 0 )->text()).asInt();

    deb(__FUNCTION__, "unqiqueCols: "+GString(unqCols->numberOfElements()));
    //msg("unqiqueCols: "+GString(unqCols->numberOfElements()));
    for( unsigned int i = 1; i <= unqCols->numberOfElements(); ++ i )
    {
        col = wrap(unqCols->elementAtPosition(i));
        colNr = m_pMainDSQL->positionOfHostVar(unqCols->elementAtPosition(i));
        //msg("unqiqueCol: "+col);
        val = m_pMainDSQL->rowElement(rowNr, colNr);
        if( m_pMainDSQL->isForBitCol(colNr) >= 3 && val != "NULL" ) val = Helper::formatForHex(m_pMainDSQL, val);
        m_pMainDSQL->convToSQL(val);
        if( !val.strip().length() || val.subString(1, 6) == "@DSQL@"  ||
                m_pMainDSQL->isTruncated(row+1, colNr) || m_pMainDSQL->isXMLCol(colNr)
                ||  m_pMainDSQL->simpleColType(colNr) == CT_DBCLOB
                || m_pMainDSQL->isLongTypeCol(colNr) ) continue;
        if( val == "NULL") cmd += col + " IS NULL AND ";
        //else if( m_pMainDSQL->isForBitCol(i) >= 3 ) val = "="+formatForHex(m_pMainDSQL, val);
        else cmd += col + "=" + val +" AND ";
    }
    cmd = cmd.stripTrailing(" AND ");
    //msg(cmd);
    deb(__FUNCTION__, "end. cmd: "+cmd);
    return cmd;
}

/****************************************
*
* Write XML or LOB Data to selected file
*
****************************************/
int TabEdit::writeDataToFile(QTableWidgetItem* pItem, GString file, int * outSize)
{
    GSeq <GString> unqColSeq;
    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    pDSQL->getUniqueCols(currentTable(), &unqColSeq);


    *outSize = 0;
    if( !pItem ) return 1;

    if( currentTable().length() < 2 || !mainWdgt->item(pItem->row(), 1) )
    {
        msg("Please open a table first");
        return 2;
    }

    GString txt = mainWdgt->item(pItem->row(), 1)->text();
    if( txt == _NEWMark || txt == _INSMark )
    {
        return 0;
    }

    int erc = 0;
    GString cmd = "SELECT "+wrap(m_pMainDSQL->hostVariable(pItem->column()-1))+" FROM "+currentTable();

    GString constraint = createUniqueColConstraintForItem(pItem,&unqColSeq);
    if( constraint.length() ) cmd += constraint;
    else cmd += createFullConstraint(pItem->row(), &unqColSeq);


    deb(__FUNCTION__, "CMD: "+cmd+", calling descrToFile");
    remove(file);
    GString err = m_pMainDSQL->descriptorToFile(cmd, file, outSize);

    if( err.length() )
    {
        msg("Possibly wrong cell, right-click a cell containing valid LOB/XML data\n\nDetailed error: "+err);
        erc = 1;
    }
    else if( !outSize )
    {
        msg("The LOB has 0 bytes, no file was generated. ");
        erc = 1;
    }

    return erc;
}

int TabEdit::writeDataToFile(GString cmd, GString file, int * outSize)
{
    *outSize = 0;
    GString err = m_pMainDSQL->descriptorToFile(cmd, file, outSize);
    if( err.length() || !outSize)
    {
        return 1;
    }
    return 0;
}


void TabEdit::slotRightClickSaveBlob()
{
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);
    QSettings settings(_CFG_DIR, "pmf5");
    GString lastPath = settings.value("lobSavePath", "").toString();
    QString name = QFileDialog::getSaveFileName(this, "Save", lastPath);
    if( name.isNull() ) return;
    settings.setValue("lobSavePath", name);
    int outSize;
    writeDataToFile(pItem, name, &outSize);
}

void TabEdit::setItemSelected(QTableWidgetItem* pItem)
{
    if( !pItem ) return;
    mainWdgt->clearSelection();
    pItem->setSelected(true);
}

void TabEdit::slotRightClickOpenBlob()
{
    Helper::showHintMessage(this, 1001);
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);
    int outSize;
    if( pItem->text() == "NULL") return;
    GString path = Helper::tempPath();
    GString fileName = path + "PMF5_LOB.TMP";
    int erc = writeDataToFile(pItem, fileName, &outSize);
    if( erc ) return;
    GString ext = getFileExtensionFromMime(fileName);
	
    if( ext == "gz" && outSize < 100000000)
    {
        int rx = Helper::uncompressGZ(fileName);
        if( rx == 0 || rx == 2 )
		{
			ext = getFileExtensionFromMime(fileName);
            if( ext == "txt" && rx == 2)
			{
                ext = "XML";
			}			
		}
    }
    else if(outSize < 100000000 && ext == "txt")
    {
        GString newExt;
        if( Helper::convertBase64toPNG(fileName, &newExt) == 0 ) ext = newExt;
    }
    fileName = setTmpFileExtension(fileName, ext);

#ifndef MAKE_VC
    if( !QDesktopServices::openUrl(QUrl(fileName, QUrl::TolerantMode)) ) msg("There is no default application associated with this LOB");
#else
    /*
        UINT uErr = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
        SetErrorMode(0);

        SHELLEXECUTEINFO ShExecInfo;
        ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        ShExecInfo.hwnd = m_pPMF->getMainWindowHandle();
        ShExecInfo.lpVerb = NULL;
        ShExecInfo.lpFile = fileName;
        ShExecInfo.lpParameters = NULL;
        ShExecInfo.lpDirectory = NULL;
        ShExecInfo.nShow = SW_SHOW;
        ShExecInfo.hInstApp = NULL;
        ShExecInfo.fMask = SEE_MASK_FLAG_NO_UI;

        int rx = ShellExecuteEx(&ShExecInfo);
        msg("rx:"+GString(rx));
        */

    int rc = (int)ShellExecute(0, 0, fileName, 0, 0 , SW_SHOW );

    if( rc < 32 )
    {
        QProcess * qp = new QProcess(this);
        QString prog = "RUNDLL32.EXE SHELL32.DLL,OpenAs_RunDLL "+fileName;
        qp->start(prog);
        //qp->waitForStarted();
    }
#endif
}
void TabEdit::slotRightClickOpenBlobAs()
{
    Helper::showHintMessage(this, 1001);
    QTableWidgetItem* pItem = m_pActionItem;

    if( !pItem ) return;
    setItemSelected(pItem);
    int outSize;
    if( pItem->text() == "NULL") return;
    GString path = Helper::tempPath();
    GString fileName = path + "PMF5_LOB.TMP";
    int erc = writeDataToFile(pItem, fileName, &outSize);
    if( erc ) return;
    GString ext = getFileExtensionFromMime(fileName);
    fileName = setTmpFileExtension(fileName, ext);

#ifndef MAKE_VC
    if( !QDesktopServices::openUrl(QUrl(fileName, QUrl::TolerantMode)) ) msg("There is no default application associated with this LOB");
#else
    QProcess * qp = new QProcess(this);
    QString prog = "RUNDLL32.EXE SHELL32.DLL,OpenAs_RunDLL "+fileName;
    qp->start(prog);
    //ShellExecuteEx(&ShExecInfo);
    //qp->waitForStarted();
#endif
}
void TabEdit::slotRightClickLoadBlob()
{
    QTableWidgetItem * pItem  = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);
    if( colType(pItem->column()-1) == 0 )
    {
        QMessageBox::information(this, "pmf", "Column '"+m_pMainDSQL->hostVariable(pItem->column()-1)+"' is not recognized as LOB or XML column.");
        return;
    }
    GString fileName = QFileDialog::getOpenFileName(this, "Select file to upload", "");
    if( !fileName.length() ) return;
    fileName = GStuff::formatPath(fileName);
    pItem->setText(fileName);
    return;

    /*
    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    GSeq <GString> unqColSeq;
    pDSQL->getUniqueCols(currentTable(), &unqColSeq);
    GString err = updateRow(pItem, pDSQL, &unqColSeq, pItem->column()-1, fileName);
    delete pDSQL;
    if( err.length() ) msg(err);
    reload();*/
}


GString TabEdit::getFileExtensionFromMime(GString inFile)
{
    QString ext = "";
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(inFile);
    ext = type.preferredSuffix();
#endif
    return ext;
}

GString TabEdit::setTmpFileExtension(GString inFile, GString ext)
{    
    if(ext.length())
    {
        GString newName;
        if( inFile.occurrencesOf(".") ) newName = inFile.subString(1, inFile.lastIndexOf("."))+GString(ext);
        else newName = inFile+"."+GString(ext);
        remove(newName);
        rename(inFile, newName );
        return newName;
    }
    return inFile;
}

/*
int tabEdit::loadContextApp(GString targetFile)
{
        HINSTANCE hInstance = LoadLibrary ( ("shell32.dll"));
        if (hInstance == NULL)
        return FALSE;

        RCTMETHODCALLTYPE * fnOpenAs = (FUNC_OPENAS) GetProcAddress (hInstance, FUNC_OPEN_AS_NAME);
        if (fnOpenAs == NULL) return 1;
        HWND hWnd = (HWND)this->winId();
        fnOpenAs (hWnd, hInstance, targetFile, 0);
        FreeLibrary (hInstance);
        return 0;
}
*/
void TabEdit::slotRightClickActionEditLong()
{
    loadDoubleByteEditor();
}

void TabEdit::slotRightClickActionEditText()
{
    //loadDoubleByteEditor(CT_PMF_RAW);
    loadDoubleByteEditor(-1);
}

void TabEdit::slotRightClickActionEditGraphic()
{
    loadDoubleByteEditor();
}

void TabEdit::slotRightClickActionEditDBClob()
{
    loadDoubleByteEditor();
}

void TabEdit::loadDoubleByteEditor(int type)
{
    deb(__FUNCTION__, "start");
    QTableWidgetItem* pItem = m_pActionItem;
    if( !pItem ) return;
    setItemSelected(pItem);

    int colPos = pItem->column()-1;
    if( type < 0 ) type = m_pMainDSQL->simpleColType(colPos);
    deb(__FUNCTION__, "creating EditDoubleByte...");
    EditDoubleByte* edit = new EditDoubleByte(m_pGDeb, this, pItem, m_pMainDSQL, type);
    if( edit->loadData() ) return;
    deb(__FUNCTION__, "pre modal...");
    edit->setWindowModality(Qt::ApplicationModal);
    edit->exec();
    deb(__FUNCTION__, "modal done");
    if( !edit->runRefresh() ) return;
    if( cmdLineLE->toPlainText().length() ) reload(cmdLineLE->toPlainText());
    else reload("SELECT * FROM "+currentTable());
    deb(__FUNCTION__, "end");
}

void TabEdit::slotRightClickActionShowXML()
{

    //Better way: Find the column containing the XML (via dsqlobj::isXMLCol) and use this column. What if there are multiple XMLs?
    QTableWidgetItem* pItem = currentItem();
    printf("FromCurItem: %i\n", (long) pItem);
    if( !pItem ) return;
    setItemSelected(pItem);

    int erc;
    if( m_pShowXML ) //Already open
    {
        //QEvent event(QEvent::WindowActivate);
        //QApplication::sendEvent(m_pShowXML, &event);
        //msg("The XML editor is already open.");
        return;
    }
    if( colType(pItem->column()-1) != 2 )
    {
        msg("Possibly wrong cell, right-click a cell containing valid LOB/XML data");
        return;
    }

    GString path = Helper::tempPath();
    GString file = path + "XML.TMP";
    int outSize;
    if( pItem->text() != "NULL")
    {
        if( cmdLineLE->toPlainText().length() ) erc = writeDataToFile(cmdLineLE->toPlainText(), file, &outSize);
        if( erc || !cmdLineLE->toPlainText().length() ) erc = writeDataToFile(pItem, file, &outSize);
        if( erc ) return;
    }
    else file = "";

    m_pShowXML = new ShowXML(m_pGDeb, this, pItem, file, m_strLastXmlSearchString);
    m_pShowXML->installEventFilter(this);
    m_pShowXML->show();
    //m_pShowXML->exec();



    // showXML used to be modal, not anymore. Therefore, code below stopped working
    // showXML now calls   tabEdit::updateXML

    //    if( !m_pShowXML->needsReload() ) return;
    //    DSQLPlugin * pDSQL = new DSQLPlugin(m_pMainDSQL->getDBTypeName());


    //    /*******************************************************
    //     * Code below works in UTF8 environs only. Normally I would just set
    //     * the path to the updated XML and let DB2 handle the rest, as with a LOB.
    //     * But apparently DB2 tries to read the file in UTF8, which is not the way files
    //     * are handled on WIN. So we force-feed the XML in a regular UPDATE SQL.
    //     * The statement would look like this:
    //     * update <table> set <col>= XMLPARSE (DOCUMENT('<...content of XML file...>')) where <constraint>
    //     * See ::updateRow(...)
    //     ******************************************************/
    //    //Set item's text to temp path where we stored the updated XML.
    //    //pItem->setText(file);
    //    //GString err = updateRow(pItem, &aDSQL);
    //    /*** Code above works in UTF8 environs only.  ***/

    //    //for tables with more than one XML column we need to provide the col
    //    GString err = updateRow(pItem, pDSQL, m_pShowXML->newXML(), pItem->column()-1);

    //    if( err.length() ) msg(err);
    //    okClick();
}
GString TabEdit::updateLargeXML(QTableWidgetItem *pItem, GString pathToXML)
{    
    deb(__FUNCTION__, "start");
    if( !pItem )
    {
        return "Invalid item. Refresh/Open the main view. ";
    }
    //pItem->setText(pathToXML);
    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    GSeq <GString> unqColSeq;
    pDSQL->getUniqueCols(currentTable(), &unqColSeq);
    GString err = updateRow(pItem, pDSQL, &unqColSeq, pItem->column()-1, pathToXML);
    delete pDSQL;
    deb(__FUNCTION__, "err: "+err);

    if(err.length()) return err;
    GString path = Helper::tempPath();
    GString file = path + "XML.TMP";
    int outSize;
    writeDataToFile(pItem, file, &outSize);
    deb(__FUNCTION__, "end");
    return err;
}

//GString TabEdit::updateXML(QTableWidgetItem * pItem, GString newXML)
//{
//    deb(__FUNCTION__, "start");
//    if( !pItem )
//    {
//        return "Invalid item. Refresh/Open the main view. ";
//    }

//    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
//    GSeq <GString> unqColSeq;
//    pDSQL->getUniqueCols(currentTable(), &unqColSeq);

//    newXML = newXML.change("'", "''");
//    //DSQLPlugin * pDSQL = new DSQLPlugin(*m_pPMF->getConnection());
//    deb(__FUNCTION__, " calling ::updateRow...");
//    GString err = updateRow(pItem, pDSQL, &unqColSeq, newXML, pItem->column());
//    deb(__FUNCTION__, " calling ::updateRow...done.");
//    delete pDSQL;
//    if(err.length()) return err;
//    GString path = Helper::tempPath();
//    GString file = path + "XML.TMP";
//    int outSize;
//    writeDataToFile(pItem, file, &outSize);
//    deb(__FUNCTION__, "end");
//    return err;
//}
/*
void tabEdit::PMF_QTableWidget::keyPressEvent(QKeyEvent *event)
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
        if( myTabEdit->m_iEnterToSave ) myTabEdit->chgClick();
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

void tabEdit::PMF_QTableWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
void tabEdit::PMF_QTableWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}
void tabEdit::PMF_QTableWidget::dropEvent(QDropEvent *event)
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
*/

int TabEdit::colType(GString colName)
{
    for(int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++i )
    {
        if( colName == m_pMainDSQL->hostVariable(i) ) return colType(i);
    }
    return 0;
}

int TabEdit::colType(int i)
{
    if( m_pMainDSQL->isLOBCol(i)) return 1;
    else if( m_pMainDSQL->isXMLCol(i)) return 2;
    else if( m_pMainDSQL->isLongTypeCol(i)) return 3;
    else if( m_pMainDSQL->simpleColType(i) == CT_GRAPHIC ) return 4;
    else if( m_pMainDSQL->simpleColType(i) == CT_DBCLOB ) return 5;
    else if( m_pMainDSQL->simpleColType(i) == CT_CLOB ) return 6;
    return 0;
}

int TabEdit::cellDataChanged(QTableWidgetItem * pItem)
{
    deb(__FUNCTION__, "start");
    if( !pItem ) return 0;
    deb(__FUNCTION__, "pItem->row: "+GString(pItem->row())+", col: "+GString(pItem->column()));
    //Row was inserted   
    if( mainWdgt->item(pItem->row(), 0)->text() == "I" ) return 1;

    //Sorting (clicking the header) will change the view: pItem->row will differ from the internal row of m_pMainDSQL
    //We need to compare the value at the current row with the internal value.
    int orgRow = mainWdgt->item(pItem->row(), 0)->text().toInt();
    //If the cell holds binary stuff (e.g. LONG type columns), we need to compare
    //the item's current value with the equally converted original data
    GString newVal = mainWdgt->item(pItem->row(), pItem->column())->text();
    GString oldVal = QString::fromLocal8Bit(m_pMainDSQL->rowElement(orgRow, pItem->column() - 1));
    int row = pItem->row();
    int col = pItem->column();
    deb("Row: "+GString(row)+", col: "+GString(col-1));
    deb("In DSQL: "+m_pMainDSQL->rowElement(orgRow, pItem->column() - 1));
    deb(__FUNCTION__, "org: "+oldVal+"\nnew: "+newVal);

    int res = 0;
    if( colType(pItem->column()-1) == 2 ) //XML column, compare first 20 characters
    {
        deb(__FUNCTION__, "checking XML...");
        if( newVal.subString(1,20) != oldVal.subString(1,20)) res = 1;
    }
    else if( newVal.strip() == "" && oldVal == "NULL" ) res = 0;
    else if( newVal != oldVal ) res = 2;
    deb(__FUNCTION__, "result: "+GString(res));
    return res;
}
GString TabEdit::orgCellData(QTableWidgetItem * pItem)
{
    if( !pItem ) return "<NULL Item>";
    int orgRow = mainWdgt->item(pItem->row(), 0)->text().toInt();
    return m_pMainDSQL->rowElement(orgRow, pItem->column() - 1);
}
int TabEdit::hasXMLCols()
{
    for(int i = 1; i <= (int) m_pMainDSQL->numberOfColumns(); ++i)
    {
        deb(__FUNCTION__, "value at "+GString(i)+": "+GString(m_pMainDSQL->isXMLCol(i)));
        if( m_pMainDSQL->isXMLCol(i)) return 1;
    }
    return 0;

}
void TabEdit::setWhatsThisTexts()
{
    msgLE->setWhatsThis("Number of rows displayed, and the cost for SELECT in TIMERONS. Timerons is an internal value used to measure the performance of a SELECT");
    //msgLE->setToolTip("Number of rows displayed, and the cost for SELECT in TIMERONS. Timerons is an internal value used to measure the performance of a SELECT");
}
int TabEdit::hasLOBCols()
{
    for(int i = 1; i <= (int)m_pMainDSQL->numberOfColumns(); ++i)
    {
        if( m_pMainDSQL->isLOBCol(i) ) return 1;
    }
    return 0;

}
void TabEdit::waitForThread()
{    
    if( !m_iUseThread) return;
    while( _threadRunning ) GStuff::dormez(100);

}

void TabEdit::setHistTableName(GString table)
{
    m_strHistTableName = table;
}

int TabEdit::isGeneratedColumn(int i)
{
    if( i < 1 || i > (int) m_seqGenerated.numberOfElements() ) return 0;
    return m_seqGenerated.elementAtPosition(i);
}

void TabEdit::fillGeneratedColSeq(GString table)
{
    m_seqGenerated.removeAll();
    if( !table.length() || table.occurrencesOf('.') == 0 ) return;
    GString schema = table.subString(1, table.indexOf('.')-1);
    GString name   = table.subString(table.indexOf('.')+1, table.length()).strip();
    GString cmd = "select  identity, generated from syscat.columns where tabschema = '"+schema+"' and tabname ='"+name+"' order by colno";

    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);

    pDSQL->initAll(cmd);
    for(int i = 1; i <= (int)pDSQL->numberOfRows();++i )
    {
        if( pDSQL->rowElement(i,1) == "'Y'" && pDSQL->rowElement(i,2) == "'A'" ) m_seqGenerated.add(1);
        else m_seqGenerated.add(0);
    }
    delete pDSQL;
}

void TabEdit::slotRightClickActionCopy()
{

    GString allCmds;
    QItemSelectionModel* selectionModel = mainWdgt->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

    if( selected.count() == 0 )
    {
        msg("No rows selected.");
        return;
    }
    QTableWidgetItem* pItem;


    GString rowToPaste;
    //if( hasLOBCols() || hasXMLCols() ) msg("Please note: XMLs and LOBs will be not copied, they will be set to NULL");
    for( int i = 0; i < selected.count(); ++i )
    {
        QModelIndex index = selected.at(i);
        pItem = mainWdgt->item(index.row(), index.column());
        //printf("ADDING: %s\n", (char*) GString(mainWdgt->verticalHeaderItem(index.row())->text()));
        if( GString(mainWdgt->verticalHeaderItem(index.row())->text()) == _NEWRow ) continue;
        //allCmds += createInsertCmd(pItem, _PMF_PASTE_SEP)+_PMF_CMD;
        rowToPaste = createDataToPaste(pItem, _PMF_PASTE_SEP)+_PMF_CMD;
        deb("RowToPaste: "+rowToPaste);
        if( !rowToPaste.length() || rowToPaste == _PMF_CMD) return;
        else allCmds += rowToPaste;
    }

    QApplication::clipboard()->setText(allCmds);
    m_qaPaste->setEnabled(true);

}

GString TabEdit::createInsertCmd(QTableWidgetItem *pItem, GString separator)
{
    if( !pItem ) return "";
    int row = pItem->row();


    GString values = " VALUES(";
    GString hostVars = " (";
    GString val;


    for( int i = 1; i <= (int) m_pMainDSQL->numberOfColumns(); ++ i)
    {
        //Do not copy LOBs and XMLs
        if( m_pMainDSQL->isLOBCol(i)  || m_pMainDSQL->isXMLCol(i) ) continue;

        hostVars += m_pMainDSQL->hostVariable(i) +separator;

        val = mainWdgt->item(row, i+1)->text();
        if( m_pMainDSQL->isForBitCol(i) >= 3 && val != "NULL" ) val = Helper::formatForHex(m_pMainDSQL, val);
        m_pMainDSQL->convToSQL(val);
        values += val + separator;

    }
    values = values.stripTrailing(separator) +")";
    hostVars = hostVars.stripTrailing(separator) +")";
    return hostVars+values;
}

GString TabEdit::createDataToPaste(QTableWidgetItem *pItem, GString separator)
{
    if( !pItem ) return "";
    int row = pItem->row();

    GString values, val, err;

    GSeq <GString> unqColSeq;
    DSQLPlugin aDSQL(*m_pMainDSQL);
    aDSQL.getUniqueCols(currentTable(), &unqColSeq);

    for( int i = 1; i <= (int) m_pMainDSQL->numberOfColumns(); ++ i)
    {
        //Do not copy LOBs and XMLs
        if( (m_pMainDSQL->isLOBCol(i)  || m_pMainDSQL->isXMLCol(i)) && mainWdgt->item(row, i+1)->text() != "NULL" )
        {
            err = writeLobToTemp(mainWdgt->item(row, i+1), &val, &unqColSeq);
            if( !err.length() ) val = _PMF_IS_LOB+val;
            else
            {
                msg("Cannot create temporary file (disk full?)\n"+err+"\n\nYou should probably reopen the table (hit F5)");
                return "";
            }
        }
        else if(isIdentityColumn(m_pMainDSQL->hostVariable(i)) ) val = (char*)_IDENTITY;
        else val = mainWdgt->item(row, i+1)->text();
        if( m_pMainDSQL->isForBitCol(i) >= 3 && val != "NULL" ) val = Helper::formatForHex(m_pMainDSQL, val);
        m_pMainDSQL->convToSQL(val);
        values += val + separator;

    }
    values = values.stripTrailing(separator);
    return values;
}

GString TabEdit::writeLobToTemp(QTableWidgetItem * pItem, GString * fileName, GSeq <GString> *unqColSeq)
{
    GString cmd = createSelectForThisItem(pItem, unqColSeq);


    int outSize;

    GString path = Helper::tempPath()+_TMP_DIR+"/";
    QDir().mkpath(path);
    *fileName = path+GString(QUuid::createUuid().toString());

    return m_pMainDSQL->descriptorToFile(cmd, *fileName, &outSize);
}

GString TabEdit::pasteByGuessing(GString line)
{

    /*
    line = line.strip();
    if( line[line.length()] == ',') line += "NULL";
    if( line[1UL] == ',' ) line = "NULL"+line;

    line = line.change("\"", "'").change(",,", ",NULL,");

    GString cmd = "INSERT INTO "+currentTable()+" (";
    for(int i = 1; i <= m_pMainDSQL->numberOfColumns(); ++i )
    {
        cmd += m_pMainDSQL->hostVariable(i)+",";
    }
    cmd = cmd.stripTrailing(",")+") VALUES (";
    cmd += line+")";
    GString err = pDSQL->initAll(cmd);
    if( err.length() )err += "\nCmd was: "+cmd;

    return err;
*/
    GSeq <GString> dataSeq;
    deb(__FUNCTION__, "line: "+line);
    if( !line.strip().length() ) return "";
    GStuff::splitStringDelFormat(line, &dataSeq);

    QTableWidgetItem *newItem;
    GString temp;
    int rows = mainWdgt->rowCount();
    mainWdgt->insertRow( rows );
    mainWdgt->setItem(rows, 0, new QTableWidgetItem("-"));
    mainWdgt->setItem(rows, 1, new QTableWidgetItem("I"));
    for( int i = 2; i < mainWdgt->columnCount(); ++ i )
    {
        if( i-1 <= (int)dataSeq.numberOfElements() ) temp = dataSeq.elementAtPosition(i-1).change("\"", "\'");
        else temp = "";
        if(m_pMainDSQL->isDateTime(i-1) ) temp = formatAsDateTime(temp);
        newItem = new QTableWidgetItem(QString::fromLocal8Bit(temp));
        mainWdgt->setItem(rows, i, newItem);
    }
    //To set the verticalHeader's text, we need to (re)set rownumbers
    //setVHeader();
    //ToDo: SetFocus, SetEdit, ColorNewRow?
    //mainWdgt->verticalHeaderItem(rows)->setText(_INSRow);
    return "";

}

void TabEdit::createPastedRow(GString line)
{
    deb(__FUNCTION__, "start");
    //No rows, because no table was opened:
    if( myTabText() == _newTabString || myTabText() == GString(_SQLCMD) )
    {
        msg(GString("Please select a table first."));
        return;
    }
    //create empty row and scroll down:
    QTableWidgetItem *newItem;
    GString temp;
    int rows = mainWdgt->rowCount();
    mainWdgt->insertRow( rows );
    mainWdgt->setItem(rows, 0, new QTableWidgetItem("-"));
    mainWdgt->setItem(rows, 1, new QTableWidgetItem("I"));

    for( int i = 2; i < mainWdgt->columnCount(); ++ i )
    {
        if( line.occurrencesOf(_PMF_PASTE_SEP) )
        {
            temp = line.subString(1, line.indexOf(_PMF_PASTE_SEP)-1);
            line = line.remove(1, line.indexOf(_PMF_PASTE_SEP)+_PMF_PASTE_SEP.length()-1);
        }
        else temp = line;
        if(m_pMainDSQL->isDateTime(i-1) ) temp = formatAsDateTime(temp);
        newItem = new QTableWidgetItem(QString::fromLocal8Bit(temp));
        //newItem = new QTableWidgetItem((char*)temp);
        mainWdgt->setItem(rows, i, newItem);
    }
    //To set the verticalHeader's text, we need to (re)set rownumbers
    //setVHeader();
    //ToDo: SetFocus, SetEdit, ColorNewRow?
    //mainWdgt->verticalHeaderItem(rows)->setText(_INSRow);

    return;
}

GSeq <GString> TabEdit::clipBoardToSeq(GString data)
{
    GSeq <GString> lineSeq;
    GString tmp;

    int count = data.occurrencesOf('\n');
    if( !count && data.strip().length()) count = 1;
    if( !count )
    {
        return lineSeq;
    }
    if( count == 1 )
    {
        lineSeq.add(data);
    }
    else
    {
        for(int i = 1; i <= count; ++i)
        {
            tmp = data.subString(1, data.indexOf('\n')-1).strip();
            if( tmp.length() )lineSeq.add(tmp);
            data = data.remove(1, data.indexOf('\n'));
        }
        if( data.length() )lineSeq.add(data);
    }    
    return lineSeq;
}

void TabEdit::slotRightClickActionPaste()
{
    int count = 0;

    GString allCmds = GString(QApplication::clipboard()->text());

    //Clipboard data was copied from a DEL File
    if( allCmds.occurrencesOf(_PMF_CMD) == 0 )
    {
        GSeq <GString> lineSeq = clipBoardToSeq(allCmds);
        count = lineSeq.numberOfElements();
        if( count == 0 )
        {
            msg("Clipboard is empty, no rows to paste");
            return;
        }
        if( QMessageBox::question(this, "PMF", "Pasting "+GString(count)+" row(s), continue?", "Yes", "No", 0, 1) == 1 ) return;
        for(int i = 1; i <= count; ++i)
        {
            lineSeq.elementAtPosition(i);
            pasteByGuessing(lineSeq.elementAtPosition(i));
        }
        //To set the verticalHeader's text, we need to (re)set rownumbers
        setVHeader();
        //ToDo: SetFocus, SetEdit, ColorNewRow?
        //mainWdgt->verticalHeaderItem(rows)->setText(_INSRow);
    }
    else
    {
        //Clipboard was copied from mainWdgt:
        while( allCmds.occurrencesOf(_PMF_CMD) )
        {
            createPastedRow(allCmds.subString(1, allCmds.indexOf(_PMF_CMD)-1));
            allCmds = allCmds.remove(1, allCmds.indexOf(_PMF_CMD)+_PMF_CMD.length()-1);
            count++;
        }
        setVHeader();
    }


    QItemSelectionModel *selectionModel = mainWdgt->selectionModel();

    msg(GString(count)+" row(s) pasted. Click 'Save' to commit.");

    mainWdgt->scrollToBottom();
    mainWdgt->selectRow(mainWdgt->rowCount()-1); //Last row
    QItemSelection itemSelection = selectionModel->selection();
    selectionModel->select(itemSelection,QItemSelectionModel::Deselect);


    for(int i = 2; i <= count; ++i)
    {
        mainWdgt->selectRow(mainWdgt->rowCount()-i);
        itemSelection.merge(selectionModel->selection(), QItemSelectionModel::Select);
    }
    selectionModel->clearSelection();
    selectionModel->select(itemSelection,QItemSelectionModel::Select);

}

int TabEdit::hasTwin(QTableWidgetItem * pItem)
{

    //this rows ID is hidden in the first column of mainWdgt:
    unsigned long row = GString(mainWdgt->item(pItem->row(), 0)->text()).asLong();


    GSeq <GString> dataSeq;
    unsigned long i, k;

    //Read select row's data
    for( i = 1; i <= m_pMainDSQL->numberOfColumns(); ++i )
    {
        dataSeq.add(m_pMainDSQL->rowElement(row, i));
    }


    //use a cursor to scroll through the result set:
    int rc = m_pMainDSQL->initRowCrs();
    if( rc ) return 0;

    int found = 0;
    for(k = 1; k <= m_pMainDSQL->numberOfRows(); ++k )
    {
        if( k == row )
        {
            rc = m_pMainDSQL->nextRowCrs();
            if( rc ) break;
            continue;
        }
        found = 1;
        for( i = 1; i <= m_pMainDSQL->numberOfColumns(); ++i )
        {
            if( dataSeq.elementAtPosition(i) != m_pMainDSQL->dataAtCrs(i) )
            {
                found = 0;
                break;
            }
        }
        if( found ) return 1; //Identical data found.
        rc = m_pMainDSQL->nextRowCrs();
    }
    return found;
}
/* Some people name columns ORDER or similar, which causes havoc in dynamic SQL ("SELECT ORDER FROM table ORDER BY ORDER")
 * We'll wrap this to get "SELECT "ORDER" FROM table ORDER BY "ORDER"
 */
GString TabEdit::wrap(GString in)
{
    if( m_pMainDSQL->getDBType() == MARIADB ) return in;
    return GStuff::wrap(in);
}

void TabEdit::setTableNameFromStmt(GString in)
{
    GString tabName = GStuff::TableNameFromStmt(in);

    GString temp = in;
    temp = temp.strip().upperCase();
    if( temp.subString(1, 6) != "SELECT") return;
    in = in.subString(temp.indexOf("FROM")+4, in.length()).strip();
    //A JOIN statmenent: "SELECT * FROM tab1, tab2, ... WHERE...
    if(in.occurrencesOf(",")) return;

    if( currentTable().strip().upperCase() == tabName.upperCase() ) return;
    setCurrentTable(tabName);
    return;


    temp = GString(in).upperCase();
    //substring from tableName to next blank (in front of WHERE)
    //stmt can look like "Select .. from TABLE x WHERE ..."
    //or "Select .. from TABLE ... group by ... (no WHERE clause)"
    if( in.occurrencesOf(" "))in = in.subString(1, temp.indexOf(" ")-1); //First blank
    else if (in.length() == 0 ) return;

    //A JOIN statmenent: "SELECT * FROM tab1, tab2, ... WHERE...
    if(in.occurrencesOf(",")) return;

    if( currentTable().strip().upperCase() == GString(in).strip().upperCase() ) return;

    //if( QMessageBox::question(this, "PMF", "Current table changed from\n"+currentTable()+"\nto "+in+"\nSet new table?", "Yes", "No", 0, 1) == 0 )
    {
        setCurrentTable(in);
    }
}
int TabEdit::tabID()
{
    return m_iTabID;
}


void TabEdit::setMyTabText(GString txt)
{
    TabEdit * pTE;
    for( int i = 0; i < m_qTabWDGT->count(); ++i )
    {
        pTE = (TabEdit*) m_qTabWDGT->widget(i);
        if( pTE )
        {
            if( pTE->tabID() == m_iTabID )
            {
                m_qTabWDGT->setTabText(i, txt);
                return;
            }
        }
    }
}

GString TabEdit::myTabText()
{
    TabEdit * pTE;
    for( int i = 0; i < m_qTabWDGT->count(); ++i )
    {
        pTE = (TabEdit*) m_qTabWDGT->widget(i);
        if( pTE )
        {
            if( pTE->tabID() == m_iTabID )
            {
                return m_qTabWDGT->tabText(i);
            }
        }
    }
    return "";
}

GString TabEdit::formatAsDateTime(GString text)
{
    if( m_pMainDSQL->getDBTypeName() == _DB2 || m_pMainDSQL->getDBTypeName() == _DB2ODBC )
    {
        if( text.length() == 8 && text.occurrencesOf('-') == 0 ) return text.insert("-", 7).insert("-", 5);
    }
    return text;
}

GString TabEdit::formatText(GString text)
{
    if( !text.length() ) return "NULL";
    if( text.subString(1, 6) == "@DSQL@" ) return text;
    if( Helper::isSystemString(text)) return text;
    if( text[1UL] == '\'' && text[text.length()] == '\'' )  return text;
    return "'"+text+"'";
}

void TabEdit::setLastXmlSearchString(GString txt)
{
    m_strLastXmlSearchString = txt;
}

void TabEdit::setGDebug(GDebug * gd)
{
    if( gd == NULL ) return;
    m_pMainDSQL->setGDebug(gd);
}

void TabEdit::setCellFont(QFont *font)
{

//    int i, j;
//    for( i = 0; i < mainWdgt->rowCount(); ++i )
//    {
//        for( j = 2; j < mainWdgt->columnCount(); ++j )
//        {
//            mainWdgt->item(i,j)->setFont(*font);
//        }
//    }
//    mainWdgt->setFont(*font);
}

int TabEdit::tableHasUniqueCols(GString tableName)
{    
    //Todo: this gets called on every update, we could try to work around that.
    deb(__FUNCTION__, "start");
    //Needs to be a local copy of mainDSQL, we don't want to change mainDSQL.
    DSQLPlugin *pDSQL = new DSQLPlugin(*m_pMainDSQL);
    int res = pDSQL->hasUniqueConstraint(tableName);
    delete pDSQL;
    deb(__FUNCTION__, "end");
    return res;
}

int TabEdit::colorScheme()
{
    return m_iUseColorScheme;
}

//After tabs were moved we need to reset our index
void TabEdit::setNewIndex(int index)
{
    m_iCurrentIndex = index;
}

GString TabEdit::lastSqlSelectCmd()
{
    return m_pMainDSQL->lastSqlSelectCommand();
}

void TabEdit::exportData()
{
    QSettings settings(_CFG_DIR, "pmf5");
    m_gstrPrevExportPath = settings.value("exportPath", "").toString();

    if( !m_gstrPrevExportPath.length() ) m_gstrPrevExportPath = m_gstrPrevImportPath;

        ExportBox * foo = new ExportBox(m_pMainDSQL, this, &m_gstrPrevExportPath);
//    if( !pTE->isSelectStatement(pTE->getSQL()) )
//    {
//        msg("The current statement in the field 'SQL cmd' below ist not a SELECT statement. ");
//        return;
//    }
    foo->setSelect(m_pMainDSQL->lastSqlSelectCommand(), this->currentTable() );
    foo->exec();
}
void TabEdit::importData()
{
    QSettings settings(_CFG_DIR, "pmf5");
    m_gstrPrevImportPath = settings.value("importPath", "").toString();

    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pMainDSQL);
    if( !m_gstrPrevImportPath.length() ) m_gstrPrevImportPath = m_gstrPrevExportPath;
    ImportBox * foo = new ImportBox(pDSQL, this, currentSchema(), currentTable(), &m_gstrPrevImportPath, isChecked(_HIDESYSTABS) );
    foo->setGDebug(m_pGDeb);
    //foo->setFilesToImport(pPmfDropZone->text());
    GString ret = foo->setFilesToImport(pPmfDropZone->fileList());
    if( ret != "OK" )
    {
        if( ret.length()) msg(ret);
        pPmfDropZone->clearFileList();
        return;
    }

    //Note to self:
    //foo->exec() will block the drag source until foo is closed.
    foo->setWindowModality(Qt::WindowModal);
    foo->show();
    delete pDSQL;

    pPmfDropZone->clearFileList();
}

void TabEdit::loadSqlEditor()
{
    if( m_pExtSQL ) return;
    m_pExtSQL = new ExtSQL(m_pGDeb, m_pMainDSQL, this);

    if( m_gstrExtSqlCmd.length() ) m_pExtSQL->setCmd(m_gstrExtSqlCmd);

    m_pExtSQL->installEventFilter(this);
    m_pExtSQL->show();
    m_gstrExtSqlCmd = m_pExtSQL->orgCmd();

}

void TabEdit::loadSqlEditor(QDropEvent *event)
{
    if( m_pExtSQL ) return;
    m_pExtSQL = new ExtSQL(m_pGDeb, m_pMainDSQL, this);

    m_pExtSQL->installEventFilter(this);
    m_pExtSQL->callFileDropEvent(event);
    m_pExtSQL->show();
    m_gstrExtSqlCmd = m_pExtSQL->orgCmd();
}

void TabEdit::itemDoubleClicked(int row, int column)
{
}

int TabEdit::isNewRow(QTableWidgetItem *pItem)
{
    GString txt = mainWdgt->item(pItem->row(), 1)->text();
    if( txt == _NEWMark || txt == _INSMark ) return 1;
    return 0;
}

GDebug* TabEdit::getGDeb()
{
    return m_pGDeb;
}


