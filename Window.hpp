#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>

#ifndef QT_NO_SYSTEMTRAYICON

#include <QDialog>
#include <QTableView>
#include <QTableWidget>

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
QT_END_NAMESPACE

//! [0]
class Window : public QDialog
{
Q_OBJECT

public:
    Window();

    void setVisible(bool visible) override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void setIcon(int index);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMessage();
    void messageClicked();

private:
    void createTable();
    void createActions();
    void createTrayIcon();

    QTableWidget* m_contact_list;

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
};
//! [0]

#endif // QT_NO_SYSTEMTRAYICON

#endif