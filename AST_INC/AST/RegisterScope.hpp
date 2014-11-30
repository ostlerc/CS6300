#ifndef CS6300_REGISTER_SCOPE_HPP
#define CS6300_REGISTER_SCOPE_HPP

#include <set>
#include <map>
#include <string>
#include <memory>
#include <iostream>

namespace cs6300
{
    class BasicBlock;
    class ThreeAddressInstruction;

#define START_REGISTER 8
#define END_REGISTER 26

struct RegisterScope
{
  std::set<int> dead;
  std::set<int> used;
  std::set<int> alive;

  void clear()
  {
    dead.clear();
    used.clear();
    alive.clear();
  }
};

template <class T>
void printset(std::string name, std::set<T> s)
{
    std::cout << name << ": [";
    for(auto&i : s)
        std::cout << i << ", ";
    std::cout << "]" << std::endl;
}
std::set<std::shared_ptr<cs6300::BasicBlock>> allBlocks(
        std::pair<std::shared_ptr<cs6300::BasicBlock>,
        std::shared_ptr<cs6300::BasicBlock>> b);

struct ExprNode
{
    ExprNode(std::string key) : key(key) {}
    std::string key;
    std::set<ExprNode*> nodes;
    ExprNode* find(std::string);
    std::set<ExprNode*> recurse();

};

struct MotionSet
{
    std::set<ExprNode*> DE; // downwardly exposed
    std::set<ExprNode*> UE; // upwardly exposed
    std::set<ExprNode*> Kill; // upwardly exposed
    std::set<ExprNode*> Avo; // available out
    std::set<ExprNode*> Avi; // available in
    std::set<ExprNode*> Ano; // anticipatible out
    std::set<ExprNode*> Ani; // anticipatible in

    void printall();

    std::set<ExprNode*> latest(MotionSet, MotionSet, bool entry = false);
};

std::ostream& operator<<(std::ostream&, ExprNode*);

struct Motion
{
    std::map<int,ExprNode*> nmap;
    std::map<std::string, ExprNode*> smap;

    std::pair<std::shared_ptr<BasicBlock>,std::shared_ptr<BasicBlock>> graph;

    static void optimize(std::pair<std::shared_ptr<BasicBlock>,
            std::shared_ptr<BasicBlock>>);

    static Motion init(std::pair<std::shared_ptr<BasicBlock>,
            std::shared_ptr<BasicBlock>>);

    void mapInstr(ThreeAddressInstruction);
    ExprNode* getExpr(int);
    ExprNode* getExpr(std::string);
    ExprNode* getExpr(ThreeAddressInstruction tal);
    ExprNode* createExpr(std::string);
    ExprNode* createExpr(int,int,int);
    void nmapcalc();
    void printmap();
};

}
#endif
