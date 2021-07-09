#include "clickLabel.h"


ClickLabel::ClickLabel(const QString& text, QWidget* parent) : QLabel(parent)
{
    QFont f = font();
    f.setUnderline(true);
    setFont(f);
    setText(text);
    setStyleSheet("QLabel { color : blue; }");
    setAlignment(Qt::AlignRight);
    connect(this, SIGNAL(clicked()), SLOT(wasClicked()));
    _parent = parent;

}

void ClickLabel::setDisplayText(GString txt)
{
    _txt = txt;
}

void ClickLabel::wasClicked()
{
    if(_txt.length())  QMessageBox::information(_parent, "pmf", _txt);
}

ClickLabel::~ClickLabel()
{
}

void ClickLabel::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
}
