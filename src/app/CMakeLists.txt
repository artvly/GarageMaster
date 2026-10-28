# garage_app — исполняемый файл с интерфейсом на Qt Widgets.
qt_add_executable(garage_app
    main.cpp
    MainWindow.h
    MainWindow.cpp
)

set_target_properties(garage_app PROPERTIES
    OUTPUT_NAME "GarageMaster"
    WIN32_EXECUTABLE ON   # без консольного окна в Windows
)

target_compile_definitions(garage_app PRIVATE
    PROJECT_VERSION_STRING="${PROJECT_VERSION}"
)

target_link_libraries(garage_app PRIVATE
    garage_core
    Qt6::Widgets
    Qt6::PrintSupport
)
