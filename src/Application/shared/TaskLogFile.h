////////////////////////////////////////////////////////////////////////////////
//
// File:   TaskLogFile.h
//
// Date:   Feb 8, 2004
//
// Author: juergen.mellinger@uni-tuebingen.de
//
// Description: A std::ofstream descendant that centralizes/encapsulates details
//         of a user task log file.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef TaskLogFileH
#define TaskLogFileH

#include <fstream>
#include <string>
#include "UEnvironment.h"

class TaskLogFile: public std::ofstream, Environment
{
  public:
    TaskLogFile() {}

  private:
    TaskLogFile( const TaskLogFile& );
    TaskLogFile& operator=( const TaskLogFile& );

  public:
    void  Preflight() const;
    void  Initialize();

  private:
    std::string FileName() const;
};

#endif // TaskLogFileH
