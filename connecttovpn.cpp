#include "connecttovpn.h"
#include "clVPNInterface.h"
#include "abfunctions.h"

#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

connectToVPN* connectToVPN::s_instance = 0;

connectToVPN* connectToVPN::getInstance(){
    if(!s_instance) {
        s_instance = new connectToVPN();
        // There is no instance so we created one.
        return s_instance;
    } else {
        //This is the same instance!
        return s_instance;
    }
}
void connectToVPN::connectVPN(QWidget * parent, QString openVPNCmd, QString ovpnFile, QString authFile){
    m_failed=false;
    if (!clVPNInterface::getInstance()->vpnIsConnected()){
        disconnectVPN();
        //needs super user privileges!!!
        QString program = openVPNCmd + " --config " + ovpnFile + " --auth-user-pass " + authFile;

        p_connect = new QProcess(this);

        connect(p_connect, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_process_finished()));
        //m_issueAlarm=true;

        m_openVPNStatus = new dlgRunCLIApp(parent, p_connect, program, false, false);

        m_currentConnection=ovpnFile.mid(ovpnFile.lastIndexOf("/"));
        //m_connected=true;

    } else {
        qDebug() << "Attempt to connect to VPN was made when VPN already connected";
    }
}

void connectToVPN::on_process_finished(){
    //m_connected=false;
    killWindow();
    m_failed=true;
    //if (m_issueAlarm) emit connectionLost();
}

void connectToVPN::killWindow(){
    if (m_openVPNStatus != nullptr) {
        delete m_openVPNStatus;
        m_openVPNStatus=nullptr;
    }
}
void connectToVPN::disconnectVPN(){
   // m_issueAlarm=false;
    killVPNConnections();
}
void connectToVPN::killVPNConnections(){
    QProcess p;
    QString k = "sudo /usr/bin/killall -2 openvpn" ;
    p.start(k);
    p.waitForFinished();
    killWindow();
    //m_connected=false;
}
void connectToVPN::showStatus(){
   if (m_openVPNStatus != nullptr) {
       m_openVPNStatus->show();
   } else {
       QMessageBox msgBox;
       msgBox.setText("openVPN Not Running");
       msgBox.exec();
   }
}
