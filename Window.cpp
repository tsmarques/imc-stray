#include "Window.hpp"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <iostream>

#include <IMC/Base/Packet.hpp>
#include <IMC/Spec/Announce.hpp>

#include "AnnounceListener.hpp"

//! [0]
Window::Window() :
    should_listen(true)
{
  createTable();
  createActions();
  createTrayIcon();

//    connect(showMessageButton, &QAbstractButton::clicked, this, &Window::showMessage);

  connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &Window::messageClicked);
  connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);
  connect(&list, &SystemListener::announceEvent, this, &Window::on);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_contact_list);
  setLayout(mainLayout);
  trayIcon->show();

  setWindowTitle(tr("IMC System Listener"));
  resize(225, 300);

  // @todo handle fail
  list.bind(30100);
  listener_thread = std::thread([this]()
              {
                std::cout << "starting listener\n";
                while(this->should_listen)
                {
                  try
                  {
                    std::cout << "waiting for announce..\n";
                    IMC::Announce* announce = list.read();
                    if (announce == nullptr)
                      continue;

                    emit list.announceEvent(announce);
                  } catch(std::runtime_error& e)
                  {
                    std::cerr << e.what() << std::endl;
                  }
                }
              }
  );

//    udpSocket = new QUdpSocket(this);
//
////    udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
////    udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
////    int value = 1;
////    setsockopt(udpSocket->socketDescriptor(), SOL_SOCKET, SO_BROADCAST, &value, sizeof(int));
//
//
//
//    //    udpSocket->bind(QHostAddress::LocalHost, 7755);
////    bool ret = udpSocket->bind(30100, QUdpSocket::ShareAddress);
////    if(!ret)
////        std::cout << "failed" << std::endl;
////    udpSocket->bind(QHostAddress::AnyIPv4, 30101, QUdpSocket::ShareAddress);
////    udpSocket->bind(QHostAddress::AnyIPv4, 30102, QUdpSocket::ShareAddress);
////    udpSocket->bind(QHostAddress::AnyIPv4, 30103, QUdpSocket::ShareAddress);
//    if (!udpSocket->bind(QHostAddress::Any, 30101, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
//        std::cout << "nop\n";
//
//    udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(1));
//    connect(udpSocket, &QUdpSocket::readyRead,
//            this, &Window::onNewData);
}

void Window::init()
{
//    delete udpSocket;
//    udpSocket = nullptr;
//
//    udpSocket = new QUdpSocket(this);
//
////    udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
////    udpSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
////    int value = 1;
////    setsockopt(udpSocket->socketDescriptor(), SOL_SOCKET, SO_BROADCAST, &value, sizeof(int));
//
//
//
//    //    udpSocket->bind(QHostAddress::LocalHost, 7755);
////    bool ret = udpSocket->bind(30100, QUdpSocket::ShareAddress);
////    if(!ret)
////        std::cout << "failed" << std::endl;
////    udpSocket->bind(QHostAddress::AnyIPv4, 30101, QUdpSocket::ShareAddress);
////    udpSocket->bind(QHostAddress::AnyIPv4, 30102, QUdpSocket::ShareAddress);
////    udpSocket->bind(QHostAddress::AnyIPv4, 30103, QUdpSocket::ShareAddress);
//    udpSocket->bind(QHostAddress::Any, 30104, QUdpSocket::DontShareAddress);
//
//    for(auto& itf : QNetworkInterface::allInterfaces())
//    {
//        if (itf.type() != QNetworkInterface::InterfaceType::Wifi)
//            continue;
//
//        std::cout << "joining multicast on " << itf.name().constData() << std::endl;
//        udpSocket->setMulticastInterface(itf);
//        udpSocket->joinMulticastGroup(QHostAddress("224.0.75.69"), itf);
//    }
}


void Window::setVisible(bool visible)
{
  minimizeAction->setEnabled(visible);
  maximizeAction->setEnabled(!isMaximized());
  restoreAction->setEnabled(isMaximized() || !visible);
  QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_MACOS
  if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
  if (trayIcon->isVisible()) {
    QMessageBox::information(this, tr("Systray"),
                             tr("The program will keep running in the "
                                "system tray. To terminate the program, "
                                "choose <b>Quit</b> in the context menu "
                                "of the system tray entry."));
    hide();
    event->ignore();
  }
}

void Window::setIcon(int index)
{
//    QIcon icon = iconComboBox->itemIcon(index);
//    trayIcon->setIcon(icon);
//    setWindowIcon(icon);
//
//    trayIcon->setToolTip(iconComboBox->itemText(index));
}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  (void) reason;
  setVisible(!isVisible());
}

void Window::showMessage()
{
//    showIconCheckBox->setChecked(true);
//    int selectedIcon = typeComboBox->itemData(typeComboBox->currentIndex()).toInt();
//    QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(selectedIcon);
//
//    if (selectedIcon == -1) { // custom icon
//        QIcon icon(iconComboBox->itemIcon(iconComboBox->currentIndex()));
//        trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), icon,
//                              durationSpinBox->value() * 1000);
//    } else {
//        trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), msgIcon,
//                              durationSpinBox->value() * 1000);
//    }
}

void Window::messageClicked()
{
  QMessageBox::information(nullptr, tr("Systray"),
                           tr("Sorry, I already gave what help I could.\n"
                              "Maybe you should try asking a human?"));
}


void Window::createTable()
{
  m_contact_list = new QTableWidget;
  m_contact_list->setColumnCount(2);
  m_contact_list->setHorizontalHeaderLabels({"System", "IPv4"});
  m_contact_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Window::createActions()
{
  minimizeAction = new QAction(tr("Mi&nimize"), this);
  connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

  maximizeAction = new QAction(tr("Ma&ximize"), this);
  connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

  quitAction = new QAction(tr("&Quit"), this);
  connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

  connect(quitAction, &QAction::triggered,
          this, &Window::onClose);
}

void Window::createTrayIcon()
{
  trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(minimizeAction);
  trayIconMenu->addAction(maximizeAction);
  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(this);
  trayIcon->setIcon(QIcon(":/assets/disconnected.png"));
  trayIcon->setContextMenu(trayIconMenu);
}

void Window::on(IMC::Announce* announce)
{
  std::cout << announce->services << std::endl;
  trayIcon->showMessage("Announce", "From something", trayIcon->icon(), 5000);

  delete announce;
}

void Window::onClose()
{
  std::cout << "closing..." << std::endl;
  should_listen = false;
  listener_thread.join();
}

#endif