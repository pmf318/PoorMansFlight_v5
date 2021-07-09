//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#ifndef _xmlEdit_
#define _xmlEdit_

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
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QColor>




class XmlEdit : public QTextEdit
{

	class XmlHighlighter : public QSyntaxHighlighter
	{
		public:
            XmlHighlighter(QObject* parent, int useColorScheme );
            XmlHighlighter(QTextDocument* parent, int useColorScheme );
            XmlHighlighter(QTextEdit* parent, int useColorScheme );
			~XmlHighlighter();
		
			enum HighlightType
			{
				SyntaxChar,
				ElementName,
				Comment,
				AttributeName,
				AttributeValue,
				Error,
				Other
			};
		
			void setHighlightColor(HighlightType type, QColor color, bool foreground = true);
			void setHighlightFormat(HighlightType type, QTextCharFormat format);
			void setParent(QWidget *parent);
		
		protected:
			void highlightBlock(const QString& rstrText);
			int  processDefaultText(int i, const QString& rstrText);


		private:
            void init(int useColorScheme );
			void msg(GString txt);
		
			QTextCharFormat fmtSyntaxChar;
			QTextCharFormat fmtElementName;
			QTextCharFormat fmtComment;
			QTextCharFormat fmtAttributeName;
			QTextCharFormat fmtAttributeValue;
			QTextCharFormat fmtError;
			QTextCharFormat fmtOther;
		
			enum ParsingState
			{
				NoState = 0,
				ExpectElementNameOrSlash,
				ExpectElementName,
				ExpectAttributeOrEndOfElement,
				ExpectEqual,
				ExpectAttributeValue
			};		
			enum BlockState
			{
				NoBlock = -1,
				InComment,
				InElement
			};		
			ParsingState state;
			QWidget *m_parent;
	}; 	
public:
    XmlEdit(QWidget *parent, int colorScheme);
	XmlHighlighter * pXmlHighlighter;
	bool Conform();
	GString checkSyntax();
	void highlightLine(int line);
	void setCursorPosition( unsigned int line, unsigned int pos  = 0);
    QString selectedText();
    QString partialXml(int requireNode);
    int cursorPosition();
    QString text();
    void scrollToTop();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void mousePressEvent(QMouseEvent *mouseEvent);
    void mouseDoubleClickEvent(QMouseEvent *e);


private:

		
	QWidget *m_parent;
    int m_iColorScheme;
	void msg(GString txt);
	/*
	
	//QDomDocument xml_document();
	void setPlainText( const QString txt );
	void Syntaxcheck();
	//void contextMenuEvent ( QContextMenuEvent * e );
	//QMenu *createOwnStandardContextMenu();
	bool event( QEvent *event );
	*/
};
#endif

