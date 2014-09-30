#ifndef CS6300_BASIC_BLOCK_HPP
#define CS6300_BASIC_BLOCK_HPP

using namespace std;

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include <string>

#include "ThreeAddressInstruction.hpp"

namespace cs6300
{
    struct RegMeta
    {
        set<int> dead;
        set<int> used;
        set<int> alive;

        void clear()
        {
            dead.clear();
            used.clear();
            alive.clear();
        }
    };

    class BasicBlock
    {
        public:
            vector<ThreeAddressInstruction> instructions;
            shared_ptr<BasicBlock> jumpTo;
            shared_ptr<BasicBlock> branchTo;

            //Used for register allocation
            RegMeta m;

            int branchOn;
            std::string getLabel();
            void printInstructions(bool sets=false)
            {
                for(auto i : instructions)
                    cout << i.str() << endl;

                if(branchOn)
                    cout << " branchOn: " << branchOn << endl;;

                if(sets)
                {
                    printSet("dead", m.dead);
                    printSet("used", m.used);
                    printSet("alive", m.alive);
                    if(m.dead.size() || m.used.size() || m.alive.size())
                        cout << endl;
                }
            }

            void printSet(string str, set<int> s)
            {
                if(!s.size()) return;

                cout << str << ":";

                for(auto i : s)
                    cout << i << " ";
            }

            void initSets()
            {
                m.clear();
                for(auto i : instructions)
                {
                    auto t = meta(i);
                    m.used.insert(t.used.begin(),t.used.end());
                    m.dead.insert(t.dead.begin(),t.dead.end());
                }

                if(branchOn)
                    m.used.insert(branchOn);

                vector<int> t;
                set_difference(
                        m.used.begin(), m.used.end(),
                        m.dead.begin(), m.dead.end(),
                        std::back_inserter(t));
                m.alive = set<int>(t.begin(), t.end());
            }

            void remap(std::map<int,int> m)
            {
                for(auto& i : instructions)
                {
                    switch(i.op)
                    {
                        case ThreeAddressInstruction::LoadValue: //LoadValue has constants
                            if(i.dest && m.count(i.dest))
                                i.dest = m[i.dest];
                        default:
                            if(i.dest && m.count(i.dest))
                                i.dest = m[i.dest];

                            if(i.src1 && m.count(i.src1))
                                i.src1 = m[i.src1];

                            if(i.src2 && m.count(i.src2))
                                i.src2 = m[i.src2];
                    }
                }
            }

            static RegMeta meta(ThreeAddressInstruction tal)
            {
                RegMeta m;
                switch(tal.op)
                {
                    case ThreeAddressInstruction::StoreMemory: //special case store memory dest is src1
                        m.dead.insert(tal.src1);
                        m.used.insert(tal.dest);
                        if(tal.src2)
                            m.used.insert(tal.src2);
                        break;
                    case ThreeAddressInstruction::LoadValue: //LoadValue has constants
                        m.dead.insert(tal.dest);
                        break;
                    default:
                        if(tal.dest)
                            m.dead.insert(tal.dest);
                        if(tal.src1)
                            m.used.insert(tal.src1);
                        if(tal.src2)
                            m.used.insert(tal.src2);
                }
                return m;
            }
        private:
            std::string label;
    };
}
#endif
