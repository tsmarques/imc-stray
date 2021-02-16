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
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QUdpSocket;
QT_END_NAMESPACE

//! [0]
class Window : public QDialog
{
Q_OBJECT
public:
  Window();
  void init();

  void setVisible(bool visible) override;

private slots:
  void setIcon(int index);
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void showMessage();
  void messageClicked();
  void on(IMC::Announce* announce, QString addr);

  void onClose();

private:
  void createTable();
  void createActions();
  void createTrayIcon();
  void addContact(const IMC::Announce* announce, const QString& addr);

  QTableWidget* m_contact_list;

  QAction* m_minimize_action;
  QAction* m_maximize_action;
  QAction* m_restore_action;
  QAction* m_quit_action;

  QSystemTrayIcon* m_tray_icon;
  QMenu* m_tray_icon_menu;

  std::atomic<bool> m_should_listen;
  SystemListener m_announce_listener;
  std::thread m_listener_thread;
  std::map<std::string, double> m_contacts;
};
//! [0]

#endif // QT_NO_SYSTEMTRAYICON

#endif