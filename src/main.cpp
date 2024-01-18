#include "cli/cli-handler.h"

int main(int argc, const char** argv)
{
    ASM::CLI::CommandLineInterfaceHandler cliHandler(argc, argv);
    
    bool result = cliHandler.Handle();

    return 0;
}