#include "domain/OrderStatus.h"

#include <QtTest>

using gm::OrderStatus;
namespace os = gm::order_status;

class TestOrderStatus : public QObject
{
    Q_OBJECT

private slots:
    void dbRoundTrip()
    {
        for (auto s : {OrderStatus::Accepted, OrderStatus::InProgress, OrderStatus::Ready,
                       OrderStatus::Issued, OrderStatus::Cancelled}) {
            QCOMPARE(os::fromDb(os::toDb(s)), std::optional<OrderStatus>(s));
            QVERIFY(!os::displayName(s).isEmpty());
        }
        QVERIFY(!os::fromDb(QStringLiteral("garbage")).has_value());
    }

    void transitions_data()
    {
        QTest::addColumn<OrderStatus>("from");
        QTest::addColumn<OrderStatus>("to");
        QTest::addColumn<bool>("allowed");

        // Основной путь
        QTest::newRow("принят → в работе")   << OrderStatus::Accepted   << OrderStatus::InProgress << true;
        QTest::newRow("в работе → готов")    << OrderStatus::InProgress << OrderStatus::Ready      << true;
        QTest::newRow("готов → выдан")       << OrderStatus::Ready      << OrderStatus::Issued     << true;
        // Возвраты
        QTest::newRow("в работе → принят")   << OrderStatus::InProgress << OrderStatus::Accepted   << true;
        QTest::newRow("готов → в работе")    << OrderStatus::Ready      << OrderStatus::InProgress << true;
        // Отмена
        QTest::newRow("принят → отменён")    << OrderStatus::Accepted   << OrderStatus::Cancelled  << true;
        QTest::newRow("в работе → отменён")  << OrderStatus::InProgress << OrderStatus::Cancelled  << true;
        // Запрещено
        QTest::newRow("принят → выдан")      << OrderStatus::Accepted   << OrderStatus::Issued     << false;
        QTest::newRow("принят → готов")      << OrderStatus::Accepted   << OrderStatus::Ready      << false;
        QTest::newRow("готов → отменён")     << OrderStatus::Ready      << OrderStatus::Cancelled  << false;
        QTest::newRow("выдан → в работе")    << OrderStatus::Issued     << OrderStatus::InProgress << false;
        QTest::newRow("отменён → принят")    << OrderStatus::Cancelled  << OrderStatus::Accepted   << false;
        QTest::newRow("принят → принят")     << OrderStatus::Accepted   << OrderStatus::Accepted   << false;
    }

    void transitions()
    {
        QFETCH(OrderStatus, from);
        QFETCH(OrderStatus, to);
        QFETCH(bool, allowed);
        QCOMPARE(os::canTransition(from, to), allowed);
    }

    void finalStates()
    {
        QVERIFY(os::isFinal(OrderStatus::Issued));
        QVERIFY(os::isFinal(OrderStatus::Cancelled));
        QVERIFY(os::isOpen(OrderStatus::Ready));
        QVERIFY(os::allowedTransitions(OrderStatus::Issued).isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestOrderStatus)
#include "tst_order_status.moc"
