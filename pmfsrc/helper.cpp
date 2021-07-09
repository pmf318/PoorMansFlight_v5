#ifndef HELPER_H
#include "helper.h"
#endif

#ifdef MAKE_VC
#include <windows.h>
#include <process.h>
#include "zlib.h"
#else
#include "../zlib/zlib.h"
#endif

#include <QMessageBox>
#include <QTextStream>
#include <QDir>
#include <QSettings>
#include <QErrorMessage>
#include <QCheckBox>
#include <QDomDocument>
#include <QHeaderView>
#include <QDate>

#ifdef MAKE_VC
#include <io.h>          // open, close, lseek,...
#include <fcntl.h>       // O_RDONLY,...
#include <stdio.h>       // flushall
#include <sys/stat.h>    // stat
#endif

#include "pmfdefines.h"
#include "debugMessenger.hpp"
#include "messenger.hpp"
#include "gstuff.hpp"


#include <gfile.hpp>


/***************************************************
 *  On Win7_64 this code crashes:
 *    QItemSelectionModel* selectionModel = mainLV->selectionModel();
 *    QModelIndexList selected = selectionModel->selectedRows();
 *  on destructing QModelIndexList.
 *  Env: VS2010, Qt 5.3.0
 *
 *  This helper is a workarond.
 */
QModelIndexList Helper::getSelectedRows(QItemSelectionModel *selectionModel)
{
#if QT_VERSION >= 0x050000
    QModelIndexList selected = selectionModel->selectedRows();
    return selected;
#else

    QModelIndexList lstIndex ;

    QItemSelection ranges = selectionModel->selection();
    for (int i = 0; i < ranges.count(); ++i)
    {
        QModelIndex parent = ranges.at(i).parent();
        int right = ranges.at(i).model()->columnCount(parent) - 1;
        if (ranges.at(i).left() == 0 && ranges.at(i).right() == right)
            for (int r = ranges.at(i).top(); r <= ranges.at(i).bottom(); ++r)
                lstIndex.append(ranges.at(i).model()->index(r, 0, parent));
    }
    return lstIndex;
#endif
}

void Helper::msgBox(QWidget *parent, QString title, QString msg )
{
    QMessageBox mbox(QMessageBox::Information, title, msg, QMessageBox::Ok, parent);
    mbox.setText(msg);
    mbox.exec();
}
GString Helper::tempPath()
{
    GString path = QDir::tempPath();
#if defined(MAKE_VC) || defined (__MINGW32__)
    path = path.translate('/', '\\').stripTrailing("\\")+"\\";
#else
    path = path.stripTrailing("/")+"/";
#endif
    return path;
}

GString Helper::filePath(GString path, GString file)
{
#if defined(MAKE_VC) || defined (__MINGW32__)
    path = path.translate('/', '\\').stripTrailing("\\")+"\\";
#else
    path = path.stripTrailing("/")+"/";
#endif
    return path+file;
}
bool Helper::removeDir(const QString & dirName)
{
    //In Qt5 this can be done via "QDir::removeRecursively()."

    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName))
    {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir()) result = removeDir(info.absoluteFilePath());
            else result = QFile::remove(info.absoluteFilePath());
            if (!result) return result;
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

void Helper::showHintMessage(QWidget * parent, int id)
{

    QSettings hintSettings(_CFG_DIR, "pmf5");
    GString notAgain = hintSettings.value("Hint_"+GString(id), "").toString();
    if( notAgain.asInt() > 0 ) return;


    GString msg;
    if(id == 1001 ) 
	{
        msg = "Plase note:\nThe LOB data will be copied to a local file.\nAny changes you make there will NOT be stored in the database.";
		msg += "\n\nTo update LOB data, simply drag&drop a file into a cell\n";
		msg += "or put a question mark (without quotes) into a cell and click 'save'";
	}
    else if(id == 1002 ) 
	{
		msg = "Hint: When editing cells, the quotes around strings will be set automatically.\nSaves you some typing.";
	}
    else if(id == 1003 )
    {
        msg = "Closing PMF via ESC key has been disabled\n(to prevent PMF from preventing Windows-shutdown)\n\nTo enable ESC, click 'Settings'.";
    }
    else return;

    QMessageBox *mb = new QMessageBox(parent);
    mb->setText(msg);
    mb->setIcon(QMessageBox::Information);
    mb->addButton(QMessageBox::Ok);
    QCheckBox notAgainCB("don't show this message again");
    notAgainCB.blockSignals(true);
    mb->addButton(&notAgainCB, QMessageBox::ResetRole);
    mb->exec();

    if(notAgainCB.checkState() == Qt::Checked) hintSettings.setValue("Hint_"+GString(id), 1);
    else hintSettings.setValue("Hint_"+GString(id), 0);
}
int Helper::askFileOverwrite(QWidget *parent, GString fileName, GString question)
{
#ifdef MAKE_VC
    if( (_access( fileName, 0 )) == -1 ) return 0;
#else
    if( (access( fileName, 0 )) == -1 ) return 0;
#endif
    GString qst = "File exists. Overwrite?";
    if( question.length() ) qst = question;
    if( QMessageBox::question(parent, "PMF", qst, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return 1;
	return 0;	
}


int Helper::fileExists(GString fileName)
{
#ifdef MAKE_VC
    if( (_access( fileName, 0 )) != -1 ) return 1;
#else
    if( (access( fileName, F_OK )) != -1 ) return 1;
#endif
    return 0;
}

int Helper::fileIsUtf8(GString filename)
{
    //1. via BOM
    FILE *file;

    printf("Start, in: %s\n", (char*) filename);

    file=fopen(filename,"rb");
    if (!file) return 0 ;
    char buffer[4];
    fread(buffer,sizeof(buffer),1,file);
    fclose(file);
    if (buffer[0] == 0XEF && buffer[1] == 0XBB && buffer[2] == 0XBF) return 1;

    //2. the hard way
    int ret = 0;
    GString path = GString(filename);
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text)) return 0;
    QTextStream in(&f);
    while(!in.atEnd())
    {
        GString line = in.readAll();
        if(line.isProbablyUtf8()) ret = 1;
    }
    f.close();
    printf("end, in: %s, res: %i\n", (char*) filename, ret);
    return ret;
}

GString Helper::fileNameFromPath(GString fullPath)
{
    fullPath = fullPath.translate('\\', '/');
    if( fullPath.occurrencesOf('/') ) return fullPath.subString(fullPath.lastIndexOf('/')+1, fullPath.length()).strip();
    return fullPath;
}

int Helper::uncompressGZ(GString fileName)
{
    QFile qfile(fileName);
    if (!qfile.open(QIODevice::ReadOnly)) return 1;
    QByteArray blob = qfile.readAll();
    qfile.close();
    remove(fileName);

    QByteArray out = gUncompress(blob);
    QFile outFile(fileName);
    outFile.open(QIODevice::WriteOnly);
    outFile.write(out);
    outFile.close();
    if( dataIsXML(&out)) return 2;
	return 0;
}

bool Helper::dataIsXML(QByteArray * array)
{

    if( array->length() )
    {
        QDomDocument doc;
        return doc.setContent(*array, false);
    }
    return false;
}

int Helper::convertBase64toPNG(GString fileName, GString *newExt)
{
    *newExt = "";
    FILE *dataFile = fopen(fileName, "rb");
    if( !dataFile ) return 1;
    fseek(dataFile, 0, SEEK_END);
    long fsize = ftell(dataFile);
    if( fsize < 21 )
    {
        fclose(dataFile);
        return 2;
    }
    fseek(dataFile, 0, SEEK_SET);
    char *data = (char*)malloc(fsize + 1);
    fread(data, 21, 1, dataFile);
    if( GString(data).subString(1, 21) == "data:image/png;base64" ) *newExt = "PNG";
    else if( GString(data).subString(1, 21) == "data:image/jpg;base64" ) *newExt = "JPG";
    else
    {
        free(data);
        fclose(dataFile);
        return 3;
    }
    fseek(dataFile, 0, SEEK_SET);
    fread(data, fsize, 1, dataFile);
    fclose(dataFile);
    unsigned char *out = (unsigned char*)malloc(fsize/4*3);
    size_t outLen = fsize/4*3;
    int rc = GStuff::base64_decode(data+22, fsize-22, out, &outLen);
    if( !rc )
    {
        remove(fileName);
        dataFile = fopen(fileName, "wb");
        fwrite(out, 1, outLen, dataFile);
        fclose(dataFile);
    }
    free(data);
    free(out);
    return rc;
}

QByteArray Helper::gUncompress(const QByteArray &data)
{
    if (data.size() <= 4) {
        qWarning("gUncompress: Input data is truncated");
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)(data.data());

    ret = inflateInit2(&strm, 15 +  32); // gzip decoding
    if (ret != Z_OK)
        return QByteArray();

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}

GString Helper::createIndexSortString(GString in)
{
    GString out;
    int pos;
    in = in.strip();
    if( in.occurrencesOf('+') == 0 && in.occurrencesOf('-') == 0 && in.occurrencesOf('*') == 0 ) return in;
    while( in.length() )
    {
        pos = Helper::findNextSortToken(in) - 1;
        if( pos == 0 ) break;
        if( in[1] == '+')  out += in.subString(2, pos) + " ASC, ";
        if( in[1] == '-')  out += in.subString(2, pos) + " DESC, ";
        if( in[1] == '*')  out += in.subString(2, pos) + " RAND, ";
        in = in.remove(1, pos+1);
    }
    return out.strip().stripTrailing(',');
}

int Helper::findNextSortToken(GString in)
{
    in = in.subString(2, in.length()).strip();
    if( in.occurrencesOf('+') == 0 && in.occurrencesOf('-') == 0 && in.occurrencesOf('*') == 0 ) return in.length()+1;
    for( int i = 1; i <= in.length(); ++i )
    {
        if( in[i] == '+' || in[i] == '-' || in[i] == '*' ) return i;
    }
    return in.length()+1;
}

GString Helper::tableName(GString table, ODBCDB type)
{
    table = table.removeAll('\"');
    if( type == SQLSERVER)
    {
        if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
        if( table.occurrencesOf(".") == 2 ) table = table.remove(1, table.indexOf("."));
    }
    return table.subString(table.indexOf(".")+1, table.length()).strip();
}

GString Helper::tableContext(GString table, ODBCDB type)
{
    if( type != SQLSERVER ) return "";
    table.removeAll('\"');
    if( table.occurrencesOf(".") != 2 ) return "";
    return table.subString(1, table.indexOf(".")-1);
}

GString Helper::tableSchema(GString table, ODBCDB type)
{
    table.removeAll('\"');
    if( type == SQLSERVER)
    {
        if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
        if( table.occurrencesOf(".") == 2 ) table.remove(1, table.indexOf("."));
    }
    return table.subString(1, table.indexOf(".")-1);
}

GString Helper::pmfVersion()
{
    return "v."+GString(PMF_VER).insert(".", 3).insert(".", 2);
}

GString Helper::pmfNameAndVersion(GString database)
{
    QDate aDate = QLocale("en_US").toDate(QString(__DATE__).simplified(), "MMM d yyyy");
    if( database.length() ) database = database +" - ";
#if defined( _WIN64 ) || defined( __x86_64__ )
    return database + "Poor Man's Flight v."+GString(PMF_VER).insert(".", 3).insert(".", 2) + " (64bit, Build "+GString(aDate.year()).subString(3,2)+GString(aDate.dayOfYear()).rightJustify(3, '0')+")";
#elif defined(__MINGW32__ )
    return database + "Poor Man's Flight v."+GString(PMF_VER).insert(".", 3).insert(".", 2)+" Portable (Build "+GString(aDate.year()).subString(3,2)+GString(aDate.dayOfYear()).rightJustify(3, '0')+", MinGW32)";
#elif defined(MSVC_STATIC_BUILD)
    return database + "Poor Man's Flight v."+GString(PMF_VER).insert(".", 3).insert(".", 2)+" Portable (Build "+GString(aDate.year()).subString(3,2)+GString(aDate.dayOfYear()).rightJustify(3, '0')+", MSVC)";
#else
    return database + "Poor Man's Flight v."+GString(PMF_VER).insert(".", 3).insert(".", 2)+" (Build "+GString(aDate.year()).subString(3,2)+GString(aDate.dayOfYear()).rightJustify(3, '0')+")";
#endif
}



void Helper::setVHeader(QTableWidget * someLV)
{
    someLV->setWordWrap(false);
#if QT_VERSION >= 0x050000
    someLV->horizontalHeader()->setSectionsMovable(true);
#else
    someLV->horizontalHeader()->setMovable(true);
#endif

    if( !someLV->isSortingEnabled() ) someLV->setSortingEnabled(true);
    QTableWidgetItem * newItem;
    for ( int i = 0; i < someLV->rowCount(); ++i)
    {
        newItem = new QTableWidgetItem(i);
        someLV->setRowHeight(i, QFontMetrics( someLV->font()).height()+5);
        someLV->setVerticalHeaderItem(i, newItem);
    }
//    for(int i = 0; i < someLV->columnCount(); ++i)
//    {
//        someLV->resizeColumnToContents ( i );
//        if( someLV->columnWidth(i) > 200 ) someLV->setColumnWidth(i, 200);
//    }
}

void Helper::setLastSelectedPath(GString className, GString path)
{
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue(className, QString((char*)path));
}

GString Helper::getLastSelectedPath(GString className)
{
    QSettings settings(_CFG_DIR, "pmf5");
    if( settings.value(className, -1).toInt() >= 0 )
    {
        return settings.value(className).toString();
    }
    return "";
}

GString Helper::createSearchConstraint(DSQLPlugin * pDSQL, GSeq<COL_SPEC*> *colDescSeq, GString val, int col, int exactMatch)
{
    if( !val.strip().length() ) return "";

    val = pDSQL->cleanString(val);
    GString modifier1 = "";
    GString modifier2 = "";

    if( val.occurrencesOf("@DSQL@") || (pDSQL->isXMLCol(col) && val != "NULL") )
    {
        return "";
    }
    if( pDSQL->isForBitCol(col) >= 3  && val != "NULL"  )
    {
        int lng = 0;
        for(int i = 1; i <= (int)colDescSeq->numberOfElements(); ++i)
        {
            if( i == col ) lng =  colDescSeq->elementAtPosition(i)->Length.asInt();
        }
        modifier1 = "";
        //val = " LIKE '%' || "+formatForHex(m_pMainDSQL, "'"+val+"'")+" || '%'";

        if( lng*2 != (int)val.length() && !exactMatch ) val = " LIKE '%' || "+Helper::formatForHex(pDSQL, "'"+val+"'")+" || '%'";
        else val = " = "+Helper::formatForHex(pDSQL, "'"+val+"'");
    }
    else if( val == "NULL") val = " IS NULL";
    else if( pDSQL->isNumType(col) && pDSQL->getDBType() != SQLSERVER)
    {
        modifier1 = "CHAR";
        if( exactMatch) val = "="+val.upperCase();
        else val = " LIKE '%"+val.upperCase()+"%'";
    }
    else if( pDSQL->isNumType(col) && pDSQL->getDBType() == SQLSERVER)
    {
        if( exactMatch) val = " = '"+val+"'";
        else  val = " LIKE '%"+val.upperCase()+"%'";
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
        pDSQL->convToSQL(val);
        if( exactMatch )
        {
            modifier1 = "";
            val = " = '"+val+"'";
        }
        else
        {
            val = " LIKE '%"+val.upperCase()+"%'";
            modifier1 = "UPPER";
        }
    }
    return modifier1+"("+GStuff::wrap(pDSQL->hostVariable(col))+modifier2+")"+val;
}

GString Helper::formatForHex(DSQLPlugin* pDSQL, QTableWidgetItem * pItem)
{
    if( !pItem ) return "<NULL Item>";
    GString text = pItem->text();
    if( text.indexOf('x') == 1 ) return text;
    if( pDSQL->getDBType() == DB2 || pDSQL->getDBType() == DB2ODBC ) return "x"+Helper::formatItemText(pDSQL, pItem);
    return text;
}

GString Helper::formatForHex(DSQLPlugin* pDSQL, GString in)
{
    //For DB2, we need to set 'x' before a Hex string
    if( GString(in).strip().strip("'").indexOf('x') == 1 ) return in; //Already set
    if( pDSQL->getDBType() == DB2 || pDSQL->getDBType() == DB2ODBC ) return "x"+in;
    return in;
}

GString Helper::formatItemText(DSQLPlugin * pDSQL, QTableWidgetItem * pItem)
{
    int colNr = pItem->column()-1;
    if( !pItem ) return "<NULL Item>";
    GString text = pItem->text();
    if( !text.length() ) return "NULL";

    if( text == "?" )
    {
        if( pDSQL->isLOBCol(colNr) )return text;
        if( pDSQL->isXMLCol(colNr) )return text;
    }
    if( text.subString(1, 6) == "@DSQL@" ) return text;
    if( Helper::isSystemString(text)) return text;
    if( pDSQL->isNumType(colNr) || GString(text).upperCase() == "NULL") return text;
    if( pDSQL->isDateTime(colNr) )
    {
        if( GString(text).upperCase().occurrencesOf("CURRENT") ) return text;
        else if( text[1UL] == '\'' && text[text.length()] == '\'' )  return text;
        return "'"+text+"'";
    }
    if( text[1UL] == '\'' && text[text.length()] == '\'' )  return text;
    return "'"+text+"'";
}

int Helper::isSystemString(GString in)
{
    in = in.upperCase();
    if( in.occurrencesOf("CURRENT"))
    {
        if( in.occurrencesOf("DATE") || in.occurrencesOf("TIMESTAMP")) return 1;
    }
    if( in.occurrencesOf("GETDATE()")) return 1;
    return 0;
}

int Helper::createFileIfNotExist(GString fileName)
{
    QDir dir;
    GString path = GStuff::pathFromFullPath(fileName);
    if (!dir.exists(path)) dir.mkpath(path);
    QFile file(path + GStuff::fileFromPath(fileName));
    file.open(QIODevice::WriteOnly);
    file.close();
    return 0;
}

GString Helper::convertGuid(GString in)
{
    int toDb2 = 0;
    if( in.length() != 32 && in.length() != 36 ) return in;
    if( in.occurrencesOf('-') != 0 && in.occurrencesOf('-') != 4 ) return in;
    if( in.occurrencesOf('-') == 0 ) toDb2 = 1; //convert to DB2 format

    in = in.removeAll('-');
    GSeq <GString> blockSeq;
    for( int i = 1; i < 32; i+=2 )
    {
        blockSeq.add(in.subString(i, 2));
    }
    GString out;
    out = blockSeq.elementAtPosition(4) + blockSeq.elementAtPosition(3)+blockSeq.elementAtPosition(2)+blockSeq.elementAtPosition(1);
    if( toDb2 ) out += "-";
    out += blockSeq.elementAtPosition(6)+blockSeq.elementAtPosition(5);
    if( toDb2 ) out += "-";
    out += blockSeq.elementAtPosition(8)+blockSeq.elementAtPosition(7);
    if( toDb2 ) out += "-";
    out += blockSeq.elementAtPosition(9)+blockSeq.elementAtPosition(10);
    if( toDb2 ) out += "-";
    out += blockSeq.elementAtPosition(11)+blockSeq.elementAtPosition(12)+blockSeq.elementAtPosition(13)+blockSeq.elementAtPosition(14);
    out += blockSeq.elementAtPosition(15)+blockSeq.elementAtPosition(16);
    return out;

}
