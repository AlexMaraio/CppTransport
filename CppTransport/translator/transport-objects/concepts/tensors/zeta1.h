//
// Created by David Seery on 20/12/2015.
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

#ifndef CPPTRANSPORT_ZETA1_H
#define CPPTRANSPORT_ZETA1_H


#include "transport-objects/concepts/transport_tensor.h"
#include "transport-objects/concepts/flattened_tensor.h"

#include "indices.h"
#include "lambdas.h"
#include "index_literal.h"


constexpr auto ZETA1_TENSOR_INDICES = 1;


class zeta1: public transport_tensor
  {

    // CONSTRUCTOR, DESTRUCTOR

  public:

    //! constructor is default
    zeta1() = default;

    //! destructor is default
    virtual ~zeta1() = default;


    // INTERFACE

  public:

    //! evaluate full tensor, returning a flattened list
    virtual std::unique_ptr<flattened_tensor>
    compute(const index_literal_list& indices) = 0;

    //! evaluate component of tensor
    virtual GiNaC::ex
    compute_component(phase_index i) = 0;

    //! evaluate lambda for tensor
    virtual std::unique_ptr<map_lambda>
    compute_lambda(const index_literal& i) = 0;

  };



#endif //CPPTRANSPORT_ZETA1_H
