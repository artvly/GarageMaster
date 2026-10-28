#include "security/PasswordHasher.h"

#include <QtTest>

using gm::PasswordHasher;

class TestSecurity : public QObject
{
    Q_OBJECT

private slots:
    void verifiesCorrectPassword()
    {
        const auto h = PasswordHasher::hash(QStringLiteral("Секрет-123"));
        QCOMPARE(h.hash.size(), PasswordHasher::kHashBytes * 2);
        QCOMPARE(h.salt.size(), PasswordHasher::kSaltBytes * 2);
        QVERIFY(PasswordHasher::verify(QStringLiteral("Секрет-123"), h.hash, h.salt));
    }

    void rejectsWrongPassword()
    {
        const auto h = PasswordHasher::hash(QStringLiteral("admin"));
        QVERIFY(!PasswordHasher::verify(QStringLiteral("Admin"), h.hash, h.salt));
        QVERIFY(!PasswordHasher::verify(QString(), h.hash, h.salt));
        QVERIFY(!PasswordHasher::verify(QStringLiteral("admin"), h.hash, QString()));
    }

    void saltsAreUnique()
    {
        const auto a = PasswordHasher::hash(QStringLiteral("same"));
        const auto b = PasswordHasher::hash(QStringLiteral("same"));
        QVERIFY(a.salt != b.salt);
        QVERIFY(a.hash != b.hash);
    }
};

QTEST_GUILESS_MAIN(TestSecurity)
#include "tst_security.moc"
