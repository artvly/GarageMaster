#include "domain/Entities.h"
#include "domain/Money.h"

#include <QtTest>

namespace money = gm::money;

class TestMoney : public QObject
{
    Q_OBJECT

private slots:
    void format()
    {
        const QChar nb(0x00A0);
        QCOMPARE(money::format(0, false), QStringLiteral("0,00"));
        QCOMPARE(money::format(5, false), QStringLiteral("0,05"));
        QCOMPARE(money::format(123450, false), QStringLiteral("1") + nb + QStringLiteral("234,50"));
        QCOMPARE(money::format(100000000, false),
                 QStringLiteral("1") + nb + QStringLiteral("000") + nb + QStringLiteral("000,00"));
        QCOMPARE(money::format(-150, false), QStringLiteral("-1,50"));
        QVERIFY(money::format(100).endsWith(QStringLiteral("₽")));
    }

    void parse_data()
    {
        QTest::addColumn<QString>("text");
        QTest::addColumn<qint64>("expected"); // -1 = ошибка

        QTest::newRow("целое")       << "1234"     << qint64(123400);
        QTest::newRow("запятая")     << "1234,5"   << qint64(123450);
        QTest::newRow("точка")       << "1234.50"  << qint64(123450);
        QTest::newRow("пробелы")     << "1 234,50" << qint64(123450);
        QTest::newRow("ноль")        << "0"        << qint64(0);
        QTest::newRow("пусто")       << ""         << qint64(-1);
        QTest::newRow("минус")       << "-5"       << qint64(-1);
        QTest::newRow("буквы")       << "12abc"    << qint64(-1);
        QTest::newRow("3 знака")     << "1,005"    << qint64(-1);
    }

    void parse()
    {
        QFETCH(QString, text);
        QFETCH(qint64, expected);
        const auto result = money::parse(text);
        if (expected < 0)
            QVERIFY(!result.has_value());
        else
            QCOMPARE(result.value_or(-1), expected);
    }

    void orderTotal()
    {
        gm::Order order;
        order.services.append({0, std::nullopt, QStringLiteral("Диагностика"), 120000, 1});
        order.parts.append({0, 1, QStringLiteral("Фильтр"), 45050, 2});
        QCOMPARE(order.total(), money::Kopecks(120000 + 90100));
    }
};

QTEST_GUILESS_MAIN(TestMoney)
#include "tst_money.moc"
