#ifndef LEXER_H
#define LEXER_H

#include "tag.h"
#include <string>
#include <fstream>

using std::string;
using std::ifstream;

struct Token	//词法单元,由类型和属性值构成
{
	Tag tag;		//词法单元类型
	int line;		//记录出错位置
	int col;
	string value;	//属性值,空值用null表示,全部用str存储,需要时使用sstream解析
};

class Lexer
{
private:
	char peek;
	int line;		//当前行
	int col;		//当前列
	ifstream file_in;

	bool getNextChar(char&, const bool = true);

public:
	Lexer();
	~Lexer();

	//打开输入文件
	bool openFile(const char*);

    //清空数据
    void clear_data();

	//获取下一个词法单元
	State getNextLexical(Token& next_token);
	//扫描整个文件并输出词法分析结果
	State scanFile(const char*);
};

#endif // !LEXER_H
