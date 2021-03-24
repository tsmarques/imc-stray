#include "Window.hpp"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <iostream>

#include <IMC/Spec/Announce.hpp>

#include "AnnounceListener.hpp"

Window::Window() :
  m_should_listen(true)
{
  createTable();
  createActions();
  createTrayIcon();

  connect(m_tray_icon, &QSystemTrayIcon::activated, this, &Window::iconActivated);
  connect(&m_announce_listener, &SystemListener::announceEvent, this, &Window::on);
  connect(&m_announce_listener, &SystemListener::checkPurgeEvent, this, &Window::checkDeadSystems);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_contact_list);
  setLayout(mainLayout);
  m_tray_icon->show();

  setWindowTitle(tr("IMC System Listener"));
  resize(300, 300);

  startListener();
}

void
Window::startListener()
{
  if (!m_announce_listener.bind())
  {
    std::cout << "Failed to bind to any port" << std::endl;
    m_tray_icon->setIcon(QIcon(":/assets/disconnected.png"));
    return;
  }

  m_listener_thread = std::thread([this]()
                      {
                        std::cout << "starting listener at " << m_announce_listener.getPort() << std::endl;
                        while(this->m_should_listen)
                        {
                          try
                          {
                            auto curr_t = QDateTime::currentMSecsSinceEpoch() / 1000.0;
                            if (std::abs(curr_t - m_last_purge_time) >= 10.0)
                              emit m_announce_listener.checkPurgeEvent();

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


void Window::setVisible(bool visible)
{
  QDialog::setVisible(visible);
}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
  (void) reason;
  setVisible(!isVisible());
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
  m_tray_icon->setIcon(QIcon(":/assets/dune.png"));
  m_tray_icon->setContextMenu(m_tray_icon_menu);
}

void Window::addContact(const IMC::Announce* announce, const QString& addr)
{
  QTableWidgetItem* item_addr = new QTableWidgetItem(addr.toStdString().c_str());
  QTableWidgetItem* item_sysname = new QTableWidgetItem(announce->sys_name.c_str());

  item_addr->setTextAlignment(Qt::AlignCenter);
  item_sysname->setTextAlignment(Qt::AlignCenter);
  m_sys2row[announce->sys_name] = m_contact_list->rowCount();
  m_contact_list->setRowCount(m_contact_list->rowCount() + 1);
  m_contact_list->setItem(m_contact_list->rowCount() - 1, 0, item_sysname);
  m_contact_list->setItem(m_contact_list->rowCount() - 1, 1, item_addr);
  m_contact_list->resizeColumnsToContents();
}

void Window::removeContact(const std::string& sysname)
{
  if (m_sys2row.find(sysname) == m_sys2row.end())
  {
    std::cout << "error: " << sysname << " not known" << std::endl;
  }

  m_contact_list->removeRow(m_sys2row[sysname]);
  m_contact_list->resizeColumnsToContents();

  m_contacts.erase(sysname);
  m_sys2row.erase(sysname);
}

void Window::on(IMC::Announce* announce, QString addr)
{
  std::scoped_lock lock(m_contacts_lock);

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

void Window::checkDeadSystems()
{
  std::cout << "checking for inactive contacts" << std::endl;
  std::scoped_lock lock(m_contacts_lock);

  std::vector<std::string> dead_sys;
  auto curr_time_s = QDateTime::currentSecsSinceEpoch();
  for (auto [sys, time] : m_contacts)
  {
    if (std::abs(curr_time_s - time) >= 10.0)
    {
      m_tray_icon->showMessage("Purge", sys.c_str(), m_tray_icon->icon(), 5000);
      dead_sys.push_back(sys);
    }
  }

  // remove dead systems
  for (const std::string& dead : dead_sys)
    removeContact(dead);

  m_last_purge_time = QDateTime::currentMSecsSinceEpoch() / 1000.0;
}

void Window::onClose()
{
  std::cout << "closing..." << std::endl;
  m_should_listen = false;
  if (m_listener_thread.joinable())
    m_listener_thread.join();
}

#endif