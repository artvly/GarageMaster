#pragma once

#include <QList>
#include <QString>
#include <optional>

namespace gm {

/// Статус заказ-наряда. Жизненный цикл:
///   Accepted → InProgress → Ready → Issued
/// с возможностью вернуть заказ на шаг назад (InProgress → Accepted, Ready → InProgress)
/// и отменить его до начала выдачи (Accepted/InProgress → Cancelled).
/// Issued и Cancelled — конечные состояния.
enum class OrderStatus {
    Accepted,
    InProgress,
    Ready,
    Issued,
    Cancelled,
};

namespace order_status {

/// Код для хранения в БД ('accepted', 'in_progress', ...).
QString toDb(OrderStatus status);

/// Разбор кода из БД; std::nullopt для неизвестного значения.
std::optional<OrderStatus> fromDb(const QString& code);

/// Название для интерфейса и печатных форм («Принят», «В работе», ...).
QString displayName(OrderStatus status);

/// Допустим ли переход from → to.
bool canTransition(OrderStatus from, OrderStatus to);

/// Все статусы, в которые можно перейти из текущего (для кнопок/меню UI).
QList<OrderStatus> allowedTransitions(OrderStatus from);

/// Конечный статус: заказ закрыт, его состав больше не редактируется.
bool isFinal(OrderStatus status);

/// Открытый заказ (не выдан и не отменён) — блокирует удаление клиента.
inline bool isOpen(OrderStatus status) { return !isFinal(status); }

} // namespace order_status
} // namespace gm
