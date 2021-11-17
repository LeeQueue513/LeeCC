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
	//ignore_whitespace ��ʾ�Ƿ���Կհ׷�
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
			//����ɹ�,�������һ��
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
	next_token.value = "null";	//Ĭ��ֵ,ռλ��
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
		//�ؼ��� or ��ʶ��
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
		// 1 1. .98 1.67 1e9 1.e7�Ⱦ�Ϊ�Ϸ���ʽ
		//�����������﷨���������

		//���������ξ���: 123 , 1.23 , 12. , .23 ,���ҿ��Ը�e+������׺
		regex reg(R";(^(\d+|\d+\.\d+|\d+\.|\.\d+)(e\d+)?$);");	//ʹ��ԭʼ�ַ���,����ת���������
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
		//����
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
					//����ע��
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
					//����ע��
					getNextChar(ch);	//ȥ��*
					while (true) {
						while (getNextChar(ch) && ch != '*')
							;
						if (ch != '*')	//��/*����û��*/��֮��Ӧ
							return State::ERROR;
						if (this->peek == 0)	// /*����û��
							return State::ERROR;
						if (this->peek == '/') {		//ע�ͽ���
							getNextChar(ch, false);		//ȥ��/
							return getNextLexical(next_token);
						}
					}
				}
				//һ���򵥵�/��
				next_token.tag = Tag::sb_divide;
				break;
			case '=':
				if (this->peek == '=') {
					getNextChar(ch);	//ȥ��=
					next_token.tag = Tag::sb_equal;
				}
				else
					next_token.tag = Tag::sb_assign;
				break;
			case '>':
				if (this->peek == '=') {
					getNextChar(ch);	//ȥ��=
					next_token.tag = Tag::sb_ge;
				}
				else
					next_token.tag = Tag::sb_g;
				break;
			case '<':
				if (this->peek == '=') {
					getNextChar(ch);	//ȥ��=
					next_token.tag = Tag::sb_le;
				}
				else
					next_token.tag = Tag::sb_l;
				break;
			case '!':
				//�Ժ���Ҫ��չ!�����
				if (this->peek == '=') {
					getNextChar(ch);	//����=
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
			//��֤�ʷ����������������Դ�ļ�����һ��
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
//	lex.openFile("D:/Mydata/homework/������/�γ�/����ԭ��/����ҵ/test/lex_test.txt");
//	/*Token t;
//	while (lex.getNextLexical(t) == State::OK) {
//		cout << TAG2STR.at(t.tag) << " " << endl;
//	}*/
//	lex.scanFile("D:/Mydata/homework/������/�γ�/����ԭ��/����ҵ/test/lex_test_out.txt");
//
//}
