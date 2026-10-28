#pragma once

#include <QSqlDatabase>
#include <QString>

namespace gm::bootstrap {

/// Логин и пароль администратора, создаваемого при первом запуске.
/// Пароль помечается must_change_password = 1 — его нужно сменить при первом входе.
inline const QString kDefaultAdminLogin = QStringLiteral("admin");
inline const QString kDefaultAdminPassword = QStringLiteral("admin");

/// Если в таблице users нет ни одного пользователя — создаёт администратора по умолчанию.
/// Возвращает true, если пользователь был создан.
bool ensureDefaultAdmin(QSqlDatabase db, QString* error = nullptr);

} // namespace gm::bootstrap
