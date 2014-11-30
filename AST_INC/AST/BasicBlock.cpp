#include "BasicBlock.hpp"
#include "SymbolTable.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <set>
#include <algorithm>

std::string cs6300::BasicBlock::getLabel()
{
    static size_t blockNumber = 0;
    if (label.empty()) label = std::string("BB") + std::to_string(++blockNumber);
    return label;
}

void cs6300::BasicBlock::initSets()
{
    m.clear();
    for (auto i : instructions)
    {
        auto t = scope(i);
        m.used.insert(t.used.begin(), t.used.end());
        m.dead.insert(t.dead.begin(), t.dead.end());
    }


    std::vector<int> t;
    std::set_difference(m.used.begin(),
            m.used.end(),
            m.dead.begin(),
            m.dead.end(),
            std::back_inserter(t));
    m.alive = std::set<int>(t.begin(), t.end());
}

cs6300::RegisterScope cs6300::BasicBlock::scope(
        cs6300::ThreeAddressInstruction tal)
{
    cs6300::RegisterScope m;
    switch (tal.op)
    {
        case ThreeAddressInstruction::CallFunction: // CallFunction should have no allocation
            break;
        case ThreeAddressInstruction::LoadMemory:
            if(validReg(tal.dest)) m.dead.insert(tal.dest);
            if(validReg(tal.src1)) m.used.insert(tal.src1);
            break;
        case ThreeAddressInstruction::LoadMemoryOffset:
        case ThreeAddressInstruction::LoadValue: // LoadValue has constants
            if(validReg(tal.dest)) m.dead.insert(tal.dest);
            break;
            case ThreeAddressInstruction::
                StoreMemory: // special case store memory dest is src1
                if(validReg(tal.dest)) m.used.insert(tal.dest);
            if(validReg(tal.src1)) m.used.insert(tal.src1);
            break;
        case ThreeAddressInstruction::AddValue:
            if(validReg(tal.dest)) m.dead.insert(tal.dest);
            if(validReg(tal.src1)) m.used.insert(tal.src1);
            break;
        case ThreeAddressInstruction::WriteStr: // none
            break;
        default:
            if(validReg(tal.dest)) m.dead.insert(tal.dest);
            if(validReg(tal.src1)) m.used.insert(tal.src1);
            if(validReg(tal.src2)) m.used.insert(tal.src2);
    }

    return m;
}

bool cs6300::validReg(int reg)
{
    return reg > 100;
}

void cs6300::BasicBlock::remap(std::map<int, int> m)
{
    for (auto& i : instructions)
    {
        switch (i.op)
        {
            case ThreeAddressInstruction::CallFunction: // CallFunction should have no allocation
                break;
            case ThreeAddressInstruction::LoadMemoryOffset:
                if (validReg(i.dest) && m.count(i.dest)) i.dest = m[i.dest];
                break;
            case ThreeAddressInstruction::LoadValue: // LoadValue has constants
                if (validReg(i.dest) && m.count(i.dest)) i.dest = m[i.dest];
                break;
            case ThreeAddressInstruction::StoreMemory:
            case ThreeAddressInstruction::AddValue:
            case ThreeAddressInstruction::LoadMemory:
                if (validReg(i.dest) && m.count(i.dest)) i.dest = m[i.dest];
                if (validReg(i.src1) && m.count(i.src1)) i.src1 = m[i.src1];
                break;
            case ThreeAddressInstruction::WriteStr: // none
                break;
            default:
                if (validReg(i.dest) && m.count(i.dest)) i.dest = m[i.dest];

                if (validReg(i.src1) && m.count(i.src1)) i.src1 = m[i.src1];

                if (validReg(i.src2) && m.count(i.src2)) i.src2 = m[i.src2];
        }
    }
    if(branchTo && m.count(branchOn))
    {
        branchOn = m[branchOn];
    }
}

void cs6300::BasicBlock::InitMotion(Motion _m)
{
    mset.Avi.clear();
    mset.Avo.clear();
    mset.Ani.clear();
    mset.Ano.clear();

    //available in = is everything initially (if not entry)
    //anticipatible out = everything (if not exit)

    bool entry = !parents.size();
    bool exit = !jumpTo && !branchTo;

    std::set<ExprNode*> allExprs;
    for(auto&v : _m.smap) allExprs.insert(v.second);

    if(!entry)
        mset.Avi = std::set<ExprNode*>(allExprs.begin(), allExprs.end());
    if(!exit)
        mset.Ano = std::set<ExprNode*>(allExprs.begin(), allExprs.end());

    //available out = DE + (AVIN - KILL)
    std::set_difference(mset.Avi.begin(), mset.Avi.end(),
                        mset.Kill.begin(), mset.Kill.end(),
                        std::inserter(mset.Avo, mset.Avo.end())); //(AVIN - KILL)
    mset.Avo.insert(mset.DE.begin(), mset.DE.end()); //DE + ^

    //anticipatible in  = UE + (ANTOUT - KILL)
    std::set_difference(mset.Ano.begin(), mset.Ano.end(),
                        mset.Kill.begin(), mset.Kill.end(),
                        std::inserter(mset.Ani, mset.Ani.end())); //(AVIN - KILL)
    mset.Ani.insert(mset.UE.begin(), mset.UE.end()); //UE + ^
}

void cs6300::BasicBlock::DEcalc(Motion _m)
{
    mset.DE.clear();
    mset.Kill.clear();
    for (auto it = instructions.rbegin(); it != instructions.rend(); it++)
    {
        auto i = *it;
        switch(i.op) {
            case ThreeAddressInstruction::Stop: continue;
        }

        auto e = _m.getExpr(i);
        if(!e) { std::cerr << "skipping null expr for instruction: '" << i << "'" << std::endl; continue; }

        switch (i.op) {
            case ThreeAddressInstruction::LoadMemory:
                break;
            case ThreeAddressInstruction::LoadValue: // LoadValue has constants
                mset.DE.insert(_m.nmap[i.dest]);
                break;
            case ThreeAddressInstruction::StoreMemory: // special case store memory dest is src1
                {
                    if(!mset.Kill.count(e))
                        mset.DE.insert(e);
                    join(mset.Kill, e->recurse());
                }
                break;
            default:
                if(!mset.Kill.count(e))
                    mset.DE.insert(e);
                break;
        }
    }
}

void cs6300::BasicBlock::UEcalc(Motion _m)
{
    mset.UE.clear();
    std::set<ExprNode*> computed;
    for (auto it = instructions.begin(); it != instructions.end(); it++)
    {
        auto i = *it;
        switch(i.op) {
            case ThreeAddressInstruction::Stop: continue;
        }

        auto e = _m.getExpr(i);
        if(!e) { std::cerr << "skipping null expr for instruction: '" << i << "'" << __LINE__ << std::endl; continue; }

        bool computing = true;
        switch (i.op) {
            case ThreeAddressInstruction::LoadValue: // LoadValue has constants
                mset.UE.insert(_m.nmap[i.dest]);
                break;
            case ThreeAddressInstruction::LoadMemory:
                computing = false;
            default:
                bool argsComputed = false;
                for(auto& v : computed)
                {
                    if(v->recurse().count(e))
                    {
                        argsComputed = true;
                        break;
                    }
                }

                if(!argsComputed)
                    mset.UE.insert(e);

                if(computing)
                    computed.insert(e);
                break;
        }
    }
}

bool cs6300::BasicBlock::Avcalc(Motion _m)
{
    int o = mset.Avo.size();
    int i = mset.Avi.size();

    mset.Avo.clear();
    mset.Avi.clear();

    bool init = true;

    //in: intersection of parents out
    for(auto&p : parents)
    {
        if(init)
        {
            mset.Avi.insert(p->mset.Avo.begin(), p->mset.Avo.end());
            init = false;
            continue;
        }

        std::set_intersection(mset.Avi.begin(), mset.Avi.end(),
                              p->mset.Avo.begin(), p->mset.Avo.end(),
                              std::inserter(mset.Avi, mset.Avi.end()));
    }

    //out: DE + (IN - KILL)
    std::set_difference(mset.Avi.begin(), mset.Avi.end(),
            mset.Kill.begin(), mset.Kill.end(),
            std::inserter(mset.Avo, mset.Avo.end())); //(AVIN - KILL)
    mset.Avo.insert(mset.DE.begin(), mset.DE.end()); //DE + ^
    return o != mset.Avo.size() || i != mset.Avi.size();
}

bool cs6300::BasicBlock::Ancalc(Motion _m)
{
    int i = mset.Ani.size();
    int o = mset.Ano.size();

    mset.Ano.clear();
    mset.Ani.clear();

    bool init = true;

    //out: intersection of parents in
    for(auto&p : parents)
    {
        if(init)
        {
            mset.Ano.insert(p->mset.Ani.begin(), p->mset.Ani.end());
            init = false;
            continue;
        }

        std::set_intersection(mset.Ano.begin(), mset.Ano.end(),
                p->mset.Ani.begin(), p->mset.Ani.end(),
                std::inserter(mset.Ano, mset.Ano.end()));
    }

    //in: UE + (OUT - KILL)
    std::set_difference(mset.Ano.begin(), mset.Ano.end(),
            mset.Kill.begin(), mset.Kill.end(),
            std::inserter(mset.Ani, mset.Ani.end())); //(OUT - KILL)
    mset.Ani.insert(mset.UE.begin(), mset.UE.end()); //UE + ^
    return o != mset.Ano.size() || i != mset.Ani.size();
}

std::ostream& cs6300::operator<<(std::ostream& out,
        std::shared_ptr<cs6300::BasicBlock> b)
{
    out << b->getLabel();
    return out;
}

std::ostream& cs6300::operator<<(std::ostream& out,
        BasicBlock b)
{
    out << b.getLabel();
    return out;
}
