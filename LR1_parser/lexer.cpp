#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>

#include "tag.h"
#include "lexer.h"

using namespace std;

inline bool isWhiteSpace(const char ch)
{
	static const string WHITE_SPACE = " \n\t\r";
	return WHITE_SPACE.find(ch) != string::npos;
}

inline bool isLetter(const char ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

inline bool isDigit(const char ch)
{
	return ch >= '0' && ch <= '9';
}

bool Lexer::getNextChar(char& next_char, const bool ignore_whitespace)
{
	//ignore_whitespace 表示是否忽略空白符
	while (true) {
		if (this->peek == 0)
			return false;
		next_char = this->peek;
		if (next_char == '\n') {
			++this->line;
			this->col = 0;
		}
		else
			++this->col;
		if (!ignore_whitespace || !isWhiteSpace(next_char)) {
			//读入成功,再往后读一个
			if (!file_in.get(this->peek))
				this->peek = 0;
			return true;
		}
		else if (!file_in.get(this->peek))
			this->peek = 0;
	}
}

Lexer::Lexer()
{
	this->line = 1;
    this->col = 0;
}

Lexer::~Lexer()
{
	if (this->file_in.is_open())
		file_in.close();
}

void Lexer::clear_data()
{
    this->line = 1;
    this->col = 0;
}

bool Lexer::openFile(const char* filename)
{
	bool is_open;
    if (this->file_in.is_open())
        file_in.close();
	this->file_in.open(filename);
	is_open = this->file_in.is_open();
	if (is_open && !file_in.get(this->peek))
		this->peek = 0;
	return is_open;
}

State Lexer::getNextLexical(Token& next_token)
{
	next_token.value = "null";	//默认值,占位用
	char ch;
	if (!getNextChar(ch)) {
		next_token.tag = Tag::the_end;
		next_token.line = this->line;
        next_token.col = this->col;
		return State::END_OF_FILE;
	}
	next_token.line = this->line;
    next_token.col = this->col;

	if (isLetter(ch)) {
		//关键字 or 标识符
		string buffer;
		buffer += ch;
		while (true) {
			if (!isLetter(this->peek) && !isDigit(this->peek) ||
				!getNextChar(ch, false))
				break;
			else
				buffer += ch;
		}
		if (KEYWORD_STR2TAG.count(buffer)) {
			next_token.tag = KEYWORD_STR2TAG.at(buffer);
			next_token.value = buffer;
		}
		else {
			next_token.tag = Tag::id;
			next_token.value = buffer;
		}
	}
	else if (isDigit(ch) || ch == '.') {
		//int or double
		// 1 1. .98 1.67 1e9 1.e7等均为合法格式
		//负数可以在语法分析中求解

		//从左到右依次举例: 123 , 1.23 , 12. , .23 ,并且可以跟e+整数后缀
		regex reg(R";(^(\d+|\d+\.\d+|\d+\.|\.\d+)(e\d+)?$);");	//使用原始字符串,避免转义符出问题
		string buffer;
		buffer += ch;
		while (true) {
			if (!isDigit(this->peek) && this->peek != '.' && this->peek != 'e' ||
				!getNextChar(ch, false))
				break;
			else
				buffer += ch;
		}
		if (!regex_match(buffer, reg))
			return State::ERROR;
		next_token.tag = Tag::num;
		next_token.value = buffer;
	}
	else {
		//符号
		switch (ch)
		{
			case '+':
			case '-':
			case '*':
			case ';':
			case ',':
			case '(':
			case ')':
			case '{':
			case '}':
				next_token.tag = convStr2Tag(string("") + ch);
				break;
			case '/':
				if (this->peek == '/') {
					//单行注释
					while (getNextChar(ch, false) && ch != '\n')
						;
					if (ch != '\n') {
						next_token.tag = Tag::the_end;
						return State::END_OF_FILE;
					}
					else
						return getNextLexical(next_token);
				}
				else if (this->peek == '*') {
					//多行注释
					getNextChar(ch);	//去掉*
					while (true) {
						while (getNextChar(ch) && ch != '*')
							;
						if (ch != '*')	//有/*但是没有*/与之呼应
							return State::ERROR;
						if (this->peek == 0)	// /*后面没了
							return State::ERROR;
						if (this->peek == '/') {		//注释结束
							getNextChar(ch, false);		//去掉/
							return getNextLexical(next_token);
						}
					}
				}
				//一个简单的/号
				next_token.tag = Tag::sb_divide;
				break;
			case '=':
				if (this->peek == '=') {
					getNextChar(ch);	//去掉=
					next_token.tag = Tag::sb_equal;
				}
				else
					next_token.tag = Tag::sb_assign;
				break;
			case '>':
				if (this->peek == '=') {
					getNextChar(ch);	//去掉=
					next_token.tag = Tag::sb_ge;
				}
				else
					next_token.tag = Tag::sb_g;
				break;
			case '<':
				if (this->peek == '=') {
					getNextChar(ch);	//去掉=
					next_token.tag = Tag::sb_le;
				}
				else
					next_token.tag = Tag::sb_l;
				break;
			case '!':
				//以后需要扩展!运算符
				if (this->peek == '=') {
					getNextChar(ch);	//跳过=
					next_token.tag = Tag::sb_ne;
				}
				else
					return State::ERROR;
				break;
			default:
				return State::ERROR;
		}

	}
	return State::OK;
}

State Lexer::scanFile(const char* file_out_path)
{
	ofstream file_out(file_out_path);
	if (!file_out.is_open())
		return State::ERROR;

	Token t;
	int old_line = this->line;

	while (true) {
		State ret = getNextLexical(t);
		if (ret != State::OK)
			return ret;
		if (this->line != old_line) {
			//保证词法分析器输出的行与源文件的行一致
			file_out << endl;
			old_line = this->line;
		}
#ifdef DEBUG
		file_out << TAG2STR.at(t.tag) << ' ' << t.value << '\t';
#else
		file_out << (int)(t.tag) << ' ' << t.value << '\t';
#endif
	}

	return State::OK;
}


//int main()
//{
//	/*ifstream srcfile("../test/simple.c");
//	string buffer;*/
//	double a = .0;
//	double b = 0.;
//	double c = .8e23;
//
//
//	Lexer lex;
//	lex.openFile("D:/Mydata/homework/大三上/课程/编译原理/大作业/test/lex_test.txt");
//	/*Token t;
//	while (lex.getNextLexical(t) == State::OK) {
//		cout << TAG2STR.at(t.tag) << " " << endl;
//	}*/
//	lex.scanFile("D:/Mydata/homework/大三上/课程/编译原理/大作业/test/lex_test_out.txt");
//
//}
