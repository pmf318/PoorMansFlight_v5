
#include "userInput.h"

#include <QVBoxLayout>
#include <QLabel>


UserInput::UserInput(QWidget *parent, GString title, GString label, GString head, GString foot)
    :QDialog(parent)
{
    this->setWindowTitle(title);
    QVBoxLayout *topLayout = new QVBoxLayout( );

    QGridLayout *grid = new QGridLayout(this);
    topLayout->addLayout( grid, 10 );

    QLabel* tmpQLabel;
    QLabel* headQLabel;
    QLabel* footQLabel;

    headQLabel = new QLabel( this );
    headQLabel->setText( head );

    footQLabel = new QLabel( this );
    footQLabel->setText( foot );

    tmpQLabel = new QLabel( this );
    tmpQLabel->setText( label );
    tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );



//    grid->setColumnStretch(1, 100);
//    grid->setColumnStretch(2, 100);
//    grid->setColumnStretch(0, 0);



    okB = new QPushButton();
    connect( okB, SIGNAL(clicked()), SLOT(okClicked()) );
    okB->setText( "OK" );
    okB->setAutoRepeat( false );
    okB->setDefault(true);
    okB->setFixedHeight( okB->sizeHint().height() );
    cancelB = new QPushButton();
    connect( cancelB, SIGNAL(clicked()), SLOT(cancelClicked()) );
    cancelB->setText( "Cancel" );
    cancelB->setAutoRepeat( false );
    cancelB->setFixedHeight( cancelB->sizeHint().height() );

    grid->addWidget(headQLabel, 0, 0, 1, 3);
    grid->addWidget(tmpQLabel, 1, 0);
    dataLE = new QLineEdit( this );
    grid->addWidget(dataLE, 1, 1, 1, 3);
    grid->addWidget(footQLabel, 2, 0, 1, 3);
    grid->addWidget(okB, 3, 0);
    grid->addWidget(cancelB, 3, 1);



    setGeometry(parent->pos().rx()+parent->width()/2 - 185,parent->pos().ry()+30, 370, 160);
}

void UserInput::cancelClicked()
{
    m_strSaveFile = "";
    close();
}
void UserInput::okClicked()
{
    m_strSaveFile = dataLE->text();
    close();
}
GString UserInput::data()
{
    return m_strSaveFile;
}
