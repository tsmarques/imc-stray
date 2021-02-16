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

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

  std::atomic<bool> should_listen;
  SystemListener list;
  std::thread listener_thread;
  std::map<std::string, double> m_contacts;
};
//! [0]

#endif // QT_NO_SYSTEMTRAYICON

#endif