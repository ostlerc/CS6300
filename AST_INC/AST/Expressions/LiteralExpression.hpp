#ifndef CS6300_AST_LITERAL_EXPRESSION_HPP
#define CS6300_AST_LITERAL_EXPRESSION_HPP
#include "Expression.hpp"
namespace cs6300
{
class LiteralExpression : public Expression
{
public:
  LiteralExpression(char a);
  LiteralExpression(int a);
  std::shared_ptr<BasicBlock> emit() const;
  static std::shared_ptr<BasicBlock> emit(int, int);
  std::shared_ptr<Type> type() const;
  int value() const;
  bool isConst() const;
private:
  std::shared_ptr<Type> m_type;
  int m_value;
};
}
#endif

