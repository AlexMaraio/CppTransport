//
// Created by David Seery on 31/12/2013.
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


#ifndef CPPTRANSPORT_EXCEPTIONS_H
#define CPPTRANSPORT_EXCEPTIONS_H


#include <string>
#include <stdexcept>


namespace transport
  {

     enum class exception_type
       {
          FATAL_ERROR,                                       // unrecoverable error
          RUNTIME_ERROR,                                     // generic runtime error
          SCHEDULING_ERROR,                                  // error while scheduling a task
          TASK_STRUCTURE_ERROR,                              // disallowed configuration while building task hierarchy
          MPI_ERROR,                                         // error associated with MPI message passing
          RECORD_NOT_FOUND, REPO_NOT_FOUND,                  // missing items in the repository
          REPOSITORY_ERROR,                                  // generic repository error
          REPOSITORY_BACKEND_ERROR,                          // error from repository database backend
          TRANSACTION_ERROR,                                 // error generated by repository transaction management
          DATA_CONTAINER_ERROR,                              // generic data-container error
          DATA_MANAGER_BACKEND_ERROR,                        // error from data manager database backend
          MISSING_MODEL_INSTANCE,                            // could not find model instance to dispatch to
          INTEGRATION_FAILURE,                               // failure of integration step
          REFINEMENT_FAILURE,                                // failure of mesh refinement
          STORAGE_ERROR,                                     // error while storing the output of an integration
          BACKEND_ERROR,                                     // problem encountered by a compute backend
          SERIALIZATION_ERROR,                               // error encountered during serialization
          DATAPIPE_ERROR,                                    // generic datapipe error
          DERIVED_PRODUCT_ERROR,                             // error encountered when generating a derived product
          JOURNAL_ERROR,                                     // error in work journal
          SPLINE_ERROR,                                      // error while performing a spline
          REPORTING_ERROR,                                   // error while constructing a report
          SEEDING_ERROR                                      // error while seeding a writer from a previous content group
      };


    class runtime_exception: public std::runtime_error
      {
        // CONSTRUCTOR, DESTRUCTOR

      public:

        runtime_exception(exception_type t, const std::string msg)
          : type(t), std::runtime_error(msg)
          {
          }

        exception_type get_exception_code() { return(this->type); }

        // INTERNAL DATA
      protected:
        exception_type type;
      };

  }   // namespace transport



#endif //CPPTRANSPORT_EXCEPTIONS_H
