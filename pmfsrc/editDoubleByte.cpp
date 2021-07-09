

#include <qlayout.h>
#include <QGridLayout>
#include <gstuff.hpp>
#include <gfile.hpp>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSettings>
#include <QMessageBox>
#include <QGroupBox>
#include "tabEdit.h"
#include "helper.h"

#include "editDoubleByte.h"
#include "pmfdefines.h"

EditDoubleByte::EditDoubleByte(GDebug *pGDeb,TabEdit *parent, QTableWidgetItem * pItem, DSQLPlugin * pIDSQL, int type)
{
    m_pItem = pItem;
    m_Master = parent;
    m_pGDeb = pGDeb;
    if( pIDSQL->getDBType() == MARIADB ) m_strColName =  pIDSQL->hostVariable(pItem->column()-1);
    else m_strColName =  GStuff::wrap(pIDSQL->hostVariable(pItem->column()-1));

    m_pIDSQL = new DSQLPlugin(*pIDSQL);
    m_iType = type;


    this->resize(560, 400);
    this->setWindowTitle("Edit byte and double-byte data");

    QGridLayout * grid = new QGridLayout(this);

    exitB = new QPushButton(this);
    exitB->setText("Exit");
    connect(exitB, SIGNAL(clicked()), SLOT(exitClicked()));


    saveToFile = new QPushButton(this);
    saveToFile->setText("Write to file");
    connect(saveToFile, SIGNAL(clicked()), SLOT(saveToFileClicked()));

    updateToDB = new QPushButton(this);
    updateToDB->setText("&Save");
    connect(updateToDB, SIGNAL(clicked()), SLOT(updateDB()));

    dataTE = new TxtEdit(m_pGDeb, this);
    dataTE->setGeometry( 20, 20, 520, 300);
    dataTE->setLineWrapMode(QTextEdit::NoWrap);
    dataTE->setAcceptRichText(false);
    connect(dataTE, SIGNAL(textChanged()), this, SLOT(onTextChanged()));

    //  infoLB->setFont(QFont("courier"));
    info = new QLabel(this);
    QFont font  = info->font();
    font.setBold(true);
    info->setFont(font);
    info->setText("Hit CTRL+B to format text");
    grid->addWidget(info, 0, 0, 1, 4);

    if( m_iType == CT_LONG )
    {
        m_cbDisplay = new QCheckBox("Display as HEX");
        m_cbDisplay->setChecked(false);
    }
    else if( m_iType == CT_GRAPHIC ) m_cbDisplay = new QCheckBox("Cast to VARCHAR");
    else
    {
        m_cbDisplay = new QCheckBox("<unused>");
        m_cbDisplay->hide();
    }

    grid->addWidget(m_cbDisplay, 2, 0, 1, 4);
    connect(m_cbDisplay, SIGNAL(stateChanged(int)), SLOT(displayChecked()));

    grid->addWidget(dataTE, 3, 0, 1, 4);

    QGroupBox * btGroupBox = new QGroupBox();
    QHBoxLayout *frmLayout = new QHBoxLayout;
    frmLayout->addWidget(updateToDB);
    frmLayout->addWidget(saveToFile);
    frmLayout->addWidget(exitB);
    btGroupBox->setLayout(frmLayout);
    grid->addWidget(btGroupBox, 5, 0, 1, 4);


    //FIND Box
    QHBoxLayout *findLayout = new QHBoxLayout;
    QGroupBox * findGroupBox = new QGroupBox();
    QLabel * findLabel = new QLabel(this);
    findLabel->setText("Find (next: F3):");
    findLayout->addWidget(findLabel);

    findLE = new QLineEdit(this);
    findLayout->addWidget(findLE);


    m_cbCaseSensitive = new QCheckBox("Case sens.");
    findLayout->addWidget(m_cbCaseSensitive);
    findGroupBox->setLayout(findLayout);

    grid->addWidget(findGroupBox, 6, 0, 1, 4);

    QSettings settings(_CFG_DIR, "pmf5");
    this->restoreGeometry(settings.value("editDoubleByteGeometry").toByteArray());
    m_iRunRefresh = 0;
    m_iLastFindPos = -1;
}

void EditDoubleByte::onTextChanged()
{
    QFont font = updateToDB->font();
    font.setBold(true);
    updateToDB->setFont(font);
    font.setBold(true);
    if( m_qstrOrigTxt == dataTE->document()->toPlainText() )
    {
        font.setBold(false);
    }
    updateToDB->setFont(font);
}

int EditDoubleByte::loadFromItem()
{
    if( m_iType == CT_STRING ) setSrcData(m_pIDSQL->cleanString(m_pItem->text()));
    else setSrcData(m_pItem->text());
    return 0;
}

int EditDoubleByte::loadGraphicData()
{

    GString cmd;
    //Initializing this DSQL: Select the item to update
    if( !m_cbDisplay->isChecked() ) cmd = m_Master->createSelectForThisItem(m_pItem);
    else cmd = m_Master->createSelectForThisItem(m_pItem, NULL, "VARCHAR");
    m_pIDSQL->setTruncationLimit(0);

    {
        m_pIDSQL->getResultAsHEX(0);
        m_pIDSQL->setCharForBit(0); //This is a local instance of DSQLPlugin
    }
    m_pIDSQL->setCLOBReader(1);
    GString err = m_pIDSQL->initAll(cmd);
    if( err.length() )
    {
        Helper::msgBox(this, "Read data", err);
        return 1;
    }

    m_pIDSQL->setCLOBReader(0);
    setSrcData(m_pIDSQL->cleanString(m_pIDSQL->rowElement(1,1)),  m_pIDSQL->isBinary(1,1));
    return 0;
}

int EditDoubleByte::loadFromFile()
{
    int outSize;
    if( m_pItem->text() == "NULL") return 0;
    GString path = Helper::tempPath();
    GString fileName = path + "PMF5_LOB.TMP";
    if( m_Master->writeDataToFile(m_pItem, fileName, &outSize) ) return 1;
    this->setSrcFile(fileName);
    return 0;
}

int EditDoubleByte::loadVarcharData()
{
    //Initializing this DSQL: Select the item to update
    GString cmd = m_Master->createSelectForThisItem(m_pItem);
    m_pIDSQL->setTruncationLimit(0);
    GString err;
    if( m_cbDisplay->isChecked() ) m_pIDSQL->getResultAsHEX(1);
    else
    {
        m_pIDSQL->getResultAsHEX(0);
        //Need to set this for large VARCHAR && CHAR FOR BIT cols.
        //This is a local instance of DSQLPlugin, setting setCharForBit here will not change this setting globally.
        m_pIDSQL->setCharForBit(0); //This is a local instance of DSQLPlugin
    }

    err = m_pIDSQL->initAll(cmd);
    if( err || m_pIDSQL->rowElement(1,1) == "@OutOfReach" ) setSrcData("[PMF: Invalid data. Try refreshing the table.]", 0);
    else setSrcData(m_pIDSQL->cleanString(m_pIDSQL->rowElement(1,1)),  m_pIDSQL->isBinary(1,1));
    return 0;
}

int EditDoubleByte::loadDoubleByteData()
{
    //Initializing this DSQL: Select the item to update
    GString cmd = m_Master->createSelectForThisItem(m_pItem);
    m_pIDSQL->setTruncationLimit(0);
    GString err;
    m_pIDSQL->setCLOBReader(1);
    err = m_pIDSQL->initAll(cmd);

    if( err || m_pIDSQL->rowElement(1,1) == "@OutOfReach" ) setSrcData("[PMF: Invalid data. Try refreshing the table.]", 0);
    else setSrcData(m_pIDSQL->cleanString(m_pIDSQL->rowElement(1,1)),  m_pIDSQL->isBinary(1,1));
    return 0;
}

EditDoubleByte::~EditDoubleByte()
{
    delete m_pIDSQL;
    remove(m_strFile);
}
void EditDoubleByte::exitClicked()
{
    closeMe();
}

int EditDoubleByte::loadData()
{
    int erc = 0;
    if(m_iType == CT_LONG ) erc = loadVarcharData();
    else if(m_iType == CT_GRAPHIC ) erc = loadGraphicData();
    else if( m_iType == CT_DBCLOB ) erc = loadGraphicData(); //loadBinaryFile();
    else if( m_iType == CT_CLOB ) erc = loadFromFile(); //loadBinaryFile();
    else if( m_iType == CT_PMF_RAW ) erc = loadFromItem();
    else if( m_iType == CT_BLOB ) erc = loadFromFile();
    else erc = loadFromItem();
    m_qstrOrigTxt = dataTE->document()->toPlainText();
    QFont font = updateToDB->font();
    font.setBold(false);
    updateToDB->setFont(font);
    return erc;
}

void EditDoubleByte::displayChecked()
{
    loadData();
}

void EditDoubleByte::updateDB()
{
    m_iRunRefresh = 1;
    GString err;
    GString constraint = m_Master->createUniqueColConstraint(m_pItem);
    if( !constraint.length() )constraint = m_Master->createConstraintForThisItem(m_pItem);

    if(m_iType == CT_LONG )
    {        
        GString cmd = "UPDATE "+m_Master->currentTable()+" SET "+m_strColName+"=?" +constraint;
        int erc = m_pIDSQL->uploadLongBuffer(cmd, this->data(), (m_cbDisplay->isChecked()  || this->data().length() > 32000) );
        if( erc ) msg("ErrorCode "+GString(erc)+", msg: "+m_pIDSQL->sqlError());
    }
    else if(m_iType == CT_GRAPHIC || m_iType == CT_DBCLOB || m_iType == CT_STRING || m_iType == CT_CLOB)
    {
        GString data = "'"+this->data()+"'";
        if( m_iType == CT_STRING || m_iType == CT_CLOB) m_pIDSQL->convToSQL(data);
        GString cmd = "UPDATE "+m_Master->currentTable()+" SET "+m_strColName+"="+data+" " +constraint;
        err = m_pIDSQL->initAll(cmd);
        if( err.length() ) msg(err);
    }
    else if(m_iType == 4  )
    {
        GString path = Helper::tempPath();
        GString fileName = path + "PMF5_LOB2.TMP";
        saveDataToFile(fileName);
        GSeq <GString> fileSeq;
        GSeq <long> lobTypeSeq;

        fileSeq.add(fileName);
        GString cmd = "UPDATE "+m_Master->currentTable()+" SET "+m_strColName+"=? " +constraint;
        int col = m_pItem->column()-1;
        lobTypeSeq.add(m_Master->m_pMainDSQL->sqlType(col));
        int erc = m_pIDSQL->uploadBlob(cmd, &fileSeq, &lobTypeSeq);
        if( erc ) msg("Failed: Error "+GString(erc)+", "+m_pIDSQL->sqlError());
        remove(fileName);
    }
    else
    {
        GString data = this->data();
        GString cmd = "UPDATE "+m_Master->currentTable()+" SET "+m_strColName+"="+data+" " +constraint;
        err = m_pIDSQL->initAll(cmd);
        if( err.length() ) msg(err);
    }
    if(err.length()) return;
    m_qstrOrigTxt = dataTE->document()->toPlainText();
    QFont font = updateToDB->font();
    font.setBold(false);
    updateToDB->setFont(font);
}

void EditDoubleByte::saveDataToFile(GString fileName)
{
    m_iRunRefresh = 0;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::information(this, "pmf", "Cannot write file "+fileName);
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");

    //out.setGenerateByteOrderMark(false);
    out << dataTE->document()->toPlainText();
    file.close();
}

void EditDoubleByte::saveToFileClicked()
{
    m_iRunRefresh = 0;
    QString name = QFileDialog::getSaveFileName(this, "Save", "");
    if( name.isNull() ) return;
    saveDataToFile(name);
}

int EditDoubleByte::runRefresh()
{
    return m_iRunRefresh;
}

void EditDoubleByte::setSrcData(GString data, int isBinary)
{
    if( data == "NULL") data = "";
    if( isBinary )
    {
        data.stripTrailing("0");
        if( data % 2 ) data += "0";
        info->setText("Size: "+GString(data.length()/2)+" bytes, represented as HEX");
    }
    else info->setText("Size: "+GString(data.length())+" (binary data will look strange and may be incomplete)");

    dataTE->setText(data);
}

void EditDoubleByte::setInfo(GString text)
{
    info->setText(text);
}
GString EditDoubleByte::data()
{
    GString data = dataTE->document()->toPlainText();
    if( m_iType == CT_GRAPHIC) data = data.stripTrailing("0");
 //   if( data % 2 ) data += "0";
    return data;
}

void EditDoubleByte::setSrcFile(GString file, int isBinary)
{
     m_strFile = file;
     QFile dataFile(file);
     if( isBinary )
     {
         dataFile.open(QFile::ReadOnly | QFile::Text);
         QTextStream ReadFile(&dataFile);
         dataTE->setText(ReadFile.readAll());
     }
     else
     {
        dataFile.open(QFile::ReadOnly);
        QTextStream ReadFile(&dataFile);
#ifndef MAKE_VC
        ReadFile.setCodec("UTF-8");
#endif
        dataTE->setPlainText(ReadFile.readAll());
     }
     dataFile.close();
}

void EditDoubleByte::saveWdgtGeometry()
{
    m_qrGeometry = this->geometry();
}
void EditDoubleByte::restoreWdgtGeometry()
{
    this->setGeometry(m_qrGeometry);
}
void EditDoubleByte::closeEvent(QCloseEvent * event)
{
    ////m_Master->getShowXMLClosed();
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue("editDoubleByteGeometry", this->saveGeometry());
    event->accept();
}

void EditDoubleByte::closeMe()
{
    if( m_qstrOrigTxt != dataTE->document()->toPlainText() )
    {
        if( QMessageBox::question(this, "PMF", "Unsaved changes. Quit anyway?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes )
        {
            return;
        }
    }
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue("editLongGeometry", this->saveGeometry());
    close();
}

void EditDoubleByte::msg(QString txt)
{
    QMessageBox::information(this, "pmf", txt);
}


int EditDoubleByte::findText(int offset)
{
    if( offset < 0 ) offset = 0;

    findLE->setStyleSheet("");
    GString textToFind = findLE->text();
    m_strLastSearchString = textToFind;
    if( !m_cbCaseSensitive->isChecked() ) textToFind = textToFind.upperCase();
    if( !textToFind.length() )
    {
        highlightLine(-1);
        return -1;
    }
    QString fullText = dataTE->toPlainText();
    QStringList lines = fullText.split("\n");

    GString line;
    for( int i = offset; i < lines.count(); ++i )
    {
        if( m_cbCaseSensitive->isChecked() ) line = GString(lines.value(i));
        else line = GString(lines.value(i)).upperCase();
        if( line.occurrencesOf(textToFind))
        {
            highlightLine(i);
            m_iLastFindPos = i;
            return i;
        }
    }
    findLE->setStyleSheet("background: red;");
    return -1;
}

void EditDoubleByte::keyReleaseEvent(QKeyEvent *event)
{
    if( event->modifiers().testFlag(Qt::ControlModifier)) return;
    switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_F3:
    case Qt::Key_Escape:
        return;
    default:
        QWidget::keyPressEvent(event);
        if( findLE->hasFocus() ) findText();
    }
}

void EditDoubleByte::keyPressEvent(QKeyEvent *event)
{
    if( event->modifiers().testFlag(Qt::ControlModifier))
    {
        if( event->key() == Qt::Key_F )
        {
            findLE->selectAll();
            findLE->setFocus();
            return;
        }
        if( event->key() == Qt::Key_S ) updateDB();
    }
    switch (event->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        findNextTextOccurrence();
        break;
    case Qt::Key_F3:
        findNextTextOccurrence();
        break;
    case Qt::Key_Escape:
        if( findLE->hasFocus() && findLE->text().length() > 0)
        {
            m_iLastFindPos = -1;
            highlightLine(m_iLastFindPos);
            findLE->setText("");
            dataTE->setFocus();
        }
        else closeMe();

        break;
    default:
        QWidget::keyPressEvent(event);

    }
}

void EditDoubleByte::highlightLine(int line)
{

    QList<QTextEdit::ExtraSelection> extras;
    QTextEdit::ExtraSelection highlight;
    if( line >= 0 )
    {
        setCursorPosition(line+2); //Scroll down a little.
        setCursorPosition(line);
        highlight.cursor = dataTE->textCursor();
        highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
        highlight.format.setBackground( Qt::lightGray );
    }
    extras << highlight;
    dataTE->setExtraSelections( extras );
}

void EditDoubleByte::setCursorPosition( unsigned int line, unsigned int pos )
{
    PMF_UNUSED(pos);
    QTextCursor cursor = dataTE->textCursor();
    cursor.movePosition( QTextCursor::Start );
    cursor.movePosition( QTextCursor::Down, QTextCursor::MoveAnchor, line  );
    dataTE->setTextCursor(cursor);
}

void EditDoubleByte::findNextTextOccurrence()
{
    m_iLastFindPos = findText(++m_iLastFindPos);
    if( m_iLastFindPos < 0 ) findText(0);
}
