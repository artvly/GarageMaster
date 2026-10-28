#include "data/Database.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>

namespace gm {

namespace {

struct Migration {
    int version;
    QString resourcePath;
};

/// Список миграций из ресурсов, отсортированный по номеру.
QList<Migration> availableMigrations()
{
    static const QRegularExpression re(QStringLiteral(R"(^(\d+)_.*\.sql$)"));
    QList<Migration> result;
    const QDir dir(QStringLiteral(":/migrations"));
    for (const QString& name : dir.entryList({QStringLiteral("*.sql")}, QDir::Files)) {
        const auto m = re.match(name);
        if (m.hasMatch())
            result.append({m.captured(1).toInt(), dir.filePath(name)});
    }
    std::sort(result.begin(), result.end(),
              [](const Migration& a, const Migration& b) { return a.version < b.version; });
    return result;
}

} // namespace

Database::Database(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

Database::~Database()
{
    close();
}

bool Database::open(const QString& filePath)
{
    close();
    m_filePath = filePath;
    m_lastError.clear();

    if (!QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE")))
        return fail(QStringLiteral("Драйвер QSQLITE недоступен"));

    const QFileInfo info(filePath);
    if (!info.absoluteDir().exists() && !QDir().mkpath(info.absolutePath()))
        return fail(QStringLiteral("Не удалось создать папку %1").arg(info.absolutePath()));

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
        db.setDatabaseName(filePath);
        if (!db.open())
            return fail(QStringLiteral("Не удалось открыть БД: %1").arg(db.lastError().text()));
    }

    if (!configure() || !migrate()) {
        const QString error = m_lastError;
        close();
        m_lastError = error;
        return false;
    }
    return true;
}

void Database::close()
{
    if (!QSqlDatabase::contains(m_connectionName))
        return;
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName, false);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::isOpen() const
{
    return QSqlDatabase::contains(m_connectionName)
        && QSqlDatabase::database(m_connectionName, false).isOpen();
}

QSqlDatabase Database::connection() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

int Database::schemaVersion() const
{
    QSqlQuery q(connection());
    if (q.exec(QStringLiteral("PRAGMA user_version")) && q.next())
        return q.value(0).toInt();
    return -1;
}

int Database::latestSchemaVersion()
{
    const auto migrations = availableMigrations();
    return migrations.isEmpty() ? 0 : migrations.last().version;
}

QStringList Database::splitScript(const QString& script)
{
    QStringList statements;
    QString current;
    const QStringList lines = script.split(QChar('\n'));
    for (QString line : lines) {
        line.remove(QChar('\r'));
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QStringLiteral("--")))
            continue;
        current += line + QChar('\n');
        if (trimmed.endsWith(QChar(';'))) {
            statements << current.trimmed();
            current.clear();
        }
    }
    if (!current.trimmed().isEmpty())
        statements << current.trimmed();
    return statements;
}

bool Database::configure()
{
    QSqlQuery q(connection());
    // Ссылочная целостность (в SQLite по умолчанию выключена — включаем на каждое подключение).
    if (!q.exec(QStringLiteral("PRAGMA foreign_keys = ON")))
        return fail(q.lastError().text());
    // WAL: при аварийном завершении файл БД не повреждается.
    if (!q.exec(QStringLiteral("PRAGMA journal_mode = WAL")))
        return fail(q.lastError().text());
    if (!q.exec(QStringLiteral("PRAGMA synchronous = NORMAL")))
        return fail(q.lastError().text());
    if (!q.exec(QStringLiteral("PRAGMA busy_timeout = 3000")))
        return fail(q.lastError().text());
    return true;
}

bool Database::migrate()
{
    QSqlDatabase db = connection();
    const int current = schemaVersion();
    if (current < 0)
        return fail(QStringLiteral("Не удалось прочитать версию схемы"));
    if (current > latestSchemaVersion())
        return fail(QStringLiteral("Файл БД создан более новой версией программы (схема %1, поддерживается %2)")
                        .arg(current).arg(latestSchemaVersion()));

    for (const Migration& migration : availableMigrations()) {
        if (migration.version <= current)
            continue;

        QFile file(migration.resourcePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return fail(QStringLiteral("Не найдена миграция %1").arg(migration.resourcePath));
        const QStringList statements = splitScript(QString::fromUtf8(file.readAll()));

        if (!db.transaction())
            return fail(db.lastError().text());

        QSqlQuery q(db);
        for (const QString& sql : statements) {
            if (!q.exec(sql)) {
                const QString error = q.lastError().text();
                db.rollback();
                return fail(QStringLiteral("Миграция %1: %2\n%3").arg(migration.version).arg(error, sql));
            }
        }
        // PRAGMA не принимает параметры, номер подставляется из имени файла (только цифры).
        if (!q.exec(QStringLiteral("PRAGMA user_version = %1").arg(migration.version))) {
            const QString error = q.lastError().text();
            db.rollback();
            return fail(error);
        }
        if (!db.commit())
            return fail(db.lastError().text());
    }
    return true;
}

bool Database::fail(const QString& message)
{
    m_lastError = message;
    return false;
}

} // namespace gm
