//
// Created by David Seery on 12/06/2013.
// Copyright (c) 2013-15 University of Sussex. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//

#include <iostream>
#include <sstream>

#include "core.h"
#include "basic_error.h"
#include "error.h"


// ******************************************************************


void warn(std::string const msg)
  {
    basic_warn(msg);
  }


void error(std::string const msg)
  {
    basic_error(msg);
  }


void warn(std::string const msg, std::shared_ptr<filestack> path)
  {
    warn(msg, path, WARN_PATH_LEVEL);
  }


void error(std::string const msg, std::shared_ptr<filestack> path)
  {
    error(msg, path, ERROR_PATH_LEVEL);
  }


void warn(std::string const msg, std::shared_ptr<filestack> path, unsigned int level)
  {
    std::ostringstream out;

    if(path) out << ERROR_MESSAGE_AT_LINE << " " << path->write(level) << std::endl << ERROR_MESSAGE_WRAP_PAD;
    out << msg;

    basic_warn(out.str());
  }


void error(std::string const msg, std::shared_ptr<filestack> path, unsigned int level)
  {
    std::ostringstream out;

    if(path) out << ERROR_MESSAGE_AT_LINE << " " << path->write(level) << std::endl << ERROR_MESSAGE_WRAP_PAD;
    out << msg;

    basic_error(out.str());
  }
