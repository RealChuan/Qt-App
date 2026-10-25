add_app_test(tst_fortune
             SOURCES tst_fortune.cpp
                     ${PROJECT_SOURCE_DIR}/src/plugins/fortuneplugin/fortuneengine.cc
                     ${PROJECT_SOURCE_DIR}/src/plugins/fortuneplugin/fortunelunar.cc
                     ${PROJECT_SOURCE_DIR}/src/plugins/fortuneplugin/fortunecontent.cc)
target_include_directories(tst_fortune PRIVATE ${PROJECT_SOURCE_DIR}/src/plugins/fortuneplugin)
