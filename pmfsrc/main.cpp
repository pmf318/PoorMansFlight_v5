
#include <qglobal.h>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QDir>
#include <QSettings>
#include <QDesktopWidget>

#include "pmf.h"
#include "helper.h"
#include <gfile.hpp>
#include <gstuff.hpp>
#include "pmfdefines.h"
#include <qvarlengtharray.h>
#include <QStyleFactory>
#include <QMessageBox>

#ifndef PMF_VER
#define PMF_VER "[No Version]"
#endif


#ifndef max
  #define max(A,B) ((A) >(B) ? (A):(B))
#endif

#ifndef min
  #define min(A,B) ((A) <(B) ? (A):(B))
#endif

int readInitFile(int * x, int * y, int * w, int *h);
void saveInitFile(int x, int y, int w, int h);
void checkGeometry(Pmf * mainApp);
QRect getDefaultGeometry();
GString getArgValue(int argc, char** argv, GString key);
int setDarkPalette();

QPalette pmfDarkPalette;

int _isBlocked = 0;

class myEventFilter: public QObject
{
    Pmf* _pmf;
    QApplication * _pApp;
public:
    myEventFilter(Pmf* pPmf):QObject()
    {
        _pmf = pPmf;
    }
    myEventFilter(QApplication* pApp, Pmf* pPmf):QObject()
    {
        _pApp = pApp;
        _pmf = pPmf;
    }

    ~myEventFilter(){}

    bool eventFilter(QObject* object,QEvent* event)
    {

        if( object != _pApp ) return false;
        if( event->type() == 214 ) return false;

        if(event->type() == QEvent::WindowBlocked && object == _pApp)
        {
            _isBlocked = 1;
        }
        if(event->type() == QEvent::WindowUnblocked && object == _pApp)
        {
            _isBlocked = 0;
        }

//        if( object == _pApp ) printf("event->type(): %i\n",event->type());


        if(event->type() == QEvent::ApplicationDeactivate && !_isBlocked && object == _pApp)
        {
            if( event->type() == QEvent::WindowBlocked ) return true;
            if( event->type() == QEvent::WindowUnblocked ) return true;

//            printf("WE'VE BEEN DEACTIVATED\n");
//            if(_pmf->isActiveWindow())printf("pmf ACT\n");
//            else printf("pmf not act\n");
            return true;
        }
        if(event->type() == QEvent::ApplicationActivate && !_isBlocked  && object == _pApp)
        {
            if( event->type() == QEvent::WindowBlocked ) return true;
            if( event->type() == QEvent::WindowUnblocked ) return true;

//            printf("WE'VE BEEN ACTIVATED\n");
//            if(_pmf->isActiveWindow())printf("pmf ACT\n");
//            else printf("pmf not act\n");
            //_pmf->curTabChg(0);

            return true;
        }

        if(event->type() == QEvent::ActivationChange && event->type() == QEvent::FocusIn)
        {
            //printf("ACT CHG \n");
        }
        //if( event->type() == QEvent::FocusIn ) printf("FOCUS IN\n");
        //if( _pmf == object )
        //if( _pApp == (QApplication *)object)
        {
            //printf("IS QAPP\n");
            //return QObject::eventFilter(object,event);

            /*
            if( event->type() == QEvent::FocusIn ) printf("IS QAPP & FOCUS IN\n");
            if( event->type() == QEvent::FocusOut ) printf("IS QAPP & FOCUS OUT\n");
            if( event->type() == QEvent::WindowActivate )
            {
                //if( this == object)
                {
                    printf("PMF Activate\n");
                }
                return true;
            }
            if( event->type() == QEvent::WindowDeactivate )
            {
                printf("PMF DeActivate\n");
            }
            */
        }
        return QObject::eventFilter(object,event);
    }
};

int main(int argc, char** argv)
{
#ifdef MAKE_VC
LPDWORD lpOldMode = 0;
SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
SetThreadErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX, lpOldMode);
#endif
    QSettings settings(_CFG_DIR, "pmf5");
    if( settings.value("font", -1).toInt() >= 0 )
    {
        QFont f;
        f.fromString(settings.value("font").toString());
        QApplication::setFont(f);
    }
	QApplication app(argc, argv);
	#ifdef MAKE_VC
	QApplication::setStyle("plastique");
	#else
	QApplication::setStyle("plastique");
	#endif	

    QString message;

    int debugMode = 0;
    int threaded = 0;
    GString arg;
    GString logFile = "";

	for( int i = 1; i < argc; ++i )
	{
        arg   = GString(argv[i]);
        if( arg.occurrencesOf("?") || GString(arg).upperCase().occurrencesOf("HELP") )
		{
            printf(" optional params: \n");
            printf(" debug=n \n");
			printf("          n=0: debugging disabled (default)\n");
			printf("          n=1: enable debugging for base classes (mainly SQL)\n");
			printf("          n=2: enable debugging for GUI\n");
			printf("          n=3: enable debugging for base and GUI\n");
            printf("          \n");
            printf(" logFile=<fileName>\n");
            printf("          path+file for debug messages. Only valid if debugging is enabled.\n");
            printf("         \n");
            printf(" Example: pmf debug=3 logFile=someDebugFile\n");
            printf("         \n");
            printf(" --> debugging will slow down the program considerably. <--\n");
            printf(" \n");
			return 1;
		}
	}
    debugMode = getArgValue(argc, argv, "DEBUG").asInt();
    threaded = getArgValue(argc, argv, "THREADED").asInt();
    logFile  = getArgValue(argc, argv, "LOGFILE");
    if( Helper::askFileOverwrite(NULL, logFile, "Debug file exists. Overwrite?") ) return 0;

    if( logFile.length() )
    {
        remove(logFile);
        FILE *f;
        if(( f = fopen( logFile, "w" )) == NULL )
        {
            QMessageBox::information(0, "PMF", "Cannot create logFile");
            return 1;
        }
        else fclose(f);
        remove(logFile);
    }
    GDebug* pGDeb = GDebug::getGDebug(debugMode, logFile);



	message = "Hit a massive bug. If you can reproduce this, drop me a mail. PMF will crash now.";
	try {
        pGDeb->debugMsg("main", 1, "Creating pmf...");


        Pmf pmf(pGDeb, threaded);

        app.installEventFilter(new myEventFilter(&app, &pmf));
        //qApp->installEventFilter(new myEventFilter(qApp, &foo));
        pmfDarkPalette = qApp->palette(&pmf);
        int res = setDarkPalette();
        pmf.setColorScheme(res, pmfDarkPalette);
//        foo.connect(qApp, SIGNAL(applicationStateChanged(Qt::ApplicationState)), SLOT(onApplicationStateChanged(Qt::ApplicationState)));
        pGDeb->debugMsg("main", 1, "Creating pmf...OK");
		GString ver = GString(PMF_VER);
        pGDeb->debugMsg("main", 1, "version: "+ver);
		if( ver.length() != 4 || !ver.isDigits() ) ver = "[No Version]";
		else 
		{
            ver = ver.insert(GString("."), ver.length() - 1);
            ver = ver.insert(GString("."), 2);
		}
        pmf.setWindowTitle(Helper::pmfNameAndVersion());
		
        pGDeb->debugMsg("main", 1, "Get settings...");

        if( settings.value("geometry", -1).toInt() < 0 )
        {
            printf("Can not restore geometry\n");
            pmf.setGeometry(getDefaultGeometry());
        }
        else
        {
            pmf.restoreGeometry(settings.value("geometry").toByteArray());
        }
        //This is rather ham-fisted, alas we cannot get a QRect from "settings.value("geometry").toByteArray()"
        //but as a safe guard, this will do.
        checkGeometry(&pmf);

        QColor col =  pmf.palette().color(pmf.backgroundRole()); //.color(QWidget::backgroundRole());
        QString prevStyle = settings.value("style", "").toString();
        //if( prevStyle.length() == 0  )
        {
            if( col.red() < 99 && col.green() < 99 && col.blue() < 99 )pmf.setColorScheme(0, pmfDarkPalette);
        }

        pmf.show();
#ifndef MAKE_VC
        pmf.loginClicked();
#endif		
        pGDeb->debugMsg("main", 1, "calling exec()");
        //foo.setPalette( fusionDarkPalette );
        /*
        QList<QWidget*> widgets = foo.findChildren<QWidget*>();
         foreach (QWidget* w, widgets)
         {
               w->setPalette(fusionDarkPalette);
        }
         */        
        app.exec();
        pGDeb->debugMsg("main", 1, "app done.");
        settings.setValue("geometry", pmf.saveGeometry());
        //settings.setValue("font", pmf.font().toString());
        //settings.remove("font");

	}catch(...){QMessageBox::information(NULL, "PMF", message);}
    pGDeb->debugMsg("main", 1, "FINAL.");
	return 0;	
}
GString getArgValue(int argc, char** argv, GString key)
{
    GString arg, in, ret;
    for( int i = 1; i < argc; ++i )
    {
        arg  = GString(argv[i]);
        in   = GString(argv[i]).upperCase();
        if( in.occurrencesOf(key) )
        {
            ret = arg.subString(key.length()+1, arg.length()).strip().strip('=');
            return ret;
        }
    }
    return "";
}

void checkGeometry(Pmf * mainApp)
{

    //In case something went very wrong, we try and set a sensible geometry
    QDesktopWidget widget;
    QRect screen = widget.availableGeometry(widget.primaryScreen());
    QRect current = mainApp->rect();

//    printf("checkAndSet geometry: screenW: %i, curW: %i\n", screen.width(), current.width());
//    printf("checkAndSet geometry: screenH: %i, curH: %i\n", screen.height(), current.height());
    int w = min(screen.width(), current.width());
    int h = min(screen.height(), current.height());
    int x = mainApp->pos().x();
    int y = max(mainApp->pos().y(), 30); //In some cases (GTK+) the title bar's height appears to be missing
    //printf("checkAndSet geometry final: w: %i, h: %i\n", w, h);
    mainApp->setGeometry(x, y, w, h);
}

QRect getDefaultGeometry()
{
    int x = 60;
    int y = 60;
    int w = 680;
    int h = 480;

    QDesktopWidget widget;
    QRect screen = widget.availableGeometry(widget.primaryScreen());
    h = max(h, screen.height()-y);
    w = max(w, screen.width()-x);
    return QRect(x, y, w, h);
}


int readInitFile(int * px, int * py, int * pw, int * ph)
{
    QString home = QDir::homePath ();
	if( !home.length() ) return 1;
	GString initFile = GString(home)+"\\"+_CFG_DIR+"\\"+_INIT_FILE; 	
	GFile f(initFile);	
	if( !f.initOK() ) return 2;
	int x = f.getKey("POSX").asInt();
	int y = f.getKey("POSY").asInt();
	int w = f.getKey("WIDTH").asInt();
	int h = f.getKey("HEIGHT").asInt();
	//if( 0 == x && 0 == y || ( 0 == x+y ) ) return 3;	
	*px = x < 0 ? 0 : x;
	*py = y < 0 ? 0 : y;
	*pw = w < 0 ? 0 : w;
	*ph = h < 0 ? 0 : h;
	
	return 0;
}

void saveInitFile(int x, int y, int w, int h)
{
	QString home = QDir::homePath ();
	if( !home.length() ) return;
	GString initFile = GString(home)+"\\"+_CFG_DIR+"\\"+_INIT_FILE; 	
	GFile f(initFile, GF_APPENDCREATE);
	if( !f.initOK() ) return;
	GSeq <GString> aSeq;
	aSeq.add("POSX="+GString(x));
	aSeq.add("POSY="+GString(y));	
	aSeq.add("WIDTH="+GString(w));	
	aSeq.add("HEIGHT="+GString(h));	
	f.overwrite(&aSeq);
}

int setDarkPalette()
{

    QSettings settings(_CFG_DIR, "pmf5");


    QString prevStyle = settings.value("style", "<notSet>").toString();
    if( !GString(prevStyle).occurrencesOf(_DARK_THEME))
    {        
        return 1;
    }
    GString prev = prevStyle;
    qApp->setStyle(QStyleFactory::create(prev.subString(1, prev.indexOf(_DARK_THEME)-1)));

    qApp->setStyle(QStyleFactory::create(prevStyle));


    pmfDarkPalette.setColor(QPalette::Window, QColor(53,53,53));
    pmfDarkPalette.setColor(QPalette::WindowText, Qt::white);
    pmfDarkPalette.setColor(QPalette::Base, QColor(25,25,25));
    pmfDarkPalette.setColor(QPalette::AlternateBase, QColor(51, 51, 51));
    pmfDarkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    pmfDarkPalette.setColor(QPalette::ToolTipText, Qt::white);
    pmfDarkPalette.setColor(QPalette::Text, Qt::white);
    pmfDarkPalette.setColor(QPalette::Button, QColor(53,53,53));
    pmfDarkPalette.setColor(QPalette::ButtonText, Qt::white);
    pmfDarkPalette.setColor(QPalette::BrightText, Qt::red);

    pmfDarkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    pmfDarkPalette.setColor(QPalette::Disabled, QPalette::Light, QColor(0, 0, 0));
    pmfDarkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(152, 152, 152));

    pmfDarkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    pmfDarkPalette.setColor(QPalette::HighlightedText, Qt::black);
    pmfDarkPalette.setColor(QPalette::Midlight,QColor(102, 153, 0));

    qApp->setPalette(pmfDarkPalette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    return 0;
}
