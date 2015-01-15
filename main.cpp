#include <string>
#include <iostream>

#include "FrontEnd/FrontEnd.hpp"
#include "Optimizations/Optimizer.hpp"
#include "BackEnd/BackEnd.hpp"

int main(int argc, char* argv[])
{
  try
  {
    std::string outFile = "out.asm";
    std::string inFile = "in.cpsl";

    if (argc < 2)
    {
        return EXIT_FAILURE;
    }
    if (argv[1] == std::string("-o"))
    {
      outFile = argv[2];
      inFile = argv[3];
    }
    else
    {
      inFile = argv[1];
    }

    cs6300::compileCPSL(inFile, outFile);
  }
  catch (std::exception& e)
  {
    std::cerr << "Error: " << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
