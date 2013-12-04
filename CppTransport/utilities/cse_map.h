//
// Created by David Seery on 14/11/2013.
// Copyright (c) 2013 University of Sussex. All rights reserved.
//


#ifndef __cse_map_H_
#define __cse_map_H_

#include <vector>

#include "ginac/ginac.h"

#include "macro.h"
#include "cse.h"

class cse_map
  {
  public:
    cse_map(std::vector<GiNaC::ex>* l, replacement_data& d);
    ~cse_map() { delete list; }

    // not returning a reference disallows using [] as an lvalue
    std::string operator[](unsigned int index);

  protected:
    replacement_data&       data;

    std::vector<GiNaC::ex>* list;
    std::vector<GiNaC::ex>  cse_symbols;
  };


#endif //__cse_map_H_