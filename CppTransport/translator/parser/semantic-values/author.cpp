//
// Created by David Seery on 30/05/2017.
// --@@
// Copyright (c) 2017 University of Sussex. All rights reserved.
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


#include "author.h"


bool author::set_institute(const std::string& i, const y::lexeme_type& l)
  {
    return SetContextedValue(this->institute, i, l, ERROR_INSTITUTE_REDECLARATION);
  }


bool author::set_email(const std::string& e, const y::lexeme_type& l)
  {
    return SetContextedValue(this->email, e, l, ERROR_EMAIL_REDECLARATION);
  }


bool author::set_name(const std::string& n, const y::lexeme_type& l)
  {
    return SetContextedValue(this->name, n, l, ERROR_AUTHORNAME_REDECLARATION);
  }
