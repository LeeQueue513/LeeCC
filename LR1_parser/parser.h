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

struct TNode	//�����
{
	Tag tag;	//tagֵ
	int p;		//ָ��
	list<int> childs;	//���ӽ�㼯
};

struct PTree	//�﷨��
{
	vector<TNode> TNode_List;	//��㼯��
	int RootNode = -1;				//�����ָ��
};

struct Grammar	//�ķ�,left->right
{
	Tag left;
	vector<Tag> right;
};

enum class Action		//LR1����ʱ�Ķ���ö��ֵ
{
	shift_in,	//�ƽ�
	reduction,	//��Լ
	accept,		//����
	//����ʹ��map�洢����,�ʳ���״̬���ٴ洢
};

struct Movement		//LR�������е�һ���
{
	Action action;
	int go;		//��actionΪsʱ,��ʾת����״̬;��actionΪrʱ,��ʾʹ�õڼ�������ʽ��Լ
};

struct GrammarProject	//LR(1)��Ŀ
{
	int p_grammar;			//����Ŀ�Ĳ���ʽָ��,�洢����ʽ��vector�ж�Ӧ���±�
	int point;				//���λ��
	/* S->.E  point=0 */
	/* S->E.  point=1 */

	set<Tag> follows;		//��Ŀ������Ը�����ս��
	/* S->.E,#/a/b	follows={# a b} */


	bool operator<(const GrammarProject& gp) const {
		//����ȽϺ���ʹ��GrammarProject���Է���set��map��.ע�����ʹ������̶�
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
		//�����������������Ŀ��Ƚ�
		return this->p_grammar == gp.p_grammar && this->point == gp.point && this->follows == gp.follows;
	}
	bool operator!=(const GrammarProject& gp) const {
		//�����������������Ŀ��Ƚ�
		return !(*this == gp);
	}
};

class LR1_Parser
{
private:
    Lexer lexer;						//�ʷ�������

    vector<Grammar> grammar_list;		//�ķ�����
    map<Tag, set<Tag>> first_list;		//���ս��first��
    //map<Tag, set<Tag>> follow_list;	//���ս��follow��
    vector<set<GrammarProject>> project_set_list;	//��Ŀ��
    //map<int, map<Tag, int>> state_trans_map;		//��Ŀ֮���ת�ƹ�ϵ(int�洢��Ŀ�����±�)
    map<int, map<Tag, Movement>> action_go_map;		//action���goto��,�洢��һ��

    PTree pTree;						//�﷨��

private:
    State openGrammarFile(const char*);	//�����ķ�����ʽ
    set<GrammarProject> getClosure(const set<GrammarProject>&);	//��CLOSURE��
    int findSameProjectSet(const set<GrammarProject>&);			//������ͬ��CLOSURE����ʧ�ܷ���-1
    void initFirstList();		//��ʼ��First��
    State initActionGotoMap();	//��ʶ���ǰ׺��DFA

public:		//�ǵø�Ϊprivate
    LR1_Parser();
    ~LR1_Parser();

    State init(const char*);			//�﷨��������ʼ��
    State parser(const char*, Token&);	//�﷨����
    void printTree(ostream& out);		//��ӡ��
    void printVP_DFA(ostream& out);     //��ӡDFA
    void clear_all();                   //�����һ������
};

#endif // !PARSER_H
