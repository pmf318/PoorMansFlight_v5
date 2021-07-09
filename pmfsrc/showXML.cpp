
//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//


#include <Qt>
#include "showXML.h"
#include <qlayout.h>
#include <QGridLayout>
#include <QGroupBox>
#include <gstuff.hpp>
#include <gfile.hpp>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSettings>
#include <QMessageBox>
#include <QDomDocument>
#include "tabEdit.h"
#include "helper.h"
#include "pmfdefines.h"
#include "showXPath.h"


ShowXML::ShowXML(GDebug *pGDeb, TabEdit *parent, QTableWidgetItem * pItem, GString srcFile, GString lastSearchString)
  :QDialog(parent)
{
    m_pItem = pItem;
    m_iItemAddr =(long) m_pItem;
    m_Master = parent;
	m_strFile = srcFile;
    m_pGDeb = pGDeb;
    //GString m = "ParentFont: "+GString(parent->font().pointSize())+", pix: "+GString(parent->font().pixelSize());
    //QMessageBox::information(this, "pmf", m);



	this->resize(560, 400);
    this->setWindowTitle("XML");

	QGridLayout * grid = new QGridLayout(this);

	//Buttons
    checkSyntaxButton = new QPushButton(this);
    checkSyntaxButton->setDefault(true);
    checkSyntaxButton->setText("Check syntax");
    connect(checkSyntaxButton, SIGNAL(clicked()), SLOT(checkSyntaxClicked()));
	
    okButton = new QPushButton(this);
    okButton->setDefault(true);
    okButton->setText("Exit");
    connect(okButton, SIGNAL(clicked()), SLOT(OKClicked()));


    saveToFileButton = new QPushButton(this);
    saveToFileButton->setText("&Write to file");
    connect(saveToFileButton, SIGNAL(clicked()), SLOT(saveToFileClicked()));

    updateButton = new QPushButton(this);
    updateButton->setText("&Save");
    connect(updateButton, SIGNAL(clicked()), SLOT(updateClicked()));

    reloadButton = new QPushButton(this);
    reloadButton->setText("&Reload (F5)");
    connect(reloadButton, SIGNAL(clicked()), SLOT(reloadClicked()));
		
	//TextEdit
    //xmlTE = new QTextEdit(this);
    xmlTE = new XmlEdit(this, parent->colorScheme());
    //xmlTE->setAcceptRichText(false);

    xmlTE->setGeometry( 20, 20, 520, 300);



    m_qaShowXPath = new QAction( "Show XPath", this );
    actionsMenu.addAction(m_qaShowXPath);

    m_qaShowAttrSql = new QAction( "Show SQL for Attribute", this );
    actionsMenu.addAction(m_qaShowAttrSql);

    xmlTE->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( m_qaShowXPath, SIGNAL( triggered() ), this, SLOT( slotShowXPath() ) );
    connect( m_qaShowAttrSql, SIGNAL( triggered() ), this, SLOT( slotShowAttrSql() ) );
    connect( xmlTE, SIGNAL( customContextMenuRequested(const QPoint &) ), this, SLOT( popUpXmlActions(const QPoint &) ) );





    QLabel * info = new QLabel(this);
    QFont font  = info->font();
    font.setBold(true);
    info->setText("Hint: Drag&drop XML files here, right-click a start-node to view XPath");


	m_cbFormat = new QCheckBox("Format data");
	m_qlStatus = new QLabel(this);	
	
	
	grid->addWidget(m_cbFormat, 0, 0, 1, 1);
    grid->addWidget(info, 0, 1, 1, 4);
    grid->addWidget(xmlTE, 2, 0, 1, 5);
    grid->addWidget(m_qlStatus, 3, 0, 1, 5);

    
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
	
    grid->addWidget(findGroupBox, 5, 0, 1, 5);
	

		
    QSettings settings(_CFG_DIR, "pmf5");
    QString prev = settings.value("showXMLformatData", "showXMLformatData").toString();
    if( prev == "N") m_cbFormat->setChecked(false);
    else m_cbFormat->setChecked(true);


    connect(m_cbFormat, SIGNAL(stateChanged(int)), SLOT(formatChecked()));

    
    grid->addWidget(checkSyntaxButton, 4, 0);
    grid->addWidget(updateButton, 4, 1);
    grid->addWidget(reloadButton, 4, 2);
    grid->addWidget(saveToFileButton, 4, 3);
    grid->addWidget(okButton, 4, 4);
    m_iNeedsReload = 0;

    this->restoreGeometry(settings.value("showXMLgeometry").toByteArray());


    //For setFont(...) to work, parent needs to be pmf (i.e., the main widget), see tabEdit::slotRightClickActionShowXML()
	
	m_iLastFindPos = -1;
	m_strLastSearchString = lastSearchString;
	findLE->setText(m_strLastSearchString);
    if( m_strFile.length() ) displayData();
    installEventFilter(this);
    xmlTE->scrollToTop();
    connect(xmlTE, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(parent, SIGNAL(mainWidgetCleared()), this, SLOT(mainWidgetWasCleared()));
}

void ShowXML::mainWidgetWasCleared()
{
    m_qlStatus->setStyleSheet("QLabel { color : blue; }");
    m_qlStatus->setText("Content has become invalid through changes in the background. Close and reopen this window.");
    //QMessageBox::information(this, "pmf", "The View has become invalid through changes in the background. Please close and reopen this window.");
    checkSyntaxButton->setEnabled(false);
    reloadButton->setEnabled(false);
    updateButton->setEnabled(false);
    saveToFileButton->setEnabled(false);
}

void ShowXML::slotShowXPath()
{    
    GString txt = getAllPaths();
    if( !txt.length() )
    {
        QMessageBox::information(this, "pmf", "Select an XML-element (starts with '<')");
        return;
    }
    ShowXPath *sxp = new ShowXPath(m_pGDeb, this, m_Master);
    sxp->setText( txt );
    sxp->exec();
}

void ShowXML::slotShowAttrSql()
{
    GString txt = createXPathText(1);
    if( !txt.length() )
    {
        QMessageBox::information(this, "pmf", "Select an XML-attribute (colored bright red, looks like 'element=\"...\")");
        return;
    }
    ShowXPath *sxp = new ShowXPath(m_pGDeb, this, m_Master);
    sxp->setText( txt );
    sxp->exec();
}

void  ShowXML::popUpXmlActions(const QPoint & p)
{
    GString txt;
    //txt = getAllPaths();
    txt = createXPathText();
    if( !txt.length() ) return;

    if(p.isNull()) return;
    actionsMenu.exec(xmlTE->mapToGlobal(p));
}


void ShowXML::onTextChanged()
{
    QFont font = updateButton->font();
    if( m_qstrOrgTxt == xmlTE->toPlainText() )font.setBold(false);
    else font.setBold(true);
    updateButton->setFont(font);
}

GString ShowXML::getAllPaths()
{
    if( checkRefresh() ) return "";
    QDomDocument doc;
    QString partialXML = xmlTE->partialXml(1);


    if( !partialXML.length() ) return "";
    doc.setContent(partialXML);


    QString selTxt = xmlTE->selectedText();
    if( !selTxt.length() ) return "";

    GString xpathConstraints = getXPathWithConstraints(doc, selTxt);


    GString colName = m_Master->m_pMainDSQL->hostVariable(m_pItem->column()-1);
    GString constraint = m_Master->createUniqueColConstraint(m_pItem);
    if( !constraint.length() ) constraint = m_Master->createConstraintForThisItem(m_pItem).change("\"", "");

    GString out = "--------------------------------------------------\n";
    out += "-- XPath to selected node:\n";
    out += "--------------------------------------------------\n";
    out += getXPath(doc, selTxt);
    out += "\n\n";

    out += "--------------------------------------------------\n";
    out += "-- XPath to selected node with all attributes:\n";
    out += "--------------------------------------------------\n";
    out += getXPathWithConstraints(doc, selTxt);
    out += "\n\n";

    if( m_Master->m_pMainDSQL->getDBType() == DB2 || m_Master->m_pMainDSQL->getDBType() == DB2ODBC )
    {
        out += "--------------------------------------------------\n";
        out += "-- xquery select (may not work in namespaced XMLs):\n";
        out += "--------------------------------------------------\n";
        out += "xquery db2-fn:sqlquery(\"select "+colName+" from "+m_Master->currentTable(0)+" "+constraint.change("\"", "")+"\")\n"+xpathConstraints;
        out += "\n\n";

        out += "--------------------------------------------------\n";
        out += "-- SQL/XML select (may not work in namespaced XMLs):\n";
        out += "--------------------------------------------------\n";
        out += "select xmlquery('$i"+xpathConstraints+"' passing "+colName+"  as \"i\") as xmldata from "+m_Master->currentTable(1)+" where ";
        out += "xmlexists( '$i"+xpathConstraints+"' passing "+colName+"  as \"i\")  ";
        out += "\n"+m_Master->createConstraintForThisItem(m_pItem).change("WHERE", "AND");
        out += "\n\n";
    }
    else if( m_Master->m_pMainDSQL->getDBType() == SQLSERVER )
    {
        out += "--------------------------------------------------\n";
        out += "-- query select (may not work in namespaced XMLs):\n";
        out += "--------------------------------------------------\n";
        out += "SELECT "+colName+".query('"+xpathConstraints+"') FROM "+m_Master->currentTable(1)+" "+constraint;
        out += "\n\n";
    }
    return out;
    /*
    xpathConstraints = getXPathWithConstraints(doc, selTxt);

    selCmd = "select xmlquery('$i"+xpathConstraints+"' passing \"XMLDATA\"  as \"i\") as xmldata from "+m_Master->currentTable(1)+" where ";
    selCmd += "xmlexists( '$i"+xpathConstraints+"' passing xmldata as \"i\")  ";
    selCmd += "\n"+m_Master->createConstraintForThisItem(m_pItem).change("WHERE", "AND");
    printf("########################\n");
    printf("%s\n", (char*) selCmd);
    printf("########################\n");
    printf("%s\n", (char*) xpath);
    */

}

GString ShowXML::getXPath(QDomDocument doc, QString nodeName)
{
    QDomElement root = doc.documentElement();

    QDomNodeList tags = root.elementsByTagName(nodeName);
    if( tags.count() <= 0 ) return nodeName;
    QDomElement lastSelTag = tags.at(tags.count()-1).toElement();
    QDomNode parent = lastSelTag.parentNode();
    lastSelTag.attributes();
    GString xpath = "/"+ tags.at(tags.count()-1).toElement().tagName();
    while(!parent.parentNode().isNull())
    {
        xpath = "/"+GString(parent.toElement().tagName()) + xpath;
        parent = parent.parentNode();
    }
    return xpath;
}

GString ShowXML::createXPathText(int checkForAttribute)
{

    QDomDocument partialDoc;
    QString partialXML = xmlTE->partialXml(0);
    if( !partialXML.length() ) return "";
    partialDoc.setContent(partialXML);

    QString selTxt = xmlTE->selectedText();
    if( !selTxt.length() ) return "";


    QDomElement domElement = partialDoc.documentElement();

    int childCount;
    QDomNode domNode = domElement.lastChild();
    while ( 1 )
    {
        childCount = domNode.childNodes().count();
        if( childCount == 0 )break;
        domNode = domNode.lastChild();
    }

    GString constraint = m_Master->createUniqueColConstraint(m_pItem);
    if( !constraint.length() ) constraint = m_Master->createConstraintForThisItem(m_pItem).change("\"", "");
    constraint = constraint.stripLeading(" WHERE");




    QDomNode selectedDomNode = domNode;
    GString nodeName = selectedDomNode.nodeName();
    GString xpath = getXPath(partialDoc, nodeName);

    if( checkForAttribute && !domNode.attributes().contains(selTxt) )
    {
        if( domElement.attributes().contains(selTxt) )
        {
            selectedDomNode = domElement;
        }
        else return "";
    }






    GString xmlConstraint = getXPathWithConstraints(partialDoc, nodeName);
    GString colName = m_Master->m_pMainDSQL->hostVariable(m_pItem->column()-1);

    GString attrSql;
    nodeName += "--"+attributesString(selectedDomNode);

    if( m_Master->m_pMainDSQL->getDBType() == DB2 || m_Master->m_pMainDSQL->getDBType() == DB2ODBC )
    {
        attrSql = "-- Minimal SQL to get an attribute's value.\n";
        attrSql += "-- This will give an error if more than one result is returned.\n";
        attrSql += "-- In that case, use the fully constrained select below.\n";

        attrSql += "select xmlcast( xmlquery('$i"+xpath+"/@"+GString(selTxt)+"'\n passing "+colName+"  as \"i\") as varchar(128))\n"
                +" from "+m_Master->currentTable(0) + "\n WHERE " +constraint;

        attrSql +="\n\n";
        attrSql +="-- SQL to get the selected attribute with full constraint. \n";
        attrSql += "select xmlcast( xmlquery('$i"+xmlConstraint+"/@"+GString(selTxt)+"'\n passing "+colName+"  as \"i\") as varchar(128))\n"
                +" from "+m_Master->currentTable(0)
                +"\n where xmlexists('$i"+xmlConstraint+"' passing "+colName+"  as \"i\")\n "+
                " AND " + constraint;

    }
    else if( m_Master->m_pMainDSQL->getDBType() == SQLSERVER )
    {
        attrSql = "--SQL to get first valid result, denoted by '[1]' in command below.\n";
        attrSql += "SELECT "+colName+".value('("+xpath+"/@"+GString(selTxt)+")[1]', 'varchar(100)') AS RESULT\n"
        +" from "+m_Master->currentTable(0)+
        "\nWHERE \n" +constraint;

        attrSql += "\n\n";
        attrSql +="-- SQL to get the selected attribute with full constraint. \n";
        attrSql += "SELECT "+colName+".value('("+xpath+attributesString(domNode)+"/@"+GString(selTxt)+")[1]', 'varchar(100)') AS RESULT"
        +" from "+m_Master->currentTable(0)+
        "\nWHERE \n" +constraint;


//        " WHERE XmlData.exist('"+attributesString(dn, xpath)+"')=1\n"
//        +" AND "+constraint;

    }


    return attrSql;
}

GString ShowXML::attributeSql()
{
    GString data = "HHHH";
    QDomDocument partialDoc;
    QString partialXML = xmlTE->partialXml(0);
    if( !partialXML.length() ) return "";
    partialDoc.setContent(partialXML);

    QString selTxt = xmlTE->selectedText();
    if( !selTxt.length() ) return "";


    QDomElement rt = partialDoc.documentElement();
    int chCnt;
    QDomNode dn = rt.lastChild();
    while ( 1 )
    {
        chCnt = dn.childNodes().count();
        if( chCnt == 0 )break;
        dn = dn.lastChild();
    }
    ;
    GString x = dn.nodeName();
    x += "--"+attributesString(dn);
    printf("+++++\nx: %s\n+++++\n", (char*) x);
    GString xpathConstraints = getXPathWithConstraints(partialDoc, dn.nodeName());

    GString colName = m_Master->m_pMainDSQL->hostVariable(m_pItem->column()-1);
    GString constraint = m_Master->createUniqueColConstraint(m_pItem);
    if( !constraint.length() ) constraint = m_Master->createConstraintForThisItem(m_pItem).change("\"", "");

    GString xpath = getXPath(partialDoc, selTxt);

    GString attrSql = "select xmlcast( xmlquery('$i"+xpath+"/@"+GString(selTxt)+"' passing XMLDATA  as \"i\") as varchar(128))"
            +" from "+m_Master->currentTable(0)+
            constraint;
    printf("ATTRSQL: %s\n-----\n", (char*)attrSql);
    return attrSql;

}

GString ShowXML::getXPathWithConstraints(QDomDocument doc, QString selTxt)
{
    QDomElement root = doc.documentElement();       
    QDomNodeList tags = root.elementsByTagName(selTxt);

    if( tags.count() <= 0 ) return "/"+GString(selTxt)+attributesString( doc.documentElement());
    QDomElement lastSelTag = tags.at(tags.count()-1).toElement();
    QDomNode parent = lastSelTag.parentNode();
    lastSelTag.attributes();

    QDomNode selectedNode = tags.at(tags.count()-1);
    GString xpath = "/"+ GString(selectedNode.toElement().tagName()) + attributesString(selectedNode);

    while(!parent.parentNode().isNull())
    {
        xpath = "/"+GString(parent.toElement().tagName())+attributesString(parent) + xpath;
        parent = parent.parentNode();
    }
    return xpath;
}

GString ShowXML::attributesString(QDomNode node, GString xPath)
{
    GString out;
    QDomNamedNodeMap map = node.attributes();
    if( map.length() == 0 ) return "";
    if( xPath.length() ) xPath = xPath.stripTrailing("/")+"/";
    for( int i = 0 ; i < map.length(); ++i )
    {
        if(!(map.item(i).isNull()))
        {
            QDomNode attrNode = map.item(i);
            QDomAttr attr = attrNode.toAttr();
            if(!attr.isNull())
            {
                out += "   "+ xPath+"@"+GString(attr.name())+"=\""+GString(attr.value())+"\"";
                if( i < map.length() -1 ) out += " and\n ";
            }
        }
    }
    if( xPath.length() ) return out;
    return "[\n"+out+"]\n";
}

ShowXML::~ShowXML()
{
    deb("DTor start");
    m_Master->setLastXmlSearchString(m_strLastSearchString);
    remove(m_strFile);
    deb("DTor done");
}
void ShowXML::checkSyntaxClicked()
{
	m_qlStatus->setText(xmlTE->checkSyntax());
}

void ShowXML::reloadClicked()
{
    if( checkRefresh() ) return;
    int outSize;
    m_Master->writeDataToFile(m_pItem, m_strFile, &outSize);
	displayData();
}

void ShowXML::updateClicked()
{
    QFont font = updateButton->font();
    font.setBold(false);
    updateButton->setFont(font);

    if( checkRefresh() ) return;

    m_gstrNewXML = xmlTE->toPlainText();
    m_iNeedsReload = 1;
    m_qlStatus->setText("Saving...");
    GString err;
    //if( m_gstrNewXML.length() > 32000 )
    {
        GString tmpFile = Helper::tempPath()+"XML.TMP";
        remove(tmpFile);
        QFile file(tmpFile);

        if( !file.open(QIODevice::ReadWrite) )
        {
            QMessageBox::information(this, "pmf", "Cannot creat temp file "+tmpFile);
            return;
        }
        QTextStream stream(&file);
        stream << xmlTE->toPlainText();
        file.flush();
        file.close();
        err = m_Master->updateLargeXML(m_pItem, tmpFile);
        //m_pItem->setText(restoreText);
    }
//    else err = m_Master->updateXML(m_pItem, m_gstrNewXML);

    if( err.length() )QMessageBox::information(this, "pmf", "Error on update XML: "+err+", errCode: "+GString());
    else
    {
        m_qstrOrgTxt = (char*)m_gstrNewXML;
        m_qlStatus->setText("XML updated.");
        m_qlStatus->repaint();
    }
    m_qlStatus->setText("");

}

void ShowXML::OKClicked()
{
    //m_iNeedsReload means that "UPDATE" was clicked
    if( m_qstrOrgTxt != xmlTE->toPlainText() && !m_iNeedsReload )
    {
        if( QMessageBox::question(this, "PMF", "XML changed, close anyway?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;
    }
    closeMe();
}
void ShowXML::saveToFileClicked()
{
	QString name = QFileDialog::getSaveFileName(this, "Save", "");
	if( name.isNull() ) return;


    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::information(this, "pmf", "Cannot write file "+name);
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    //out.setGenerateByteOrderMark(false);
    out << xmlTE->document()->toPlainText();
    file.close();



}

short ShowXML::fillLB()
{

	return 0;
}
void ShowXML::closeMe()
{
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue("showXMLgeometry", this->saveGeometry());
    if( m_cbFormat->isChecked()) settings.setValue("showXMLformatData", "Y");
    else settings.setValue("showXMLformatData", "N");

    close();
}


void ShowXML::formatChecked()
{
    displayData();
}

void ShowXML::displayData()
{
    xmlTE->clear();
    if (m_cbFormat->isChecked() )
    {

        //xmlTE->insertPlainText(readXmlFile());
        //xmlTE->setText(readXmlFile());
        //return;
        QFile inputFile(m_strFile);
		deb(m_strFile);
        if (!QFile::exists(m_strFile)) return;
        QString xmlOut;

        QXmlStreamReader reader(&inputFile);

        //Need to open file for streamReader
        if( !inputFile.open(QIODevice::ReadOnly) )
        {
            QMessageBox::information(this, "pmf", "Cannot open "+m_strFile);
            return;
        }

        QXmlStreamWriter writer(&xmlOut);
        writer.setAutoFormatting(true);

		deb("Start read");
        while (!reader.atEnd())
        {			            
            reader.readNext();
            //deb("while read: "+GString((int)reader.characterOffset ()));
            if (!reader.isWhitespace())
            {
                writer.writeCurrentToken(reader);
            }
        }
        if( reader.hasError() )QMessageBox::information(this, "pmf", "XML-parse error on reading "+m_strFile+": "+GString(reader.errorString()));
		deb("end ");
        xmlTE->insertPlainText(xmlOut);
        inputFile.close();
    }
    else
    {
        QFile xmlFile(m_strFile);
        xmlFile.open(QFile::ReadOnly | QFile::Text);
        QTextStream ReadFile(&xmlFile);
        xmlTE->insertPlainText(ReadFile.readAll());
        xmlFile.close();
    }
    m_qstrOrgTxt = xmlTE->toPlainText();
    QFont font = updateButton->font();
    font.setBold(false);
    updateButton->setFont(font);

}



GString ShowXML::readXmlFile()
{
    deb(m_strFile);
    if (!QFile::exists(m_strFile)) return "[Error: Cannot open file "+m_strFile+"]";

    QDomDocument input;

    QFile inFile(m_strFile);

    inFile.open(inFile.Text | inFile.ReadOnly);
    input.setContent(&inFile);
    QDomDocument output(input);
    GString s = output.toString(5);
    QMessageBox::information(this, "pmf", s);
    return s;
}

void ShowXML::setSrcFile(GString file)
{

     m_strFile = file;
     displayData();

}
int ShowXML::needsReload()
{
    return m_iNeedsReload;
}
GString ShowXML::lastSearchString()
{
	return m_strLastSearchString;
}
GString ShowXML::newXML()
{
    return m_gstrNewXML;
}
void ShowXML::saveWdgtGeometry()
{
    m_qrGeometry = this->geometry();
}
void ShowXML::restoreWdgtGeometry()
{
    this->setGeometry(m_qrGeometry);
}
void ShowXML::closeEvent(QCloseEvent * event)
{
    m_Master->getShowXMLClosed();
    event->accept();
}
void ShowXML::keyReleaseEvent(QKeyEvent *event)
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

void ShowXML::keyPressEvent(QKeyEvent *event)
{

	if( event->modifiers().testFlag(Qt::ControlModifier))
    {
        if( event->key() == Qt::Key_F ) 
		{
			findLE->selectAll();
			findLE->setFocus();
			return;
		}
        if( event->key() == Qt::Key_S ) updateClicked();
	}
    switch (event->key())
    {
        case Qt::Key_F5:
            reloadClicked();
            break;

        case Qt::Key_Enter:
        case Qt::Key_Return:
			findNextTextOccurrence();
            //OKClicked();
            break;
        case Qt::Key_F3:
			findNextTextOccurrence();
            break;			
		case Qt::Key_Escape:
            if( findLE->hasFocus() && findLE->text().length() > 0)
			{
				m_iLastFindPos = -1;
				xmlTE->highlightLine(m_iLastFindPos);                
                findLE->setText("");
                xmlTE->setFocus();
			}
            else OKClicked();
            break;
		default:
			QWidget::keyPressEvent(event);
			
    }

}

void ShowXML::msg(QString txt)
{
    Helper::msgBox(this, "pmf", txt);
}

void ShowXML::findNextTextOccurrence()
{
    m_iLastFindPos = findText(++m_iLastFindPos);
    if( m_iLastFindPos < 0 ) findText(0);
}

int ShowXML::findText(int offset)
{
    if( offset < 0 ) offset = 0;

    findLE->setStyleSheet("");
    GString textToFind = findLE->text();
	m_strLastSearchString = textToFind;
    if( !m_cbCaseSensitive->isChecked() ) textToFind = textToFind.upperCase();
    if( !textToFind.length() )
    {
        xmlTE->highlightLine(-1);
        return -1;
    }
    QString fullText = xmlTE->toPlainText();
    QStringList lines = fullText.split("\n");

    GString line;
    for( int i = offset; i < lines.count(); ++i )
    {
        if( m_cbCaseSensitive->isChecked() ) line = GString(lines.value(i));
        else line = GString(lines.value(i)).upperCase();
        if( line.occurrencesOf(textToFind))
        {
            xmlTE->highlightLine(i);
            m_iLastFindPos = i;
            return i;
        }		
    }
    findLE->setStyleSheet("background: red;");
    return -1;
}

void ShowXML::deb(GString msg)
{
    m_pGDeb->debugMsg("showXML", 1, msg);
}

int ShowXML::checkRefresh()
{
    if( m_pItem )
    {
        if( m_pItem->column() < 0 )
        {
            QMessageBox::information(this, "pmf", "The View has been refreshed. Please close and reopen this window.");
            return 1;
        }
    }
    else
    {
        QMessageBox::information(this, "pmf", "The View has become invalid (because of changes in the background). Please close and reopen this window.");
        return 1;
    }
    return 0;
}
