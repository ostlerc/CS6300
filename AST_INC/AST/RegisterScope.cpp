#include "RegisterScope.hpp"
#include "BasicBlock.hpp"
#include "ThreeAddressInstruction.hpp"

#include <vector>
#include <algorithm>
#include <set>
#include <iterator>

using namespace std;

cs6300::Motion cs6300::Motion::init(std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> graph)
{
    Motion m;
    m.graph = graph;
    m.nmapcalc();
    m.printmap();

    // setup DE, UE, and Kill sets
    for (auto&& bb : allBlocks(graph))
    {
        bb->DEcalc(m);
        bb->UEcalc(m);
    }

    //Initialize available and anticipatible sets
    for (auto&& bb : allBlocks(graph))
        bb->InitMotion(m);

    //crunch available and anticipatible sets
    bool change = false;
    do
    {
        change = false;
        for (auto&& bb : allBlocks(graph))
        {
            if(bb->Avcalc(m)) change = true;
            if(bb->Ancalc(m)) change = true;
        }
    } while(change);

    for (auto&& bb : allBlocks(graph))
    {
        cout << endl << bb->getLabel() << endl;
        bb->mset.printall();
        auto latest = bb->LatestCalc();
        printset("jumpTo",latest.first);
        printset("branchTo",latest.second);
    }

    return m;
}

void cs6300::Motion::optimize(std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> graph)
{
    init(graph);
}

void cs6300::Motion::printmap()
{
    std::cout << "digraph G {" << std::endl;
    for(auto&& v: nmap)
    {
        std::cout << "\"$" << v.first << "\" -> \"" << v.second->key << "\";" << std::endl;
    }
    std::cout << std::endl;
    for(auto&& v: smap)
    {
        for(auto&& n : v.second->nodes)
            std::cout << "\"" << v.first << "\" -> \"" << n->key << "\";" << std::endl;
    }
    std::cout << "}" << std::endl;
}

void cs6300::Motion::nmapcalc()
{
    for (auto&& bb : allBlocks(graph))
    {
        for (auto i = bb->instructions.begin(); i != bb->instructions.end(); i++)
        {
            mapInstr(*i);
        }
    }
}

void cs6300::Motion::mapInstr(ThreeAddressInstruction tal)
{
    switch (tal.op)
    {
        case ThreeAddressInstruction::CallFunction: // none
        case ThreeAddressInstruction::WriteStr:
        case ThreeAddressInstruction::WriteInt:
            break;
        case ThreeAddressInstruction::LoadMemory:
            nmap[tal.dest] = createExpr(to_string(tal.src2) + "($" + to_string(tal.src1) + ")");
            break;
        case ThreeAddressInstruction::LoadMemoryOffset:
            break;
        case ThreeAddressInstruction::LoadValue: // LoadValue has constants
            nmap[tal.dest] = createExpr(to_string(tal.src1));
            break;
        case ThreeAddressInstruction::StoreMemory: // special case store memory dest is src1
            {
                auto e = createExpr(to_string(tal.src2) + "($" + to_string(tal.src1) + ")");
                e->nodes.insert(nmap[tal.dest]);
            }
            break;
        case ThreeAddressInstruction::AddValue:
            {
                auto rhs = createExpr(to_string(tal.src2));
                auto lhs = getExpr(tal.src1);
                std::string key = "("+lhs->key + ThreeAddressInstruction::opstr(tal.op) + rhs->key+")";
                auto e = createExpr(key);
                lhs->nodes.insert(e);
                rhs->nodes.insert(e);
                nmap[tal.dest] = e;
            }
            break;
        default:
            if(validReg(tal.dest)) nmap[tal.dest] = createExpr(tal.src1, tal.src2, tal.op);
    }
}

cs6300::ExprNode* cs6300::Motion::getExpr(int reg)
{
    if(!nmap.count(reg))
    {
        std::cerr << "error getting " << reg << std::endl;
        nmap[reg] = new ExprNode("__" + to_string(reg) + "__");
    }
    return nmap[reg];
}

cs6300::ExprNode* cs6300::Motion::getExpr(std::string reg)
{
    if(!smap.count(reg))
    {
        std::cerr << "error getting " << reg << std::endl;
        smap[reg] = new ExprNode("__" + reg + "__");
    }
    return smap[reg];
}

cs6300::ExprNode* cs6300::Motion::getExpr(cs6300::ThreeAddressInstruction tal)
{
    switch (tal.op)
    {
        case ThreeAddressInstruction::CallFunction: // none
        case ThreeAddressInstruction::Stop:
            break;
        case ThreeAddressInstruction::LoadValue: // LoadValue has constants
            return getExpr(to_string(tal.src1));
            break;
        case ThreeAddressInstruction::WriteStr:
        case ThreeAddressInstruction::WriteInt:
            return getExpr(tal.src1);
            break;
        case ThreeAddressInstruction::LoadMemory:
        case ThreeAddressInstruction::StoreMemory: // special case store memory dest is src1
            return getExpr(to_string(tal.src2) + "($" + to_string(tal.src1) + ")");
            break;
        case ThreeAddressInstruction::AddValue:
            {
                auto lhs = getExpr(tal.src1);
                auto rhs = getExpr(to_string(tal.src2));
                std::string key = "("+lhs->key + ThreeAddressInstruction::opstr(tal.op) + rhs->key+")";
                return createExpr(key);
            }
            break;
        default:
            auto lhs = getExpr(tal.src1);
            auto rhs = getExpr(tal.src2);
            std::string key = "("+lhs->key + ThreeAddressInstruction::opstr(tal.op) + rhs->key+")";
            return getExpr(key);
    }
    return NULL;
}

cs6300::ExprNode* cs6300::Motion::createExpr(std::string reg)
{
    if(!smap.count(reg))
    {
        //no error in here
        smap[reg] = new ExprNode(reg);
    }
    return smap[reg];
}

cs6300::ExprNode* cs6300::Motion::createExpr(int reg1, int reg2, int op)
{
    ExprNode* lhs = getExpr(reg1);
    ExprNode* rhs = getExpr(reg2);

    std::string key = "("+lhs->key + ThreeAddressInstruction::opstr(op) + rhs->key+")";
    ExprNode* ret = createExpr(key);
    lhs->nodes.insert(ret);
    rhs->nodes.insert(ret);
    return ret;
}

cs6300::ExprNode* cs6300::ExprNode::find(std::string key)
{
    for(auto&&n : nodes)
        if(n->key == key)
            return n;

    return NULL;
}

std::set<cs6300::ExprNode*> cs6300::ExprNode::recurse()
{
    std::set<ExprNode*> all = { this };
    for(auto&&i : nodes)
    {
        auto exprs = i->recurse();
        all.insert(exprs.begin(), exprs.end());
    }
    return all;
}

std::set<cs6300::ExprNode*> cs6300::MotionSet::latest(MotionSet m1, MotionSet m2, bool entry)
{
    auto ret = std::set<ExprNode*>();
    auto rhs = std::set<ExprNode*>();
    auto lhs = std::set<ExprNode*>();

    //Latest(i,j) = ANTIN(j) - AVAILOUT(i) ^ (Kill(i) - ANTOUT(i))

    std::set_difference(m2.Ani.begin(), m2.Ani.end(),
            m1.Avo.begin(), m1.Avo.end(),
            std::inserter(lhs, lhs.end())); //lhs = ANTIN(j) - AVAILOUT(i)

    if(entry) //entry block only uses lhs
        return lhs;

    std::set_difference(m1.Kill.begin(), m1.Kill.end(),
            m1.Ano.begin(), m1.Ano.end(),
            std::inserter(rhs, rhs.end())); //lhs = Kill(i) - ANTOUT(i)

    std::set_intersection(lhs.begin(), lhs.end(),
            rhs.begin(), rhs.end(),
            std::inserter(ret, ret.end())); //lhs = Kill(i) - ANTOUT(i)

    return ret;
}

void cs6300::MotionSet::printall()
{
    printset("DE", DE);
    printset("UE", UE);
    printset("Kill", Kill);
    printset("Avi", Avi);
    printset("Avo", Avo);
    printset("Ani", Ani);
    printset("Ano", Ano);
}

std::ostream& cs6300::operator<<(std::ostream& out, ExprNode* e)
{
    if(e)
        out << e->key;
    else
        out << "empty-node";
    return out;
}
