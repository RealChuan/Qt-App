add_subdirectory(coreplugin)

# 功能插件：每个演示一种不同的扩展方式
add_subdirectory(helloplugin)        # 最小模板：页面 + 命令 + 设置
add_subdirectory(particlesplugin)    # 自绘 + 帧驱动
add_subdirectory(danmakuplugin)      # 服务提供者
add_subdirectory(pomodoroplugin)     # 服务消费者 + 后台线程
add_subdirectory(fractalplugin)      # 后台计算 + 取消
add_subdirectory(colorpickerplugin)  # 独立工具窗 + 系统能力
add_subdirectory(snakeplugin)        # 游戏循环 + 键盘焦点 + 存档
add_subdirectory(tetrisplugin)       # 60 Hz 帧循环 + SRS + DAS/ARR
