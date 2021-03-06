#include <string>
#include "SymbolTable.hpp"
#include "Expressions/Expression.hpp"
#include "Type.hpp"

std::shared_ptr<cs6300::Type> cs6300::SymbolTable::lookupType(std::string id)
{
  auto found = m_types.find(id);
  if (found != m_types.end())
    return found->second;
  if (m_parent)
    return m_parent->lookupType(id);
  return nullptr;
}

void cs6300::SymbolTable::addConstant(std::string id,
                                      std::shared_ptr<Expression> val)
{
  auto found = m_consts.find(id);
  if (found == m_consts.end())
  {
    m_consts[id] = val;
  }
}

void cs6300::SymbolTable::addType(std::string id, std::shared_ptr<Type> type)
{
  auto found = m_types.find(id);
  if (found == m_types.end())
  {
    m_types[id] = type;
  }
}

void cs6300::SymbolTable::addVariable(std::string id,
                                      std::shared_ptr<Type> type)
{
  auto found = m_variables.find(id);
  if (found == m_variables.end())
  {
    m_variables[id] = type;
  }
}

