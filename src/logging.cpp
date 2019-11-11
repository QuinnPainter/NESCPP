#include "includes.hpp"
#include "logging.hpp"

//Prints out what you want with a newline
void logging::log(std::string toLog, bool newline, bool warning)
{
    //todo: add different colours for logs, warnings and errors
    std::cout << toLog;
    if (newline)
    {
        std::cout << std::endl;
    }
}

void logging::logerr(std::string toLog, bool fatal)
{
    std::cerr << toLog << std::endl;
    //if the error is fatal, we need to exit
    if (fatal)
    {
        exit(1);
    }
}