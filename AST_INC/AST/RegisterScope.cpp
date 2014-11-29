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
        bb->DEcalc(m);
        bb->UEcalc(m);
        bb->Killcalc(m);
    }

    auto change = false;
    do
    {
        for (auto&& bb : allBlocks(graph))
        {
            printset(bb->getLabel(), bb->parents);
            if(bb->Avicalc(m)) change = true;
            if(bb->Avocalc(m)) change = true;
        }
    } while(change);


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
