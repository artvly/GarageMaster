#pragma once

// Сущности предметной области — простые структуры без логики доступа к БД.
// Их заполняют репозитории и потребляют сервисы и модели представления.

#include "domain/Money.h"
#include "domain/OrderStatus.h"

#include <QDateTime>
#include <QList>
#include <QString>
#include <optional>

namespace gm {

using Id = qint64;

enum class Role { Admin, Manager };

struct User {
    Id id = 0;
    QString login;
    QString fullName;
    Role role = Role::Manager;
    bool isActive = true;
    bool mustChangePassword = false;
};

struct Client {
    Id id = 0;
    QString fullName;
    QString phone;
    QString email;
    QString note;
};

struct Car {
    Id id = 0;
    Id clientId = 0;
    QString plate;   // госномер
    QString brand;
    QString model;
    QString vin;
    std::optional<int> year;
    std::optional<int> mileage;
};

struct CatalogService {
    Id id = 0;
    QString name;
    money::Kopecks price = 0;
    bool isActive = true;
};

struct Part {
    Id id = 0;
    QString article;
    QString name;
    QString unit = QStringLiteral("шт");
    money::Kopecks price = 0;
    int quantity = 0;
    bool isActive = true;
};

struct OrderServiceLine {
    Id id = 0;
    std::optional<Id> serviceId;
    QString name;
    money::Kopecks price = 0;
    int quantity = 1;
    money::Kopecks total() const { return price * quantity; }
};

struct OrderPartLine {
    Id id = 0;
    Id partId = 0;
    QString name;
    money::Kopecks price = 0;
    int quantity = 1;
    money::Kopecks total() const { return price * quantity; }
};

struct Order {
    Id id = 0;
    Id clientId = 0;
    Id carId = 0;
    std::optional<Id> createdBy;
    OrderStatus status = OrderStatus::Accepted;
    std::optional<int> mileage;
    QString complaint;
    QDateTime createdAt;
    QDateTime closedAt;
    QList<OrderServiceLine> services;
    QList<OrderPartLine> parts;

    money::Kopecks total() const
    {
        money::Kopecks sum = 0;
        for (const auto& s : services) sum += s.total();
        for (const auto& p : parts)    sum += p.total();
        return sum;
    }
};

} // namespace gm
