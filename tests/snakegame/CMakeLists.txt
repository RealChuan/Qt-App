# 模型是纯逻辑（无 QObject、无控件），测试直接把源文件编进来，不依赖插件二进制。
add_app_test(tst_snakegame
             SOURCES tst_snakegame.cpp
                     ${PROJECT_SOURCE_DIR}/src/plugins/snakeplugin/snakegame.cc)
target_include_directories(tst_snakegame PRIVATE ${PROJECT_SOURCE_DIR}/src/plugins/snakeplugin)
