#include "dlgruncliapp.h"
#include "abfunctions.h"
#include "ui_dlgruncliapp.h"
#include <QDebug>
#include <QSettings>
#include <QTextStream>
#include <QScreen>
#include <QDate>
#include <QFile>
#include <QStandardPaths>
#include <QCoreApplication>

dlgRunCLIApp::dlgRunCLIApp(QWidget *parent, QProcess *process, QString command, bool userCanAbort, bool runDetached, QStringList abort) :
    QDialog(parent),
    ui(new Ui::dlgRunCLIApp)
{
    ui->setupUi(this);
    ui->hideWindow->setVisible(!userCanAbort);
    ui->abortProcess->setVisible(userCanAbort);
    ui->output->addItem(command);

    //Put window in centre of screen

    // Get current screen size
    QScreen *screen= QGuiApplication::screens().at(0);
    QSize rec =screen->availableSize();

    // Using base size of window
    QSize size = this->baseSize();

    // Set top left point
    QPoint topLeft = QPoint((rec.width() / 2) - (size.width() / 2), (rec.height() / 2) - (size.height() / 2));

    // set window position
    setGeometry(QRect(topLeft, size));
    m_abort=abort;
    m_process = process;
    connect(m_process,SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(saveLog()));
    if (runDetached){
        m_process->startDetached(command);
    } else {
        m_process->start(command);
    }
}

dlgRunCLIApp::~dlgRunCLIApp()
{
    delete ui;
}
void dlgRunCLIApp::closeEvent(QCloseEvent *event) {
    saveLog();
}
void dlgRunCLIApp::readyReadStandardOutput(){
    m_process->setReadChannel(QProcess::StandardOutput);
    QTextStream stream(m_process);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        ui->output->addItem(line);
        abortProcess(line);
    }
}
void dlgRunCLIApp::abortProcess(QString line){
    if (!aborting){
        for (QString item : m_abort){
            if (line.contains(item, Qt::CaseInsensitive)){
                abortProcess();
                break;
            }
        }
    }
}
void dlgRunCLIApp::abortProcess(){
    aborting=true;
    saveLog();
    m_process->kill();
    emit abort();
    m_process->waitForFinished();
    ui->output->addItem("Process terminated");
}

void dlgRunCLIApp::saveLog(){
    if (!logSaved){
        logSaved=true;
        QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        path = path + "/" + QCoreApplication::organizationName() + "/logs/abStartupManager";
        if (abFunctions::mkdir(path)){
            path = path + "/" + QDate::currentDate().toString("yyMMdd") + ".log";
            QFile f(path);
            if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
                QString txt = "...\n" ;
                f.write(txt.toUtf8());
                for(int i = 0; i < ui->output->count(); ++i)
                {
                    QListWidgetItem* item = ui->output->item(i);
                    txt = item->text() + "\n";
                    f.write(txt.toUtf8());
                }
                f.close();
            }
        }
    }
}


