#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <fstream>
#include <sstream>
#include <list>
#include <queue>

#include "parser.h"

using namespace std;

/********************************************************
* @Author : LeeQueue & Gao
*
* zcgg好帅!
*
********************************************************/


LR1_Parser::LR1_Parser()
{
	//grammar_list = {
	//	/*{Tag::E,{Tag::T,Tag::E1}},
	//	{Tag::E1,{Tag::sb_add,Tag::T,Tag::E1}},
	//	{Tag::E1,{Tag::epsilon}},
	//	{Tag::T,{Tag::F,Tag::T1}},
	//	{Tag::T1,{Tag::sb_time,Tag::F,Tag::T1}},
	//	{Tag::T1,{Tag::epsilon}},
	//	{Tag::F,{Tag::sb_llb,Tag::E,Tag::sb_rlb}},
	//	{Tag::F,{Tag::id}},*/


	//	////测试用例: S'->S,S->BB,B->aB,B->b
	//	//{Tag::S0,{Tag::S}},
	//	//{Tag::S,{Tag::B,Tag::B}},
	//	//{Tag::B,{Tag::a,Tag::B}},
	//	//{Tag::B,{Tag::b}},

	//	//测试用例: S'->S,S->BB,B->b,B->空
	//	{Tag::S0,{Tag::S}},
	//	{Tag::S,{Tag::B,Tag::B}},
	//	{Tag::B,{Tag::b}},
	//	{Tag::B,{}},
	//};
}

LR1_Parser::~LR1_Parser()
{
}

State LR1_Parser::openGrammarFile(const char* filepath)	//读入文法产生式
{
	ifstream in(filepath, ios::in);
	if (!in.is_open())
		return State::ERROR;

	grammar_list.push_back({ Tag::S0,{} });
	char buffer[1024];
	string temp;
	try {
		while (in.getline(buffer, 1024, '\n')) {
			istringstream ss(buffer);
			Grammar new_grammar;
			ss >> temp;
			new_grammar.left = convStr2Tag(temp);
			ss >> temp;
			while (ss >> temp)
				new_grammar.right.push_back(convStr2Tag(temp));
			grammar_list.push_back(new_grammar);
		}
		grammar_list[0].right.push_back(grammar_list[1].left);	//构造拓广文法
	}
	catch (const std::exception& e) {
		return State::ERROR;
	}
	return State::OK;
}

State LR1_Parser::init(const char* filepath)
{
	if (openGrammarFile(filepath) != State::OK)
		return State::ERROR;
	initFirstList();
	return initActionGotoMap();
}

void LR1_Parser::initFirstList()
{
	for (int i = int(Tag::epsilon); isVT(Tag(i)); ++i)
		first_list[Tag(i)] = { Tag(i) };	//终结符First集为自身

	vector<int> grammar_pointer;	//记录产生式右部第一个符号为非终结符的文法
	for (int i = 0; i < grammar_list.size(); i++) {
		if (grammar_list[i].right.size() == 0)
			first_list[grammar_list[i].left].insert(Tag::epsilon);
		else {
			auto first_elem = grammar_list[i].right.front();
			if (isVT(first_elem))
				first_list[grammar_list[i].left].insert(first_elem);
			else
				grammar_pointer.emplace_back(i);
		}
	}

	bool flag;
	while (true) {
		flag = false;
		for (const auto& i : grammar_pointer) {
			bool have_epsilon = false;
			for (const auto& elem_A : grammar_list[i].right) {
				have_epsilon = false;
				if (isVN(elem_A)) {
					//考虑A->A..的特殊情况
					if (grammar_list[i].left == elem_A) {
						if (first_list[elem_A].count(Tag::epsilon))
							continue;
						else
							break;
					}
					//若出现A->B...,则将B的first集全部加到A中
					for (const auto& elem_B : first_list[elem_A]) {
						if (elem_B == Tag::epsilon) {
							have_epsilon = true;
							continue;	//epsilon不加入
						}
						int before = first_list[grammar_list[i].left].size();
						first_list[grammar_list[i].left].insert(elem_B);
						int after = first_list[grammar_list[i].left].size();
						if (before < after)
							flag = true;
					}
					if (!have_epsilon)
						break;	//若不含空,则后续不用继续加入
				}
				else
					break;
			}
			if (have_epsilon)	//如果产生式最后一个符号也含空,则将空加入First集
				first_list[grammar_list[i].left].insert(Tag::epsilon);
		}
		if (!flag)	//如果first集不再增加,则返回
			break;
	}
}

set<GrammarProject> LR1_Parser::getClosure(const set<GrammarProject>& project_set)
{
	set<GrammarProject> ret(project_set);			//project_set自身的所有项目都在闭包中
	set<GrammarProject> old_project(project_set);	//辅助集合
	set<GrammarProject> new_project;

	bool flag;
	while (true) {
		flag = false;
		for (const auto& i : old_project) {	//扫描上一次产生的所有项目
			if (grammar_list[i.p_grammar].right.size() > i.point && isVN(grammar_list[i.p_grammar].right[i.point])) {
				//A->α.Bβ型
				Tag vn = grammar_list[i.p_grammar].right[i.point];

				//求出first(βa)
				set<Tag> firstba;
				if (i.point + 1 < grammar_list[i.p_grammar].right.size()) {
					firstba = first_list[grammar_list[i.p_grammar].right[i.point + 1]];
					auto p = firstba.find(Tag::epsilon);
					if (p != firstba.cend()) {
						//如果含有epsilon,则删除epsilon并把原项目的follows加入
						firstba.erase(p);
						for (const auto& follow : i.follows)
							firstba.insert(follow);
					}
				}
				else {
					for (const auto& follow : i.follows)
						firstba.insert(follow);
				}

				for (int gp = 0; gp < grammar_list.size(); gp++) {
					//扫描所有B->γ型的产生式
					if (grammar_list[gp].left == vn) {
						//若CLOSURE中不存在{B->γ,firstba},则加入
						bool have = false;
						for (auto it = ret.begin(); it != ret.end(); ++it) {
							if (it->p_grammar == gp && it->point == 0) {
								//项目在集合
								have = true;
								if (it->follows != firstba) {
									//若follows不完整,则插入新的follows
									flag = true;
									//由于集合元素的值无法修改,故只能覆盖之
									auto ngp = *it;
									for (Tag firstba_elem : firstba)
										ngp.follows.insert(firstba_elem);
									ret.erase(it);
									ret.insert(ngp);
									new_project.insert(ngp);
								}
								break;
							}
						}
						if (!have) {
							//否则插入新项目
							flag = true;
							ret.insert({ gp,0,firstba });
							new_project.insert({ gp,0,firstba });
						}
					}
				}
			}
		}
		if (!flag)	//不再增加,则返回
			break;
		old_project = new_project;	//对新添加项目进行下一轮扫描
		new_project.clear();
	}

	return ret;
}

int LR1_Parser::findSameProjectSet(const set<GrammarProject>& new_pset)
{
	for (int i = 0; i < project_set_list.size(); i++) {
		if (project_set_list[i].size() != new_pset.size())	//长度不同一定不重复
			continue;
		else {
			bool same = true;
			for (auto it1 = new_pset.cbegin(), it2 = project_set_list[i].cbegin();
				it1 != new_pset.cend();
				++it1, ++it2) {
				if (*it1 != *it2) {
					same = false;
					break;
				}
			}
			if (same)
				return i;
		}
	}
	return -1;	//未找到
}

State LR1_Parser::initActionGotoMap()
{
	project_set_list.clear();
	//初始状态为CLOSURE({S0->.program,#})
	project_set_list.emplace_back(getClosure({ { 0,0,{Tag::the_end} } }));

	int new_index = 0;		//新项目集下标
	while (new_index < project_set_list.size()) {
		set<GrammarProject>& pset_now = project_set_list[new_index];
		map<Tag, set<GrammarProject>> new_pset_map;		//当前项目集可以产生的新项目集

		//扫描所有项目
		for (const auto& i : pset_now) {
			if (i.point < grammar_list[i.p_grammar].right.size()) {
				//不是归约项目
				new_pset_map[grammar_list[i.p_grammar].right[i.point]].insert({ i.p_grammar,i.point + 1,i.follows });
			}
			else {
				//是归约项目
				if (i.p_grammar == 0 && i.point == 1 && i.follows.size() == 1 && *i.follows.cbegin() == Tag::the_end)
					action_go_map[new_index][Tag::the_end] = { Action::accept,i.p_grammar };	//可接受状态
				else {
					for (const auto& follow : i.follows) {
						if (action_go_map[new_index].count(follow))
							return State::ERROR;	//如果转移表该项已经有动作,则产生多重入口,不是LR(1)文法,报错
						else
							action_go_map[new_index][follow] = { Action::reduction,i.p_grammar };	//用该产生式归约
					}
				}
			}
		}

		//生成新closure集，填写转移表
		for (const auto& i : new_pset_map) {
			set<GrammarProject> NS = getClosure(i.second);	//生成新closure集
			int it = findSameProjectSet(NS);				//查重
			if (it == -1) {
				project_set_list.emplace_back(NS);
				action_go_map[new_index][i.first] = { Action::shift_in, int(project_set_list.size()) - 1 };	//移进
			}
			else {
				action_go_map[new_index][i.first] = { Action::shift_in, it };	//移进
			}
		}

		++new_index;	//处理下一个项目集的转移关系
	}

	return State::OK;
}

/*********************************************************************************************************************
* parser里我直接调用了getNextLexical函数，但是lexer此时并没有为其指明文件路径file_in，可以在parser的构造函数里指明一下
* 归约的时候可以构造语法树，这里先空了，等我们商量好再加进去，嘿
**********************************************************************************************************************/
State LR1_Parser::parser(const char* src_path, Token& err_token)
{
    err_token.line = err_token.col = 0;
    if (!this->lexer.openFile(src_path))
        return State::ERROR;

    stack<int> SStack;	//状态栈
    stack<Tag> TStack;	//输入栈
    stack<int> NStack;  //树结点栈，存放树节点下标

    SStack.push(0);		//初始化
    TStack.push(Tag::the_end);	//初始化
    //NStack.push(-1);			//初始化

    bool use_lastToken = false;	//判断是否使用上次的token
    Token t_now;	//当前token
    int s_now;		//当前state
    Movement m_now;	//当前动作
    while (true) {
        //需要新获取一个token
        if (!use_lastToken) {
            State ret = this->lexer.getNextLexical(t_now);
            if (ret == State::ERROR)
                return ret;
        }
        s_now = SStack.top();						//获取当前状态
        if (action_go_map.count(s_now) == 0 || action_go_map[s_now].count(t_now.tag) == 0) {
            //若对应表格项为空,则出错
            err_token = t_now;
            return State::ERROR;
        }
        m_now = action_go_map[s_now][t_now.tag];	//获取当前动作
            //移进
        if (m_now.action == Action::shift_in) {
            SStack.push(m_now.go);
            TStack.push(t_now.tag);

            TNode node_in;	//移进的树结点
            node_in.tag = t_now.tag;	//初始化tag值
            node_in.p = pTree.TNode_List.size();	//指定树节点在TNode_List中的下标
            pTree.TNode_List.push_back(node_in);	//移进树结点
            NStack.push(node_in.p);					//将树节点下标移进树栈（保证栈内结点和TNode_List中的结点一一对应）

            use_lastToken = false;
        }	//归约
        else if (m_now.action == Action::reduction) {
            int len = grammar_list[m_now.go].right.size();	//产生式右部长度

            TNode node_left;								//产生式左部
            node_left.tag = grammar_list[m_now.go].left;	//产生式左部tag
            node_left.p = pTree.TNode_List.size();			//移进树结点

            //移出栈
            while (len-- > 0) {
                SStack.pop();
                TStack.pop();

                node_left.childs.push_front(NStack.top());	//创建子结点链表
                NStack.pop();
            }

            pTree.TNode_List.push_back(node_left);			//移进树栈

            s_now = SStack.top();	//更新当前状态
            if (action_go_map.count(s_now) == 0 ||
                action_go_map[s_now].count(node_left.tag) == 0) {
                //若对应表格项为空,则出错
                err_token = t_now;
                return State::ERROR;
            }

            m_now = action_go_map[s_now][node_left.tag];	//更新当前动作
            //入栈操作
            SStack.push(m_now.go);
            TStack.push(node_left.tag);
            NStack.push(node_left.p);

            use_lastToken = true;
        }
        else //接受
        {
            pTree.RootNode = pTree.TNode_List.size() - 1;	//根结点即为最后一个移进树结点集的结点
            return State::OK;		//accept
        }
    }
}

void LR1_Parser::printTree(ostream& out)
{
	if (pTree.RootNode == -1)	//没有根节点，树都不存在，没得画咯
		return;
	queue<int> Q;
	out << "digraph parser_tree{" << endl;
	out << "rankdir=TB;" << endl;
	//初始化结点
	for (int i = 0; i < pTree.TNode_List.size(); i++)
	{
		out << "node_" << i << "[label=\"" << TAG2STR.at(pTree.TNode_List[i].tag) << "\" ";
		out << "shape=\"";
		if (isVT(pTree.TNode_List[i].tag)) //终结符，蓝色字体，无圆框
			out << "none\" fontcolor=\"blue\"];" << endl;
		else                               //非终结符，黑色字体，有圆框
			out << "box\" fontcolor=\"black\"];" << endl;
	}
	out << endl;

	Q.push(pTree.RootNode);	//根节点入队列，即将开始BFS输出语法树
	while (!Q.empty())
	{
		TNode node = pTree.TNode_List[Q.front()];	//取第一个结点，对其进行画树
		Q.pop();

		if (node.childs.size() == 0)	//若无子结点，不用画他的子树
			continue;
		//若有子结点，则画其子树
		for (auto it = node.childs.cbegin(); it != node.childs.cend(); it++)	//声明连接关系
		{
			out << "node_" << node.p << "->node_" << *it << ";" << endl;
			Q.push(*it);
		}
	}

	out << "}" << endl;
	return;
}

void LR1_Parser::printVP_DFA(ostream& out)
{
	out << "digraph{" << endl;
	out << "rankdir=LR;" << endl;
	//声明每一个项目集
	for (int i = 0; i < project_set_list.size(); i++)
	{
		out << "node_" << i << "[label=\"";
		//输出项目集中的每一个项目
		for (const auto& gp : project_set_list[i])
		{
			//输出产生式
			out << convTag2Str(grammar_list[gp.p_grammar].left) << "->";
            int p;
            for (p = 0; p < grammar_list[gp.p_grammar].right.size(); p++)
			{
				if (gp.point == p)
					out << ".";
				out << convTag2Str(grammar_list[gp.p_grammar].right[p]);
			}
            if (gp.point == p)
                out << ".";
			out << ", ";
			//输出follows
			for (auto it = gp.follows.cbegin(); it != gp.follows.cend(); it++)
			{
				if (it != gp.follows.cbegin())
					out << "/";
				out << convTag2Str(*it);
			}
			out << "\n";
		}
		//声明结点属性
		out << "\" shape=\"box\"];" << endl;
	}

	//声明转移关系
	for (int i = 0; i < project_set_list.size(); i++)
	{
		for (const auto& tag_mov : action_go_map[i])
		{
			//只有移进才会转移
			if (tag_mov.second.action != Action::shift_in)
				continue;
			else
				out << "node_" << i << "->node_" << tag_mov.second.go << "[label=\"" << convTag2Str(tag_mov.first) << "\"];" << endl;
		}
	}

	out << "}" << endl;
	return;
}

void LR1_Parser::clear_all()
{
    //清空树
    this->pTree.TNode_List.clear();
    this->pTree.RootNode = -1;
    //清空grammar_list、first_list等
    this->grammar_list.clear();
    this->first_list.clear();
    this->project_set_list.clear();
    this->action_go_map.clear();
    //清空lexer的数据
    this->lexer.clear_data();
}
/*int main()
{
	LR1_Parser lr1;

	//lr1.init(R"(D:\Mydata\homework\大三上\课程\编译原理\大作业\test\grammar.txt)");

	ofstream T_out;
	ofstream D_out;
    T_out.open(R"(C:\QT_Projects\LR1_parser\graph_Tree.dot)");
    D_out.open(R"(C:\QT_Projects\LR1_parser\graph_DFA.dot)");

    lr1.openGrammarFile(R"(C:\QT_Projects\LR1_parser\grammar.txt)");
	lr1.initFirstList();
	lr1.initActionGotoMap();

	lr1.printVP_DFA(D_out);

    lr1.parser(R"(C:\QT_Projects\LR1_parser\test_program.txt)");
	lr1.printTree(T_out);
	return 0;
}*/
