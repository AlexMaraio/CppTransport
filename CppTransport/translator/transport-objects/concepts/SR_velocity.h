//
// Created by David Seery on 19/12/2015.
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

#ifndef CPPTRANSPORT_SR_VELOCITY_H
#define CPPTRANSPORT_SR_VELOCITY_H


#include "transport_tensor.h"
#include "flattened_tensor.h"

#include "indices.h"
#include "lambdas.h"


class SR_velocity: public transport_tensor
  {

    // CONSTRUCTOR, DESTRUCTOR

  public:

    //! constructor is default
    SR_velocity() = default;

    //! destructor is default
    virtual ~SR_velocity() = default;


    // INTERFACE

  public:

    //! evaluate full tensor, returning a flattened list
    virtual std::unique_ptr<flattened_tensor> compute() = 0;

    //! evaluate component of tensor
    virtual GiNaC::ex compute_component(field_index i) = 0;

    //! evaluate lambda for tensor
    virtual std::unique_ptr<atomic_lambda> compute_lambda(const abstract_index& i) = 0;

    //! invalidate cache
    virtual void reset_cache() = 0;

  };



#endif //CPPTRANSPORT_SR_VELOCITY_H