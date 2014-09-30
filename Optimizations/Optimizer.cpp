#include "Optimizer.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>

#include "../AST_INC/AST/Program.hpp"

using namespace std;

typedef shared_ptr<cs6300::BasicBlock> sBBlock;
typedef shared_ptr<cs6300::Program> sProgram;
typedef pair<sBBlock, sBBlock> blockPair;
typedef vector<shared_ptr<cs6300::Statement>> statements;

void cs6300::test()
{
    //testSubExprElim();
    testRegAlloc();
    cout << "all tests have passed" << endl;
}

/*Add new AST based optimizations here*/
sProgram cs6300::optimizer(sProgram original)
{
    return original;
}

void printInstructions(blockPair b)
{
    auto at = b.first;
    int count = 0;

    while(at)
    {
        cout << "----------" << endl;
        at->printInstructions();
        count++;
        at = at->jumpTo;
    }
    cout << "----------" << endl;
}

/*Add new control flow graph based optimizations here*/
blockPair cs6300::optimizeFlow(blockPair original)
{
    locRegAlloc(original);
    //printInstructions(original);
    return original;
}

struct RegColorNode
{
    int color;
    set<RegColorNode*> nodes;
    set<int> cant;
};

void cs6300::locRegAlloc(blockPair b)
{
    auto at = b.first;
    int count = 0;

    while(at)
    {
        at->initSets();
        at = at->jumpTo;
    }

    //propogate block meta
    bool change;
    do
    {
        change = false;
        at = b.first;
        while(at)
        {
            if(pushUp(at, at->jumpTo))
                change = true;

            if(pushUp(at, at->branchTo))
                change = true;

            at = at->jumpTo;
        }
    } while(change);

    map<int,RegColorNode*> nodes;
    at = b.first;
    while(at)
    {
        auto t = regDeps(at);
        auto s = set<set<int>>(t.begin(),t.end());
        for(auto& v : s)
        {
            for(int reg : v)
            {
                for(int nreg : v)
                {
                    if(!nodes.count(nreg)) nodes[nreg] = new RegColorNode{-1};
                    nodes[reg]->nodes.insert(nodes[nreg]);
                }
            }
        }

        at = at->jumpTo;
    }

    map<int,int> regRemap;
    //color nodes
    for(auto& p : nodes)
    {
        if(p.second->color != -1)
        {
            continue; //already colored
        }

        for(int i =8;; i++)
        {
            if(!p.second->cant.count(i))
            {
                p.second->color = i;
                regRemap[p.first] = i;
                //cout << "remapping " << p.first << " to " << i << endl;
                for(auto& n : p.second->nodes)
                {
                    n->cant.insert(i);
                }
                break;
            }
            if(i==24)
            {
                cout << "Too many colors tried" << endl;
                break;
            }
        }
    }

    at = b.first;
    while(at)
    {
        at->remap(regRemap);
        at = at->jumpTo;
    }
}

vector<set<int>> cs6300::regDeps(sBBlock b)
{
    vector<set<int>> res;

    set<int> prop = b->m.alive;
    for(auto i=b->instructions.rbegin(); i!=b->instructions.rend(); i++)
    {
        auto m = BasicBlock::meta(*i);
        prop.insert(m.used.begin(), m.used.end());
        vector<int> t;
        set_difference(
                prop.begin(), prop.end(),
                m.dead.begin(), m.dead.end(),
                back_inserter(t));
        prop = set<int>(t.begin(), t.end());
        res.insert(res.begin(), prop);
    }

    return res;
}

bool cs6300::pushUp(sBBlock parent, sBBlock child)
{
    if(!child) return false;

    vector<int> t;
    set_difference(
            child->m.alive.begin(), child->m.alive.end(),
            parent->m.dead.begin(), parent->m.dead.end(),
            back_inserter(t));

    int origSize = parent->m.alive.size();
    parent->m.alive.insert(t.begin(), t.end());

    return origSize != parent->m.alive.size();
}

void cs6300::testRegAlloc()
{
    vector<shared_ptr<Statement>> tstatements;
    auto one   = make_shared<BasicBlock>(); //declare i and j
    auto two   = make_shared<BasicBlock>(); //while condition
    auto three = make_shared<BasicBlock>(); //while body
    auto four  = make_shared<BasicBlock>(); //after while

    one->jumpTo = two;
    two->branchTo = four;
    two->jumpTo = three;
    three->jumpTo = four;

    one->instructions.emplace_back(ThreeAddressInstruction::LoadValue,1,12,0); // i := 12
    one->instructions.emplace_back(ThreeAddressInstruction::LoadValue,2,2,0);  // j := 2

    two->instructions.emplace_back(ThreeAddressInstruction::LoadValue,3,0,0);  // $3 := 0
    two->instructions.emplace_back(ThreeAddressInstruction::IsGreaterEqual,4,1,3); //$4 := i >= $3

    three->instructions.emplace_back(ThreeAddressInstruction::WriteInt,0,1,0);    // write(i)
    three->instructions.emplace_back(ThreeAddressInstruction::WriteInt,0,2,0);    // write(j)
    three->instructions.emplace_back(ThreeAddressInstruction::LoadValue,5,1,0);   // $5 := 1
    three->instructions.emplace_back(ThreeAddressInstruction::Subtract,6,1,5);    // $6 := i - $5
    three->instructions.emplace_back(ThreeAddressInstruction::StoreMemory,6,1,0); // i = $6

    four->instructions.emplace_back(ThreeAddressInstruction::WriteInt,0,1,0); // write(i)

    auto blocks = make_pair(one, four);
    cout << "before" << endl;
    printInstructions(blocks);
    locRegAlloc(blocks);
    cout << "after" << endl;
    printInstructions(blocks);
}

shared_ptr<cs6300::BasicBlock> cs6300::subExprElim(shared_ptr<cs6300::BasicBlock> b)
{
    map<string,int> opMap;
    map<int,int> dMap;
    auto ret = make_shared<cs6300::BasicBlock>();

    for(auto i : b->instructions)
    {
        if(i.op == ThreeAddressInstruction::StoreMemory) {
            for (auto iter = dMap.begin(); iter != dMap.end();) {
                    iter = dMap.erase(iter);
            }
        }
        auto key = i.key();
        if(opMap.count(key)) {
            auto dest = opMap[key];
            if (i.dest != dest) {
                dMap[i.dest] = dest;
            }
        } else {
            if(dMap.count(i.dest)) {
                i.dest = dMap[i.dest];
            }
            opMap[key] = i.dest;
        }
        if(!dMap.count(i.dest)) {
            cout << i.str() << endl;
            ret->instructions.emplace_back(i);
        }
    }

    return ret;
}

void cs6300::testSubExprElim()
{
    vector<shared_ptr<Statement>> tstatements;
    auto blocks = emitList(tstatements);

    blocks.first->instructions.emplace_back(ThreeAddressInstruction::LoadValue,1,2,0); //2
    blocks.first->instructions.emplace_back(ThreeAddressInstruction::StoreMemory,1,2,0); //a := 2
    blocks.first->instructions.emplace_back(ThreeAddressInstruction::LoadValue,3,2,0); // 2
    blocks.first->instructions.emplace_back(ThreeAddressInstruction::LoadValue,5,7,0); //7
    blocks.first->instructions.emplace_back(ThreeAddressInstruction::LoadValue,6,7,0); //7
    blocks.first->instructions.emplace_back(ThreeAddressInstruction::LoadValue,7,7,0); //7
    blocks.first->instructions.emplace_back(ThreeAddressInstruction::LoadValue,8,7,0); //7
    blocks.first->instructions.emplace_back(ThreeAddressInstruction::StoreMemory,3,4,0); // b := 2

    int s = subExprElim(blocks.first)->instructions.size();
    if(s != 5) {
        cerr << "Incorrect optimized size " << s << endl;
        exit(1);
    }
}
