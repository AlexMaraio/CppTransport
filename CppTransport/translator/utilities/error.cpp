//
// Created by David Seery on 12/06/2013.
// --@@
// Copyright (c) 2016 University of Sussex. All rights reserved.
//
// This file is part of the CppTransport platform.
//
// CppTransport is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// CppTransport is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CppTransport.  If not, see <http://www.gnu.org/licenses/>.
//
// @license: GPL-2
// @contributor: David Seery <D.Seery@sussex.ac.uk>
// --@@
//

#include <iostream>
#include <sstream>

#include <cstdlib>

#include "core.h"
#include "error.h"


constexpr auto ANSI_BOLD_RED     = "\033[1;31m";
constexpr auto ANSI_BOLD_GREEN   = "\033[1;32m";
constexpr auto ANSI_BOLD_MAGENTA = "\033[1;35m";
constexpr auto ANSI_NORMAL       = "\033[0m";


// ******************************************************************


void warn(const std::string& msg, const argument_cache& cache, const local_environment& env)
  {
    std::ostringstream out;

    bool colour_output = cache.colourize() && env.has_colour_terminal_support();

    out << CPPTRANSPORT_NAME << ": " << (colour_output ? ANSI_BOLD_MAGENTA : "") << WARNING_TOKEN << (colour_output ? ANSI_NORMAL : "") << msg;
    std::cout << out.str() << '\n';
  }


void error(const std::string& msg, const argument_cache& cache, const local_environment& env)
  {
    std::ostringstream out;

    bool colour_output = cache.colourize() && env.has_colour_terminal_support();

    out << CPPTRANSPORT_NAME << ": " << (colour_output ? ANSI_BOLD_MAGENTA : "") << ERROR_TOKEN << (colour_output ? ANSI_NORMAL : "") << msg;
    std::cout << out.str() << '\n';
  }


// CONTEXT BASED ERROR MESSAGES, WITH INPUT LINE AND CHARACTER POSITION


void warn(const std::string& msg, const argument_cache& cache, const local_environment& env, const error_context& context)
  {
    warn(msg, cache, env, context, WARN_PATH_LEVEL);
  }


void error(const std::string& msg, const argument_cache& cache, const local_environment& env, const error_context& context)
  {
    error(msg, cache, env, context, ERROR_PATH_LEVEL);
  }


void warn(const std::string& msg, const argument_cache& cache, const local_environment& env, const error_context& context, unsigned int level)
  {
    std::ostringstream out;

    bool colour_output = cache.colourize() && env.has_colour_terminal_support();

    try
      {
        out << ERROR_MESSAGE_AT_LINE << " " << context.get_filestack().write(level) << '\n';
      }
    catch(std::runtime_error& xe)
      {
      }

    out << ERROR_MESSAGE_WRAP_PAD << (colour_output ? ANSI_BOLD_RED : "") << msg << (colour_output ? ANSI_NORMAL : "") << '\n';

    if(context.has_full_context())
      {
        out << ERROR_MESSAGE_WRAP_PAD << context.get_line() << '\n';

        out << ERROR_MESSAGE_WRAP_PAD;
        for(unsigned int i = 0; i < context.get_position(); ++i)
          {
            out << " ";
          }
        out << (colour_output ? ANSI_BOLD_GREEN : "") << "^" << (colour_output ? ANSI_NORMAL : "");
      }

    warn(out.str(), cache, env);
  }


void error(const std::string& msg, const argument_cache& cache, const local_environment& env, const error_context& context, unsigned int level)
  {
    std::ostringstream out;

    bool colour_output = cache.colourize() && env.has_colour_terminal_support();

    try
      {
        out << ERROR_MESSAGE_AT_LINE << " " << context.get_filestack().write(level) << '\n';
      }
    catch(std::runtime_error& xe)
      {
      }

    out << ERROR_MESSAGE_WRAP_PAD << (colour_output ? ANSI_BOLD_RED : "") << msg << (colour_output ? ANSI_NORMAL : "") << '\n';

    if(context.has_full_context())
      {
        out << ERROR_MESSAGE_WRAP_PAD << context.get_line() << '\n';

        out << ERROR_MESSAGE_WRAP_PAD;
        for(unsigned int i = 0; i < context.get_position(); ++i)
          {
            out << " ";
          }
        out << (colour_output ? ANSI_BOLD_GREEN : "") << "^" << (colour_output ? ANSI_NORMAL : "");
      }

    error(out.str(), cache, env);
  }
