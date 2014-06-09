//
// Created by David Seery on 30/12/2013.
// Copyright (c) 2013 University of Sussex. All rights reserved.
//


#ifndef __repository_H_
#define __repository_H_


#include <iostream>
#include <string>

#include "transport-runtime-api/serialization/serializable.h"
#include "transport-runtime-api/manager/instance_manager.h"

#include "boost/filesystem/operations.hpp"
#include "boost/log/core.hpp"
#include "boost/log/trivial.hpp"
#include "boost/log/sources/severity_feature.hpp"
#include "boost/log/sources/severity_logger.hpp"
#include "boost/log/sinks/sync_frontend.hpp"
#include "boost/log/sinks/text_file_backend.hpp"
#include "boost/log/utility/setup/common_attributes.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"


// log file name
#define __CPP_TRANSPORT_LOG_FILENAME_A  "worker_"
#define __CPP_TRANSPORT_LOG_FILENAME_B  "_%3N.log"


// JSON node names

#define __CPP_TRANSPORT_REPO_REPOSITORY_LEAF                     "database.unqlite"
#define __CPP_TRANSPORT_REPO_TASKOUTPUT_LEAF                     "output"
#define __CPP_TRANSPORT_REPO_LOGDIR_LEAF                         "logs"
#define __CPP_TRANSPORT_REPO_TEMPDIR_LEAF                        "tempfiles"
#define __CPP_TRANSPORT_REPO_TASKFILE_LEAF                       "tasks.sqlite"
#define __CPP_TRANSPORT_REPO_DATABASE_LEAF                       "integration.sqlite"

#define __CPP_TRANSPORT_NODE_PACKAGE_NAME                        "package-name"
#define __CPP_TRANSPORT_NODE_PACKAGE_MODELUID                    "package-model-uid"
#define __CPP_TRANSPORT_NODE_PACKAGE_METADATA                    "package-metadata"
#define __CPP_TRANSPORT_NODE_PACKAGE_ICS                         "package-ics"

#define __CPP_TRANSPORT_NODE_PKG_METADATA_NAME                   "model-name"
#define __CPP_TRANSPORT_NODE_PKG_METADATA_AUTHOR                 "model-author"
#define __CPP_TRANSPORT_NODE_PKG_METADATA_TAG                    "model-tag"
#define __CPP_TRANSPORT_NODE_PKG_METADATA_CREATED                "creation-time"
#define __CPP_TRANSPORT_NODE_PKG_METADATA_EDITED                 "last-edit-time"
#define __CPP_TRANSPORT_NODE_PKG_METADATA_RUNTIMEAPI             "runtime-api-version"

#define __CPP_TRANSPORT_NODE_TASK_NAME                           "task-name"
#define __CPP_TRANSPORT_NODE_TASK_METADATA                       "task-metadata"
// used for integration tasks
#define __CPP_TRANSPORT_NODE_TASK_INTEGRATION_DETAILS            "integration-details"
#define __CPP_TRANSPORT_NODE_TASK_OUTPUT_GROUPS                  "output-groups"
// used for output tasks
#define __CPP_TRANSPORT_NODE_TASK_OUTPUT_DETAILS                 "output-task"

#define __CPP_TRANSPORT_NODE_TASK_METADATA_PACKAGE               "package-name"
#define __CPP_TRANSPORT_NODE_TASK_METADATA_CREATED               "creation-time"
#define __CPP_TRANSPORT_NODE_TASK_METADATA_EDITED                "last-edit-time"
#define __CPP_TRANSPORT_NODE_TASK_METADATA_RUNTIMEAPI            "runtime-api-version"

#define __CPP_TRANSPORT_NODE_DERIVED_PRODUCT_NAME                "product-name"
#define __CPP_TRANSPORT_NODE_DERIVED_PRODUCT_METADATA            "product-metadata"
#define __CPP_TRANSPORT_NODE_DERIVED_PRODUCT_METADATA_CREATED    "creation-time"
#define __CPP_TRANSPORT_NODE_DERIVED_PRODUCT_METADATA_EDITED     "last-edit-time"
#define __CPP_TRANSPORT_NODE_DERIVED_PRODUCT_METADATA_RUNTIMEAPI "runtime-api-version"

#define __CPP_TRANSPORT_NODE_DERIVED_PRODUCT_DETAILS             "product_details"

#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_TASK_NAME               "parent-task"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_REPO_ROOT               "repo-path"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_DATA_ROOT               "output-path"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_CREATED                 "creation-time"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_LOCKED                  "locked"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTES                   "notes"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTE                    "note"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_TAGS                    "tags"
#define __CPP_TRANSPORT_NODE_OUTPUTGROUP_TAG                     "tag"

#define __CPP_TRANSPORT_NODE_PAYLOAD_INTEGRATION_BACKEND         "backend"
#define __CPP_TRANSPORT_NODE_PAYLOAD_INTEGRATION_DATABASE        "database-path"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_ARRAY               "output-array"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_PRODUCT_NAME        "parent-product"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_FILENAME            "filename"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_CREATED             "creation-time"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTES               "notes"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTE                "note"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAGS                "tags"
#define __CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAG                 "tag"

#define __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_WALLCLOCK_TIME     "total-wallclock-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_AGG_TIME           "total-aggregation-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_INT_TIME           "total-integration-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_MIN_MEAN_INT_TIME        "min-mean-integration-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_MAX_MEAN_INT_TIME        "max-mean-integration-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MIN_INT_TIME      "global-min-integration-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MAX_INT_TIME      "global-max-integration-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_BATCH_TIME         "total-batching-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_MIN_MEAN_BATCH_TIME      "min-mean-batching-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_MAX_MEAN_BATCH_TIME      "max-mean-batching-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MIN_BATCH_TIME    "global-min-batching-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MAX_BATCH_TIME    "global-max-batching-time"
#define __CPP_TRANSPORT_NODE_TIMINGDATA_NUM_CONFIGURATIONS       "total-configurations"


namespace transport
	{

    // forward-declare model to avoid circular inclusion
    template <typename number>
    class model;


    // forward-declare initial_conditions
    template <typename number>
    class initial_conditions;


    // forward-declare tasks
    template <typename number>
    class task;


    template <typename number>
    class integration_task;


    template <typename number>
    class twopf_task;


    template <typename number>
    class threepf_task;


    template <typename number>
    class output_task;

    // forward-declare derived_product object to avoid circular inclusion
    namespace derived_data
	    {
        template <typename number>
        class derived_product;
	    }

    // forward-declare 'key' class used to create repositories
    // the complete declaration is in a separate file,
    // which must be included to allow creation of repositories
    class repository_creation_key;


    template <typename number>
    class repository
	    {

      public:

		    //! Task finder service
		    typedef std::function<task<number>*(const std::string&, model<number>*&)> task_finder;

        //! Types needed for logging
        typedef enum
	        {
            normal, notification, warning, error, critical
	        }                                                                               log_severity_level;
        typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend> sink_t;

        //! Read-only/Read-write access to the repository
        typedef enum
	        {
            readonly, readwrite
	        }                                                                               access_type;


        // DATA CONTAINER WRITE HANDLE

		    class integration_writer;

		    //! Define a commit callback object. Used by integration_writer to commit its data products to the repository
		    typedef std::function<void(integration_writer&)> commit_callback;

		    //! Timing metadata for an integration
		    class timing_metadata: public serializable
			    {
		      public:

				    //! null constructor - set all fields to zero
				    timing_metadata(void)
					    : total_wallclock_time(0),
					      total_aggregation_time(0),
					      total_integration_time(0),
					      min_mean_integration_time(0),
					      global_min_integration_time(0),
					      global_max_integration_time(0),
					      total_batching_time(0),
					      min_mean_batching_time(0),
					      max_mean_batching_time(0),
					      global_min_batching_time(0),
					      global_max_batching_time(0),
					      total_configurations(0)
					    {
					    }

				    //! value constructor - ensure all fields get set at once
		        timing_metadata(boost::timer::nanosecond_type wc, boost::timer::nanosecond_type ag,
		                        boost::timer::nanosecond_type it, boost::timer::nanosecond_type min_m_it, boost::timer::nanosecond_type max_m_it,
		                        boost::timer::nanosecond_type min_it, boost::timer::nanosecond_type max_it,
		                        boost::timer::nanosecond_type bt, boost::timer::nanosecond_type min_m_bt, boost::timer::nanosecond_type max_m_bt,
		                        boost::timer::nanosecond_type min_bt, boost::timer::nanosecond_type max_bt,
		                        unsigned int num)
			        : total_wallclock_time(wc),
			          total_aggregation_time(ag),
			          total_integration_time(it),
			          min_mean_integration_time(min_m_it),
			          max_mean_integration_time(max_m_it),
			          global_min_integration_time(min_it),
			          global_max_integration_time(max_it),
			          total_batching_time(bt),
			          min_mean_batching_time(min_m_bt),
			          max_mean_batching_time(max_m_bt),
			          global_min_batching_time(min_bt),
			          global_max_batching_time(max_bt),
			          total_configurations(num)
			        {
			        }

				    //! deserialization constructor
				    timing_metadata(serialization_reader* reader);


				    // SERIALIZE -- implements a 'serializable' interface

		      public:

				    //! serialize this object
				    void serialize(serialization_writer& writer) const override;


				    // DATA

		      public:

				    //! total wallclock time (measured on master process)
				    boost::timer::nanosecond_type total_wallclock_time;

				    //! total aggregation time (measured on master process)
				    boost::timer::nanosecond_type total_aggregation_time;

				    //! total integration time (measured by aggregating times reported by workers)
				    boost::timer::nanosecond_type total_integration_time;

				    //! minimum mean integration time (taken over all workers)
				    boost::timer::nanosecond_type min_mean_integration_time;

				    //! maxmimum mean integration time (taken over all workers)
				    boost::timer::nanosecond_type max_mean_integration_time;

				    //! global minimum integration time (per configuration)
				    boost::timer::nanosecond_type global_min_integration_time;

				    // global maximum integration time (per configuration)
				    boost::timer::nanosecond_type global_max_integration_time;

				    // total batching time (measured by aggregating times reported by workers)
				    boost::timer::nanosecond_type total_batching_time;

				    //! minimum mean batching time (taken over all workers)
				    boost::timer::nanosecond_type min_mean_batching_time;

				    // maximum mean batching time (taken over all workers)
				    boost::timer::nanosecond_type max_mean_batching_time;

				    // global minimum batching time (per configuration)
				    boost::timer::nanosecond_type global_min_batching_time;

				    // global maximum batching time (per configuration)
				    boost::timer::nanosecond_type global_max_batching_time;

				    // total number of configurations processed
				    unsigned int total_configurations;

			    };

        //! Integration container writer: forms a handle for a data container when writing the output of an integration
        class integration_writer
	        {

	          // CONSTRUCTOR, DESTRUCTOR

          public:

            //! Construct an integration container object.
            //! After creation it is not yet associated with anything in the data_manager backend; that must be done later
            //! by the task_manager, which can depute a data_manager object of its choice to do the work.
            integration_writer(integration_task<number>* tk, const std::list<std::string>& tg,
                               commit_callback c,
                               const boost::filesystem::path& root,
                               const boost::filesystem::path& output, const boost::filesystem::path& data,
                               const boost::filesystem::path& log, const boost::filesystem::path& task,
                               const boost::filesystem::path& temp,
                               unsigned int w, bool s);

		        //! override copy constructor to perform a deep copy
		        integration_writer(const integration_writer& obj);

            //! Destroy an integration container object
            ~integration_writer();


		        // ADMINISTRATION

          public:

            //! Set data_manager handle for data container
            template <typename data_manager_type>
            void set_data_manager_handle(data_manager_type data);

            //! Return data_manager handle for data container

            //! Throws a REPOSITORY_ERROR exception if the handle is unset
            template <typename data_manager_type>
            void get_data_manager_handle(data_manager_type* data);

            //! Set data_manager handle for taskfile
            template <typename data_manager_type>
            void set_data_manager_taskfile(data_manager_type data);

            //! Return data_manager handle for taskfile

            //! Throws a REPOSITORY_ERROR exception if the handle is unset
            template <typename data_manager_type>
            void get_data_manager_taskfile(data_manager_type* data);


		        // COMMIT TO DATABASE

          public:

            //! Commit contents of this integration_writer to the database
		        void commit() { this->committer(*this); }

		        
		        // LOGGING
		        
          public:

            //! Return logger
            boost::log::sources::severity_logger<log_severity_level>& get_log() { return (this->log_source); }

		        
		        // ABSOLUTE PATHS
		        
          public:
		        
            //! Return path to data container
            boost::filesystem::path get_abs_container_path() const { return(this->repo_root/this->data_path); }

            //! Return path to log directory
            boost::filesystem::path get_abs_logdir_path() const { return(this->repo_root/this->log_path); }

            //! Return path to task-data container
            boost::filesystem::path get_abs_taskfile_path() const { return(this->repo_root/this->task_path); }

            //! Return path to directory for temporary files
            boost::filesystem::path get_abs_tempdir_path() const { return(this->repo_root/this->temp_path); }


		        // RELATIVE PATHS

          public:

		        //! Return path to output directory
		        boost::filesystem::path get_relative_output_path() const { return(this->output_path); }

		        //! Return path to data container
		        boost::filesystem::path get_relative_container_path() const { return(this->data_path); }


		        // STATISTICS

          public:

            //! Return whether we're collecting per-configuration statistics
            const bool collect_statistics() const { return(this->supports_stats); }


		        // METADATA

          public:

		        //! Return task
		        integration_task<number>* get_task() const { return(this->parent_task); }

		        //! Return tags
		        const std::list<std::string>& get_tags() const { return(this->tags); }

		        //! Set timing metadata
		        void set_timing_metadata(const timing_metadata& timing) { this->timing_data = timing; }

		        //! Get timing metadata
		        const timing_metadata& get_timing_metadata() const { return(this->timing_data); }


            // INTERNAL DATA

          private:

		        // COMMIT CALLBACK

		        //! commit data products to the repository
		        commit_callback committer;


		        // METADATA

		        //! task associated with this integration writer
		        integration_task<number>* parent_task;

		        //! tags
		        std::list<std::string> tags;

		        //! timing metadata for this integration
		        timing_metadata timing_data;


		        // PATHS

		        //! root directory of the parent repository
		        const boost::filesystem::path repo_root;

		        //! relative path to output directory within the repository
            const boost::filesystem::path output_path;

		        //! relative path to data container
            const boost::filesystem::path data_path;

		        //! relative path to log direcotry
            const boost::filesystem::path log_path;

		        //! relative path to task file
            const boost::filesystem::path task_path;

		        //! relative path to temporary direcotry
            const boost::filesystem::path temp_path;

		        //! are we collecting per-configuration statistics?
            bool supports_stats;

		        //! our MPI worker number
            const unsigned int worker_number;

		        //! internal handle used by data_manager to associate this writer with an integration database
            void* data_manager_handle;

		        //! internal handle used by data_manager to associate this writer with a task database
            void* data_manager_taskfile;

            //! Logger source
            boost::log::sources::severity_logger<log_severity_level> log_source;

            //! Logger sink
            boost::shared_ptr<sink_t> log_sink;

	        };


        // OUTPUT GROUPS


        //! Derived product descriptor. Used to enumerate the content associated with
        //! a output task output group.
        class derived_content: public serializable
	        {

          public:

            //! Create a derived_product descriptor
            derived_content(const std::string& prod, const std::string& fnam, const boost::posix_time::ptime& now,
            const std::list<std::string>& notes, const std::list<std::string>& tags)
	            : parent_product(prod), filename(fnam), created(now)
	            {
	            }

            //! Deserialization constructor
            derived_content(serialization_reader* reader);

            //! Destroy a derived_product descriptor
            ~derived_content() = default;


            // INTERFACE

            //! Get product name
            const std::string& get_parent_product() const { return (this->parent_product); }

            //! Get product pathname
            const boost::filesystem::path& get_filename() const { return(this->filename); }

            //! Get notes
            const std::list<std::string>& get_notes() const { return(this->notes); }

            //! Add note
            void add_note(const std::string& note) { this->notes.push_back(note); }

            //! Get creation time
            const boost::posix_time::ptime& get_creation_time() const { return(this->created); }


            // SERIALIZATION -- implements a 'serializable' interface

            //! Serialize this object
            virtual void serialize(serialization_writer& writer) const override;

            // INTERNAL DATA

          protected:

            //! Name of parent derived product
            std::string parent_product;

            //! Path to output
            boost::filesystem::path filename;

            //! Creation time
            boost::posix_time::ptime created;

            //! Notes
            std::list<std::string> notes;

            //! Tags
            std::list<std::string> tags;
	        };


		    // PAYLOADS FOR OUTPUT GROUPS

		    //! Integration payload
		    class integration_payload: public serializable
			    {

		      public:

				    //! Create a payload
				    integration_payload()
				      : timing_data()
					    {
					    }

            //! Deserialization constructor
            integration_payload(serialization_reader* reader);

				    //! Destroy a payload
				    ~integration_payload() = default;


				    // ADMIN

		      public:

		        //! Get name of backend used to generate this output group
		        const std::string& get_backend() const { return(this->backend); }
            //! Set name of backend used to generate this output group
            void set_backend(const std::string& be) { this->backend = be; }

				    //! Get path of data container
				    const boost::filesystem::path& get_container_path() const { return(this->container); }
            //! Set path of data container
            void set_container_path(const boost::filesystem::path& pt) { this->container = pt; }

				    //! Get timing metadata
				    const timing_metadata& get_timing_metadata() const { return(this->timing_data); }
				    //! Set timing metadata
				    void set_timing_metadata(const timing_metadata& timings) { this->timing_data = timings; }


            // WRITE TO A STREAM

          public:

            void write(std::ostream& out) const;


            // SERIALIZATION -- implements a 'serializable' interface

            //! Serialize this object
            virtual void serialize(serialization_writer& writer) const override;


				    // INTERNAL DATA

		      protected:

				    //! Backend used to generate this payload
				    std::string backend;

				    //! Path to data container
				    boost::filesystem::path container;

				    //! Timing metadata
				    timing_metadata timing_data;

			    };


        //! Derived product payload
        class output_payload: public serializable
	        {

          public:

						//! Create a payload
            output_payload() = default;

            //! Deserialization constructor
            output_payload(serialization_reader* reader);

		        //! Destroy a payload
		        ~output_payload() = default;


		        // ADMIN

          public:

            //! Add an output
            void add_derived_content(const derived_content& prod) { this->content.push_back(prod); }


            // WRITE TO A STREAM

          public:

            void write(std::ostream& out) const;


            // SERIALIZATION -- implements a 'serializable' interface

            //! Serialize this object
            virtual void serialize(serialization_writer& writer) const override;


            // INTERNAL DATA

          protected:

            //! List of derived outputs
            std::list<derived_content> content;
	        };


        //! Output group descriptor. Used to enumerate the output groups available for a particular task
		    template <typename Payload>
        class output_group: public serializable
	        {

          public:

            //! Create an output_group descriptor
            output_group(const std::string& tn, const boost::filesystem::path& root, const boost::filesystem::path& path,
                         const boost::posix_time::ptime& ctime, bool lock, const std::list<std::string>& nt, const std::list<std::string>& tg);

            //! Deserialization constructor
            output_group(serialization_reader* reader);

            //! Destroy an output_group descriptor
            ~output_group() = default;

            //! Write self to output stream
            void write(std::ostream& out) const;


            // INTERFACE

          public:

            //! Get creation time
            const boost::posix_time::ptime& get_creation_time() const { return (this->created); }


            //! Get path to repository root
            const boost::filesystem::path& get_repo_root_path() const { return (this->repo_root_path); }


            //! Get path to output root (typically a subdir of the repository root)
            const boost::filesystem::path& get_data_root_path() const { return (this->data_root_path); }


            //! Get locked flag
            bool get_lock_status() const { return (this->locked); }


            //! Get notes
            const std::list<std::string>& get_notes() const { return (this->notes); }


            //! Get tags
            const std::list<std::string>& get_tags() const { return (this->tags); }

            //! Check whether we match a set of tags
            bool check_tags(std::list<std::string> match_tags) const;

		        //! Get payload
		        Payload& get_payload() { return(this->payload); }


            // SERIALIZATION -- implements a 'serializable' interface

            //! Serialize this object
            void serialize(serialization_writer& writer) const;

            // PRIVATE DATA

          private:

            //! Parent task associated with this output. Not ordinarily user-visible.
            std::string task;

            //! Path to root of repository. Other paths are relative to this.
            boost::filesystem::path repo_root_path;

            //! Path of parent directory containing the output, usually residing within the repository
            boost::filesystem::path data_root_path;

            //! Creation time
            boost::posix_time::ptime created;

            //! Flag indicating whether or not this output group is locked
            bool locked;

            //! Array of strings representing notes attached to this group
            std::list<std::string> notes;

            //! Array of strings representing metadata tags
            std::list<std::string> tags;

		        //! Payload
		        Payload payload;
	        };


        // DATA CONTAINER READ HANDLE

        //! Integration container reader: forms a handle for a data container when reading the an integration from the database

        class derived_content_writer
	        {

          public:

            //! Construct a derived-content writer object
            derived_content_writer(const boost::filesystem::path& dir, const boost::filesystem::path& log,
                                   const boost::filesystem::path& task, const boost::filesystem::path& temp,
                                   unsigned int w);

            //! Destroy a derived-content writer object
            ~derived_content_writer();

            //! Set data_manager handle for taskfile
            template <typename data_manager_type>
            void set_data_manager_taskfile(data_manager_type data);

            //! Return data_manager handle for data container

            //! Throws a REPOSITORY_ERROR exception if the handle is unset
            template <typename data_manager_type>
            void get_data_manager_taskfile(data_manager_type* data);


            //! Return logger
            boost::log::sources::severity_logger<log_severity_level>& get_log() { return (this->log_source); }


            //! Return path to log directory
            const boost::filesystem::path& log_directory_path() const { return (this->path_to_log_directory); }


            //! Return path to task-data container
            const boost::filesystem::path& taskfile_path() const { return (this->path_to_taskfile); }


            //! Return path to directory for temporary files
            const boost::filesystem::path& temporary_files_path() const {  return (this->path_to_temp_directory); }


            // INTERNAL DATA

          private:

            const boost::filesystem::path path_to_directory;

            const boost::filesystem::path path_to_log_directory;
            const boost::filesystem::path path_to_taskfile;
            const boost::filesystem::path path_to_temp_directory;

            const unsigned int worker_number;

            void* data_manager_taskfile;

            //! Logger source
            boost::log::sources::severity_logger<log_severity_level> log_source;

            //! Logger sink
            boost::shared_ptr<sink_t> log_sink;

	        };


        // CONSTRUCTOR, DESTRUCTOR

      public:

        //! Create a repository object
        repository(const std::string& path, access_type mode)
	        : root_path(path), access_mode(mode)
	        {
	        }


        //! Close a repository, including the corresponding containers and environment. In practice this would always be delegated to the implementation class
        virtual ~repository() = default;


        // ADMIN DATA

      public:

        //! Get path to root of repository
        const boost::filesystem::path& get_root_path() const { return (this->root_path); };


        //! Get access mode
        const access_type& get_access_mode() const { return (this->access_mode); }


        // PUSH TASKS TO THE REPOSITORY DATABASE

      public:

        //! Write a 'model/initial conditions/parameters' combination (a 'package') to the package database.
        //! No combination with the supplied name should already exist; if it does, this is considered an error.
        virtual void write_package(const initial_conditions<number>& ics, const model<number>* m) = 0;

        //! Write an integration task to the database.
        virtual void write_task(const integration_task<number>& tk, const model<number>* m) = 0;

        //! Write an output task to the database
        virtual void write_task(const output_task<number>& tk) = 0;


        // PULL TASKS FROM THE REPOSITORY DATABASE

      public:

        //! Query the database for a named task, and reconstruct it if present.
        //! Supports both integration_task<> and output_task<> items.
        //! Output tasks write nullptr to the model* handle.
        virtual task<number>* lookup_task(const std::string& name, model<number>*& m,
                                          typename instance_manager<number>::model_finder finder) = 0;


        // ADD AN OUTPUT-GROUP TO A TASK

      public:

        //! Insert a record for new twopf output in the task database, and set up paths to a suitable data container
        virtual integration_writer new_integration_task_output(twopf_task<number>* tk, const std::list<std::string>& tags,
                                                               model<number>* m, unsigned int worker) = 0;

        //! Insert a record for new threepf output in the task database, and set up paths to a suitable data container
        virtual integration_writer new_integration_task_output(threepf_task<number>* tk, const std::list<std::string>& tags,
                                                               model<number>* m, unsigned int worker) = 0;


        // PULL OUTPUT-GROUPS FROM A TASK

      public:

        //! Enumerate the output groups available from a named task
        virtual std::list< output_group<integration_payload> > enumerate_integration_task_output(const std::string& name) = 0;


        // PUSH DERIVED-PRODUCT SPECIFICATIONS TO THE DATABASE

      public:

        //! Write a derived product specification
        virtual void write_derived_product(const derived_data::derived_product<number>& d) = 0;


        // PULL DERIVED-PRODUCT SPECIFICATIONS FROM THE DATABASE

      public:

        //! Query a derived product specification
        virtual derived_data::derived_product<number>*
          lookup_derived_product(const std::string &product, typename instance_manager<number>::model_finder finder) = 0;


        // ADD DERIVED CONTENT FROM AN OUTPUT TASK

      public:

        //! Add derived content
        virtual derived_content_writer
          new_output_task_output(output_task<number>* tk, const std::list<std::string>& tags, unsigned int worker) = 0;

        //! Lookup an output group for a task, given a set of tags
        virtual output_group<integration_payload>
          find_integration_task_output_group(const integration_task<number>* tk, const std::list<std::string>& tags) = 0;


        // PRIVATE DATA

      protected:

        //! Access mode
        const access_type access_mode;

        //! BOOST path to the repository root directory
        const boost::filesystem::path root_path;

	    };


    // INTEGRATION_WRITER METHODS


    template <typename number>
    repository<number>::timing_metadata::timing_metadata(serialization_reader* reader)
	    {
				assert(reader != nullptr);

		    reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_WALLCLOCK_TIME, total_wallclock_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_AGG_TIME, total_aggregation_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_INT_TIME, total_integration_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_MIN_MEAN_INT_TIME, min_mean_integration_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_MAX_MEAN_INT_TIME, max_mean_integration_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MIN_INT_TIME, global_min_integration_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MAX_INT_TIME, global_max_integration_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_BATCH_TIME, total_batching_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_MIN_MEAN_BATCH_TIME, min_mean_batching_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_MAX_MEAN_BATCH_TIME, max_mean_batching_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MIN_BATCH_TIME, global_min_batching_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MAX_BATCH_TIME, global_max_batching_time);
        reader->read_value(__CPP_TRANSPORT_NODE_TIMINGDATA_NUM_CONFIGURATIONS, total_configurations);
	    }


    template <typename number>
    void repository<number>::timing_metadata::serialize(serialization_writer& writer) const
	    {
				this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_WALLCLOCK_TIME, this->total_wallclock_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_AGG_TIME, this->total_aggregation_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_INT_TIME, this->total_integration_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_MIN_MEAN_INT_TIME, this->min_mean_integration_time);
				this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_MAX_MEAN_INT_TIME, this->max_mean_integration_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MIN_INT_TIME, this->global_min_integration_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MAX_INT_TIME, this->global_max_integration_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_TOTAL_BATCH_TIME, this->total_batching_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_MIN_MEAN_BATCH_TIME, this->min_mean_batching_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_MAX_MEAN_BATCH_TIME, this->max_mean_batching_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MIN_BATCH_TIME, this->global_min_batching_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_GLOBAL_MAX_BATCH_TIME, this->global_max_batching_time);
		    this->write_value_node(writer, __CPP_TRANSPORT_NODE_TIMINGDATA_NUM_CONFIGURATIONS, this->total_configurations);
	    }


    template <typename number>
    repository<number>::integration_writer::integration_writer(integration_task<number>* tk, const std::list<std::string>& tg,
                                                               typename repository<number>::commit_callback c,
                                                               const boost::filesystem::path& root,
                                                               const boost::filesystem::path& output, const boost::filesystem::path& data,
                                                               const boost::filesystem::path& log, const boost::filesystem::path& task,
                                                               const boost::filesystem::path& temp,
                                                               unsigned int w, bool s)
	    : parent_task(dynamic_cast<integration_task<number>*>(tk->clone())), tags(tg),
	      committer(c),
	      repo_root(root),
	      output_path(output), data_path(data),
	      log_path(log), task_path(task),
	      temp_path(temp),
	      worker_number(w), supports_stats(s),
	      data_manager_handle(nullptr), data_manager_taskfile(nullptr),
		    timing_data()
	    {
        std::ostringstream log_file;
        log_file << __CPP_TRANSPORT_LOG_FILENAME_A << worker_number << __CPP_TRANSPORT_LOG_FILENAME_B;
        boost::filesystem::path logfile_path = repo_root/log_path / log_file.str();

        boost::shared_ptr<boost::log::core> core = boost::log::core::get();

        boost::shared_ptr<boost::log::sinks::text_file_backend> backend =
	                                                                boost::make_shared<boost::log::sinks::text_file_backend>(boost::log::keywords::file_name = logfile_path.string());

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


		template <typename number>
		repository<number>::integration_writer::integration_writer(const typename repository<number>::integration_writer& obj)
			: parent_task(dynamic_cast<integration_task<number>*>(obj.parent_task->clone())), tags(obj.tags),
			  committer(obj.committer),
			  repo_root(obj.repo_root),
			  output_path(obj.output_path), data_path(obj.data_path),
			  log_path(obj.log_path), task_path(obj.task_path),
			  temp_path(obj.temp_path),
			  supports_stats(obj.supports_stats), worker_number(obj.worker_number),
			  data_manager_handle(obj.data_manager_handle),
			  data_manager_taskfile(obj.data_manager_taskfile),
			  log_source(obj.log_source), log_sink(obj.log_sink),
				timing_data(obj.timing_data)
			{
		    std::cerr << "Warning: integration_writer object being copied" << std::endl;
			}


    template <typename number>
    repository<number>::integration_writer::~integration_writer()
	    {
        // remove logging objects
        boost::shared_ptr<boost::log::core> core = boost::log::core::get();

        core->remove_sink(this->log_sink);

		    // delete copy of task object
		    delete this->parent_task;
	    }


    template <typename number>
    template <typename data_manager_type>
    void repository<number>::integration_writer::set_data_manager_handle(data_manager_type data)
	    {
        this->data_manager_handle = static_cast<void*>(data);  // will fail if data_manager_type not (static-)castable to void*
	    }


    template <typename number>
    template <typename data_manager_type>
    void repository<number>::integration_writer::get_data_manager_handle(data_manager_type* data)
	    {
        if(this->data_manager_handle == nullptr) throw runtime_exception(runtime_exception::REPOSITORY_ERROR, __CPP_TRANSPORT_REPO_OUTPUT_WRITER_UNSETHANDLE);
        *data = static_cast<data_manager_type>(this->data_manager_handle);
	    }


    template <typename number>
    template <typename data_manager_type>
    void repository<number>::integration_writer::set_data_manager_taskfile(data_manager_type data)
	    {
        this->data_manager_taskfile = static_cast<void*>(data);   // will fail if data_manager_type not (static)-castable to void*
	    }


    template <typename number>
    template <typename data_manager_type>
    void repository<number>::integration_writer::get_data_manager_taskfile(data_manager_type* data)
	    {
        if(this->data_manager_taskfile == nullptr) throw runtime_exception(runtime_exception::REPOSITORY_ERROR, __CPP_TRANSPORT_REPO_OUTPUT_WRITER_UNSETTASK);
        *data = static_cast<data_manager_type>(this->data_manager_taskfile);
	    }


    // output a data_product descriptor to a standard stream
    template <typename number>
    std::ostream& operator<<(std::ostream& out, const typename repository<number>::data_product& product)
	    {
        product.write(out);
        return (out);
	    }


    // DERIVED_CONTENT_WRITER METHODS


    template <typename number>
    repository<number>::derived_content_writer::derived_content_writer(const boost::filesystem::path& dir, const boost::filesystem::path& log,
                                                                       const boost::filesystem::path& task, const boost::filesystem::path& temp,
                                                                       unsigned int w)
	    : path_to_directory(dir), path_to_log_directory(log),
	      path_to_taskfile(task), path_to_temp_directory(temp),
	      worker_number(w), data_manager_taskfile(nullptr)
	    {
        std::ostringstream log_file;
        log_file << __CPP_TRANSPORT_LOG_FILENAME_A << worker_number << __CPP_TRANSPORT_LOG_FILENAME_B;
        boost::filesystem::path log_path = path_to_log_directory / log_file.str();

        boost::shared_ptr<boost::log::core> core = boost::log::core::get();

        std::ostringstream log_file_path;
        boost::shared_ptr<boost::log::sinks::text_file_backend> backend =
                                                                  boost::make_shared<boost::log::sinks::text_file_backend>(boost::log::keywords::file_name = log_path.string());

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


    template <typename number>
    repository<number>::derived_content_writer::~derived_content_writer()
	    {
        // remove logging objects
        boost::shared_ptr<boost::log::core> core = boost::log::core::get();

        core->remove_sink(this->log_sink);
	    }


    template <typename number>
    template <typename data_manager_type>
    void repository<number>::derived_content_writer::set_data_manager_taskfile(data_manager_type data)
	    {
        this->data_manager_taskfile = static_cast<void*>(data);  // will fail if data_manager_type not (static-)castable to void*
	    }


    template <typename number>
    template <typename data_manager_type>
    void repository<number>::derived_content_writer::get_data_manager_taskfile(data_manager_type* data)
	    {
        if(this->data_manager_taskfile == nullptr) throw runtime_exception(runtime_exception::REPOSITORY_ERROR, __CPP_TRANSPORT_REPO_DERIVED_WRITER_UNSETTASK);
        *data = static_cast<data_manager_type>(this->data_manager_taskfile);
	    }


    // OUTPUT_GROUP METHODS


    template<typename number>
    template<typename Payload>
    repository<number>::output_group<Payload>::output_group(const std::string &tn, const boost::filesystem::path& root, const boost::filesystem::path& path,
                                                            const boost::posix_time::ptime& ctime, bool lock, const std::list<std::string> &nt, const std::list<std::string> &tg)
      : task(tn), repo_root_path(root), data_root_path(path),
        created(ctime), locked(lock), notes(nt), tags(tg),
        payload()
      {
      }


    template <typename number>
    template <typename Payload>
    void repository<number>::output_group<Payload>::write(std::ostream& out) const
	    {
        out << __CPP_TRANSPORT_OUTPUT_GROUP;
        if(this->locked) out << ", " << __CPP_TRANSPORT_OUTPUT_GROUP_LOCKED;
        out << std::endl;
        out << "  " << __CPP_TRANSPORT_OUTPUT_GROUP_REPO_ROOT << " = " << this->repo_root_path << std::endl;
        out << "  " << __CPP_TRANSPORT_OUTPUT_GROUP_DATA_ROOT << " = " << this->data_root_path << std::endl;
        out << "  " << __CPP_TRANSPORT_OUTPUT_GROUP_CREATED << " = " << this->created << std::endl;

        unsigned int count = 0;

        for(std::list<std::string>::const_iterator t = this->notes.begin(); t != this->notes.end(); t++)
	        {
            out << "  " << __CPP_TRANSPORT_OUTPUT_GROUP_NOTE << " " << count << std::endl;
            out << "    " << *t << std::endl;
            count++;
	        }

        count = 0;
        out << "  " << __CPP_TRANSPORT_OUTPUT_GROUP_TAGS << ": ";
        for(std::list<std::string>::const_iterator t = this->tags.begin(); t != this->tags.end(); t++)
	        {
            if(count > 0) out << ", ";
            out << *t;
            count++;
	        }
        out << std::endl;

        this->payload.write(out);

        out << std::endl;
	    }


    template <typename number>
    template <typename Payload>
    bool repository<number>::output_group<Payload>::check_tags(std::list<std::string> match_tags) const
	    {
        // remove all this group's tags from the matching set.
        // If any remain after this process, then the match set isn't a subset of the group's tags.
        for(std::list<std::string>::const_iterator t = this->tags.begin(); t != this->tags.end(); t++)
	        {
            match_tags.remove(*t);
	        }

        return (!match_tags.empty());
	    }


    template <typename number>
    template <typename Payload>
    repository<number>::output_group<Payload>::output_group(serialization_reader* reader)
      : payload(reader)
      {
        assert(reader != nullptr);

        reader->read_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TASK_NAME, task);

        std::string repo_root;
        reader->read_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_REPO_ROOT, repo_root);
        repo_root_path = repo_root;

        std::string data_root;
        reader->read_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_DATA_ROOT, data_root);
        data_root_path = data_root;

        std::string ctime;
        reader->read_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_CREATED, ctime);
        this->created = boost::posix_time::time_from_string(ctime);

        reader->read_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_LOCKED, this->locked);

        unsigned int num_notes = reader->start_array(__CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTES);
        for(unsigned int i = 0; i < num_notes; i++)
          {
            reader->start_array_element();

            std::string note;
            reader->read_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTE, note);
            notes.push_back(note);

            reader->end_array_element();
          }
        reader->end_element(__CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTES);

        unsigned int num_tags = reader->start_array(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TAGS);
        for(unsigned int i = 0; i < num_tags; i++)
          {
            reader->start_array_element();

            std::string tag;
            reader->read_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TAG, tag);
            tags.push_back(tag);

            reader->end_array_element();
          }
        reader->end_element(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TAGS);
      }


    template <typename number>
    template <typename Payload>
    void repository<number>::output_group<Payload>::serialize(serialization_writer& writer) const
      {
        writer.write_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TASK_NAME, this->task);
        writer.write_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_REPO_ROOT, this->repo_root_path.string());
        writer.write_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_DATA_ROOT, this->data_root_path.string());
        writer.write_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_CREATED, boost::posix_time::to_simple_string(this->created));
        writer.write_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_LOCKED, this->locked);

        writer.start_array(__CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTES, this->notes.size() == 0);
        for(std::list<std::string>::const_iterator t = this->notes.begin(); t != this->notes.end(); t++)
          {
            writer.start_node("arrayelt");   // node name doesn't matter for an array
            writer.write_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTE, *t);
            writer.end_element("arrayelt");
          }
        writer.end_element(__CPP_TRANSPORT_NODE_OUTPUTGROUP_NOTES);

        writer.start_array(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TAGS, tags.size() == 0);
        for(std::list<std::string>::const_iterator t = tags.begin(); t != tags.end(); t++)
          {
            writer.start_node("arrayelt");   // node name doesn't matter for an array
            writer.write_value(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TAG, *t);
            writer.end_element("arrayelt");
          }
        writer.end_element(__CPP_TRANSPORT_NODE_OUTPUTGROUP_TAGS);

        this->payload.serialize(writer);
      }


    // output an output_group descriptor to a standard stream
    // notice obscure syntax to declare a templated member of an explicitly named namespace
    template <typename number, typename Payload>
    std::ostream& operator<<(std::ostream& out, const typename repository<number>::template output_group<Payload>& group)
      {
        group.write(out);
        return (out);
      }


    namespace
      {

        namespace output_group_helper
          {

            // used for sorting a list of output_groups into decreasing chronological order
            template <typename number, typename Payload>
            bool comparator(const typename repository<number>::template output_group<Payload>& A, const typename repository<number>::template output_group<Payload>& B)
              {
                return (A.get_creation_time() > B.get_creation_time());
              }

          }   // namespace output_group_helper

      }   // unnamed namespace


    template <typename number>
    repository<number>::integration_payload::integration_payload(serialization_reader* reader)
      : timing_data(reader)
      {
        assert(reader != nullptr);

        reader->read_value(__CPP_TRANSPORT_NODE_PAYLOAD_INTEGRATION_BACKEND, this->backend);

        std::string ctr_path;
        reader->read_value(__CPP_TRANSPORT_NODE_PAYLOAD_INTEGRATION_DATABASE, ctr_path);
        this->container = ctr_path;
      }

    template <typename number>
    void repository<number>::integration_payload::serialize(serialization_writer& writer) const
      {
        writer.write_value(__CPP_TRANSPORT_NODE_PAYLOAD_INTEGRATION_BACKEND, this->backend);
        writer.write_value(__CPP_TRANSPORT_NODE_PAYLOAD_INTEGRATION_DATABASE, this->container.string());

	      this->timing_data.serialize(writer);
      }


    template <typename number>
    void repository<number>::integration_payload::write(std::ostream& out) const
      {
        out << __CPP_TRANSPORT_PAYLOAD_INTEGRATION_BACKEND << " " << this->backend << std::endl;
        out << __CPP_TRANSPORT_PAYLOAD_INTEGRATION_DATA << " = " << this->container << std::endl;
      }


    template <typename number>
    repository<number>::output_payload::output_payload(serialization_reader* reader)
      {
        assert(reader != nullptr);

        unsigned int num_elements = reader->start_array(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_ARRAY);
        for(unsigned int i = 0; i < num_elements; i++)
          {
            this->content.push_back( typename repository<number>::derived_content(reader) );
          }
        reader->end_element(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_ARRAY);
      }


    template <typename number>
    void repository<number>::output_payload::serialize(serialization_writer& writer) const
      {
        writer.start_array(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_ARRAY, this->content.size() == 0);
        for(typename std::list< typename repository<number>::derived_content >::const_iterator t = this->content.begin(); t != this->content.end(); t++)
          {
            writer.start_node("arrayelt", false);    // node names are ignored in an array
            (*t).serialize(writer);
            writer.end_element("arrayelt");
          }
        writer.end_element(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_ARRAY);
      }


    template <typename number>
    void repository<number>::output_payload::write(std::ostream& out) const
      {
        for(typename std::list<derived_content>::const_iterator t = this->content.begin(); t != this->content.end(); t++)
          {
            out << __CPP_TRANSPORT_PAYLOAD_OUTPUT_CONTENT_PRODUCT << " = " << (*t).get_parent_product() << ", "
                << __CPP_TRANSPORT_PAYLOAD_OUTPUT_CONTENT_PATH    << " = " << (*t).get_filename() << std::endl;
          }
      }


    template <typename number>
    repository<number>::derived_content::derived_content(serialization_reader* reader)
      {
        assert(reader != nullptr);

        reader->read_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_PRODUCT_NAME, parent_product);

        std::string fnam_string;
        reader->read_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_FILENAME, fnam_string);
        filename = fnam_string;

        std::string ctime_string;
        reader->read_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_CREATED, ctime_string);
        created = boost::posix_time::time_from_string(ctime_string);

        unsigned int num_notes = reader->start_array(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTES);
        for(unsigned int i = 0; i < num_notes; i++)
          {
            reader->start_array_element();

            std::string note;
            reader->read_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTE, note);
            notes.push_back(note);

            reader->end_array_element();
          }
        reader->end_element(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTES);

        unsigned int num_tags = reader->start_array(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAGS);
        for(unsigned int i = 0; i < num_tags; i++)
          {
            reader->start_array_element();

            std::string tag;
            reader->read_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAG, tag);
            tags.push_back(tag);

            reader->end_array_element();
          }
        reader->end_element(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAGS);
      }


    template <typename number>
    void repository<number>::derived_content::serialize(serialization_writer& writer) const
      {
        writer.write_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_PRODUCT_NAME, this->parent_product);
        writer.write_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_FILENAME, this->filename.string());
        writer.write_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_CREATED, boost::posix_time::to_iso_string(this->created));

        writer.start_array(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTES, this->notes.size() == 0);
        for(std::list<std::string>::const_iterator t = this->notes.begin(); t != this->notes.end(); t++)
          {
            writer.start_node("arrayelt", false);   // node name is ignored in array element
            writer.write_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTE, *t);
            writer.end_element("arrayelt");
          }
        writer.end_element(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_NOTES);

        writer.start_array(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAGS, this->tags.size() == 0);
        for(std::list<std::string>::const_iterator t = this->tags.begin(); t != this->tags.end(); t++)
          {
            writer.start_node("arrayelt", false);   // node name is ignored in array element
            writer.write_value(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAG, *t);
            writer.end_element("arrayelt");
          }
        writer.end_element(__CPP_TRANSPORT_NODE_PAYLOAD_CONTENT_TAGS);
      }


	}   // namespace transport



#endif //__repository_H_
