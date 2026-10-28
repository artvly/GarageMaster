find_package(Qt6 REQUIRED COMPONENTS Test)

# gm_add_test(<имя> <исходник>) — отдельный исполняемый файл Qt Test + регистрация в CTest.
function(gm_add_test name source)
    qt_add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE garage_core Qt6::Test)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

gm_add_test(tst_database      tst_database.cpp)
gm_add_test(tst_order_status  tst_order_status.cpp)
gm_add_test(tst_money         tst_money.cpp)
gm_add_test(tst_security      tst_security.cpp)
gm_add_test(tst_backup        tst_backup.cpp)
