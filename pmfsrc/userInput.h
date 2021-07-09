#ifndef USERINPUT_H
#define USERINPUT_H


#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDialog>
#include <QMessageBox>



#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include "gstring.hpp"

class UserInput : public QDialog
{
    Q_OBJECT
private:
    QPushButton * okB;
    QPushButton *cancelB;
    GString m_strSaveFile;
    QLineEdit * dataLE;

public:
    UserInput(QWidget *parent = 0, GString title = "", GString label = "", GString head = "", GString foot = "");
    GString data();

private slots:
    void cancelClicked();
    void okClicked();

};

#endif // USERINPUT_H
