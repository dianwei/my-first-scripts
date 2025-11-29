# Complex Expression Evaluator

一个用于展示 C++ 作业成果的复数表达式解析器。项目将核心逻辑切分为多个模块，以便阅读与复用。

## 功能概览
- 解析包含 `+ - * / =`、括号、共轭 `con(z)` 与模长 `mod(z)` 的复数表达式。
- 支持复数字面量：`3.14`、`.5`、`1e10`、`2.5i`、`-i`、`i` 等。
- REPL 支持命令：
  - `help`：查看帮助
  - `format sci` / `format fixed`：切换科学计数法或普通十进制输出
  - `precision N`：设置小数位数
  - `quit` / `exit`：退出

## 目录结构
```
include/
  complex_eval/
    big_complex.hpp   // 高精度复数类型
    calculator.hpp    // 运算符栈求值逻辑
    format.hpp        // 输出格式配置与字符串化
    scanner.hpp       // 词法分析，文本 -> tokens
    token.hpp         // 运算符枚举、优先级、Token 定义
    main.cpp          // REPL 入口（可单独编译运行）
```

## 构建与运行
在 VS Code 中使用任务 `C/C++: g++.exe 生成活动文件`，或直接在 PowerShell 中执行：

```powershell
g++ include/complex_eval/main.cpp -std=c++20 -Iinclude -o main.exe
./main.exe
```

## 拓展注意
本项目定位为课程演示，结构已稳定。若需扩展（如新增函数、改进格式化），建议在 `complex_eval` 中增加对应头文件，并在 `main.cpp` 中接入即可。
