# LeeGaoCC
- 同济大学编译原理课程作业，使用LR1分析实现类C语言的词法、语法分析器
- 以后会逐渐补充语义分析、中间代码生成部分

## 程序功能
- 输入文法产生式，自动生成语法分析器（相当于YACC），并画出识别活前缀的DFA
- 程序内置了类C语言的词法分析部分。输入类C语言文法和测试程序后，可以对测试程序进行LR1语法分析
- 语法分析结果以语法树形式展现，若出错则会报告出错位置和出错词法单元

## 文件目录
- LR1_parser：存储语法分析器的源代码（Qt）
- LR1_parser_EXE：存储可执行程序。双击该目录下的LR1_parser.exe即可运行程序，不需要Qt环境
- 设计说明.pdf：详细介绍词法、语法分析器实现过程，数据结构设计，实现难点，图形界面使用方法等，供参考

## 效果
![image](https://user-images.githubusercontent.com/92149428/142145385-52160d86-3efb-4088-a7a3-65e56b5e2848.png)
![image](https://user-images.githubusercontent.com/92149428/142145421-2375b2b8-09a4-40a7-830d-684ed9cc2963.png)
![image](https://user-images.githubusercontent.com/92149428/142145448-42e26752-20a0-4b61-acac-465524069233.png)
![image](https://user-images.githubusercontent.com/92149428/142145498-95a09124-2b0e-40d9-807d-a850af532b47.png)

