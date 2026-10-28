-- Миграция 001: исходная схема БД «Гараж Мастер».
-- Денежные суммы хранятся в копейках (INTEGER), даты — в ISO 8601 (TEXT, UTC).
-- Операторы разделяются символом ';' в конце строки (см. Database::applyScript).

CREATE TABLE users (
    id                   INTEGER PRIMARY KEY,
    login                TEXT    NOT NULL UNIQUE COLLATE NOCASE,
    full_name            TEXT    NOT NULL,
    password_hash        TEXT    NOT NULL,
    password_salt        TEXT    NOT NULL,
    role                 TEXT    NOT NULL CHECK (role IN ('admin', 'manager')),
    is_active            INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1)),
    must_change_password INTEGER NOT NULL DEFAULT 0 CHECK (must_change_password IN (0, 1)),
    created_at           TEXT    NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now'))
);

CREATE TABLE clients (
    id         INTEGER PRIMARY KEY,
    full_name  TEXT NOT NULL CHECK (length(trim(full_name)) > 0),
    phone      TEXT,
    email      TEXT,
    note       TEXT,
    created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now'))
);
CREATE INDEX idx_clients_name  ON clients(full_name COLLATE NOCASE);
CREATE INDEX idx_clients_phone ON clients(phone);

CREATE TABLE cars (
    id        INTEGER PRIMARY KEY,
    client_id INTEGER NOT NULL REFERENCES clients(id) ON DELETE RESTRICT,
    plate     TEXT    NOT NULL CHECK (length(trim(plate)) > 0),
    brand     TEXT,
    model     TEXT,
    vin       TEXT    CHECK (vin IS NULL OR length(vin) = 17),
    year      INTEGER CHECK (year IS NULL OR year BETWEEN 1900 AND 2100),
    mileage   INTEGER CHECK (mileage IS NULL OR mileage >= 0)
);
CREATE INDEX idx_cars_client ON cars(client_id);
CREATE INDEX idx_cars_plate  ON cars(plate COLLATE NOCASE);
CREATE INDEX idx_cars_vin    ON cars(vin);

CREATE TABLE service_catalog (
    id        INTEGER PRIMARY KEY,
    name      TEXT    NOT NULL UNIQUE COLLATE NOCASE CHECK (length(trim(name)) > 0),
    price_kop INTEGER NOT NULL CHECK (price_kop >= 0),
    is_active INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1))
);

CREATE TABLE parts (
    id        INTEGER PRIMARY KEY,
    article   TEXT,
    name      TEXT    NOT NULL CHECK (length(trim(name)) > 0),
    unit      TEXT    NOT NULL DEFAULT 'шт',
    price_kop INTEGER NOT NULL CHECK (price_kop >= 0),
    quantity  INTEGER NOT NULL DEFAULT 0 CHECK (quantity >= 0),
    is_active INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0, 1))
);
CREATE INDEX idx_parts_name    ON parts(name COLLATE NOCASE);
CREATE INDEX idx_parts_article ON parts(article);

-- Статусы: accepted → in_progress → ready → issued; cancelled — отмена.
-- Допустимые переходы проверяет OrderStatus::canTransition (слой сервисов).
CREATE TABLE orders (
    id            INTEGER PRIMARY KEY,
    client_id     INTEGER NOT NULL REFERENCES clients(id) ON DELETE RESTRICT,
    car_id        INTEGER NOT NULL REFERENCES cars(id)    ON DELETE RESTRICT,
    created_by    INTEGER REFERENCES users(id) ON DELETE SET NULL,
    status        TEXT    NOT NULL DEFAULT 'accepted'
                  CHECK (status IN ('accepted', 'in_progress', 'ready', 'issued', 'cancelled')),
    mileage       INTEGER CHECK (mileage IS NULL OR mileage >= 0),
    complaint     TEXT,
    created_at    TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now')),
    closed_at     TEXT
);
CREATE INDEX idx_orders_status  ON orders(status);
CREATE INDEX idx_orders_created ON orders(created_at);
CREATE INDEX idx_orders_closed  ON orders(closed_at);
CREATE INDEX idx_orders_client  ON orders(client_id);
CREATE INDEX idx_orders_car     ON orders(car_id);

-- Название и цена копируются в строку заказа, чтобы правка каталога
-- не меняла уже оформленные заказ-наряды.
CREATE TABLE order_services (
    id         INTEGER PRIMARY KEY,
    order_id   INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    service_id INTEGER REFERENCES service_catalog(id) ON DELETE SET NULL,
    name       TEXT    NOT NULL,
    price_kop  INTEGER NOT NULL CHECK (price_kop >= 0),
    quantity   INTEGER NOT NULL DEFAULT 1 CHECK (quantity > 0)
);
CREATE INDEX idx_order_services_order ON order_services(order_id);

CREATE TABLE order_parts (
    id        INTEGER PRIMARY KEY,
    order_id  INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    part_id   INTEGER NOT NULL REFERENCES parts(id)  ON DELETE RESTRICT,
    name      TEXT    NOT NULL,
    price_kop INTEGER NOT NULL CHECK (price_kop >= 0),
    quantity  INTEGER NOT NULL CHECK (quantity > 0)
);
CREATE INDEX idx_order_parts_order ON order_parts(order_id);

-- Журнал движения склада: приход, списание в заказ, возврат при отмене, корректировка.
CREATE TABLE stock_movements (
    id         INTEGER PRIMARY KEY,
    part_id    INTEGER NOT NULL REFERENCES parts(id) ON DELETE RESTRICT,
    delta      INTEGER NOT NULL CHECK (delta <> 0),
    reason     TEXT    NOT NULL CHECK (reason IN ('receipt', 'order', 'order_return', 'adjust')),
    order_id   INTEGER REFERENCES orders(id) ON DELETE SET NULL,
    user_id    INTEGER REFERENCES users(id)  ON DELETE SET NULL,
    created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now'))
);
CREATE INDEX idx_stock_movements_part ON stock_movements(part_id);

CREATE TABLE order_status_history (
    id          INTEGER PRIMARY KEY,
    order_id    INTEGER NOT NULL REFERENCES orders(id) ON DELETE CASCADE,
    from_status TEXT,
    to_status   TEXT NOT NULL,
    user_id     INTEGER REFERENCES users(id) ON DELETE SET NULL,
    changed_at  TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now'))
);
CREATE INDEX idx_status_history_order ON order_status_history(order_id);

-- Итоговая сумма заказа считается, а не хранится: нет риска рассинхронизации.
CREATE VIEW order_totals AS
SELECT o.id AS order_id,
       COALESCE((SELECT SUM(price_kop * quantity) FROM order_services WHERE order_id = o.id), 0) AS services_kop,
       COALESCE((SELECT SUM(price_kop * quantity) FROM order_parts    WHERE order_id = o.id), 0) AS parts_kop
FROM orders o;
