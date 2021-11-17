#ifndef LEXER_H
#define LEXER_H

#include "tag.h"
#include <string>
#include <fstream>

using std::string;
using std::ifstream;

struct Token	//�ʷ���Ԫ,�����ͺ�����ֵ����
{
	Tag tag;		//�ʷ���Ԫ����
	int line;		//��¼����λ��
	int col;
	string value;	//����ֵ,��ֵ��null��ʾ,ȫ����str�洢,��Ҫʱʹ��sstream����
};

class Lexer
{
private:
	char peek;
	int line;		//��ǰ��
	int col;		//��ǰ��
	ifstream file_in;

	bool getNextChar(char&, const bool = true);

public:
	Lexer();
	~Lexer();

	//�������ļ�
	bool openFile(const char*);

    //�������
    void clear_data();

	//��ȡ��һ���ʷ���Ԫ
	State getNextLexical(Token& next_token);
	//ɨ�������ļ�������ʷ��������
	State scanFile(const char*);
};

#endif // !LEXER_H
