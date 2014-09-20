#include "Optimizer.hpp"
#include <iostream>
#include "../AST_INC/AST/Program.hpp"

using namespace std;

typedef shared_ptr<cs6300::BasicBlock> sBBlock;
typedef shared_ptr<cs6300::Program> sProgram;
typedef pair<sBBlock, sBBlock> blockPair;
typedef vector<shared_ptr<cs6300::Statement>> statements;

/*Add new AST based optimizations here*/
sProgram cs6300::optimizer(sProgram original)
{
    optimize(original->main);
    return original;
}

/*Add new control flow graph based optimizations here*/
blockPair cs6300::optimizeFlow(blockPair original)
{
    int count = 0;
    auto at = original.first;

    while(at)
    {
        at->printInstructions();
        count++;
        at = at->jumpTo;
    }
    cout << "count is " << count << endl;
    return original;
}

void cs6300::optimize(statements s)
{
}
