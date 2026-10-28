#include "domain/Money.h"

#include <QRegularExpression>

namespace gm::money {

QString format(Kopecks value, bool withCurrency)
{
    const bool negative = value < 0;
    const quint64 abs = negative ? quint64(-(value + 1)) + 1 : quint64(value);

    QString rubles = QString::number(abs / 100);
    for (int i = rubles.size() - 3; i > 0; i -= 3)
        rubles.insert(i, QChar(0x00A0));

    QString result = QStringLiteral("%1%2,%3")
                         .arg(negative ? QStringLiteral("-") : QString())
                         .arg(rubles)
                         .arg(abs % 100, 2, 10, QChar('0'));
    if (withCurrency)
        result += QStringLiteral(" ₽");
    return result;
}

std::optional<Kopecks> parse(const QString& text)
{
    QString s = text;
    s.remove(QChar(' ')).remove(QChar(0x00A0)).remove(QStringLiteral("₽"));
    s.replace(QChar(','), QChar('.'));

    static const QRegularExpression re(QStringLiteral(R"(^(\d{1,13})(?:\.(\d{1,2}))?$)"));
    const auto m = re.match(s);
    if (!m.hasMatch())
        return std::nullopt;

    const Kopecks rubles = m.captured(1).toLongLong();
    QString frac = m.captured(2);
    while (frac.size() < 2)
        frac += QChar('0');
    return rubles * 100 + frac.toLongLong();
}

} // namespace gm::money
