#include "data/Bootstrap.h"
#include "data/Database.h"

#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

using gm::Database;

class TestDatabase : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        QVERIFY(m_dir.isValid());
        m_path = m_dir.filePath(QStringLiteral("test_%1.sqlite").arg(++m_counter));
    }

    void createsSchemaOnFirstOpen()
    {
        Database db(QStringLiteral("t1"));
        QVERIFY2(db.open(m_path), qPrintable(db.lastError()));
        QCOMPARE(db.schemaVersion(), Database::latestSchemaVersion());
        QVERIFY(Database::latestSchemaVersion() >= 1);

        QSqlQuery q(db.connection());
        for (const char* table : {"users", "clients", "cars", "service_catalog", "parts", "orders",
                                  "order_services", "order_parts", "stock_movements",
                                  "order_status_history"}) {
            QVERIFY(q.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(QLatin1String(table))));
        }
    }

    void reopenIsIdempotent()
    {
        {
            Database db(QStringLiteral("t2a"));
            QVERIFY(db.open(m_path));
        }
        Database db(QStringLiteral("t2b"));
        QVERIFY2(db.open(m_path), qPrintable(db.lastError()));
        QCOMPARE(db.schemaVersion(), Database::latestSchemaVersion());
    }

    void rejectsNewerSchema()
    {
        {
            Database db(QStringLiteral("t3a"));
            QVERIFY(db.open(m_path));
            QSqlQuery q(db.connection());
            QVERIFY(q.exec(QStringLiteral("PRAGMA user_version = 999")));
        }
        Database db(QStringLiteral("t3b"));
        QVERIFY(!db.open(m_path));
        QVERIFY(!db.lastError().isEmpty());
        QVERIFY(!db.isOpen());
    }

    void foreignKeysAreEnforced()
    {
        Database db(QStringLiteral("t4"));
        QVERIFY(db.open(m_path));
        QSqlQuery q(db.connection());
        // Автомобиль несуществующего клиента
        QVERIFY(!q.exec(QStringLiteral("INSERT INTO cars (client_id, plate) VALUES (42, 'А123ВС77')")));

        // Нельзя удалить клиента, у которого есть автомобиль
        QVERIFY(q.exec(QStringLiteral("INSERT INTO clients (full_name) VALUES ('Иванов И.И.')")));
        const auto clientId = q.lastInsertId();
        q.prepare(QStringLiteral("INSERT INTO cars (client_id, plate) VALUES (?, 'А123ВС77')"));
        q.addBindValue(clientId);
        QVERIFY(q.exec());
        q.prepare(QStringLiteral("DELETE FROM clients WHERE id = ?"));
        q.addBindValue(clientId);
        QVERIFY(!q.exec());
    }

    void checkConstraints()
    {
        Database db(QStringLiteral("t5"));
        QVERIFY(db.open(m_path));
        QSqlQuery q(db.connection());
        QVERIFY(!q.exec(QStringLiteral("INSERT INTO clients (full_name) VALUES ('   ')")));
        QVERIFY(!q.exec(QStringLiteral("INSERT INTO service_catalog (name, price_kop) VALUES ('Диагностика', -1)")));
        QVERIFY(!q.exec(QStringLiteral("INSERT INTO parts (name, price_kop, quantity) VALUES ('Фильтр', 100, -5)")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO clients (full_name) VALUES ('Петров П.П.')")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO cars (client_id, plate) VALUES (1, 'В001ОР99')")));
        QVERIFY(!q.exec(QStringLiteral("INSERT INTO orders (client_id, car_id, status) VALUES (1, 1, 'unknown')")));
        QVERIFY(!q.exec(QStringLiteral("INSERT INTO cars (client_id, plate, vin) VALUES (1, 'В002ОР99', 'SHORTVIN')")));
    }

    void orderTotalsView()
    {
        Database db(QStringLiteral("t6"));
        QVERIFY(db.open(m_path));
        QSqlQuery q(db.connection());
        QVERIFY(q.exec(QStringLiteral("INSERT INTO clients (full_name) VALUES ('Сидоров С.С.')")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO cars (client_id, plate) VALUES (1, 'Е777КХ77')")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO parts (name, price_kop, quantity) VALUES ('Масло 5W-30', 90000, 10)")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO orders (client_id, car_id) VALUES (1, 1)")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO order_services (order_id, name, price_kop, quantity) "
                                      "VALUES (1, 'Замена масла', 150000, 1)")));
        QVERIFY(q.exec(QStringLiteral("INSERT INTO order_parts (order_id, part_id, name, price_kop, quantity) "
                                      "VALUES (1, 1, 'Масло 5W-30', 90000, 4)")));
        QVERIFY(q.exec(QStringLiteral("SELECT services_kop, parts_kop FROM order_totals WHERE order_id = 1")));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toLongLong(), 150000LL);
        QCOMPARE(q.value(1).toLongLong(), 360000LL);
    }

    void defaultAdminCreatedOnce()
    {
        Database db(QStringLiteral("t7"));
        QVERIFY(db.open(m_path));
        QString error;
        QVERIFY(gm::bootstrap::ensureDefaultAdmin(db.connection(), &error));
        QVERIFY(error.isEmpty());
        QVERIFY(!gm::bootstrap::ensureDefaultAdmin(db.connection(), &error));

        QSqlQuery q(db.connection());
        QVERIFY(q.exec(QStringLiteral("SELECT COUNT(*), MAX(must_change_password) FROM users WHERE role = 'admin'")));
        QVERIFY(q.next());
        QCOMPARE(q.value(0).toInt(), 1);
        QCOMPARE(q.value(1).toInt(), 1);
    }

    void splitScript()
    {
        const QString script = QStringLiteral(
            "-- комментарий\n"
            "CREATE TABLE a (\n  x INTEGER\n);\r\n"
            "\n"
            "CREATE INDEX i ON a(x);\n");
        const QStringList parts = Database::splitScript(script);
        QCOMPARE(parts.size(), 2);
        QVERIFY(parts[0].startsWith(QStringLiteral("CREATE TABLE a")));
        QVERIFY(parts[1].startsWith(QStringLiteral("CREATE INDEX")));
    }

private:
    QTemporaryDir m_dir;
    QString m_path;
    int m_counter = 0;
};

QTEST_GUILESS_MAIN(TestDatabase)
#include "tst_database.moc"
