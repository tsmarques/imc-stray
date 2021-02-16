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
  resize(300, 300);

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
                    auto [addr, announce] = list.read();
                    if (announce == nullptr)
                      continue;

                    emit list.announceEvent(announce, addr);
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
  minimizeAction->setEnabled(visible);
  maximizeAction->setEnabled(!isMaximized());
  restoreAction->setEnabled(isMaximized() || !visible);
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
  m_contact_list->setColumnCount(2);
  m_contact_list->setHorizontalHeaderLabels({"System", "IPv4"});
  m_contact_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_contact_list->setFocusPolicy(Qt::NoFocus);
  m_contact_list->setSelectionMode(QAbstractItemView::NoSelection);
  m_contact_list->horizontalHeader()->setStretchLastSection(true);
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
    trayIcon->showMessage("Announce", str.c_str(), trayIcon->icon(), 5000);
    addContact(announce, addr);
  }

  m_contacts[announce->sys_name] = announce->getTimeStamp();

  delete announce;
}

void Window::onClose()
{
  std::cout << "closing..." << std::endl;
  should_listen = false;
  listener_thread.join();
}

#endif