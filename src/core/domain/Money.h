#pragma once

#include <QString>
#include <QtGlobal>
#include <optional>

namespace gm::money {

/// Все суммы в программе — целые копейки (qint64), без плавающей точки.
using Kopecks = qint64;

/// 123450 → «1 234,50 ₽» (неразрывный пробел между разрядами).
QString format(Kopecks value, bool withCurrency = true);

/// «1234,5» / «1 234.50» / «1234» → копейки.
/// std::nullopt для пустой, отрицательной или некорректной строки,
/// а также при более чем двух знаках после запятой.
std::optional<Kopecks> parse(const QString& text);

} // namespace gm::money
