project(qlogexplorer_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(gui_lib STATIC
    # headers
    include/GuiTypes.h
    include/GuiUtils.h
    include/Style.h
    include/MainWindow.h
    include/LongScrollBar.h
    include/LogViewWidget.h
    include/SearchParamControl.h
    include/SearchParamWidget.h
    include/LogSearchWidget.h
    include/LogTabWidget.h
    include/HeaderView.h
    include/Highlighter.h
    include/ProgressLabel.h
    include/TemplatesConfigDlg.h
    include/Settings.h
    include/SettingsDlg.h
    # src
    src/Style.cpp
    src/MainWindow.cpp
    src/LongScrollBar.cpp
    src/LogViewWidget.cpp
    src/SearchParamControl.cpp
    src/SearchParamWidget.cpp
    src/LogSearchWidget.cpp
    src/LogTabWidget.cpp
    src/HeaderView.cpp
    src/Highlighter.cpp
    src/ProgressLabel.cpp
    src/TemplatesConfigDlg.cpp
    src/Settings.cpp
    src/SettingsDlg.cpp
)

target_include_directories(gui_lib
    PRIVATE
        src
    PUBLIC
        include
)

target_link_libraries(gui_lib
    PUBLIC
        core_lib
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
)

target_precompile_headers(gui_lib PRIVATE ${CMAKE_SOURCE_DIR}/common/pch.h)
