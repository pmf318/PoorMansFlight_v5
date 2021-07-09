/*
  This file is part of WCC++, the WebContent C++ Framework.

  WCC++ is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  WCC++ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with WCC++.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "xmlEdit.h"
#include <QDomDocument>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>
#include <idsql.hpp>
#include <QScrollBar>


#include "helper.h"

static const QColor DEFAULT_SYNTAX_CHAR		= Qt::blue;
static const QColor DEFAULT_ELEMENT_NAME	= Qt::darkRed;
static const QColor DEFAULT_COMMENT			= Qt::darkGreen;
static const QColor DEFAULT_ATTRIBUTE_NAME	= Qt::red;
static const QColor DEFAULT_ATTRIBUTE_VALUE	= Qt::blue;
static const QColor DEFAULT_ERROR			= Qt::darkMagenta;
static const QColor DEFAULT_OTHER			= Qt::black;

// Regular expressions for parsing XML borrowed from:
// http://www.cs.sfu.ca/~cameron/REX.html
static const QString EXPR_COMMENT			= "<!--[^-]*-([^-][^-]*-)*->";
static const QString EXPR_COMMENT_BEGIN		= "<!--";
static const QString EXPR_COMMENT_END		= "[^-]*-([^-][^-]*-)*->";
static const QString EXPR_ATTRIBUTE_VALUE	= "\"[^<\"]*\"|'[^<']*'";
static const QString EXPR_NAME				= "([A-Za-z_:]|[^\\x00-\\x7F])([A-Za-z0-9_:.-]|[^\\x00-\\x7F])*"; 


XmlEdit::XmlHighlighter::XmlHighlighter(QObject* parent, int useColorScheme )
: QSyntaxHighlighter(parent)
{
    init(useColorScheme);
}

XmlEdit::XmlHighlighter::XmlHighlighter(QTextDocument* parent, int useColorScheme )
: QSyntaxHighlighter(parent)
{
    init(useColorScheme);
}

XmlEdit::XmlHighlighter::XmlHighlighter(QTextEdit* parent, int useColorScheme )
: QSyntaxHighlighter(parent)
{
    init(useColorScheme);
}

XmlEdit::XmlHighlighter::~XmlHighlighter()
{
}

void XmlEdit::XmlHighlighter::init(int useColorScheme)
{    
	m_parent = NULL;
    if( !useColorScheme ) return;
	fmtSyntaxChar.setForeground(DEFAULT_SYNTAX_CHAR);
    fmtElementName.setForeground(DEFAULT_ELEMENT_NAME);
	fmtComment.setForeground(DEFAULT_COMMENT);
	fmtAttributeName.setForeground(DEFAULT_ATTRIBUTE_NAME);
	fmtAttributeValue.setForeground(DEFAULT_ATTRIBUTE_VALUE);
	fmtError.setForeground(DEFAULT_ERROR);
	fmtOther.setForeground(DEFAULT_OTHER);
}

void XmlEdit::XmlHighlighter::setHighlightColor(HighlightType type, QColor color, bool foreground)
{
	QTextCharFormat format;
	if (foreground)	format.setForeground(color);
	else format.setBackground(color);
	setHighlightFormat(type, format);
}

void XmlEdit::XmlHighlighter::setHighlightFormat(HighlightType type, QTextCharFormat format)
{
	switch (type)
	{
		case SyntaxChar:
			fmtSyntaxChar = format;
			break;
		case ElementName:
			fmtElementName = format;
			break;
		case Comment:
			fmtComment = format;
			break;
		case AttributeName:
			fmtAttributeName = format;
			break;
		case AttributeValue:
			fmtAttributeValue = format;
			break;
		case Error:
			fmtError = format;
			break;
		case Other:
			fmtOther = format;
			break;
	}
	rehighlight();
}

void XmlEdit::XmlHighlighter::highlightBlock(const QString& text)
{
	
	int i = 0;
	int pos = 0;
	int brackets = 0;
	
	state = (previousBlockState() == InElement ? ExpectAttributeOrEndOfElement : NoState);

	if (previousBlockState() == InComment)
	{
		// search for the end of the comment
		QRegExp expression(EXPR_COMMENT_END);
		pos = expression.indexIn(text, i);

		if (pos >= 0)
		{
			// end comment found
			const int iLength = expression.matchedLength();
			setFormat(0, iLength - 3, fmtComment);
			setFormat(iLength - 3, 3, fmtSyntaxChar);
			i += iLength; // skip comment
		}
		else
		{
			// in comment
			setFormat(0, text.length(), fmtComment);
			setCurrentBlockState(InComment);
			return;
		}
	}

	for (; i < text.length(); i++)
	{
		if( text.at(i) == '<') {
			brackets++;
			if (brackets == 1)
			{
				setFormat(i, 1, fmtSyntaxChar);
				state = ExpectElementNameOrSlash;
			}
			else
			{
				// wrong bracket nesting
				setFormat(i, 1, fmtError);
			}
		}
		else if( text.at(i) == '>') {
			brackets--;
			if (brackets == 0)
			{
				setFormat(i, 1, fmtSyntaxChar);
			}
			else
			{
				// wrong bracket nesting
				setFormat( i, 1, fmtError);
			}
			state = NoState;
		}
		else if( text.at(i) == '/') {
			if (state == ExpectElementNameOrSlash)
			{
				state = ExpectElementName;
				setFormat(i, 1, fmtSyntaxChar);
			}
			else
			{
				if (state == ExpectAttributeOrEndOfElement)
				{
					setFormat(i, 1, fmtSyntaxChar);
				}
				else
				{
					processDefaultText(i, text);
				}
			}
		}
		else if( text.at(i) == '=') {		
			if (state == ExpectEqual)
			{
				state = ExpectAttributeValue;
				setFormat(i, 1, fmtOther);
			}
			else
			{
				processDefaultText(i, text);  
			}
		}
		else if( text.at(i) == '\'' || text.at(i) == '\"') {
			if (state == ExpectAttributeValue)
			{
				// search attribute value
				QRegExp expression(EXPR_ATTRIBUTE_VALUE);
				pos = expression.indexIn(text, i);

				if (pos == i) // attribute value found ?
				{
					const int iLength = expression.matchedLength();

					setFormat(i, 1, fmtOther);
					setFormat(i + 1, iLength - 2, fmtAttributeValue);
					setFormat(i + iLength - 1, 1, fmtOther);

					i += iLength - 1; // skip attribute value
					state = ExpectAttributeOrEndOfElement;
				}
				else
				{
					processDefaultText(i, text);
				}
			}
			else
			{
				processDefaultText(i, text);
			}
		}
		else if( text.at(i) == '!') {
			if (state == ExpectElementNameOrSlash)
			{
				// search comment
				QRegExp expression(EXPR_COMMENT);
				pos = expression.indexIn(text, i - 1);

				if (pos == i - 1) // comment found ?
				{
					const int iLength = expression.matchedLength();

					setFormat(pos, 4, fmtSyntaxChar);
					setFormat(pos + 4, iLength - 7, fmtComment);
					setFormat(iLength - 3, 3, fmtSyntaxChar);
					i += iLength - 2; // skip comment
					state = NoState;
					brackets--;
				}
				else
				{
					// Try find multiline comment
					QRegExp expression(EXPR_COMMENT_BEGIN); // search comment start
					pos = expression.indexIn(text, i - 1);

					//if (pos == i - 1) // comment found ?
					if (pos >= i - 1)
					{
						setFormat(i, 3, fmtSyntaxChar);
						setFormat(i + 3, text.length() - i - 3, fmtComment);
						setCurrentBlockState(InComment);
						return;
					}
					else
					{
						processDefaultText(i, text);
					}
				}
			}
			else
			{
				processDefaultText(i, text);
			}
		}
		else{
			const int iLength = processDefaultText(i, text);
			if (iLength > 0)
				i += iLength - 1;
		}
	}

	if (state == ExpectAttributeOrEndOfElement)
	{
		setCurrentBlockState(InElement);
	}
}

int XmlEdit::XmlHighlighter::processDefaultText(int i, const QString& text)
{
	// length of matched text
	int iLength = 0;

	switch(state)
	{
	case ExpectElementNameOrSlash:
	case ExpectElementName:
		{
			// search element name
			QRegExp expression(EXPR_NAME);
			const int pos = expression.indexIn(text, i);

			if (pos == i) // found ?
			{
				iLength = expression.matchedLength();

				setFormat(pos, iLength, fmtElementName);
				state = ExpectAttributeOrEndOfElement;
			}
			else
			{
				setFormat(i, 1, fmtOther);
			}
		}  
		break;

	case ExpectAttributeOrEndOfElement:
		{
			// search attribute name
			QRegExp expression(EXPR_NAME);
			const int pos = expression.indexIn(text, i);

			if (pos == i) // found ?
			{
				iLength = expression.matchedLength();

				setFormat(pos, iLength, fmtAttributeName);
				state = ExpectEqual;
			}
			else
			{
				setFormat(i, 1, fmtOther);
			}
		}
		break;

	default:
		setFormat(i, 1, fmtOther);
		break;
	}
	return iLength;
}   
void XmlEdit::XmlHighlighter::msg(GString txt)
{
	if( m_parent ) Helper::msgBox(m_parent, "xmlEdit", txt);
}
void XmlEdit::XmlHighlighter::setParent(QWidget * parent)
{	
	m_parent = parent;
}


/************************************************************************
*
* Class xmlEdit
* 
************************************************************************/


XmlEdit::XmlEdit(QWidget *parent, int colorScheme)
{
  m_parent = parent;
  m_iColorScheme = colorScheme;
  //setViewportMargins(50, 0, 0, 0);
  pXmlHighlighter = new XmlHighlighter(document(), m_iColorScheme);
  pXmlHighlighter->setParent(parent);
  setLineWrapMode ( QTextEdit::NoWrap );
  setAcceptRichText ( false );
  
  //connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
  //connect(this, SIGNAL(textChanged()), this, SLOT(update()));

}


bool XmlEdit::Conform()
{
  QString errorStr;
  int errorLine, errorColumn;
  QDomDocument doc;
  return doc.setContent(text(),false, &errorStr, &errorLine, &errorColumn);
}
QString XmlEdit::text()
{
	return toPlainText();
}
/*
QDomDocument xmlEdit::xml_document()
{
  QString errorStr;
  int errorLine, errorColumn;
  QDomDocument doc;
  doc.setContent(text(),false, &errorStr, &errorLine, &errorColumn);
  return doc;
}
*/
/*
void xmlEdit::setPlainText( const QString txt )
{
  QString errorStr;
  int errorLine, errorColumn;
  QDomDocument doc;
  if (!doc.setContent(txt,false, &errorStr, &errorLine, &errorColumn)) {
    QTextEdit::setPlainText(txt);
  } else {
    QTextEdit::setPlainText(doc.toString(5));
  }
}
*/

int XmlEdit::cursorPosition()
{
    QTextCursor cursor = textCursor();
    return cursor.position();
}

QString XmlEdit::selectedText()
{    
    QTextCursor cursor = textCursor();    
    return cursor.selectedText();
}

QString XmlEdit::partialXml(int requireNode)
{
    QTextCursor cursor = textCursor();
    GString selection = selectedText();
    if( !selection.length() ) return "";
    int pos = cursor.position();
    int startTagPos = pos - selection.length() - 1; //Search "<" in front of selection
    if( startTagPos >= 0 && requireNode )
    {
        if( text()[startTagPos] != '<' || text()[startTagPos+1] == '/' )
        {
            //msg("Right-click a start-node (start-nodes have '<' in front)");
            return "";
        }
    }


    int end1 = text().indexOf(">", pos);
    int end2 = text().indexOf("</", pos);
    int endPos = pmf_min(end1, end2);
    QString qtxt = text().mid(0, endPos )+"/>";
    GString s = qtxt;
    return qtxt;
}

void XmlEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    QTextCursor textCursor = cursorForPosition(e->pos());
    textCursor.select(QTextCursor::WordUnderCursor);
    setTextCursor(textCursor);
    //textCursor.selectedText();
    GString selection = selectedText();

    int endPos = textCursor.position();
    int startPos = endPos - selection.length() - 1; //Search "<" in front of selection
    while( startPos >= 0 )
    {
        if( text()[startPos] == '<') break;
        if( text()[startPos] == ' ' || text()[startPos] == '>')
        {
            textCursor.clearSelection();
            return;
        }
        startPos --;
    }
    while( endPos < text().length() )
    {
        if( text()[endPos] == '>' || text()[endPos] == ' ' || text()[endPos] == '/') break;
        endPos++;
    }
    textCursor.setPosition(startPos+1);
    textCursor.setPosition(endPos, QTextCursor::KeepAnchor);
    this->setTextCursor(textCursor);
    //QTextEdit::mouseDoubleClickEvent(e);
}

void XmlEdit::mousePressEvent(QMouseEvent *mouseEvent)
{

    if (Qt::RightButton == mouseEvent->button())
    {
        QTextCursor textCursor = cursorForPosition(mouseEvent->pos());
        textCursor.select(QTextCursor::WordUnderCursor);
        setTextCursor(textCursor);
        //textCursor.selectedText();
        GString selection = selectedText();

        int endPos = textCursor.position();
        int startPos = endPos - selection.length() - 1; //Search "<" in front of selection
        while( startPos >= 0 )
        {
            if( text()[startPos] == '<') break;
            if( text()[startPos] == ' ' || text()[startPos] == '>')
            {
                textCursor.clearSelection();
                return;
            }
            startPos --;
        }
        while( endPos < text().length() )
        {
            if( text()[endPos] == '>' || text()[endPos] == ' ' || text()[endPos] == '/') break;
            endPos++;
        }
        textCursor.setPosition(startPos+1);
        textCursor.setPosition(endPos, QTextCursor::KeepAnchor);
        this->setTextCursor(textCursor);
    }

    QTextEdit::mousePressEvent(mouseEvent);
}


GString XmlEdit::checkSyntax()
{
  if (text().size() > 0 )
  {
    QString errorStr;
    int errorLine, errorColumn;
    QDomDocument doc;
	
    if (!doc.setContent(text(),false, &errorStr, &errorLine, &errorColumn)) {
        //GString line = document()->findBlockByLineNumber( errorLine-1 ).text();
		//setCursorPosition(errorLine-1, errorColumn-1);
		highlightLine(errorLine-1);
		return GString(errorStr)+" at line "+GString(errorLine-1);      
    } else {
        highlightLine(-1);
		return "Syntax is valid.";		
    }
  }  
  return "";
}

void XmlEdit::highlightLine(int line)
{

	QList<QTextEdit::ExtraSelection> extras;
	QTextEdit::ExtraSelection highlight;
	if( line >= 0 ) 
	{	
		setCursorPosition(line);
		highlight.cursor = textCursor();
        highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
        highlight.format.setBackground( Qt::lightGray );
	}
	extras << highlight;
	setExtraSelections( extras );
}

void XmlEdit::setCursorPosition( unsigned int line, unsigned int pos )
{
    PMF_UNUSED(pos);
	QTextCursor cursor = textCursor();
	cursor.movePosition( QTextCursor::Start );
	cursor.movePosition( QTextCursor::Down, QTextCursor::MoveAnchor, line  );
	setTextCursor(cursor);
}


void XmlEdit::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
void XmlEdit::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}
void XmlEdit::dropEvent(QDropEvent *event)
{
    const QMimeData* pMimeData = event->mimeData();
    QList< QUrl > urlList = pMimeData->urls();

    if ( !pMimeData->hasUrls() )
    {
        QMessageBox::information(this, "pmf", "This is not a file.");
        return;
    }
    //event->acceptProposedAction();
    if( urlList.size() > 1 )
    {
        QMessageBox::information(this, "pmf", "Cannot process more than one file at a time.");
        return;
    }
    foreach (QUrl fileName, urlList) {
        QFile file(fileName.toLocalFile());
        file.open(QFile::ReadOnly | QFile::Text);

        QTextStream ReadFile(&file);
        this->setText(ReadFile.readAll());

    }
    /*
    QFile file(urlList.at(0).toLocalFile());
    file.open(QFile::ReadOnly | QFile::Text);

    QTextStream ReadFile(&file);
    this->setText(ReadFile.readAll());
    */
}

void XmlEdit::scrollToTop()
{
    moveCursor (QTextCursor::Start) ;
    ensureCursorVisible() ;
}

void XmlEdit::msg(GString txt)
{
	Helper::msgBox(m_parent, "xmlEdit", txt);
}
