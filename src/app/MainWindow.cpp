#include "MainWindow.h"

#include "data/BackupService.h"
#include "data/Database.h"

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabWidget>

MainWindow::MainWindow(gm::Database& db, QString backupDir, QWidget* parent)
    : QMainWindow(parent)
    , m_db(db)
    , m_backupDir(std::move(backupDir))
{
    setWindowTitle(QApplication::applicationDisplayName());
    resize(1200, 760);
    buildMenu();
    buildTabs();
    statusBar()->showMessage(QStringLiteral("БД: %1 (схема v%2)")
                                 .arg(m_db.filePath())
                                 .arg(m_db.schemaVersion()));
}

void MainWindow::buildMenu()
{
    QMenu* file = menuBar()->addMenu(QStringLiteral("&Файл"));
    file->addAction(QStringLiteral("Создать резервную копию…"), this, &MainWindow::makeBackup);
    file->addSeparator();
    QAction* quit = file->addAction(QStringLiteral("Выход"), this, &QWidget::close);
    quit->setShortcut(QKeySequence::Quit);

    QMenu* help = menuBar()->addMenu(QStringLiteral("&Справка"));
    help->addAction(QStringLiteral("О программе"), this, &MainWindow::showAbout);
}

void MainWindow::buildTabs()
{
    auto* tabs = new QTabWidget(this);
    const QStringList sections = {
        QStringLiteral("Заказ-наряды"),
        QStringLiteral("Клиенты и автомобили"),
        QStringLiteral("Каталог услуг"),
        QStringLiteral("Склад"),
        QStringLiteral("Отчёты"),
        QStringLiteral("Пользователи"),
    };
    for (const QString& title : sections) {
        auto* placeholder = new QLabel(QStringLiteral("Раздел «%1» — в разработке").arg(title));
        placeholder->setAlignment(Qt::AlignCenter);
        tabs->addTab(placeholder, title);
    }
    setCentralWidget(tabs);
}

void MainWindow::makeBackup()
{
    const QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Папка для резервной копии"),
                                                          m_backupDir);
    if (dir.isEmpty())
        return;

    gm::BackupService backup(m_db.connection());
    const QString path = backup.createBackup(dir);
    if (path.isEmpty())
        QMessageBox::warning(this, windowTitle(),
                             QStringLiteral("Не удалось создать копию:\n%1").arg(backup.lastError()));
    else
        statusBar()->showMessage(QStringLiteral("Резервная копия сохранена: %1").arg(path), 8000);
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, QStringLiteral("О программе"),
                       QStringLiteral("<b>Гараж Мастер</b> %1<br>"
                                      "Учёт заказ-нарядов, клиентов и запчастей для СТО.<br><br>"
                                      "Курсовая работа, МГТУ им. Н.Э. Баумана, ИУ6.<br>"
                                      "Лицензия MIT.")
                           .arg(QApplication::applicationVersion()));
}
