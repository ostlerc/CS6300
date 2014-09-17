#include "NotExpression.hpp"
#include "LiteralExpression.hpp"

cs6300::NotExpression::NotExpression(
  std::shared_ptr<Expression> expr)
  : m_expr(expr)
{
}

std::shared_ptr<cs6300::BasicBlock> cs6300::NotExpression::emit() const
{
    if(isConst())
        return LiteralExpression::emit(value(), getLabel());

  auto result = m_expr->emit();
  result->instructions.emplace_back(
      ThreeAddressInstruction::Not, getLabel(), m_expr->getLabel(), 0);
  return result;
}
std::shared_ptr<cs6300::Type> cs6300::NotExpression::type() const
{
  return nullptr;
}
int cs6300::NotExpression::value() const { return !m_expr->value(); }
bool cs6300::NotExpression::isConst() const { return m_expr->isConst(); }

