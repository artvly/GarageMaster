#include "domain/OrderStatus.h"

#include <QCoreApplication>

namespace gm::order_status {

QString toDb(OrderStatus status)
{
    switch (status) {
    case OrderStatus::Accepted:   return QStringLiteral("accepted");
    case OrderStatus::InProgress: return QStringLiteral("in_progress");
    case OrderStatus::Ready:      return QStringLiteral("ready");
    case OrderStatus::Issued:     return QStringLiteral("issued");
    case OrderStatus::Cancelled:  return QStringLiteral("cancelled");
    }
    Q_UNREACHABLE();
}

std::optional<OrderStatus> fromDb(const QString& code)
{
    if (code == QLatin1String("accepted"))    return OrderStatus::Accepted;
    if (code == QLatin1String("in_progress")) return OrderStatus::InProgress;
    if (code == QLatin1String("ready"))       return OrderStatus::Ready;
    if (code == QLatin1String("issued"))      return OrderStatus::Issued;
    if (code == QLatin1String("cancelled"))   return OrderStatus::Cancelled;
    return std::nullopt;
}

QString displayName(OrderStatus status)
{
    switch (status) {
    case OrderStatus::Accepted:   return QCoreApplication::translate("OrderStatus", "Принят");
    case OrderStatus::InProgress: return QCoreApplication::translate("OrderStatus", "В работе");
    case OrderStatus::Ready:      return QCoreApplication::translate("OrderStatus", "Готов к выдаче");
    case OrderStatus::Issued:     return QCoreApplication::translate("OrderStatus", "Выдан");
    case OrderStatus::Cancelled:  return QCoreApplication::translate("OrderStatus", "Отменён");
    }
    Q_UNREACHABLE();
}

QList<OrderStatus> allowedTransitions(OrderStatus from)
{
    switch (from) {
    case OrderStatus::Accepted:
        return {OrderStatus::InProgress, OrderStatus::Cancelled};
    case OrderStatus::InProgress:
        return {OrderStatus::Ready, OrderStatus::Accepted, OrderStatus::Cancelled};
    case OrderStatus::Ready:
        return {OrderStatus::Issued, OrderStatus::InProgress};
    case OrderStatus::Issued:
    case OrderStatus::Cancelled:
        return {};
    }
    Q_UNREACHABLE();
}

bool canTransition(OrderStatus from, OrderStatus to)
{
    return allowedTransitions(from).contains(to);
}

bool isFinal(OrderStatus status)
{
    return status == OrderStatus::Issued || status == OrderStatus::Cancelled;
}

} // namespace gm::order_status
