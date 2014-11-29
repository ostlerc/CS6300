#include "RegisterScope.hpp"
#include "BasicBlock.hpp"
#include <vector>

using namespace std;

cs6300::Motion cs6300::Motion::init(std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> graph)
{
    Motion m;
    m.graph = graph;
    m.nmapcalc();
    m.printmap();
    for (auto&& bb : allBlocks(graph))
    {
        bb->calcall(m);
    }
    return m;
}

void cs6300::Motion::optimize(std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> graph)
{
    init(graph);
}

void cs6300::Motion::printmap()
{
    std::cout << "map:";
    for(auto& v: nmap)
        std::cout << "(" << v.first << " " << v.second->key << ") ";
    std::cout << std::endl;
}

void cs6300::Motion::nmapcalc()
{
    nmap[108] = new ExprNode("5");
}

void printset(std::string name, std::set<int> s)
{
    std::cout << name << ": ";
    for(auto&i : s)
        std::cout << i << " ";
    std::cout << std::endl;
}
