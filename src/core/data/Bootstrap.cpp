#include "data/Bootstrap.h"

#include "security/PasswordHasher.h"

#include <QSqlError>
#include <QSqlQuery>

namespace gm::bootstrap {

bool ensureDefaultAdmin(QSqlDatabase db, QString* error)
{
    QSqlQuery q(db);
    if (!q.exec(QStringLiteral("SELECT COUNT(*) FROM users")) || !q.next()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    if (q.value(0).toInt() > 0)
        return false;

    const auto hashed = PasswordHasher::hash(kDefaultAdminPassword);
    q.prepare(QStringLiteral(
        "INSERT INTO users (login, full_name, password_hash, password_salt, role, must_change_password) "
        "VALUES (:login, :name, :hash, :salt, 'admin', 1)"));
    q.bindValue(QStringLiteral(":login"), kDefaultAdminLogin);
    q.bindValue(QStringLiteral(":name"), QStringLiteral("Администратор"));
    q.bindValue(QStringLiteral(":hash"), hashed.hash);
    q.bindValue(QStringLiteral(":salt"), hashed.salt);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    return true;
}

} // namespace gm::bootstrap
