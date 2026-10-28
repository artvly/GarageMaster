#include "data/BackupService.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSqlError>
#include <QSqlQuery>

namespace gm {

namespace {
const QString kPrefix = QStringLiteral("garage_");
const QString kSuffix = QStringLiteral(".sqlite");
} // namespace

BackupService::BackupService(QSqlDatabase db)
    : m_db(std::move(db))
{
}

QString BackupService::createBackup(const QString& dir)
{
    m_lastError.clear();
    if (!QDir().mkpath(dir)) {
        m_lastError = QStringLiteral("Не удалось создать папку %1").arg(dir);
        return {};
    }

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd_HH-mm-ss"));
    QString path = QDir(dir).filePath(kPrefix + stamp + kSuffix);
    // Две копии в одну секунду (ручная + при закрытии) — добавляем суффикс.
    for (int i = 2; QFile::exists(path); ++i)
        path = QDir(dir).filePath(kPrefix + stamp + QStringLiteral("_%1").arg(i) + kSuffix);

    QSqlQuery q(m_db);
    // VACUUM INTO не принимает путь параметром — экранируем кавычки вручную.
    QString escaped = QDir::toNativeSeparators(path);
    escaped.replace(QChar('\''), QStringLiteral("''"));
    if (!q.exec(QStringLiteral("VACUUM INTO '%1'").arg(escaped))) {
        m_lastError = q.lastError().text();
        return {};
    }
    return path;
}

int BackupService::rotate(const QString& dir, int keep)
{
    if (keep < 1)
        keep = 1;
    // Имена содержат дату в формате yyyy-MM-dd_HH-mm-ss, поэтому сортировка
    // по имени по убыванию = от новых к старым.
    const QFileInfoList files = QDir(dir).entryInfoList({kPrefix + QStringLiteral("*") + kSuffix},
                                                        QDir::Files, QDir::Name | QDir::Reversed);
    int removed = 0;
    for (int i = keep; i < files.size(); ++i) {
        if (QFile::remove(files[i].absoluteFilePath()))
            ++removed;
    }
    return removed;
}

} // namespace gm
