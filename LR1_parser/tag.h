#ifndef TAG_H
#define TAG_H

#include "common.h"
#include <string>
#include <map>

using std::string;
using std::map;

//�﷨��Ԫ������
enum class Tag
{
	//�ս��
	epsilon = 0,	//��
	the_end,		//#,��ʾ��ֹ
	id,				//��ʶ��
	num,			//����,����������С��
	//���� symbol
	sb_add,			//+
	sb_sub,			//-
	sb_time,		//*
	sb_divide,		///
	sb_assign,		//=
	sb_equal,		//==
	sb_g,			//>
	sb_ge,			//>=
	sb_l,			//<
	sb_le,			//<=
	sb_ne,			//!=
	sb_board,		//��� ;
	sb_separator,	//�ָ��� ,
	sb_llb,			//��С���� left little bracket
	sb_rlb,			//��С����
	sb_lbb,			//������� left big bracket
	sb_rbb,			//�Ҵ�����
	//�ؼ��� keyword
	kw_int,			//int����
	kw_void,
	kw_if,			//if���ؼ���
	kw_else,
	kw_while,		//while�ؼ���
	kw_return,		//return�ؼ���


	//�ս���ͷ��ս���ķֽ���
	//��ö��ֵ�����������ս���ͷ��ս��
	vtnboundary,


	//���ս��
	S0,				//�ع��ķ���ʼ����
	program,
	dec_list,		//declaration_list,�����б�
	dec,
	dec_type,
	func_dec,
	var_dec,
	params,
	stat_block,
	param_list,
	param,
	pre_param,
	in_dec,
	stat_str,
	in_var_dec,
	pre_in_var_dec,
	stat,
	if_stat,
	while_stat,
	return_stat,
	assign_stat,
	expr,
	add_expr,
	comp_add_expr,
	comp_operator,
	term,
	pre_term,
	factor,
	pre_factor,
	func_type,
	call,
	argus,
	argu_list,
	pre_expr,
};

//�ؼ��ֱ�,���ؼ����ַ���ת��Ϊ��Ӧ��Tag
extern const std::map<std::string, Tag> KEYWORD_STR2TAG;
//���ַ���ת��Ϊ��Ӧ��Tag,���ڶ����ķ�����ʽ
extern const std::map<std::string, Tag> STR2TAG;
#ifdef DEBUG
//����Tagö��ֵ��Ӧ���ַ�����ʾ,���ڵ���
extern const std::map<Tag, std::string> TAG2STR;
#endif // DEBUG


inline bool isVT(const Tag t)	//�ж�Tag�Ƿ�Ϊ�ս��
{
	return t < Tag::vtnboundary;
}
inline bool isVN(const Tag t)
{
	return t > Tag::vtnboundary;
}

inline Tag convStr2Tag(const string& s)
{
	return STR2TAG.at(s);
}

inline string convTag2Str(const Tag& t)
{
	return TAG2STR.at(t);
}

#endif // !TAG_H
