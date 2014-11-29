#include "BasicBlock.hpp"
#include "SymbolTable.hpp"

#include <algorithm>
#include <iterator>
#include <string>

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

void cs6300::BasicBlock::DEcalc(Motion _m)
{
    std::set<ExprNode*> killed;
    for (auto it = instructions.rbegin(); it != instructions.rend(); it++)
    {
        auto i = *it;
        auto e = _m.getExpr(i);
        if(!e) { std::cerr << "skipping null expr for instruction: '" << i << "'" << std::endl; continue; }
        switch (i.op)
        {
            case ThreeAddressInstruction::LoadMemory:
                break;
            case ThreeAddressInstruction::LoadValue: // LoadValue has constants
                mset.DE.insert(_m.nmap[i.dest]);
                break;
            case ThreeAddressInstruction::StoreMemory: // special case store memory dest is src1
                if(!killed.count(e))
                    mset.DE.insert(e);
                join(killed, e->recurse());
                break;
            default:
                if(!killed.count(e))
                    mset.DE.insert(e);
                break;
        }
    }
}

void cs6300::BasicBlock::UEcalc(Motion _m)
{
}

void cs6300::BasicBlock::Killcalc(Motion _m)
{
}

bool cs6300::BasicBlock::Avocalc(Motion _m)
{
    return false;
}

bool cs6300::BasicBlock::Avicalc(Motion _m)
{
    return false;
}

bool cs6300::BasicBlock::Anocalc(Motion _m)
{
    return false;
}

bool cs6300::BasicBlock::Anicalc(Motion _m)
{
    return false;
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
