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
* zcgg��˧!
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


	//	////��������: S'->S,S->BB,B->aB,B->b
	//	//{Tag::S0,{Tag::S}},
	//	//{Tag::S,{Tag::B,Tag::B}},
	//	//{Tag::B,{Tag::a,Tag::B}},
	//	//{Tag::B,{Tag::b}},

	//	//��������: S'->S,S->BB,B->b,B->��
	//	{Tag::S0,{Tag::S}},
	//	{Tag::S,{Tag::B,Tag::B}},
	//	{Tag::B,{Tag::b}},
	//	{Tag::B,{}},
	//};
}

LR1_Parser::~LR1_Parser()
{
}

State LR1_Parser::openGrammarFile(const char* filepath)	//�����ķ�����ʽ
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
		grammar_list[0].right.push_back(grammar_list[1].left);	//�����ع��ķ�
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
		first_list[Tag(i)] = { Tag(i) };	//�ս��First��Ϊ����

	vector<int> grammar_pointer;	//��¼����ʽ�Ҳ���һ������Ϊ���ս�����ķ�
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
					//����A->A..���������
					if (grammar_list[i].left == elem_A) {
						if (first_list[elem_A].count(Tag::epsilon))
							continue;
						else
							break;
					}
					//������A->B...,��B��first��ȫ���ӵ�A��
					for (const auto& elem_B : first_list[elem_A]) {
						if (elem_B == Tag::epsilon) {
							have_epsilon = true;
							continue;	//epsilon������
						}
						int before = first_list[grammar_list[i].left].size();
						first_list[grammar_list[i].left].insert(elem_B);
						int after = first_list[grammar_list[i].left].size();
						if (before < after)
							flag = true;
					}
					if (!have_epsilon)
						break;	//��������,��������ü�������
				}
				else
					break;
			}
			if (have_epsilon)	//�������ʽ���һ������Ҳ����,�򽫿ռ���First��
				first_list[grammar_list[i].left].insert(Tag::epsilon);
		}
		if (!flag)	//���first����������,�򷵻�
			break;
	}
}

set<GrammarProject> LR1_Parser::getClosure(const set<GrammarProject>& project_set)
{
	set<GrammarProject> ret(project_set);			//project_set�����������Ŀ���ڱհ���
	set<GrammarProject> old_project(project_set);	//��������
	set<GrammarProject> new_project;

	bool flag;
	while (true) {
		flag = false;
		for (const auto& i : old_project) {	//ɨ����һ�β�����������Ŀ
			if (grammar_list[i.p_grammar].right.size() > i.point && isVN(grammar_list[i.p_grammar].right[i.point])) {
				//A->��.B����
				Tag vn = grammar_list[i.p_grammar].right[i.point];

				//���first(��a)
				set<Tag> firstba;
				if (i.point + 1 < grammar_list[i.p_grammar].right.size()) {
					firstba = first_list[grammar_list[i.p_grammar].right[i.point + 1]];
					auto p = firstba.find(Tag::epsilon);
					if (p != firstba.cend()) {
						//�������epsilon,��ɾ��epsilon����ԭ��Ŀ��follows����
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
					//ɨ������B->���͵Ĳ���ʽ
					if (grammar_list[gp].left == vn) {
						//��CLOSURE�в�����{B->��,firstba},�����
						bool have = false;
						for (auto it = ret.begin(); it != ret.end(); ++it) {
							if (it->p_grammar == gp && it->point == 0) {
								//��Ŀ�ڼ���
								have = true;
								if (it->follows != firstba) {
									//��follows������,������µ�follows
									flag = true;
									//���ڼ���Ԫ�ص�ֵ�޷��޸�,��ֻ�ܸ���֮
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
							//�����������Ŀ
							flag = true;
							ret.insert({ gp,0,firstba });
							new_project.insert({ gp,0,firstba });
						}
					}
				}
			}
		}
		if (!flag)	//��������,�򷵻�
			break;
		old_project = new_project;	//���������Ŀ������һ��ɨ��
		new_project.clear();
	}

	return ret;
}

int LR1_Parser::findSameProjectSet(const set<GrammarProject>& new_pset)
{
	for (int i = 0; i < project_set_list.size(); i++) {
		if (project_set_list[i].size() != new_pset.size())	//���Ȳ�ͬһ�����ظ�
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
	return -1;	//δ�ҵ�
}

State LR1_Parser::initActionGotoMap()
{
	project_set_list.clear();
	//��ʼ״̬ΪCLOSURE({S0->.program,#})
	project_set_list.emplace_back(getClosure({ { 0,0,{Tag::the_end} } }));

	int new_index = 0;		//����Ŀ���±�
	while (new_index < project_set_list.size()) {
		set<GrammarProject>& pset_now = project_set_list[new_index];
		map<Tag, set<GrammarProject>> new_pset_map;		//��ǰ��Ŀ�����Բ���������Ŀ��

		//ɨ��������Ŀ
		for (const auto& i : pset_now) {
			if (i.point < grammar_list[i.p_grammar].right.size()) {
				//���ǹ�Լ��Ŀ
				new_pset_map[grammar_list[i.p_grammar].right[i.point]].insert({ i.p_grammar,i.point + 1,i.follows });
			}
			else {
				//�ǹ�Լ��Ŀ
				if (i.p_grammar == 0 && i.point == 1 && i.follows.size() == 1 && *i.follows.cbegin() == Tag::the_end)
					action_go_map[new_index][Tag::the_end] = { Action::accept,i.p_grammar };	//�ɽ���״̬
				else {
					for (const auto& follow : i.follows) {
						if (action_go_map[new_index].count(follow))
							return State::ERROR;	//���ת�Ʊ�����Ѿ��ж���,������������,����LR(1)�ķ�,����
						else
							action_go_map[new_index][follow] = { Action::reduction,i.p_grammar };	//�øò���ʽ��Լ
					}
				}
			}
		}

		//������closure������дת�Ʊ�
		for (const auto& i : new_pset_map) {
			set<GrammarProject> NS = getClosure(i.second);	//������closure��
			int it = findSameProjectSet(NS);				//����
			if (it == -1) {
				project_set_list.emplace_back(NS);
				action_go_map[new_index][i.first] = { Action::shift_in, int(project_set_list.size()) - 1 };	//�ƽ�
			}
			else {
				action_go_map[new_index][i.first] = { Action::shift_in, it };	//�ƽ�
			}
		}

		++new_index;	//������һ����Ŀ����ת�ƹ�ϵ
	}

	return State::OK;
}

/*********************************************************************************************************************
* parser����ֱ�ӵ�����getNextLexical����������lexer��ʱ��û��Ϊ��ָ���ļ�·��file_in��������parser�Ĺ��캯����ָ��һ��
* ��Լ��ʱ����Թ����﷨���������ȿ��ˣ��������������ټӽ�ȥ����
**********************************************************************************************************************/
State LR1_Parser::parser(const char* src_path, Token& err_token)
{
    err_token.line = err_token.col = 0;
    if (!this->lexer.openFile(src_path))
        return State::ERROR;

    stack<int> SStack;	//״̬ջ
    stack<Tag> TStack;	//����ջ
    stack<int> NStack;  //�����ջ��������ڵ��±�

    SStack.push(0);		//��ʼ��
    TStack.push(Tag::the_end);	//��ʼ��
    //NStack.push(-1);			//��ʼ��

    bool use_lastToken = false;	//�ж��Ƿ�ʹ���ϴε�token
    Token t_now;	//��ǰtoken
    int s_now;		//��ǰstate
    Movement m_now;	//��ǰ����
    while (true) {
        //��Ҫ�»�ȡһ��token
        if (!use_lastToken) {
            State ret = this->lexer.getNextLexical(t_now);
            if (ret == State::ERROR)
                return ret;
        }
        s_now = SStack.top();						//��ȡ��ǰ״̬
        if (action_go_map.count(s_now) == 0 || action_go_map[s_now].count(t_now.tag) == 0) {
            //����Ӧ�����Ϊ��,�����
            err_token = t_now;
            return State::ERROR;
        }
        m_now = action_go_map[s_now][t_now.tag];	//��ȡ��ǰ����
            //�ƽ�
        if (m_now.action == Action::shift_in) {
            SStack.push(m_now.go);
            TStack.push(t_now.tag);

            TNode node_in;	//�ƽ��������
            node_in.tag = t_now.tag;	//��ʼ��tagֵ
            node_in.p = pTree.TNode_List.size();	//ָ�����ڵ���TNode_List�е��±�
            pTree.TNode_List.push_back(node_in);	//�ƽ������
            NStack.push(node_in.p);					//�����ڵ��±��ƽ���ջ����֤ջ�ڽ���TNode_List�еĽ��һһ��Ӧ��

            use_lastToken = false;
        }	//��Լ
        else if (m_now.action == Action::reduction) {
            int len = grammar_list[m_now.go].right.size();	//����ʽ�Ҳ�����

            TNode node_left;								//����ʽ��
            node_left.tag = grammar_list[m_now.go].left;	//����ʽ��tag
            node_left.p = pTree.TNode_List.size();			//�ƽ������

            //�Ƴ�ջ
            while (len-- > 0) {
                SStack.pop();
                TStack.pop();

                node_left.childs.push_front(NStack.top());	//�����ӽ������
                NStack.pop();
            }

            pTree.TNode_List.push_back(node_left);			//�ƽ���ջ

            s_now = SStack.top();	//���µ�ǰ״̬
            if (action_go_map.count(s_now) == 0 ||
                action_go_map[s_now].count(node_left.tag) == 0) {
                //����Ӧ�����Ϊ��,�����
                err_token = t_now;
                return State::ERROR;
            }

            m_now = action_go_map[s_now][node_left.tag];	//���µ�ǰ����
            //��ջ����
            SStack.push(m_now.go);
            TStack.push(node_left.tag);
            NStack.push(node_left.p);

            use_lastToken = true;
        }
        else //����
        {
            pTree.RootNode = pTree.TNode_List.size() - 1;	//����㼴Ϊ���һ���ƽ�����㼯�Ľ��
            return State::OK;		//accept
        }
    }
}

void LR1_Parser::printTree(ostream& out)
{
	if (pTree.RootNode == -1)	//û�и��ڵ㣬���������ڣ�û�û���
		return;
	queue<int> Q;
	out << "digraph parser_tree{" << endl;
	out << "rankdir=TB;" << endl;
	//��ʼ�����
	for (int i = 0; i < pTree.TNode_List.size(); i++)
	{
		out << "node_" << i << "[label=\"" << TAG2STR.at(pTree.TNode_List[i].tag) << "\" ";
		out << "shape=\"";
		if (isVT(pTree.TNode_List[i].tag)) //�ս������ɫ���壬��Բ��
			out << "none\" fontcolor=\"blue\"];" << endl;
		else                               //���ս������ɫ���壬��Բ��
			out << "box\" fontcolor=\"black\"];" << endl;
	}
	out << endl;

	Q.push(pTree.RootNode);	//���ڵ�����У�������ʼBFS����﷨��
	while (!Q.empty())
	{
		TNode node = pTree.TNode_List[Q.front()];	//ȡ��һ����㣬������л���
		Q.pop();

		if (node.childs.size() == 0)	//�����ӽ�㣬���û���������
			continue;
		//�����ӽ�㣬��������
		for (auto it = node.childs.cbegin(); it != node.childs.cend(); it++)	//�������ӹ�ϵ
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
	//����ÿһ����Ŀ��
	for (int i = 0; i < project_set_list.size(); i++)
	{
		out << "node_" << i << "[label=\"";
		//�����Ŀ���е�ÿһ����Ŀ
		for (const auto& gp : project_set_list[i])
		{
			//�������ʽ
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
			//���follows
			for (auto it = gp.follows.cbegin(); it != gp.follows.cend(); it++)
			{
				if (it != gp.follows.cbegin())
					out << "/";
				out << convTag2Str(*it);
			}
			out << "\n";
		}
		//�����������
		out << "\" shape=\"box\"];" << endl;
	}

	//����ת�ƹ�ϵ
	for (int i = 0; i < project_set_list.size(); i++)
	{
		for (const auto& tag_mov : action_go_map[i])
		{
			//ֻ���ƽ��Ż�ת��
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
    //�����
    this->pTree.TNode_List.clear();
    this->pTree.RootNode = -1;
    //���grammar_list��first_list��
    this->grammar_list.clear();
    this->first_list.clear();
    this->project_set_list.clear();
    this->action_go_map.clear();
    //���lexer������
    this->lexer.clear_data();
}
/*int main()
{
	LR1_Parser lr1;

	//lr1.init(R"(D:\Mydata\homework\������\�γ�\����ԭ��\����ҵ\test\grammar.txt)");

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
