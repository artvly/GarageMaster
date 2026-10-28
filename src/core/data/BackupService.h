#pragma once

#include <QSqlDatabase>
#include <QString>

namespace gm {

/// Резервное копирование БД: вручную и автоматически при закрытии программы.
///
/// Копия делается командой SQLite `VACUUM INTO`: это согласованный снимок
/// даже в WAL-режиме (простое копирование файла может потерять данные из -wal).
class BackupService
{
public:
    explicit BackupService(QSqlDatabase db);

    /// Создаёт файл garage_YYYY-MM-DD_HH-mm-ss.sqlite в папке dir.
    /// Возвращает путь к копии или пустую строку при ошибке (см. lastError()).
    QString createBackup(const QString& dir);

    /// Оставляет в папке только keep самых свежих копий, остальные удаляет.
    /// Возвращает число удалённых файлов.
    static int rotate(const QString& dir, int keep);

    QString lastError() const { return m_lastError; }

private:
    QSqlDatabase m_db;
    QString m_lastError;
};

} // namespace gm
