# 适用于 Qt 的可扩展菜单项框架

菜单项是一个 GUI 编辑软件不可或缺的元素。为了丰富插件架构的功能，我们需要使菜单项能够满足用户自定义扩展的需求，因此需要一个形式化的可扩展菜单项框架。

以 JetBrains 家族的应用程序作为参考，我们要设计的菜单项框架需要符合以下要求：
- 能动态地向现有的菜单项结构中插入菜单项
- 支持用户修改菜单项结构
- 支持用户修改快捷键
- 支持本地化
- 能使用声明式语法建立菜单项结构

## Qt 菜单项机制

### QAction

QAction 是一个菜单项的实体，它是一个数据结构，存储菜单项的文字、图标、信号槽等信息。

具有一部分按钮特性，可触发鼠标移动事件与单击事件。

### QMenu

QMenu 是菜单，是一种特殊的窗体控件，具有 Popup 属性，仅在触发时显示。

QMenu 中可添加多个 QAction，QMenu 持有这些 QAction 的引用，在显示时将这些 QAction 的图标与文字渲染出来，并监视鼠标，在适当的时候触发 QAction 的事件。

QMenu 自身拥有一个 QAction，它是 QMenu 的代理。QMenu 本身是一个控件，不直接拥有数据结构，在 QMenu 被添加到其他控件中去时，实际添加的是这个 QAction，由这个 QAction 负责提供其数据。

### QMenuBar

QMenuBar 是菜单栏，将添加到其中的 QAction 的文字转化为按钮平铺持久显示。

### QToolBar

QToolBar 是工具栏，将添加到其中的 QAction 的图标转化为按钮平铺持久显示。

## 定义

为了实现以上要求，我们需要形式化地定义框架内的每个概念

### 菜单元素

- 顶级菜单（TopLevel）：能独立显示的菜单控件，我们称之为顶级菜单，如独立的 QMenu、QMenuBar、QToolBar。
- 命令（Action）：即一个 QAction；
- 次级菜单（Menu）：被添加在其他菜单中的菜单；
- 内嵌控件（Widget）：可显示在菜单项中的控件；
- 组（Group）：包含若干个 Action、Menu、Widget 的逻辑结构；

### 命令扩展

称为 ActionExtension，它是一个静态数据结构，包含若干个菜单元素以及表示组织关系的若干个树形结构，也包含一组构造例程，同时拥有版本号、哈希值等元数据。

每个菜单元素拥有自己的标识符（ID）、文本信息、目录标签，如果是命令还包含快捷键和命令类别。

### 命令域

称为 ActionDomain，一个命令域中可动态注册若干个命令扩展，每个注册的命令扩展包含的菜单元素共同组成命令域的菜单元素，它们两两之间标识符必须互不相同。

所有菜单元素所组成的树型结构，最终的形态由命令域控制。

命令域根据命令扩展定义的组织关系与构造例程构建构造结构，遵循以下构造顺序：
- 对已定义组织关系的菜单元素集合，按照定义的组织关系构造，构造为一棵树，如果树根不是顶级菜单则是游离的；
- 按照命令扩展注册的顺序，依次执行它们定义的构造例程。

以上步骤完成后，菜单元素结构的最终形态也已确定，此最终形态使用 Json 格式持久化，不仅要保存树形结构，还需要保存参与构建的所有命令扩展的哈希值。

针对此最终形态，用户可对其结构进行任意修改，修改后的结果依然使用 Json 格式持久化。

应用程序启动后，持久化记录中已存在的命令扩展，其形态直接读取，不再进行构造。因此我们可以得出以下两个结论：
- 应用程序第二次启动，一般情况下，结构形态完全来自持久化记录；
- 如果应用程序进行了更新，增加了一些模块，包含了新增的命令扩展，那么已有的命令扩展的结构形态先从持久化记录构建，之后使用上述构造顺序对新增的命令扩展进行构造，并形成新的持久化记录；
- 如果应用程序进行了更新，删除了一些模块，某些命令扩展被移除，那么所有依赖这个命令扩展中的菜单元素的其他菜单元素将不会出现在最终形态中；

### 构造例程

如上所说，在构造前期已经建立起若干棵树，而构造例程就是将一组游离的树根添加到顶级菜单或其他游离树根上的过程。

添加规则：指定目标树根的 ID，插入方式为“插入到最后”、“插入到最前”、“插入到某个子节点之后”或“插入到某个子节点之前”之一，如果是后两个则需要再指定相对的子节点的 ID。

## 扩展清单

开发者使用 XML 语法编写一个清单，列出需要添加的各种菜单元素的信息。清单的正文分为以下三个部分，分别是属性声明、结构声明、构造例程声明。

```xml
<?xml version="1.0" encoding="UTF-8"?>
<actionExtension>
    <version>2.0</version>

    <parserConfig>
        <defaultCategory>Plugins;MyPlugin</defaultCategory>
        <vars>
            <var key="NewFileClass" value="Create" />
        </vars>
    </parserConfig>

    <objects>
        <menu id="File" text="&amp;File" />
        <action id="NewFile" class="${NewFileClass}" shortcut="Ctrl+N" />
        <action id="OpenFile" class="File" shortcut="Ctrl+O" />
        <action id="SaveFile" class="File" shortcut="Ctrl+S" />
        <action id="SaveAsFile" class="File" shortcut="Ctrl+Shift+S" />
        <action id="CloseFile" class="File" />
    </objects>

    <layouts>
        <menuBar id="MainMenu">
            <menu id="File">
                <group id="FileOpenGroup">
                    <action id="NewFile" />
                    <action id="OpenFile" />
                    <action id="OpenRecent" />
                </group>
                <separator />
                <group id="FileSaveGroup">
                    <action id="SaveFile" />
                    <action id="SaveAsFile" />
                </group>
            </menu>
        </menuBar>

        <toolBar id="MainToolbar">
            <action id="OpenFile" />
            <action id="SaveFile" />
            <separator />
        </toolBar>
    </layouts>
    
    <buildRoutines>
        <buildRoutine anchor="after" parent="File" relativeTo="FileSaveGroup">
            <separator />
            <action id="CloseFile" />
        </buildRoutine>
    </buildRoutines>
</actionExtension>
```

- `version`：必选字段，固定值`2.0`
- `parserConfig`：包含解析阶段的提示，不包含任何实际数据，其存在的意义是简化该清单编写的工作量；
    - 子节点：
        - `defaultCategory`：缺省目录标签，默认为空；
        - `vars`：解析前期定义的变量，子节点满足`<var key="xx" value="yy" />`的格式
- `objects`：包含所有菜单元素的属性声明；
    - 子节点标签：
        - `action`：命令
        - `widget`：控件
        - `group`：组
        - `menu`：菜单
        - `menuBar`：菜单栏
        - `toolBar`：工具栏
    - 子节点属性：
        - `id`：标识符
        - `text`：文本，默认为`id`按照大写字母分割字符串后用空格组合
        - `class`：命令类别，仅命令可用
        - `shortcut`/`shortcuts`：仅命令，仅命令可用
        - `category/categories`：静态目录标签
        - `mode`：枝干节点模式，可选值为`plain`、`unique`、`top`，组默认`plain`，菜单默认`unique`，菜单栏与工具栏默认`top`
    - 注意事项：
        - `shortcut`与`category`可包含一个列表，使用`;`作为分隔符，`\`作为转义符（与 C 语言一致）
- `layouts`：包含具有组织关系的布局声明；
    - 子节点标签：与`objects`的子节点标签一致；
    - 子节点属性：
        - `id`：标识符
        - `flat`：是否平铺，菜单、菜单栏、工具栏可用，属于顶级菜单但不处于顶级的必须设为`true`
        - `_seq`：实例标识符，只有非顶级菜单和组可指定
            - 默认为从 0 开始的递增编号，不属于文件数据
            - 如果指定了已经出现过的值，那么从对应的实例复制布局，若出现无限递归则会报错
            - 如果没有子节点，则默认为第一个实例的`_seq`值
            - 如果指定为空，则忽略上一条规则，解析为编号
            - 如果指定为纯数字，且数字大于当前编号，则视为上一条规则的情况
    - 注意事项：
        - 如果`id`为`objects`中没有出现的，则将解析为新的菜单元素，除 ID 外其他属性为空，否则，其类型必须与`objects`中声明的类型一致；
        - 如果`objects`中某个菜单元素没有声明`category`，那么`category`将按以下规则确定：
            - 如果它没有出现在`layout`中，或者在`layout`中第一次作为根节点出现，那么使用`parserConfig`中的`defaultCategory`加上推测名称，如果当前节点声明了`_cat`则推测名称为对应值，否则推测名称为去掉所有加速键与末尾省略号的`text`；
            - 如果在`layout`中第一次作为非根节点出现，将以它父节点的`category`属性加上推测名称；
        - `category`如果前面有空项，则全部忽略；如果后面有空项，则用其`text`（去掉所有加速键）代替；
        - `mode`为`unique`的菜单或组作为非叶子节点最多只允许出现一次（即只允许声明一次其下级结构），类型为`action`的菜单元素不允许作为非叶子节点出现；
- `buildRoutines`：包含构造例程，子节点标签`buildRoutine`；
    - `buildRoutine`属性：
        - `anchor`：插入方式，可选值为`last`、`first`、`before`或`after`；
        - `parent`：插入对象 ID；
        - `relativeTo`：插入相对的子节点 ID，在`anchor`为`before`或`after`时需指定；
    - `buildRoutine`的子节点属性与`layouts`中的一致，必须是线性结构；

所有节点属性与节点值都会展开所有使用`${XXX}`引用的变量。


## 扩展元数据代码生成器

`ckaec`（全称 ChorusKit Action Extension Compiler）是通过读取清单生成`cpp`文件的元编译器，生成一份`ActionExtension`的静态数据的实现，以及适用于 Qt 语言家的字符串代码。

命令行参数
```sh
> ckaec --help
Usage: ckaec.exe [options] <file>
ChorusKit Action Extension Compiler version X.X.X.X (Qt X.X.X)

Options:
  -o <file>         Write output to file rather than stdout.
  -i <identifier>   Override extension identifier rather than the file name.
  -D <macro[=def]>  Define a variable.
  -?, -h, --help    Displays help on commandline options.
  -v, --version     Displays version information.

Arguments:
  <file>            Manifest file to read from.
```

生成的文件为`ckaec_core_actions.cpp`，需要共同参与编译链接，支持 Qt 语言家工具处理生成翻译文件。

在用户代码中获取该`ActionExtension`实例，使用以下方法获取：
```c++
CK_STATIC_ACTION_EXTENSION_GETTER(core_actions, getMyActionExtension)

namespace Core {
    void initialize() {
        const ActionExtension *ext = getMyActionExtension();
        // 后续处理
    }
}
```