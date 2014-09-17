#ifndef CS6300_OPTIMIZER_HPP
#define CS6300_OPTIMIZER_HPP

#include <memory>
#include <vector>

namespace cs6300
{
  class Program;
  class BasicBlock;
  class Statement;

  /*Returns the Abstract Syntax Tree that is the result of parsing filename*/
  std::shared_ptr<Program> optimizer(std::shared_ptr<Program>);

  /*Returns an improved control flow graph*/
  std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>> optimizeFlow(
      std::pair<std::shared_ptr<BasicBlock>, std::shared_ptr<BasicBlock>>);

  /*Returns an improves statements list*/
  void optimize(std::vector<std::shared_ptr<Statement>>);

  std::shared_ptr<BasicBlock> subExprElim(std::shared_ptr<BasicBlock> b);

  /* Runs unit tests */
  void test();
}


#endif

