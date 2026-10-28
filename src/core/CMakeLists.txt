# garage_core — ядро без зависимости от Qt Widgets:
# предметная область, доступ к данным, сервисы. Его же линкуют тесты.
add_library(garage_core STATIC
    domain/Entities.h
    domain/Money.h
    domain/Money.cpp
    domain/OrderStatus.h
    domain/OrderStatus.cpp
    data/Database.h
    data/Database.cpp
    data/BackupService.h
    data/BackupService.cpp
    data/Bootstrap.h
    data/Bootstrap.cpp
    security/PasswordHasher.h
    security/PasswordHasher.cpp
)

target_include_directories(garage_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(garage_core
    PUBLIC Qt6::Core Qt6::Sql
    PRIVATE Qt6::Network   # QPasswordDigestor (PBKDF2)
)

# SQL-миграции вшиваются в ресурсы: :/migrations/NNN_*.sql
qt_add_resources(garage_core "migrations"
    PREFIX "/"
    BASE resources
    FILES
        resources/migrations/001_init.sql
)
