# =============================================================================
# Qt Translations Utility Qt翻译文件处理工具
# =============================================================================

# -----------------------------------------------------------------------------
# 添加Qt翻译支持
# -----------------------------------------------------------------------------
function(add_translations TARGET_NAME)
  # 参数验证
  if(NOT TARGET ${TARGET_NAME})
    message(FATAL_ERROR "Target '${TARGET_NAME}' does not exist")
  endif()

  # 转换目标名称为小写用于翻译文件基名
  string(TOLOWER "${TARGET_NAME}" TS_FILE_BASE)

  # 添加Qt翻译
  qt_add_translations(
    ${TARGET_NAME}
    TS_FILE_BASE
    ${TS_FILE_BASE}
    TS_FILE_DIR
    "${PROJECT_SOURCE_DIR}/translations"
    INCLUDE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}/src"
    LUPDATE_OPTIONS
    -no-obsolete
    -tr-function-alias
    QT_TRANSLATE_NOOP+=TRANSLATE
    -tr-function-alias
    QT_TRANSLATE_NOOP+=Translate)

  # 确定翻译文件的输出位置
  if(CMAKE_HOST_APPLE)
    # macOS: 放入App Bundle的Resources目录
    set(TRANSLATIONS_OUTPUT_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.app/Contents/Resources/translations"
    )
  elseif(CMAKE_HOST_WIN32)
    # Windows: 放入translations目录
    set(TRANSLATIONS_OUTPUT_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/translations")
  else()
    # Linux/Unix: 放入translations目录
    set(TRANSLATIONS_OUTPUT_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/translations")
  endif()

  # 创建输出目录
  file(MAKE_DIRECTORY "${TRANSLATIONS_OUTPUT_DIR}")

  # 查找所有的翻译源文件(.ts)
  file(GLOB TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TS_FILE_BASE}_*.ts")

  if(NOT TS_FILES)
    message(WARNING "No translation files found for target ${TARGET_NAME}")
    return()
  endif()

  # 为每个翻译文件设置输出位置属性
  foreach(TS_FILE ${TS_FILES})
    # 获取文件名（不带路径）
    get_filename_component(TS_FILENAME "${TS_FILE}" NAME)

    # 设置对应的.qm文件输出位置
    set(QM_FILE "${TRANSLATIONS_OUTPUT_DIR}/${TS_FILENAME}")
    string(REPLACE ".ts" ".qm" QM_FILE "${QM_FILE}")

    # 设置源文件属性，指定输出的.qm文件位置
    set_source_files_properties(
      "${TS_FILE}" PROPERTIES OUTPUT_LOCATION "${TRANSLATIONS_OUTPUT_DIR}")

    message(STATUS "Translation file: ${TS_FILENAME} -> ${QM_FILE}")
  endforeach()

  # 显示配置信息
  message(STATUS "Translations for ${TARGET_NAME}:")
  message(STATUS "  TS files: ${TS_FILES}")
  message(STATUS "  Output directory: ${TRANSLATIONS_OUTPUT_DIR}")
endfunction()

# -----------------------------------------------------------------------------
# 自动配置检查
# -----------------------------------------------------------------------------
function(check_qt_translations_setup)
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/translations")
    message(
      STATUS
        "Creating translations directory: ${PROJECT_SOURCE_DIR}/translations")
    file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/translations")
  endif()
endfunction()

# 自动运行配置检查
check_qt_translations_setup()
