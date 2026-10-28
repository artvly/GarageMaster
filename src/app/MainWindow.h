#pragma once

#include <QMainWindow>

namespace gm { class Database; }

/// Главное окно. Пока — каркас: разделы-вкладки, меню и строка состояния.
/// Каждая вкладка будет заменена своим виджетом (ClientsView, OrdersView, ...).
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(gm::Database& db, QString backupDir, QWidget* parent = nullptr);

private slots:
    void makeBackup();
    void showAbout();

private:
    void buildMenu();
    void buildTabs();

    gm::Database& m_db;
    QString m_backupDir;
};
