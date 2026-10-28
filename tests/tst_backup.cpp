#include "data/BackupService.h"
#include "data/Database.h"

#include <QDir>
#include <QFile>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

class TestBackup : public QObject
{
    Q_OBJECT

private slots:
    void backupContainsData()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString backupDir = dir.filePath(QStringLiteral("backups"));

        QString backupPath;
        {
            gm::Database db(QStringLiteral("src"));
            QVERIFY(db.open(dir.filePath(QStringLiteral("main.sqlite"))));
            QSqlQuery q(db.connection());
            QVERIFY(q.exec(QStringLiteral("INSERT INTO clients (full_name) VALUES ('Клиент из копии')")));

            gm::BackupService backup(db.connection());
            backupPath = backup.createBackup(backupDir);
            QVERIFY2(!backupPath.isEmpty(), qPrintable(backup.lastError()));
            QVERIFY(QFile::exists(backupPath));
        }

        // Копия открывается как обычная БД и содержит данные
        gm::Database copy(QStringLiteral("copy"));
        QVERIFY2(copy.open(backupPath), qPrintable(copy.lastError()));
        QSqlQuery q(copy.connection());
        QVERIFY(q.exec(QStringLiteral("SELECT full_name FROM clients")));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toString(), QStringLiteral("Клиент из копии"));
    }

    void rotateKeepsNewest()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QStringList names = {
            QStringLiteral("garage_2026-01-01_10-00-00.sqlite"),
            QStringLiteral("garage_2026-01-02_10-00-00.sqlite"),
            QStringLiteral("garage_2026-01-03_10-00-00.sqlite"),
            QStringLiteral("garage_2026-01-04_10-00-00.sqlite"),
            QStringLiteral("unrelated.txt"),
        };
        for (const QString& n : names) {
            QFile f(dir.filePath(n));
            QVERIFY(f.open(QIODevice::WriteOnly));
        }

        QCOMPARE(gm::BackupService::rotate(dir.path(), 2), 2);
        const QStringList left = QDir(dir.path()).entryList(QDir::Files, QDir::Name);
        QCOMPARE(left, QStringList({QStringLiteral("garage_2026-01-03_10-00-00.sqlite"),
                                    QStringLiteral("garage_2026-01-04_10-00-00.sqlite"),
                                    QStringLiteral("unrelated.txt")}));
    }
};

QTEST_GUILESS_MAIN(TestBackup)
#include "tst_backup.moc"
