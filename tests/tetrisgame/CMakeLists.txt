# 模型是纯逻辑（无 QObject、无控件），测试直接把源文件编进来，不依赖插件二进制。
add_app_test(tst_tetrisgame
             SOURCES tst_tetrisgame.cpp
                     ${PROJECT_SOURCE_DIR}/src/plugins/tetrisplugin/tetrisgame.cc)
target_include_directories(tst_tetrisgame PRIVATE ${PROJECT_SOURCE_DIR}/src/plugins/tetrisplugin)
