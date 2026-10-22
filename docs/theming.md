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

`resource.qrc` 里挂在 `/qss/qss/` 下，所以代码里的路径是 `:/qss/qss/base.css`。

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
checkChecked=url(":/common/icon/common/checkbox-checked-selectionmode@2.png")
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

实测（Debug 构建、1060×680、约 250 个控件、样式表 25 KB）：

| 阶段                         | 耗时      |
| ---------------------------- | --------- |
| 读 qrc + 替换令牌            | ≈ 0 ms    |
| `qApp->setStyleSheet()`      | ≈ 2830 ms |
| `setPalette()` / `setFont()` | ≈ 0 ms    |

耗时几乎全在 Qt 内部：它会把整棵控件树按新样式表重新 polish 一遍，**只能发生在 GUI 线程**。所以：

- 不要把“构建样式表”放到子线程——那部分本来就只有几毫秒，省不下来；真正的开销在 `setStyleSheet()`。
- 耗时随**规则数**线性增长（空样式表约 143 ms，去掉字体声明约 2195 ms，去掉图片声明无变化）。想更快只能减规则，或者跑 Release（Debug 下 QCss 的模板匹配没有内联，通常差一个数量级）。
- 因此 `apply()` 做了两件事兜底：同一份样式表重复应用时直接返回；真正切换时先画等待光标再执行，避免看起来像卡死。

## 移植到别的 Qt 程序

`base.css` + `light.theme` + `dark.theme` 是通用层，复制过去即可复用；`shell.css` / `pages.css` 是给这个外壳写的，别的程序不需要。

```cpp
// 自己的 qrc 里：/qss/base.css、/qss/light.theme、/qss/dark.theme、/qss/app.css
Utils::theme()->setConfig({":/qss/", {"base.css", "app.css"}});
Utils::theme()->apply();
```

`app.css` 里可以继续用同一批令牌（自动跟随深浅色），也可以直接写死颜色。

如果要接入系统主题变化，注意两点：`apply()` 必须晚于 `QApplication` 创建和资源注册；Qt 6.5 以下拿不到 `colorScheme()`，此时 `systemThemeIsDark()` 恒为 `false`，“跟随系统”等价于浅色。

## 排查清单

| 现象                 | 先看这里                                                                                     |
| -------------------- | -------------------------------------------------------------------------------------------- |
| 界面完全没有样式     | 日志里有没有 `Could not parse application stylesheet`：先查残留的 `@@`、`url()` 是否少了引号 |
| 某个控件没变色       | 规则是否被更具体的选择器盖掉；`base.css` 在前、`pages.css` 在后                              |
| 深色下某些文字看不清 | 该控件是否还在用写死的颜色（例如 `setStyleSheet("color:#333")`），改成令牌或交给调色板       |
| 改了 CSS 不生效      | 文件在 qrc 里，改完要重新编译；改 `.theme` 同理                                              |
| 切换时界面卡一下     | 见上一节，属预期行为                                                                         |
