//
// Created by David Seery on 06/12/2015.
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


#include "abstract_index.h"


abstract_index::abstract_index(char l, unsigned int f, unsigned int p)
  : label(l),
    classification(identify_index(l)),
    fields(f),
    params(p),
    pre_string("__"),
    post_string("")
  {
  }


abstract_index::abstract_index(char l, enum index_class c, unsigned int f, unsigned int p)
  : label(l),
    classification(c),
    fields(f),
    params(p),
    pre_string("__"),
    post_string("")
  {
  }


unsigned int abstract_index::numeric_range() const
  {
    switch(this->classification)
      {
        case index_class::field_only:
          {
            return(this->fields);
          }

        case index_class::full:
          {
            return(2*this->fields);
          }

        case index_class::parameter:
          {
            return(this->params);
          }
      }
  }