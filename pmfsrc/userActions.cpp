#include "userActions.h"
#include "pmfdefines.h"
#include "userInput.h"

#include <QLabel>


UserActions::UserActions(GDebug* pGDeb, TabEdit *parent)
      :QDialog(parent)
{
    m_pGDeb = pGDeb;
    m_Master = parent;

    QBoxLayout *topLayout = new QVBoxLayout(this);
    QGridLayout * grid = new QGridLayout( );
    topLayout->addLayout(grid, 2);
    this->resize(840, 480);
    this->setWindowTitle("User defined actions");

    QLabel * pLB = new QLabel(this);
    pLB->setText("Editor for context-menu actions. Use -- (double dash) for comments.");
    QLabel * pLB2 = new QLabel(this);
    pLB2->setText("Hit CTRL+E for text completion.");

    QLabel * pLB3 = new QLabel(this);
    pLB3->setText("Existing actions ");
    listCB = new QComboBox();
    listCB->setFixedHeight( listCB->sizeHint().height() );
    connect(listCB, SIGNAL(activated(int)), SLOT(actionSelected(int)));


    saveBt = new QPushButton(this);
    saveBt->setText("Save");
    connect(saveBt, SIGNAL(clicked()), SLOT(saveClicked()));

    saveAsBt = new QPushButton(this);
    saveAsBt->setText("Save As...");
    connect(saveAsBt, SIGNAL(clicked()), SLOT(saveAsClicked()));

    deleteBt = new QPushButton(this);
    deleteBt->setText("Delete");
    connect(deleteBt, SIGNAL(clicked()), SLOT(deleteClicked()));

    exitBt = new QPushButton(this);
    exitBt->setText("Exit");
    connect(exitBt, SIGNAL(clicked()), SLOT(exitClicked()));

    editor = new TxtEdit(pGDeb, this);

    editor ->setGeometry( 20, 20, 920, 300);
    /*** Add to grid ***/
    grid->addWidget(pLB, 0, 0, 1, 5);
    grid->addWidget(pLB2, 1, 0, 1, 5);
    grid->addWidget(pLB3, 2, 0, 1, 1);
    grid->addWidget(listCB, 2, 1, 1, 3);

    grid->addWidget(editor, 3, 0, 1, 5);

    grid->addWidget(saveBt, 4, 0);
    grid->addWidget(saveAsBt, 4, 1);
    grid->addWidget(deleteBt, 4, 2);
    grid->addWidget(exitBt, 4, 3);

    setHelp();
    m_Master->initTxtEditor(editor);

    m_curFile = _newStringCB;
    loadActions();
}
void UserActions::setHelp()
{
    GString s = "-- Here you can add new actions to the right-click menu.\n";
    s += "-- An action is an SQL command that will run when right-clicking a cell.\n";
    s += "-- You can use place-holders in actions: \n";
    s += "--    @DATA@:  The contents of the right-clicked cell\n";
    s += "--    @COL@:   The column name of the cell\n";
    s += "--    @TABLE@: The current table\n";
    s += "-- The place-holders are case-sensitive. \n";
    s += "--  \n";
    s += "-- Examples: \n";
    s += "--   Select count(*) from @TABLE@\n";
    s += "--   Select * from myTable where @COL@=@DATA@\n";
    s += "--   select distinct(someColumn) from myTable where @COL@=@DATA@\n";
    s += "--   select @COL@, COUNT(*) as Counter from @TABLE@ group by @COL@ having count(*) > 0 order by @COL@\n";
    s += "-- Be careful with UPDATE and DELETE as actions will run whenever you select them.\n";
    s += "-- Actions will appear alphabetically sorted in the right-click menu under \"More...\"\n";
    s += "-- They are stored locally in "+userActionsDir();

    editor->clear();
#if QT_VERSION >= 0x050300
    //editor->setPlaceholderText(s);
    editor->setText("\n\n"+s);
#else
    editor->setText("\n\n"+s);
#endif
    editor->setFocus();
    m_curText = GString(editor->toPlainText());
}

void UserActions::loadActions()
{
    listCB->clear();

    listCB->addItem(_newStringCB);
    QDir dir(userActionsDir());
    QStringList files = dir.entryList(QDir::Files);
    for(int i = 0; i < files.size() && i < MaxUserActions; ++i)
    {
        listCB->addItem(files[i]);
    }
    setComboBox(m_curFile);
}

void UserActions::saveClicked()
{    
    if(!currentFile().length()) saveAsClicked();
    else if( !saveToFile(currentFile()) ) m_curText = editor->toPlainText();
}
GString UserActions::currentFile()
{
    return m_curFile;
}
int UserActions::saveToFile(GString name)
{

    name = userActionsDir() + name;
    QFile f(name);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        msg("Could not open File '"+GString(name)+"', you could try 'SaveAs...'");
        return 1;
    }
    f.write(GString(editor->toPlainText()));
    f.close();
    return 0;
}

int UserActions::saveAsClicked()
{
    UserInput *ui = new UserInput(this, "Save action", "Action name: ", "Actions are sorted by name", "[Stored in: "+userActionsDir()+"]");
    ui->exec();
    for(int i = 0; i < listCB->count(); ++i)
    {
        if( GString(listCB->itemText(i)).upperCase() == ui->data().upperCase() )
        {
            if( QMessageBox::question(this, "Editor", "Action exists. Overwrite?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return 1;
            break;
        }
    }
    if( !ui->data().length() ) return 1;
    int rc = saveToFile(ui->data());
    if( !rc )
    {
        m_curText = editor->toPlainText();
        m_curFile =  ui->data();
    }
    loadActions();
    return rc;
}
void UserActions::deleteClicked()
{
    if( QMessageBox::question(this, "Editor", "Delete this action? "+currentFile(), QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;
    remove(userActionsDir() + currentFile());
    setComboBox(_newStringCB);
    loadActions();
    setHelp();
}
void UserActions::exitClicked()
{
    if( m_curText != GString(editor->toPlainText()) )
    {
        if( QMessageBox::question(this, "Editor", "Exit without saving?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;
    }
    close();
}
int UserActions::saveCurrentAction()
{
    int rc = 0;
    if( m_curText != GString(editor->toPlainText()) )
    {
        GString trg;
        if( _newStringCB != m_curFile ) trg = "Save changes in '"+m_curFile+"'?";
        else trg = "Save changes?";


        if( QMessageBox::question(this, "Editor", trg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::No )
        {
            if( _newStringCB != m_curFile ) rc = saveToFile(m_curFile);
            else rc = saveAsClicked();
        }
        setComboBox(m_curFile);
    }
    return rc;
}
void UserActions::setComboBox(GString val)
{
    int index = listCB->findText(val);
    if ( index != -1 )
    {
       listCB->setCurrentIndex(index);
    }
}

void UserActions::actionSelected(int pos)
{    

    if( saveCurrentAction() )
    {
        setComboBox(m_curFile);
        return;
    }

    m_curFile = GString(listCB->itemText(pos));
    setComboBox(m_curFile);
    if(_newStringCB == GString(listCB->itemText(pos)))
    {
        setHelp();
        return;
    }
    GString file = userActionsDir() + m_curFile;


    QFile f(file);
    if( !f.open(QIODevice::ReadOnly ))
    {
        GString txt = "Cannot open file "+GString(file);
        QMessageBox::information(this, "Editor", txt);
        return;
    }
    editor->setText(f.readAll());
    f.close();

    m_curText = editor->toPlainText();

    if( _newStringCB == GString(listCB->currentText())) m_curFile = "";

}
void UserActions::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            exitClicked();
            break;
        default:
            QWidget::keyPressEvent(event);

    }
}
