//
// Created by David Seery on 20/12/2013.
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


#ifndef __scheduler_H_
#define __scheduler_H_


#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <set>
#include <sstream>
#include <stdexcept>

#include "transport-runtime/tasks/task.h"
#include "transport-runtime/tasks/integration_tasks.h"
#include "transport-runtime/tasks/output_tasks.h"
#include "transport-runtime/scheduler/context.h"
#include "transport-runtime/scheduler/work_queue.h"


namespace transport
  {

    //! Abstract filter function, used to filter work items when adding to a schedule
		template <typename work_element>
    class abstract_filter
      {
      public:
        virtual bool filter(const work_element& config) const = 0;
      };


    //! Empty filter
		template <typename work_element>
    class trivial_filter: public abstract_filter<work_element>
      {
      public:
        bool filter(const work_element& config) const { return(true); }
      };


    //! Filter function for work items -- used by slave nodes to filter out
    //! pieces of work intended for them
		template <typename work_element>
    class work_item_filter: public abstract_filter<work_element>
      {
      public:

        //! Construct an empty filter
        work_item_filter()
          {
          }

        //! Construct a filter from a predefined set of items
		    work_item_filter(const std::list<unsigned int>& filter_set)
			    {
				    items.clear();
		        std::copy(filter_set.begin(), filter_set.end(), std::inserter(items, items.begin()));
			    }

        //! Add an item to the list of work-items included in this filter
        void add_work_item(unsigned int serial) { this->items.insert(serial); }

        //! Check whether a work-item is part of the filter
        bool filter(const work_element& config)   const { return(this->items.find(config.get_serial()) != this->items.end()); }


        // WRITE SELF TO STREAM

      public:

        //! write details
        template <typename Stream> void write(Stream& out);


        // INTERNAL DATA

      private:

        //! std::set holding the serial number for each items we are supposed to process
        std::set<unsigned int> items;

      };


    template <typename work_element>
    template <typename Stream>
    void work_item_filter<work_element>::write(Stream& out)
      {
        out << CPPTRANSPORT_FILTER_TAG;
        for(std::set<unsigned int>::iterator t = this->items.begin(); t != this->items.end(); ++t)
          {
            out << (t != this->items.begin() ? ", " : " ") << *t;
          }
        out << '\n';
      }


    //! Write a filter to a stream
		template <typename work_element, typename Char, typename Traits>
    std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& out, const work_item_filter<work_element>& filter)
      {
        filter.write(out);
        return(out);
      }


    class scheduler
      {
      public:
        scheduler(context& c)
          : ctx(c)
          {
          }

        //! set up a work queue for a two-point function task, using a user-provided filter function
        template <typename number>
        work_queue<twopf_kconfig_record> make_queue(unsigned int size, const twopf_task<number>& task, const abstract_filter<twopf_kconfig>& F);

        //! set up a work queue for a three-point function task, using a user-provided filter function
        template <typename number>
        work_queue<threepf_kconfig_record> make_queue(unsigned int size, const threepf_task<number>& task, const abstract_filter<threepf_kconfig>& F);

        //! set up a work queue for a zeta three-point function task, using a user-provided filter function
        template <typename number>
        work_queue<threepf_kconfig_record> make_queue(unsigned int size, const zeta_threepf_task<number>& task, const abstract_filter<threepf_kconfig>& F);

		    //! set up a work queue for an output task, using a user-provided filter function
		    template <typename number>
		    work_queue< output_task_element<number> > make_queue(const output_task<number>& task, const abstract_filter< output_task_element<number> >& F);

        //! set up a work queue for a two-point function task, using no filter
        template <typename number>
        work_queue<twopf_kconfig_record> make_queue(unsigned int size, const twopf_task<number>& task);

        //! set up a work queue for a three-point function task, using no filter
        template <typename number>
        work_queue<threepf_kconfig_record> make_queue(unsigned int size, const threepf_task<number>& task);

		    //! set up a work queue for an output task, using no filter
		    template <typename number>
			  work_queue< output_task_element<number> > make_queue(const output_task<number>& task);

      protected:
        const context&    ctx;
      };


    // QUEUE-MAKING FUNCTIONS -- with a filter


    template <typename number>
    work_queue<twopf_kconfig_record> scheduler::make_queue(unsigned int size, const twopf_task<number>& task, const abstract_filter<twopf_kconfig>& F)
      {
        // set up an empty queue
        work_queue<twopf_kconfig_record> work(this->ctx, size);

        const twopf_kconfig_database& twopf_db = task.get_twopf_database();
        for(twopf_kconfig_database::const_record_iterator t = twopf_db.record_begin(); t != twopf_db.record_end(); ++t)
          {
            if(F.filter(*(*t))) work.enqueue_work_item(*t);
          }

        return(work);
      }


    template <typename number>
    work_queue<threepf_kconfig_record> scheduler::make_queue(unsigned int size, const threepf_task<number>& task, const abstract_filter<threepf_kconfig>& F)
      {
        work_queue<threepf_kconfig_record> work(this->ctx, size);

        const threepf_kconfig_database& threepf_db = task.get_threepf_database();
        for(threepf_kconfig_database::const_record_iterator t = threepf_db.record_begin(); t != threepf_db.record_end(); ++t)
          {
            if(F.filter(*(*t))) work.enqueue_work_item(*t);
          }

        return(work);
      }


    template <typename number>
    work_queue<threepf_kconfig_record> scheduler::make_queue(unsigned int size, const zeta_threepf_task<number>& task, const abstract_filter<threepf_kconfig>& F)
	    {
        work_queue<threepf_kconfig_record> work(this->ctx, size);

        const threepf_kconfig_database& threepf_db = task.get_threepf_database();
        for(threepf_kconfig_database::const_record_iterator t = threepf_db.record_begin(); t != threepf_db.record_end(); ++t)
          {
            if(F.filter(*(*t))) work.enqueue_work_item(*t);
          }

        return(work);
	    }


		template <typename number>
		work_queue< output_task_element<number> > scheduler::make_queue(const output_task<number>& task, const abstract_filter< output_task_element<number> >& F)
			{
				work_queue< output_task_element<number> > work(this->ctx, 1);

				const typename std::vector< output_task_element<number> >& config_list = task.get_elements();
				for(typename std::vector< output_task_element<number> >::const_iterator t = config_list.begin(); t != config_list.end(); ++t)
					{
						if(F.filter(*t)) work.enqueue_work_item(*t);
					}

				return(work);
			}


    // QUEUE-MAKING FUNCTIONS -- without a filter


    template <typename number>
    work_queue<twopf_kconfig_record> scheduler::make_queue(unsigned int size, const twopf_task<number>& task)
      {
        return(this->make_queue(size, task, trivial_filter<twopf_kconfig>()));
      }


    template <typename number>
    work_queue<threepf_kconfig_record> scheduler::make_queue(unsigned int size, const threepf_task<number>& task)
      {
        return(this->make_queue(size, task, trivial_filter<threepf_kconfig>()));
      }


		template <typename number>
		work_queue< output_task_element<number> > scheduler::make_queue(const output_task<number>& task)
			{
				return(this->make_queue(task, trivial_filter< output_task_element<number> >()));
			}


  } // / namespace transport



#endif //__scheduler_H_