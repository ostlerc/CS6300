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

struct ExprNode
{
    ExprNode(std::string key) : key(key) {}
    std::string key;
};

void printset(std::string name, std::set<int> s);
std::set<std::shared_ptr<cs6300::BasicBlock>> allBlocks(
        std::pair<std::shared_ptr<cs6300::BasicBlock>,
        std::shared_ptr<cs6300::BasicBlock>> b);

struct MotionSet
{
    std::set<int> DE; // downwardly exposed
    std::set<int> UE; // upwardly exposed
    std::set<int> Avo; // available out
    std::set<int> Avi; // available int
    std::set<int> Ano; // anticipatible out
    std::set<int> Ani; // anticipatible in

    void printall()
    {
        printset("DE", DE);
        printset("UE", UE);
        printset("Avo", Avo);
        printset("Avi", Avi);
        printset("Ano", Ano);
        printset("Ani", Ani);
    }
};

struct Motion
{
    std::map<int,ExprNode*> nmap; // node map

    std::pair<std::shared_ptr<BasicBlock>,std::shared_ptr<BasicBlock>> graph;

    static void optimize(std::pair<std::shared_ptr<BasicBlock>,
            std::shared_ptr<BasicBlock>>);

    static Motion init(std::pair<std::shared_ptr<BasicBlock>,
            std::shared_ptr<BasicBlock>>);

    void nmapcalc();
    void printmap();
};

struct ExprTree
{
    std::set<ExprNode*> nodes;
};

}
#endif
