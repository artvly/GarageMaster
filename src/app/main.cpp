#include "MainWindow.h"

#include "data/BackupService.h"
#include "data/Bootstrap.h"
#include "data/Database.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("GarageMaster"));
    QApplication::setApplicationName(QStringLiteral("GarageMaster"));
    QApplication::setApplicationDisplayName(QStringLiteral("Гараж Мастер"));
    QApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION_STRING));

    // %APPDATA%\GarageMaster\GarageMaster\ в Windows
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString dbPath = QDir(dataDir).filePath(QStringLiteral("garage.sqlite"));
    const QString backupDir = QDir(dataDir).filePath(QStringLiteral("backups"));

    gm::Database db;
    if (!db.open(dbPath)) {
        QMessageBox::critical(nullptr, QApplication::applicationDisplayName(),
                              QStringLiteral("Не удалось открыть базу данных:\n%1").arg(db.lastError()));
        return 1;
    }

    QString error;
    gm::bootstrap::ensureDefaultAdmin(db.connection(), &error);
    if (!error.isEmpty()) {
        QMessageBox::critical(nullptr, QApplication::applicationDisplayName(), error);
        return 1;
    }

    // TODO(этап 1): окно входа LoginDialog перед главным окном.

    MainWindow window(db, backupDir);
    window.show();
    const int rc = QApplication::exec();

    // Автоматическая резервная копия при закрытии (ТЗ, функции администратора).
    gm::BackupService backup(db.connection());
    if (!backup.createBackup(backupDir).isEmpty())
        gm::BackupService::rotate(backupDir, 10);

    return rc;
}
