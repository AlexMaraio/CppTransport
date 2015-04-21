//
// Created by David Seery on 25/03/15.
// Copyright (c) 2015 University of Sussex. All rights reserved.
//


#ifndef __integration_writer_H_
#define __integration_writer_H_


#include <iostream>
#include <string>
#include <memory>
#include <functional>

#include "transport-runtime-api/serialization/serializable.h"

#include "transport-runtime-api/exceptions.h"
#include "transport-runtime-api/localizations/messages_en.h"

#include "transport-runtime-api/repository/records/repository_records.h"
#include "transport-runtime-api/repository/writers/generic_writer.h"

#include "boost/log/core.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/sources/severity_feature.hpp"
#include "boost/log/sources/severity_logger.hpp"
#include "boost/log/sinks/sync_frontend.hpp"
#include "boost/log/sinks/text_file_backend.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"


namespace transport
	{

    // WRITER FOR INTEGRATION OUTPUT

    //! Integration writer: used to commit integration output to the database
		template <typename number>
    class integration_writer: public generic_writer
	    {

      public:

        //! Define a commit callback object. Used to commit data products to the repository
        typedef std::function<void(integration_writer<number>&)> commit_callback;

        //! Define an abort callback object. Used to abort storage of data products
        typedef std::function<void(integration_writer<number>&)> abort_callback;

        //! Define an aggregation callback object. Used to aggregate results from worker processes
        typedef std::function<bool(integration_writer<number>&, const std::string&)> aggregate_callback;

        //! Define an integrity check callback object.
        typedef std::function<void(integration_writer<number>&, integration_task<number>*)> integrity_callback;

        class callback_group
	        {
          public:
            commit_callback commit;
            abort_callback  abort;
	        };


        // CONSTRUCTOR, DESTRUCTOR

      public:

        //! Construct an integration writer object.
        //! After creation it is not yet associated with anything in the data_manager backend; that must be done later
        //! by the task_manager, which can depute a data_manager object of its choice to do the work.
        integration_writer(const std::string& n, integration_task_record<number>* rec, const callback_group& c,
                           const typename generic_writer::metadata_group& m, const typename generic_writer::paths_group& p,
                           unsigned int w, unsigned int wg);

        //! disallow copying to ensure consistency of RAII idiom
        integration_writer(const integration_writer<number>& obj) = delete;

        //! Destroy an integration container object
        virtual ~integration_writer();


        // AGGREGATION

      public:

        //! Set aggregator
        void set_aggregation_handler(aggregate_callback c) { this->aggregator = c; }

        //! Aggregate a product
        bool aggregate(const std::string& product);


        // DATABASE FUNCTIONS

      public:

        //! Commit contents of this integration_writer to the database
        void commit() { this->callbacks.commit(*this); this->committed = true; }

        //! Set integrity check callback
        void set_integrity_check_handler(integrity_callback c) { this->integrity_checker = c; }

        //! Check integrity
        void check_integrity(integration_task<number>* tk) { if(this->integrity_checker) this->integrity_checker(*this, tk); }


        // STATISTICS

      public:

        //! Return whether we're collecting per-configuration statistics
        bool collect_statistics() const { return(this->supports_stats); }


        // METADATA

      public:

        //! get workgroup number
        unsigned int get_workgroup_number() const { return(this->workgroup_number); }

        //! set workgroup number (used if seeding this writer by a previous integration)
        void set_workgroup_number(unsigned int wg) { this->workgroup_number = wg; }

        //! Return task
        integration_task_record<number>* get_record() const { return(this->parent_record); }

        //! Set metadata
        void set_metadata(const integration_metadata& data) { this->metadata = data; }

        //! Get metadata
        const integration_metadata& get_metadata() const { return(this->metadata); }

        //! Merge list of failed serials reported by backend (not all backends may support this)
        void merge_failure_list(const std::list<unsigned int>& failed) { std::list<unsigned int> temp = failed; this->set_fail(true); temp.sort(); this->failed_serials.merge(temp); }

        //! Set seed
        void set_seed(const std::string& g) { this->seeded = true; this->seed_group = g; }

        //! Query seeded status
        bool is_seeded() const { return(this->seeded); }

        //! Query seeded group name
        const std::string& get_seed_group() const { return(this->seed_group); }


        // INTEGRITY CHECK

      public:

        //! get list of missing k-configuration serials
        const std::list<unsigned int>& get_missing_serials() const { return(this->missing_serials); }

		    //! set list of missing k-configuration serials
		    void set_missing_serials(const std::list<unsigned int>& s) { this->missing_serials = s; this->missing_serials.sort(); }


        // INTERNAL DATA

      private:

        // REPOSITORY CALLBACK FUNCTIONS

        //! Repository callbacks
        callback_group callbacks;


        // DATA MANAGER CALLBACK FUNCTIONS

        //! Aggregate callback
        aggregate_callback aggregator;

        //! Integrity check callback
        integrity_callback integrity_checker;


        // METADATA

        //! workgroup number
        unsigned int workgroup_number;

        //! task associated with this integration writer
        integration_task_record<number>* parent_record;

        //! metadata for this integration
        integration_metadata metadata;

        //! was this writer seeded?
        bool seeded;

        //! name of seed group, if so
        std::string seed_group;


        // FAILURE STATUS

        //! List of failed serial numbers reported by backend (not all backends may support this)
        std::list<unsigned int> failed_serials;


        // INTEGRITY STATUS

        //! List of missing serial numbers
        //! (this isn't the same as the list of failed serials reported by the backend; we compute this by testing the
        //! integrity of the database directly and cross-check with failures reported by the backend)
        std::list<unsigned int> missing_serials;


        // MISCELLANEOUS

        //! are we collecting per-configuration statistics?
        bool supports_stats;

	    };


    // INTEGRATION_WRITER METHODS


    template <typename number>
    integration_writer<number>::integration_writer(const std::string& n, integration_task_record<number>* rec,
                                                   const typename integration_writer<number>::callback_group& c,
                                                   const generic_writer::metadata_group& m, const generic_writer::paths_group& p,
                                                   unsigned int w, unsigned int wg)
	    : generic_writer(n, m, p, w),
        workgroup_number(wg),
        seeded(false),
	      callbacks(c),
	      aggregator(nullptr),
        integrity_checker(nullptr),
	      parent_record(dynamic_cast< integration_task_record<number>* >(rec->clone())),
	      supports_stats(rec->get_task()->get_model()->supports_per_configuration_statistics()),
	      metadata()
	    {
        assert(this->parent_record != nullptr);
	    }


		template <typename number>
    integration_writer<number>::~integration_writer()
	    {
        // if not committed, abort. WARNING! Although this behaviour is common to all writers,
        // this has to happen in the derived destructor, not the base generic_writer<> destructor
        // because by the time we arrive in the generic_writer<> destructor we have lost
        // the identity of the derived class
        if(!this->committed) this->callbacks.abort(*this);

        // delete copy of task object
        delete this->parent_record;
	    }


		template <typename number>
    bool integration_writer<number>::aggregate(const std::string& product)
	    {
        if(!this->aggregator)
	        {
            assert(false);
            throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_REPO_WRITER_AGGREGATOR_UNSET);
	        }

        return this->aggregator(*this, product);
	    }


	}   // namespace transport


#endif //__integration_writer_H_
