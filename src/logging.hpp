#pragma once

class logging
{
    private:
        //private constructor means no instances of this object can be created
        logging() {}
    public:
        static void log(std::string toLog, bool newline=true, bool warning=false);
        static void logerr(std::string toLog, bool fatal=false);
};