//
// Created by David Seery on 06/12/2015.
// Copyright (c) 2015-2016 University of Sussex. All rights reserved.
//

#ifndef CPPTRANSPORT_INDEX_CLASS_H
#define CPPTRANSPORT_INDEX_CLASS_H


enum class index_class
  {
    field_only, full, parameter
  };


enum index_class identify_index (char label);



#endif //CPPTRANSPORT_INDEX_CLASS_H
