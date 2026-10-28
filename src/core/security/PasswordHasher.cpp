#include "security/PasswordHasher.h"

#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QRandomGenerator>

namespace gm {

namespace {

QByteArray derive(const QString& password, const QByteArray& salt)
{
    return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), salt,
                                              PasswordHasher::kIterations, PasswordHasher::kHashBytes);
}

/// Сравнение за постоянное время — не выдаёт по времени, сколько байт совпало.
bool constantTimeEquals(const QByteArray& a, const QByteArray& b)
{
    if (a.size() != b.size())
        return false;
    unsigned char diff = 0;
    for (int i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

} // namespace

PasswordHasher::Hashed PasswordHasher::hash(const QString& password)
{
    QByteArray salt(kSaltBytes, Qt::Uninitialized);
    QRandomGenerator::system()->fillRange(reinterpret_cast<quint32*>(salt.data()), kSaltBytes / 4);
    return {QString::fromLatin1(derive(password, salt).toHex()), QString::fromLatin1(salt.toHex())};
}

bool PasswordHasher::verify(const QString& password, const QString& hashHex, const QString& saltHex)
{
    const QByteArray salt = QByteArray::fromHex(saltHex.toLatin1());
    const QByteArray expected = QByteArray::fromHex(hashHex.toLatin1());
    if (salt.isEmpty() || expected.isEmpty())
        return false;
    return constantTimeEquals(derive(password, salt), expected);
}

} // namespace gm
