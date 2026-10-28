#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QStringList>

namespace gm {

/// Владеет подключением к SQLite: открывает файл, включает нужные PRAGMA
/// и накатывает миграции из ресурсов (:/migrations/NNN_*.sql).
///
/// Версия схемы хранится в PRAGMA user_version: миграция с номером N
/// применяется, если user_version < N. Каждая миграция — одна транзакция.
class Database
{
public:
    /// connectionName — имя подключения в QSqlDatabase (для тестов можно задать своё).
    explicit Database(QString connectionName = QStringLiteral("garage_master"));
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /// Открывает (или создаёт) файл БД и приводит схему к актуальной версии.
    /// При ошибке возвращает false, текст ошибки — в lastError().
    bool open(const QString& filePath);
    void close();

    bool isOpen() const;
    QSqlDatabase connection() const;
    QString filePath() const { return m_filePath; }
    QString lastError() const { return m_lastError; }

    /// Текущая версия схемы (PRAGMA user_version).
    int schemaVersion() const;

    /// Последняя версия схемы, известная программе.
    static int latestSchemaVersion();

    /// Делит SQL-скрипт на отдельные операторы (по ';' в конце строки),
    /// пропуская строки-комментарии. QSqlQuery исполняет только один оператор за раз.
    static QStringList splitScript(const QString& script);

private:
    bool configure();
    bool migrate();
    bool fail(const QString& message);

    QString m_connectionName;
    QString m_filePath;
    QString m_lastError;
};

} // namespace gm
