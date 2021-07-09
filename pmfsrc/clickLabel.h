#include <QLabel>
#include <QMessageBox>
#include <gstring.hpp>

#ifndef CLICKLABEL_H
#define CLICKLABEL_H

class ClickLabel: public QLabel
{
    Q_OBJECT
    public:
        explicit ClickLabel( const QString& text="", QWidget* parent=0 );
        void setDisplayText(GString txt);
        ~ClickLabel();
    signals:
        void clicked();
    protected:
        void mousePressEvent(QMouseEvent* event);

    private:
        QWidget *_parent;
        GString _txt;

    private slots:
        void wasClicked();
};

#endif

