#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>

#ifndef QT_NO_SYSTEMTRAYICON

#include "AnnounceListener.hpp"
#include <IMC/Spec/Announce.hpp>
#include <QDialog>
#include <QTableView>
#include <QTableWidget>
#include <QtNetwork>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
QT_END_NAMESPACE

class Window : public QDialog
{
Q_OBJECT
public:
  Window();
  void setVisible(bool visible) override;

private slots:
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void on(IMC::Announce* announce, QString addr);
  void onClose();

private:
  void createTable();
  void createActions();
  void createTrayIcon();

  //! Add IMC contact after receiving Annunce message
  void addContact(const IMC::Announce* announce, const QString& addr);

  //! Active IMC contacts
  QTableWidget* m_contact_list;
  //! Close aoo
  QAction* m_quit_action;
  //! System tray Icon
  QSystemTrayIcon* m_tray_icon;
  //! Tray menu
  QMenu* m_tray_icon_menu;
  //! Flag to signal Announce listener when to stop
  std::atomic<bool> m_should_listen;
  //! Announce Listener
  SystemListener m_announce_listener;
  //! Announce Listener thread
  std::thread m_listener_thread;
  //! Map between system's name and its last announce timestamp
  std::map<std::string, double> m_contacts;
};

#endif

#endif