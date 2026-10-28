#pragma once

#include <QString>

namespace gm {

/// Хеширование паролей: PBKDF2-HMAC-SHA256 со случайной солью.
/// В БД хранятся только соль и хеш (hex), сам пароль — никогда.
class PasswordHasher
{
public:
    struct Hashed {
        QString hash; // hex
        QString salt; // hex
    };

    static constexpr int kIterations = 100'000;
    static constexpr int kSaltBytes = 16;
    static constexpr int kHashBytes = 32;

    static Hashed hash(const QString& password);
    static bool verify(const QString& password, const QString& hashHex, const QString& saltHex);
};

} // namespace gm
