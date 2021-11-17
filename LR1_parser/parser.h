#ifndef PARSER_H
#define PARSER_H
#include "tag.h"
#include "lexer.h"
#include <vector>
#include <set>
#include <map>
#include <list>
#include <fstream>

using std::vector;
using std::set;
using std::map;
using std::list;
using std::ostream;

struct TNode	//树结点
{
	Tag tag;	//tag值
	int p;		//指针
	list<int> childs;	//孩子结点集
};

struct PTree	//语法树
{
	vector<TNode> TNode_List;	//结点集合
	int RootNode = -1;				//根结点指针
};

struct Grammar	//文法,left->right
{
	Tag left;
	vector<Tag> right;
};

enum class Action		//LR1分析时的动作枚举值
{
	shift_in,	//移进
	reduction,	//归约
	accept,		//接受
	//由于使用map存储动作,故出错状态不再存储
};

struct Movement		//LR分析表中的一项动作
{
	Action action;
	int go;		//当action为s时,表示转到的状态;当action为r时,表示使用第几个产生式归约
};

struct GrammarProject	//LR(1)项目
{
	int p_grammar;			//该项目的产生式指针,存储产生式在vector中对应的下标
	int point;				//点的位置
	/* S->.E  point=0 */
	/* S->E.  point=1 */

	set<Tag> follows;		//项目后面可以跟随的终结符
	/* S->.E,#/a/b	follows={# a b} */


	bool operator<(const GrammarProject& gp) const {
		//定义比较函数使得GrammarProject可以放入set或map中.注意必须使得排序固定
		if (this->p_grammar < gp.p_grammar)
			return true;
		else if (this->p_grammar > gp.p_grammar)
			return false;
		else if (this->point < gp.point)
			return true;
		else if (this->point > gp.point)
			return false;
		else if (this->follows.size() < gp.follows.size())
			return true;
		else if (this->follows.size() > gp.follows.size())
			return false;
		else {
			for (auto it1 = this->follows.cbegin(), it2 = gp.follows.cbegin();
				it1 != this->follows.cend();
				++it1, ++it2) {
				if (*it1 < *it2)
					return true;
				else if (*it1 > * it2)
					return false;
			}
			return false;
		}
	}

	bool operator==(const GrammarProject& gp) const {
		//重载运算符，方便项目间比较
		return this->p_grammar == gp.p_grammar && this->point == gp.point && this->follows == gp.follows;
	}
	bool operator!=(const GrammarProject& gp) const {
		//重载运算符，方便项目间比较
		return !(*this == gp);
	}
};

class LR1_Parser
{
private:
    Lexer lexer;						//词法分析器

    vector<Grammar> grammar_list;		//文法集合
    map<Tag, set<Tag>> first_list;		//非终结符first集
    //map<Tag, set<Tag>> follow_list;	//非终结符follow集
    vector<set<GrammarProject>> project_set_list;	//项目集
    //map<int, map<Tag, int>> state_trans_map;		//项目之间的转移关系(int存储项目集的下标)
    map<int, map<Tag, Movement>> action_go_map;		//action表和goto表,存储在一起

    PTree pTree;						//语法树

private:
    State openGrammarFile(const char*);	//读入文法产生式
    set<GrammarProject> getClosure(const set<GrammarProject>&);	//求CLOSURE集
    int findSameProjectSet(const set<GrammarProject>&);			//查找相同的CLOSURE集，失败返回-1
    void initFirstList();		//初始化First集
    State initActionGotoMap();	//求识别活前缀的DFA

public:		//记得改为private
    LR1_Parser();
    ~LR1_Parser();

    State init(const char*);			//语法分析器初始化
    State parser(const char*, Token&);	//语法分析
    void printTree(ostream& out);		//打印树
    void printVP_DFA(ostream& out);     //打印DFA
    void clear_all();                   //清除上一轮数据
};

#endif // !PARSER_H
