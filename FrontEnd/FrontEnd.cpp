#include <cstdio>
#include <memory>
#include <algorithm>
#include "FrontEnd.hpp"

#include "AST/Expressions/AdditionExpression.hpp"
#include "AST/Expressions/AndExpression.hpp"
#include "AST/Expressions/CallExpression.hpp"
#include "AST/Expressions/ChrExpression.hpp"
#include "AST/Expressions/DivExpression.hpp"
#include "AST/Expressions/EqExpression.hpp"
#include "AST/Expressions/Expression.hpp"
#include "AST/Expressions/GtExpression.hpp"
#include "AST/Expressions/GteExpression.hpp"
#include "AST/Expressions/LiteralExpression.hpp"
#include "AST/Expressions/LoadExpression.hpp"
#include "AST/Expressions/LtExpression.hpp"
#include "AST/Expressions/LteExpression.hpp"
#include "AST/Expressions/ModExpression.hpp"
#include "AST/Expressions/MultExpression.hpp"
#include "AST/Expressions/NeqExpression.hpp"
#include "AST/Expressions/NotExpression.hpp"
#include "AST/Expressions/OrExpression.hpp"
#include "AST/Expressions/OrdExpression.hpp"
#include "AST/Expressions/PredecessorExpression.hpp"
#include "AST/Expressions/StringExpression.hpp"
#include "AST/Expressions/SubtractExpression.hpp"
#include "AST/Expressions/SuccessorExpression.hpp"
#include "AST/Expressions/UnaryMinusExpression.hpp"
#include "AST/LValue.hpp"
#include "AST/Statements/Assignment.hpp"
#include "AST/Statements/Call.hpp"
#include "AST/Statements/For.hpp"
#include "AST/Statements/If.hpp"
#include "AST/Statements/Repeat.hpp"
#include "AST/Statements/Return.hpp"
#include "AST/Statements/Statement.hpp"
#include "AST/Statements/While.hpp"
#include "AST/Type.hpp"

extern FILE* yyin;
extern int yyparse();

namespace
{
template <typename T> class FEC
{
public:
  FEC() : src()
  {
  }
  std::shared_ptr<T> get(int index)
  {
    if (index < static_cast<int>(src.size()))
    {
      return src[index];
    }
    return nullptr;
  }
  int add(std::shared_ptr<T> e)
  {
    auto index = src.size();
    src.push_back(e);
    return index;
  }
  void clear()
  {
    src.clear();
  }

private:
  std::vector<std::shared_ptr<T>> src;
};

class FrontEndState
{
public:
  static std::shared_ptr<FrontEndState> instance()
  {
    if (!msInstance)
    {
      msInstance = std::make_shared<FrontEndState>();
    }
    return msInstance;
  }
  typedef std::vector<std::shared_ptr<cs6300::Statement>> slist_t;
  FEC<cs6300::Expression> expressions;
  FEC<cs6300::Type> types;
  FEC<std::vector<std::string>> idLists;
  FEC<slist_t> statementLists;
  FEC<std::vector<std::shared_ptr<cs6300::Expression>>> actualArguments;
  FEC<cs6300::Statement> statements;
  FEC<cs6300::LValue> lvals;
  FEC<std::vector<std::pair<std::shared_ptr<cs6300::Expression>, std::shared_ptr<slist_t>>>>
  clauses;
  FEC<std::pair<std::shared_ptr<cs6300::Expression>,cs6300::ForStatement::Direction>> toExprs;
  FEC<std::pair<std::string,std::shared_ptr<cs6300::Expression>>> forHeads;

private:
  static std::shared_ptr<FrontEndState> msInstance;
};

std::shared_ptr<FrontEndState> FrontEndState::msInstance;
template <typename T, typename U>
int appendList(FEC<T> &listSrc, int listIndex, U elem)
{
  if (listIndex == -1)
  {
    auto list = std::make_shared<T>();
    list->push_back(elem);
    return listSrc.add(list);
  }
  else
  {
    auto list = listSrc.get(listIndex);
    list->push_back(elem);
    return listIndex;
  }
}
template <typename T, typename U>
int
appendList(FEC<T> &listSrc, int listIndex, FEC<U> &elementSrc, int elementIndex)
{
  auto elem = elementSrc.get(elementIndex);
  return appendList(listSrc,listIndex,elem);
}

template <typename ExprType> int binaryOp(int a, int b)
{
  auto state = FrontEndState::instance();
  auto A = state->expressions.get(a);
  auto B = state->expressions.get(b);
  return state->expressions.add(std::make_shared<ExprType>(A, B));
}
template <typename ExprType> int unaryOp(int a)
{
  auto state = FrontEndState::instance();
  auto A = state->expressions.get(a);
  return state->expressions.add(std::make_shared<ExprType>(A));
}
}

std::shared_ptr<cs6300::Program> cs6300::parseCPSL(std::string filename) 
{
  FILE* inFile;
  inFile = fopen(filename.c_str(),"r");
  if(inFile == nullptr)
  {
    throw std::runtime_error("Could not open " + filename);
  }
  yyin = inFile;
  yyparse();
  return std::shared_ptr<cs6300::Program>();
}

int cs6300::AddExpr(int a, int b)
{
  return binaryOp<AdditionExpression>(a, b);
}

int cs6300::AddField(int listIndex, int typeIndex) {
  auto state = FrontEndState::instance();
  auto type = state->types.get(typeIndex);
  if (!type)
    return 0;
  auto list = state->idLists.get(listIndex);
  auto newType = std::make_shared<cs6300::RecordType>();
  for (auto &&id : *list) {
    newType->fields[id] = type;
  }
  return state->types.add(newType);
}

int cs6300::AndExpr(int a, int b)
{
  return binaryOp<AndExpression>(a, b);
}
int cs6300::ArgumentList(int listIndex, int exprIndex)
{
  auto state = FrontEndState::instance();
  return appendList(state->actualArguments, listIndex, state->expressions, exprIndex);
}
int cs6300::AppendElseList(int listIndex, int expr, int statement)
{
  auto state = FrontEndState::instance();
  auto e = state->expressions.get(expr);
  auto list = state->statementLists.get(statement);
  return appendList(state->clauses, listIndex, std::make_pair(e, list));
}

int cs6300::BuildArrayType(int lowerIndex, int upperIndex, int typeIndex)
{
  auto state = FrontEndState::instance();
  auto type = state->types.get(typeIndex);
  auto lb = state->expressions.get(lowerIndex);
  auto ub = state->expressions.get(upperIndex);
  int lower = -1;
  int upper = -1;
  if (!type)
    return -1;
  if (!lb)
    return -1;
  if (!ub)
    return -1;
  if(!lb->isConst()) return -1;
  if(!ub->isConst()) return -1;
  lower = lb->value();
  upper = ub->value();
  auto array = std::make_shared<cs6300::ArrayType>(lower,upper,type);
  return state->types.add(array);
}

int cs6300::Assign(int lval,int expr)
{
  auto state = FrontEndState::instance();
  auto l = state->lvals.get(lval);
  auto e = state->expressions.get(expr);
  return state->statements.add(std::make_shared<cs6300::Assignment>(l,e));
}

int cs6300::CallExpr(char *a, int b) {
  auto state = FrontEndState::instance();
  return state->expressions.add(
      std::make_shared<cs6300::CallExpression>(a, *(state->actualArguments.get(b))));
}

int cs6300::CallProc(char *name, int args)
{
  auto state = FrontEndState::instance();
  auto a = *(state->actualArguments.get(args));
  return state->statements.add(std::make_shared<cs6300::Call>(name, a));
}

int cs6300::CharExpr(char a)
{
  auto state = FrontEndState::instance();
  return state->expressions.add(std::make_shared<cs6300::LiteralExpression>(a));
}
int cs6300::ChrExpr(int a) {return unaryOp<ChrExpression>(a);}
int cs6300::DivExpr(int a, int b){return binaryOp<DivExpression>(a,b);}
int cs6300::DownTo(int expr)
{
  auto state = FrontEndState::instance();
  auto e = state->expressions.get(expr);
  return state->toExprs.add(std::make_shared<
      std::pair<std::shared_ptr<cs6300::Expression>, cs6300::ForStatement::Direction>>(
      e, cs6300::ForStatement::DOWNTO));
}
int cs6300::EqExpr(int a, int b){return binaryOp<EqExpression>(a,b);}
int cs6300::FieldList(int typeIndex, int field)
{
  auto state = FrontEndState::instance();
  if (typeIndex == -1)
  {
    return state->types.add(std::make_shared<cs6300::RecordType>());
  }
  else
  {
    auto type = std::dynamic_pointer_cast<cs6300::RecordType>(
        state->types.get(typeIndex));
    if (!type)
      return 0;
    auto fields =
        std::dynamic_pointer_cast<cs6300::RecordType>(state->types.get(field));
    if (!fields)
      return 0;
    for (auto &&f : fields->fields)
    {
      type->fields.insert(f);
    }
    return typeIndex;
  }
}
int cs6300::For(int head,int to,int statement)
{
  auto state = FrontEndState::instance();
  auto h = state->forHeads.get(head);
  auto t = state->toExprs.get(to);
  auto s = state->statementLists.get(statement);
  auto f = std::make_shared<cs6300::ForStatement>(
      h->first, h->second, t->first, t->second, *s);
  return state->statements.add(f);
}

int cs6300::ForHead(char* ident,int expr)
{
  auto state = FrontEndState::instance();
  auto e = state->expressions.get(expr);
  auto head = std::make_shared<
      std::pair<std::string, std::shared_ptr<cs6300::Expression>>>(
      std::string(ident), e);
  delete(ident);
  return state->forHeads.add(head);
}

int cs6300::GtExpr(int a, int b) {return binaryOp<GtExpression>(a,b);}
int cs6300::GteExpr(int a, int b) {return binaryOp<GteExpression>(a,b);}
int cs6300::IdentList(int listIndex, char *ident)
{
  auto state = FrontEndState::instance();
  return appendList(state->idLists,listIndex,ident);
}
int cs6300::If(int expr, int statement, int list, int elseStatement)
{
  auto state = FrontEndState::instance();
  auto ifClause = std::make_pair(state->expressions.get(expr),
                                 *(state->statementLists.get(statement)));
  auto otherClauses = state->clauses.get(list);
  std::vector<cs6300::IfStatement::clause_t> allClauses(otherClauses->size() +
                                                        1);
  allClauses.push_back(ifClause);
  for(auto && c: *otherClauses)
  {
    allClauses.push_back(std::make_pair(c.first,*(c.second)));
  }
  auto elseClause = state->statementLists.get(elseStatement);
  if (elseClause)
  {
    return state->statements.add(
        std::make_shared<cs6300::IfStatement>(allClauses, *elseClause));
  }
  else
  {
    return state->statements.add(std::make_shared<cs6300::IfStatement>(
        allClauses, std::vector<std::shared_ptr<Statement>>()));
  }
}

int cs6300::IntExpr(int a)
{
  auto state = FrontEndState::instance();
  return state->expressions.add(std::make_shared<cs6300::LiteralExpression>(a));
}
int cs6300::LoadArray(int lval, int exprIndex)
{
  auto state = FrontEndState::instance();
  auto base = state->lvals.get(lval);
  auto expr = state->expressions.get(exprIndex);
  return state->lvals.add(std::make_shared<cs6300::ArrayAccess>(base, expr));
}
int cs6300::LoadExpr(int lvalIndex)
{
  auto state = FrontEndState::instance();
  auto lval = state->lvals.get(lvalIndex);
  auto expr = std::make_shared<cs6300::LoadExpression>(lval);
  return state->expressions.add(expr);
}
int cs6300::LoadId(char *ident)
{
  auto lval = std::make_shared<cs6300::IdAccess>(ident);
  delete (ident);
  return FrontEndState::instance()->lvals.add(lval);
}
int cs6300::LoadMember(int lvalIndex, char *ident)
{
  auto state = FrontEndState::instance();
  auto base = state->lvals.get(lvalIndex);

  auto lval = std::make_shared<cs6300::MemberAccess>(base, ident);
  delete (ident);
  return FrontEndState::instance()->lvals.add(lval);
}
int cs6300::LookupType(char *) {return 0;}
int cs6300::LtExpr(int a, int b) {return binaryOp<LtExpression>(a,b);}
int cs6300::LteExpr(int a, int b) {return binaryOp<LteExpression>(a,b);}
int cs6300::ModExpr(int a, int b) {return binaryOp<ModExpression>(a,b);}
int cs6300::MultExpr(int a, int b) {return binaryOp<MultExpression>(a,b);}
int cs6300::NeqExpr(int a, int b) {return binaryOp<NeqExpression>(a,b);}
int cs6300::NotExpr(int a) {return unaryOp<NotExpression>(a);}
int cs6300::OrExpr(int a, int b) {return binaryOp<OrExpression>(a,b);}
int cs6300::OrdExpr(int a) {return unaryOp<OrdExpression>(a);}
int cs6300::Parameter(int /*list*/,int /*type*/){return 0;}
int cs6300::ParameterList(int /*list*/,int /*parameter*/){return 0;}
int cs6300::PredExpr(int a) {return unaryOp<PredecessorExpression>(a);}
int cs6300::ReadValue(int /*lval*/){return 0;}
int cs6300::ReadValue(int /*statement*/,int /*lval*/){return 0;}
int cs6300::Repeat(int statement,int expr)
{
  auto state = FrontEndState::instance();
  auto e = state->expressions.get(expr);
  auto s = state->statementLists.get(statement);
  return state->statements.add(std::make_shared<cs6300::RepeatStatement>(e,*s));
}
int cs6300::Return()
{
  return FrontEndState::instance()->statements.add(
      std::make_shared<cs6300::ReturnStatement>(nullptr));
}
int cs6300::Return(int expr)
{
  auto state = FrontEndState::instance();
  auto e = state->expressions.get(expr);
  return state->statements.add(std::make_shared<cs6300::ReturnStatement>(e));
}

int cs6300::Signature(char*, int /*params */){return 0;}
int cs6300::Signature(char*, int /*params*/, int /*type*/){return 0;}
int cs6300::StatementList(int listIndex, int statementIndex)
{
  auto state = FrontEndState::instance();
  return appendList(state->statementLists,listIndex,state->statements,statementIndex);
}
int cs6300::Stop(){return 0;}
int cs6300::StrExpr(char *a)
{
  auto state = FrontEndState::instance();
  return state->expressions.add(std::make_shared<cs6300::StringExpression>(a));
}
int cs6300::SubExpr(int a, int b) {return binaryOp<SubtractExpression>(a,b);}
int cs6300::SuccExpr(int a) {return unaryOp<SuccessorExpression>(a);}
int cs6300::To(int expr)
{
  auto state = FrontEndState::instance();
  auto e = state->expressions.get(expr);
  return state->toExprs.add(
      std::make_shared<std::pair<std::shared_ptr<cs6300::Expression>,
                                 cs6300::ForStatement::Direction>>(
          e, cs6300::ForStatement::TO));
}

int cs6300::UnMinusExpr(int a)
{
  return unaryOp<UnaryMinusExpression>(a);
}

int cs6300::While(int expr, int statement)
{
  auto state = FrontEndState::instance();
  auto e = state->expressions.get(expr);
  auto s = state->statementLists.get(statement);
  return state->statements.add(std::make_shared<cs6300::WhileStatement>(e, *s));
}

int cs6300::WriteExpr(int /*expr*/){return 0;}
int cs6300::WriteExpr(int /*statment*/,int /*expr*/){return 0;}
void cs6300::AddConstant(char *, int) {}
void cs6300::AddFunction(int /*signature*/){}
void cs6300::AddFunction(int /*signature*/,int /*body*/){}
void cs6300::AddProcedure(int /*signature*/){}
void cs6300::AddProcedure(int /*signature*/,int /*body*/){}
void cs6300::AddType(char *, int) {}
void cs6300::AddVariables(int /*list*/,int /*type*/){}
