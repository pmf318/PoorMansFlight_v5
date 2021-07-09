#include <QTableWidgetItem>

#include "gstring.hpp"

#ifndef PMFTABLEITEM_H
#define PMFTABLEITEM_H


class PmfTableItem : public QTableWidgetItem
{

public:
    explicit PmfTableItem() : QTableWidgetItem(){}
    explicit PmfTableItem(int type) : QTableWidgetItem(type){}
    explicit PmfTableItem(const QString &text, int type = Type): QTableWidgetItem(text, type ){}
    explicit PmfTableItem(const QIcon &icon, const QString &text, int type = Type) : QTableWidgetItem(icon, text, type){}

    PmfTableItem(const QTableWidgetItem &other) :  QTableWidgetItem(other){}


    bool operator< (const QTableWidgetItem &other) const
    {
        GString a, b;
        a = GString(this->text()).removeAll('.');
        b = GString(other.text()).removeAll('.');

        if( a.isDigits() && b.isDigits() ) return a.asLongLong() < b.asLongLong();
        return a < b;
    }

private:
    void deb(GString msg);
};

#endif
