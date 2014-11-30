#ifndef CS6300_BASIC_BLOCK_HPP
#define CS6300_BASIC_BLOCK_HPP

#include <memory>
#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include "ThreeAddressInstruction.hpp"
#include "RegisterScope.hpp"

namespace cs6300
{

    bool validReg(int reg);
class BasicBlock
{

public:
  std::vector<ThreeAddressInstruction> instructions;
  std::shared_ptr<BasicBlock> jumpTo;
  std::shared_ptr<BasicBlock> branchTo;
  std::set<std::shared_ptr<BasicBlock>> parents;
  int branchOn;
  std::string getLabel();

  cs6300::RegisterScope m; // Used for register allocation
  cs6300::MotionSet mset; // Used for code motion

  void initSets();
  static cs6300::RegisterScope scope(ThreeAddressInstruction tal);
  void remap(std::map<int, int> m);

  //Code motion functions
  void InitMotion(Motion);
  void DEcalc(Motion);
  void UEcalc(Motion);
  bool Avcalc(Motion);
  bool Ancalc(Motion);
  std::pair<std::set<ExprNode*>, std::set<ExprNode*>>
      LatestCalc();

private:
  std::string label;
};

std::ostream& operator<<(std::ostream&, BasicBlock);
std::ostream& operator<<(std::ostream&, std::shared_ptr<BasicBlock>);
}
#endif
