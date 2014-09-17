#include "Optimizer.hpp"
#include <iostream>
#include "../AST_INC/AST/Program.hpp"

using namespace std;

typedef shared_ptr<cs6300::BasicBlock> sBBlock;
typedef shared_ptr<cs6300::Program> sProgram;
typedef pair<sBBlock, sBBlock> blockPair;
typedef vector<shared_ptr<cs6300::Statement>> statements;

void cs6300::test()
{
    std::vector<std::shared_ptr<Statement>> tstatements;

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

    cout << "all tests have passed" << endl;
}

/*Add new AST based optimizations here*/
sProgram cs6300::optimizer(sProgram original)
{
    optimize(original->main);
    return original;
}

void printInstructions(blockPair b)
{
    auto at = b.first;

    while(at)
    {
        at->printInstructions();
        at = at->jumpTo;
    }
}

/*Add new control flow graph based optimizations here*/
blockPair cs6300::optimizeFlow(blockPair original)
{
    printInstructions(original);
    return original;
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
                if (iter->first == i.dest) {
                    iter = dMap.erase(iter);
                } else {
                    iter++;
                }
            }
            continue;
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

    }

    for(auto i : b->instructions)
    {
        if(!dMap.count(i.dest)) {
            cout << i.str() << endl;
            ret->instructions.emplace_back(i);
        }
    }
    return ret;
}

void cs6300::optimize(statements s)
{
    optimizeFlow(cs6300::emitList(s));
}
