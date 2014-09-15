#ifndef CS6300_THREE_ADDRESS_INSTRUCTION_HPP
#define CS6300_THREE_ADDRESS_INSTRUCTION_HPP

#include <memory>

namespace cs6300
{
class ThreeAddressInstruction
{
  public:
    enum Type
    {
      Add,
      AddValue,
      And,
      CallFunction,
      CopyArgument,
      Divide,
      IsEqual,
      IsGreater,
      IsGreaterEqual,
      IsLess,
      IsLessEqual,
      IsNotEqual,
      LoadLabel,
      LoadMemory,
      LoadValue,
      Modulo,
      Multiply,
      Not,
      Or,
      ReadChar,
      ReadInt,
      Stop,
      StoreMemory,
      Subtract,
      UnaryMinus,
      WriteBool,
      WriteChar,
      WriteInt,
      WriteStr
    };
    ThreeAddressInstruction(Type op,int dest,int src1,int src2);
    std::string str()
    {
        return typeStr(op) + " " + std::to_string(dest) + " " + std::to_string(src1) + " " + std::to_string(src2);
    }
    static std::string typeStr(Type op)
    {
        switch(op) {
            case Add: return "Add";
            case AddValue: return "AddValue";
            case And: return "And";
            case CallFunction: return "CallFunction";
            case CopyArgument: return "CopyArgument";
            case Divide: return "Divide";
            case IsEqual: return "IsEqual";
            case IsGreater: return "IsGreater";
            case IsGreaterEqual: return "IsGreaterEqual";
            case IsLess: return "IsLess";
            case IsLessEqual: return "IsLessEqual";
            case IsNotEqual: return "IsNotEqual";
            case LoadLabel: return "LoadLabel";
            case LoadMemory: return "LoadMemory";
            case LoadValue: return "LoadValue";
            case Modulo: return "Modulo";
            case Multiply: return "Multiply";
            case Not: return "Not";
            case Or: return "Or";
            case ReadChar: return "ReadChar";
            case ReadInt: return "ReadInt";
            case Stop: return "Stop";
            case StoreMemory: return "StoreMemory";
            case Subtract: return "Subtract";
            case UnaryMinus: return "UnaryMinus";
            case WriteBool: return "WriteBool";
            case WriteChar: return "WriteChar";
            case WriteInt: return "WriteInt";
            case WriteStr: return "WriteStr";
            default: return "err";
        }
    }
  private:
    Type op;
    int dest;
    int src1;
    int src2;
};
}
#endif
