#ifndef TAG_H
#define TAG_H

#include "common.h"
#include <string>
#include <map>

using std::string;
using std::map;

//语法单元的类型
enum class Tag
{
	//终结符
	epsilon = 0,	//空
	the_end,		//#,表示终止
	id,				//标识符
	num,			//数字,包括整数和小数
	//符号 symbol
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
	sb_board,		//界符 ;
	sb_separator,	//分隔符 ,
	sb_llb,			//左小括号 left little bracket
	sb_rlb,			//右小括号
	sb_lbb,			//左大括号 left big bracket
	sb_rbb,			//右大括号
	//关键字 keyword
	kw_int,			//int类型
	kw_void,
	kw_if,			//if语句关键字
	kw_else,
	kw_while,		//while关键字
	kw_return,		//return关键字


	//终结符和非终结符的分界线
	//此枚举值仅用作区分终结符和非终结符
	vtnboundary,


	//非终结符
	S0,				//拓广文法开始符号
	program,
	dec_list,		//declaration_list,声明列表
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

//关键字表,将关键字字符串转换为对应的Tag
extern const std::map<std::string, Tag> KEYWORD_STR2TAG;
//将字符串转换为相应的Tag,用于读入文法产生式
extern const std::map<std::string, Tag> STR2TAG;
#ifdef DEBUG
//返回Tag枚举值对应的字符串表示,便于调试
extern const std::map<Tag, std::string> TAG2STR;
#endif // DEBUG


inline bool isVT(const Tag t)	//判断Tag是否为终结符
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
