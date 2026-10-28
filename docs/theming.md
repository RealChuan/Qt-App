# 主题与样式表

一份样式表同时服务深色与浅色：CSS 里只写令牌，真实颜色来自两个 `.theme` 文件，应用样式表前把令牌替换掉。

## 为什么需要这一层

Qt 的 QSS 一旦写了 `color` / `background`，就会覆盖 `QPalette`。而“跟随系统深浅色”正是靠平台改 `QPalette` 实现的——所以**不写 QSS 能跟随系统，写了就不能**。反过来，QSS 又不支持 CSS 变量和媒体查询，没法像网页那样在同一个文件里写两套配色。

所以这里自己做一层最小的“变量替换”：切换主题 = 用另一个方案重新生成样式表，再交给 Qt。

## 文件在哪

| 文件                                          | 作用                                         |
| --------------------------------------------- | -------------------------------------------- |
| `src/utils/theme.hpp` / `.cc`                 | 读令牌、拼样式表、设样式表/调色板/字体       |
| `src/resource/qss/base.css`                   | 通用控件样式（可移植，不含本项目的颜色）     |
| `src/resource/qss/shell.css`                  | 主窗口外壳：标题栏、侧边栏、页面框架         |
| `src/resource/qss/pages.css`                  | 页面与对话框：卡片、插件页、命令面板、消息框 |
| `src/resource/qss/light.theme` / `dark.theme` | 令牌取值：颜色 + 图标路径                    |

`resource.qrc` 里挂在 `/qss/` 下，所以代码里的路径是 `:/qss/base.css`。

## 应用流程

```
light/dark.theme ──┐
                   ├─► 读出 key=value
base.css ──────────┤
shell.css ─────────┼─► 替换 @@key@@ ─► 拼接 ─► qApp->setStyleSheet()
pages.css ─────────┘                           qApp->setPalette()
                                               qApp->setFont()
```

- 顺序即优先级：`base` → `shell` → `pages`，同权重时后面的规则胜出。自己的样式表接在最后即可。
- 字体放在 C++（`applyFont()`）而不是 QSS 的 `font-family`：QSS 只能给一个族名，写死某个平台字体在别的平台会直接掉到默认字体。
- 调色板与样式表一起设，是为了让原生菜单、标准对话框这些 QSS 管不到的地方也保持同一套配色。

## 令牌怎么写

`@@名字@@` 出现在 CSS 里，取值在 `.theme` 文件：

```ini
[colors]
surface=#FFFFFF
text=#171A1F

[icons]
checkChecked=url(":/controls/checkbox-checked-selectionmode@2.png")
```

```css
#Card {
  background: @@surface @@;
  color: @@text @@;
  border: 1px solid @@border @@;
}
```

三条约定，踩过就知道疼：

1. **两个 `.theme` 的键必须一一对应。** 少一个键，CSS 里就会残留 `@@xxx@@`，Qt 会**整份样式表解析失败**（界面退回系统默认样式，日志里是 `Could not parse application stylesheet`）。`Theme::apply()` 会打印未替换令牌的警告。
2. **`url()` 里有 `@2x` 之类字符时必须加引号。** 不加引号同样会导致整份样式表解析失败。
3. **令牌文件是纯文本 ini，不是 `QSettings`。** `QSettings` 依赖文件系统语义，读不了 qrc 里的资源；`[colors]` / `[icons]` 两个分组只是给人看的，键名全局唯一即可。

新增一个颜色的完整步骤：两个 `.theme` 各加一行 → CSS 里用 `@@新名字@@`。

## 配色约定

令牌按屏幕上实际看到的效果定值，改色时守四条底线：

1. **三层底面要有明度差**：`window` < `surfaceAlt` < `surface`，卡片靠明度差浮起来，光有 1px 边框不够。
2. **边框分两档**：容器用 `border`，可交互控件用 `borderStrong`（输入框底色同卡片，边框是唯一能说明“这里能打字”的线索）。
3. **1px 横线用 `divider`**，比 `border` 深一档，否则等于没画。
4. **四档文字都要能读**：`text` ≥14:1、`textMuted` ≥5.5:1、`textSubtle` ≥4.5:1、`textDisabled` ≈3:1。

字号见 `base.css` 顶部：档位要少、层级用字重和颜色拉，12px 只给拉丁字母。自绘控件同样从 `QPalette` 取色、沿用这套字号（`paintOverlay` / `paintBoardFrame`）。强调色别改——`*@2x` 勾选框图标是按它画的。

## 代码入口

```cpp
Utils::theme()->setMode(Utils::ThemeMode::Dark);   // 切换（会持久化并立即生效）
Utils::theme()->mode();                            // 当前模式：System / Light / Dark
Utils::theme()->isDark();                          // 当前实际生效的是不是深色
Utils::theme()->apply();                           // 按当前模式重新应用一次
```

- `Theme` 由 `SingletonManager` 持有，和 `languageManager()` / `widgetManager()` 平级，用自由函数 `Utils::theme()` 取。
- 模式存在配置文件的 `[Widget] themeMode`（0 = 跟随系统，1 = 浅色，2 = 深色），由 `WidgetManager` 负责读写。
- 选“跟随系统”时，`QStyleHints::colorSchemeChanged` 会触发重新应用（Qt ≥ 6.5）。用户手动固定了方案就不再跟随。
- 切换完成后发 `themeApplied(bool dark)`：标题栏按钮和“外观”页的下拉框都靠它回填状态。
- 界面上的两个入口：标题栏的 `Auto / Light / Dark` 按钮（`Core::MainWindow`），和“外观”页的下拉框。改的是同一份状态，互相同步。

## 切换为什么慢

实测 Debug 构建、1060×680、约 250 个控件、样式表 25 KB：`qApp->setStyleSheet()` 约 2830 ms，读 qrc、替换令牌、`setPalette()` / `setFont()` 都在 0 ms 级。

> 开销全在 Qt 内部按新样式表重新 polish 整棵控件树，**只能发生在 GUI 线程**，且随规则数线性增长。所以别把“构建样式表”搬到子线程（那部分只有几毫秒）；想更快只能减规则或跑 Release（Debug 下 QCss 模板匹配没有内联，通常差一个数量级）。`apply()` 已做兜底：同一份样式表重复应用时直接返回，真正切换时先画等待光标。

## 移植到别的 Qt 程序

`base.css` + `light.theme` + `dark.theme` 是通用层，复制过去即可复用；`shell.css` / `pages.css` 是给这个外壳写的，别的程序不需要。

```cpp
// 自己的 qrc 里：/qss/base.css、/qss/light.theme、/qss/dark.theme、/qss/app.css
Utils::theme()->setConfig({":/qss/", {"base.css", "app.css"}});
Utils::theme()->apply();
```

`app.css` 里可以继续用同一批令牌（自动跟随深浅色），也可以直接写死颜色。

> 接入系统主题变化注意两点：`apply()` 必须晚于 `QApplication` 创建与资源注册；Qt 6.5 以下拿不到 `colorScheme()`，`systemThemeIsDark()` 恒为 `false`，“跟随系统”等价于浅色。

## 排查清单

| 现象                 | 先看这里                                                                                     |
| -------------------- | -------------------------------------------------------------------------------------------- |
| 界面完全没有样式     | 日志里有没有 `Could not parse application stylesheet`：先查残留的 `@@`、`url()` 是否少了引号 |
| 某个控件没变色       | 规则是否被更具体的选择器盖掉；`base.css` 在前、`pages.css` 在后                              |
| 深色下某些文字看不清 | 该控件是否还在用写死的颜色（例如 `setStyleSheet("color:#333")`），改成令牌或交给调色板       |
| 改了 CSS 不生效      | 文件在 qrc 里，改完要重新编译；改 `.theme` 同理                                              |
| 切换时界面卡一下     | 见上一节，属预期行为                                                                         |
