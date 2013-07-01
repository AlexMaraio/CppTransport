//
// Created by David Seery on 12/06/2013.
// Copyright (c) 2013 University of Sussex. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//

#include <iostream>
#include <sstream>

#include "core.h"
#include "error.h"

// ******************************************************************

void warn(std::string const msg)
  {
    std::ostringstream out;

    out << CPPTRANSPORT_NAME << ": " << WARNING_TOKEN << msg;
    std::cerr << out.str() << std::endl;
  }

void error(std::string const msg)
  {
    std::ostringstream out;

    out << CPPTRANSPORT_NAME << ": " << ERROR_TOKEN << msg;
    std::cerr << out.str() << std::endl;
  }

void warn(std::string const msg, unsigned int line, std::deque<inclusion> const &path)
  {
    warn(msg, line, path, WARN_PATH_LEVEL);
  }

void error(std::string const msg, unsigned int line, std::deque<inclusion> const &path)
  {
    error(msg, line, path, ERROR_PATH_LEVEL);
  }

void warn(std::string const msg, unsigned int line, std::deque<inclusion> const &path, unsigned int level)
  {
    if(path.size() < level)
      {
        level = (unsigned int)path.size();
      }

    std::ostringstream out;

    out << CPPTRANSPORT_NAME << ": " << WARNING_TOKEN << msg << " at line " << line;
    if(level >= 1)
      {
        out << " of '" << path[0].name << "'";
      }
    std::cerr << out.str() << std::endl;

    for(int i = 1; i < level; i++)
      {
        std::cerr << "  included from line " << path[i].line << " of file '" << path[i].name << "'" << std::endl;
      }
  }

void error(std::string const msg, unsigned int line, std::deque<inclusion> const &path, unsigned int level)
  {
    if(path.size() < level)
      {
        level = (unsigned int)path.size();
      }

    std::ostringstream out;

    out << CPPTRANSPORT_NAME << ": " << ERROR_TOKEN << msg << " at line " << line;
    if(level >= 1)
      {
        out << " of '" << path[0].name << "'";
      }
    std::cerr << out.str() << std::endl;

    for(int i = 1; i < level; i++)
      {
        std::cerr << "  included from line " << path[i].line << " of file '" << path[i].name << "'" << std::endl;
      }
  }
