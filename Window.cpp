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
#include <QHeaderView>
#include <iostream>

#include <IMC/Base/Packet.hpp>
#include <IMC/Spec/Announce.hpp>

#include "AnnounceListener.hpp"

//! [0]
Window::Window() : m_should_listen(true)
{
  createTable();
  createActions();
  createTrayIcon();

//    connect(showMessageButton, &QAbstractButton::clicked, this, &Window::showMessage);

  connect(m_tray_icon, &QSystemTrayIcon::messageClicked, this, &Window::messageClicked);
  connect(m_tray_icon, &QSystemTrayIcon::activated, this, &Window::iconActivated);
  connect(&m_announce_listener, &SystemListener::announceEvent, this, &Window::on);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_contact_list);
  setLayout(mainLayout);
  m_tray_icon->show();

  setWindowTitle(tr("IMC System Listener"));
  resize(300, 300);

  // @todo handle fail
  m_announce_listener.bind(30100);
  m_listener_thread = std::thread([this]()
              {
                std::cout << "starting listener\n";
                while(this->m_should_listen)
                {
                  try
                  {
                    if (!m_announce_listener.poll(1.0))
                      continue;

                    auto [addr, announce] = m_announce_listener.read();
                    if (announce == nullptr)
                      continue;

                    emit m_announce_listener.announceEvent(announce, addr);
                  } catch(std::runtime_error& e)
                  {
                    std::cerr << e.what() << std::endl;
                  }
                }
              }
  );
}

void Window::init()
{
}


void Window::setVisible(bool visible)
{
  QDialog::setVisible(visible);
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
  m_contact_list->verticalHeader()->setVisible(false);
  m_contact_list->setColumnCount(2);
  m_contact_list->setHorizontalHeaderLabels({"System", "IPv4"});
  m_contact_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_contact_list->setFocusPolicy(Qt::NoFocus);
  m_contact_list->setSelectionMode(QAbstractItemView::NoSelection);
  m_contact_list->horizontalHeader()->setStretchLastSection(true);
}

void Window::createActions()
{
  m_quit_action = new QAction(tr("&Quit"), this);
  connect(m_quit_action, &QAction::triggered, qApp, &QCoreApplication::quit);

  connect(m_quit_action, &QAction::triggered,
          this, &Window::onClose);
}

void Window::createTrayIcon()
{
  m_tray_icon_menu = new QMenu(this);
  m_tray_icon_menu->addSeparator();
  m_tray_icon_menu->addAction(m_quit_action);

  m_tray_icon = new QSystemTrayIcon(this);
  m_tray_icon->setIcon(QIcon(":/assets/disconnected.png"));
  m_tray_icon->setContextMenu(m_tray_icon_menu);
}

void Window::addContact(const IMC::Announce* announce, const QString& addr)
{
  QTableWidgetItem* item_addr = new QTableWidgetItem(addr.toStdString().c_str());
  QTableWidgetItem* item_sysname = new QTableWidgetItem(announce->sys_name.c_str());

  item_addr->setTextAlignment(Qt::AlignCenter);
  item_sysname->setTextAlignment(Qt::AlignCenter);
  m_contact_list->setRowCount(m_contact_list->rowCount() + 1);
  m_contact_list->setItem(m_contact_list->rowCount() - 1, 0, item_sysname);
  m_contact_list->setItem(m_contact_list->rowCount() - 1, 1, item_addr);
  m_contact_list->resizeColumnsToContents();
}

void Window::on(IMC::Announce* announce, QString addr)
{
  std::cout << announce->services << std::endl;

  if (m_contacts.find(announce->sys_name) == m_contacts.end())
  {
    std::string str = "[" + addr.toStdString() + "] " + announce->sys_name;
    m_tray_icon->showMessage("Announce", str.c_str(), m_tray_icon->icon(), 5000);
    addContact(announce, addr);
  }

  m_contacts[announce->sys_name] = announce->getTimeStamp();

  delete announce;
}

void Window::onClose()
{
  std::cout << "closing..." << std::endl;
  m_should_listen = false;
  m_listener_thread.join();
}

#endif