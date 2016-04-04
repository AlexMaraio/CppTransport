//
// Created by David Seery on 26/03/15.
// Copyright (c) 2015-2016 University of Sussex. All rights reserved.
//


#ifndef CPPTRANSPORT_GENERIC_BATCHER_H
#define CPPTRANSPORT_GENERIC_BATCHER_H

#include <vector>
#include <list>
#include <functional>
#include <memory>

#include "transport-runtime-api/defaults.h"

#include "transport-runtime-api/utilities/host_information.h"
#include "transport-runtime-api/utilities/formatter.h"

#include "transport-runtime-api/data/batchers/container_dispatch_function.h"
#include "transport-runtime-api/data/batchers/container_replace_function.h"


#include "boost/filesystem/operations.hpp"
#include "boost/timer/timer.hpp"
#include "boost/log/core.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/expressions.hpp"
#include "boost/log/attributes.hpp"
#include "boost/log/sources/severity_feature.hpp"
#include "boost/log/sources/severity_logger.hpp"
#include "boost/log/sinks/sync_frontend.hpp"
#include "boost/log/sinks/text_file_backend.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"


namespace transport
	{

    // Batcher objects, used by integration workers to push results into a container

    //! Abstract batcher object, from which the concrete two- and threepf-batchers are derived.
    //! The batcher has a log directory, used for logging all transaction written into it,
    //! and also has a container replacement mechanism which writes all cached data into
    //! a data_manager-managed temporary file, and then pushes it to the master process.
    class generic_batcher
	    {

      public:

        //! Internal flag indicating whether flushes occur whenever the batcher becomes full,
        //! or whether we wait until the end-of-integration is reported.
        //! To unwind integrations, we need the delayed mode.
        enum class flush_mode { flush_immediate, flush_delayed };

        //! Logging severity level
        enum class log_severity_level { datapipe_pull, normal, warning, error, critical };

        typedef boost::log::sinks::synchronous_sink< boost::log::sinks::text_file_backend > sink_t;


        // CONSTRUCTOR, DESTRUCTOR

      public:

        //! constructor
        template <typename handle_type>
        generic_batcher(unsigned int cap, unsigned int ckp,
                        const boost::filesystem::path& cp, const boost::filesystem::path& lp,
                        std::unique_ptr<container_dispatch_function> d, std::unique_ptr<container_replace_function> r,
                        handle_type h, unsigned int w, unsigned int g=0, bool no_log=false);

        //! move constructor
        generic_batcher(generic_batcher&&) = default;

        virtual ~generic_batcher();


        // ADMINISTRATION

      public:

        //! Return the maximum memory available for batchers on this worker
        size_t get_capacity() const { return(this->capacity); }

        //! Set the path to the (new) container
        void set_container_path(const boost::filesystem::path& path) { this->container_path = path; }

        //! Return the path to the (current) container
        const boost::filesystem::path& get_container_path() const { return(this->container_path); }

        //! Set an implementation-dependent handle
        template <typename handle_type>
        void set_manager_handle(handle_type h)  { this->manager_handle = static_cast<void*>(h); }

        //! Return an implementation-dependent handle
        template <typename handle_type>
        void get_manager_handle(handle_type* h) const { *h = static_cast<handle_type>(this->manager_handle); }

		    //! Return worker group
		    unsigned int get_worker_group() const { return(this->worker_group); }

        //! Return worker numbers
        unsigned int get_worker_number() const { return(this->worker_number); }

        //! Return host information
        const host_information& get_host_information() const { return(this->host_info); }

        //! Close batcher
        virtual void close();


        // LOGGING

      public:

        //! Return logger
        boost::log::sources::severity_logger<log_severity_level>& get_log() { return(this->log_source); }


        // FLUSH INTERFACE

      public:

        //! Get flush mode
        flush_mode get_flush_mode() const { return(this->mode); }

        //! Set flush mode
        virtual void set_flush_mode(flush_mode f) { this->mode = f; }


        // INTERNAL API

      protected:

        //! Compute the size of all currently-batched results
        virtual size_t storage() const = 0;

        //! Flush currently-batched results into the database, and then send to the master process
        virtual void flush(replacement_action action);

        //! Check if the batcher is ready for flush
        void check_for_flush();


        // INTERNAL DATA

      protected:

        //! Host information
        host_information                                         host_info;

		    //! Worker group associated with this batcher;
		    //! usually zero unless we are doing parallel batching.
		    //! Later, groups identify different integrations which have been chained together
		    unsigned int                                             worker_group;

        //! Worker number associated with this batcher
        unsigned int                                             worker_number;


        // OTHER INTERNAL DATA

        //! Capacity available
        unsigned int                                             capacity;

        //! Container path
        boost::filesystem::path                                  container_path;

        //! Log directory path
        boost::filesystem::path                                  logdir_path;

        //! Data manager handle
        void*                                                    manager_handle;

        //! Callback for dispatching a container
        std::unique_ptr<container_dispatch_function>             dispatcher;

        //! Callback for obtaining a replacement container
        std::unique_ptr<container_replace_function>              replacer;


        // FLUSH HANDLING

        //! Needs flushing at next opportunity?
        bool                                                     flush_due;

        //! Flushing mode
        flush_mode                                               mode;

        //! checkpoint interval in nanoseconds; 0 indicates that checkpointing is disabled
        boost::timer::nanosecond_type                            checkpoint_interval;

        //! checkpoint timer
        boost::timer::cpu_timer                                  checkpoint_timer;


        // LOGGING

        //! Logger source
        boost::log::sources::severity_logger<log_severity_level> log_source;

        //! Logger sink
        boost::shared_ptr< sink_t >                              log_sink;

	    };


    // GENERIC BATCHER METHODS


    template <typename handle_type>
    generic_batcher::generic_batcher(unsigned int cap, unsigned int ckp,
                                     const boost::filesystem::path& cp, const boost::filesystem::path& lp,
                                     std::unique_ptr<container_dispatch_function> d, std::unique_ptr<container_replace_function> r,
                                     handle_type h, unsigned int w, unsigned int g, bool no_log)
	    : capacity(cap),
        checkpoint_interval(boost::timer::nanosecond_type(ckp)*1000*1000*1000),
	      container_path(cp),
	      logdir_path(lp),
	      dispatcher(std::move(d)),
	      replacer(std::move(r)),
	      worker_group(g),
	      worker_number(w),
	      manager_handle(static_cast<void*>(h)),
	      mode(flush_mode::flush_immediate),
	      flush_due(false)
	    {
        // set up logging

        std::ostringstream log_file;
        log_file << CPPTRANSPORT_LOG_FILENAME_A << worker_number << CPPTRANSPORT_LOG_FILENAME_B;

        boost::filesystem::path log_path = logdir_path / log_file.str();

        if(!no_log)
	        {
            boost::shared_ptr<boost::log::core> core = boost::log::core::get();

//		    core->set_filter(boost::log::trivial::severity >= normal);

            boost::shared_ptr<boost::log::sinks::text_file_backend> backend =
	                                                                    boost::make_shared<boost::log::sinks::text_file_backend>(boost::log::keywords::file_name = log_path.string(),
                                                                                                                               boost::log::keywords::open_mode = std::ios::app);

            // enable auto-flushing of log entries
            // this degrades performance, but we are not writing many entries and they
            // will not be lost in the event of a crash
            backend->auto_flush(true);

            // Wrap it into the frontend and register in the core.
            // The backend requires synchronization in the frontend.
            this->log_sink = boost::shared_ptr<sink_t>(new sink_t(backend));

            core->add_sink(this->log_sink);

            boost::log::add_common_attributes();
	        }
        else
	        {
            this->log_sink.reset();
	        }

        BOOST_LOG_SEV(this->log_source, log_severity_level::normal) << "** Instantiated batcher (capacity " << format_memory(capacity) << ")"
	        << " on MPI host " << host_info.get_host_name();

        BOOST_LOG_SEV(this->log_source, log_severity_level::normal) << "** Host details: OS = " << host_info.get_os_name()
	        << ", version = " << host_info.get_os_version()
	        << " (release = " << host_info.get_os_release()
	        << ") | " << host_info.get_architecture()
	        << " | CPU vendor = " << host_info.get_cpu_vendor_id();

        if(this->checkpoint_interval == 0)
          {
            BOOST_LOG_SEV(this->log_source, log_severity_level::normal) << "** Checkpointing disabled";
          }
        else
          {
            BOOST_LOG_SEV(this->log_source, log_severity_level::normal) << "** Checkpoint interval = " << format_time(this->checkpoint_interval);
          }
	    }


    generic_batcher::~generic_batcher()
	    {
        if(this->log_sink)    // implicitly converts to bool, value true if not null
	        {
            boost::shared_ptr< boost::log::core > core = boost::log::core::get();
            core->remove_sink(this->log_sink);
	        }
	    }


    void generic_batcher::close()
	    {
        this->flush(replacement_action::action_close);
	    }


    void generic_batcher::check_for_flush()
	    {
        if(this->storage() > this->capacity)
	        {
            switch(this->mode)
              {
                case flush_mode::flush_immediate:
                  this->flush(replacement_action::action_replace);
                  break;

                case flush_mode::flush_delayed:
                  this->flush_due = true;
                  break;
              }
	        }
	    }


    void generic_batcher::flush(replacement_action action)
      {
        // reset checkpoint timer
        this->checkpoint_timer.stop();
        this->checkpoint_timer.start();
      }


    template <typename Item>
    class UnbatchPredicate
	    {
      public:
        UnbatchPredicate(unsigned int s)
	        : source_serial(s)
	        {
	        }

        bool operator()(const Item& it)
	        {
            return(it.source_serial == this->source_serial);
	        }

      private:
        unsigned int source_serial;
	    };


	}   // namespace transport


#endif //CPPTRANSPORT_GENERIC_BATCHER_H
