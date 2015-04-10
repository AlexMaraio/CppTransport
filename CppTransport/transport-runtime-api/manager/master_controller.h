//
// Created by David Seery on 19/03/15.
// Copyright (c) 2015 University of Sussex. All rights reserved.
//


#ifndef __master_controller_H_
#define __master_controller_H_


#include <list>
#include <set>
#include <vector>
#include <memory>
#include <functional>

#include "transport-runtime-api/models/model.h"
#include "transport-runtime-api/manager/instance_manager.h"
#include "transport-runtime-api/tasks/task.h"
#include "transport-runtime-api/tasks/integration_tasks.h"
#include "transport-runtime-api/tasks/output_tasks.h"

#include "transport-runtime-api/manager/mpi_operations.h"

#include "transport-runtime-api/repository/json_repository.h"
#include "transport-runtime-api/data/data_manager.h"

#include "transport-runtime-api/manager/master_scheduler.h"
#include "transport-runtime-api/manager/work_journal.h"
#include "transport-runtime-api/manager/argument_cache.h"

#include "transport-runtime-api/scheduler/context.h"
#include "transport-runtime-api/scheduler/scheduler.h"
#include "transport-runtime-api/scheduler/work_queue.h"

#include "transport-runtime-api/utilities/formatter.h"

#include "transport-runtime-api/defaults.h"
#include "transport-runtime-api/messages.h"
#include "transport-runtime-api/exceptions.h"

#include "boost/mpi.hpp"
#include "boost/serialization/string.hpp"
#include "boost/timer/timer.hpp"
#include "boost/lexical_cast.hpp"



#define __CPP_TRANSPORT_SWITCH_REPO              "--repo"
#define __CPP_TRANSPORT_SWITCH_TAG               "--tag"
#define __CPP_TRANSPORT_SWITCH_CAPACITY          "--caches"
#define __CPP_TRANSPORT_SWITCH_BATCHER_CAPACITY  "--batch-cache"
#define __CPP_TRANSPORT_SWITCH_CACHE_CAPACITY    "--data-cache"
#define __CPP_TRANSPORT_SWITCH_ZETA_CAPACITY     "--zeta-cache"
#define __CPP_TRANSPORT_SWITCH_VERBOSE           "-v"
#define __CPP_TRANSPORT_SWITCH_GANTT_CHART       "--gantt-chart"

#define __CPP_TRANSPORT_VERB_TASK                "task"
#define __CPP_TRANSPORT_VERB_GET                 "get"

#define __CPP_TRANSPORT_NOUN_TASK                "task"
#define __CPP_TRANSPORT_NOUN_PACKAGE             "package"
#define __CPP_TRANSPORT_NOUN_PRODUCT             "product"
#define __CPP_TRANSPORT_NOUN_CONTENT             "content"



namespace transport
	{

    // MASTER FUNCTIONS

		template <typename number>
		class master_controller
			{

				// TYPES

		  public:

		    //! Error-reporting callback object
		    typedef std::function<void(const std::string&)> error_callback;

		    //! Warning callback object
		    typedef std::function<void(const std::string&)> warning_callback;

		    //! Message callback object
		    typedef std::function<void(const std::string&)> message_callback;

				//! Integration-task aggregation handler
				typedef std::function<void(MPI::data_ready_payload&,integration_metadata&)> integration_aggregator;

				//! Postintegration-task aggregation handler
				typedef std::function<void(MPI::data_ready_payload&,output_metadata&)> postintegration_aggregator;

				//! Output-task aggregation handler
				typedef std::function<bool(MPI::content_ready_payload&,output_metadata&)> derived_content_aggregator;

				//! Labels for types of job
		    typedef enum { job_task, job_get_package, job_get_task, job_get_product, job_get_content } job_type;

				//! Job descriptor class
		    class job_descriptor
			    {

		      public:

		        job_descriptor(job_type t, const std::string& n, const std::list<std::string>& tg, const std::string& o)
			        : type(t),
			          name(n),
			          tags(tg),
			          output(o)
			        {
			        }

		        job_descriptor(job_type t, const std::string& n, const std::list<std::string>& tg)
			        : type(t),
			          name(n),
			          tags(tg)
			        {
			        }


		        // INTERFACE

		      public:

		        job_type get_type() const { return(this->type); }

		        const std::string& get_name() const { return(this->name); }

		        const std::list<std::string>& get_tags() const { return(this->tags); }

		        const std::string& get_output() const { return(this->output); }


		        // INTERNAL DATA

		      private:

		        //! job type
		        job_type               type;

		        //! job name
		        std::string            name;

		        //! tags associated with job
		        std::list<std::string> tags;

		        //! output destination, if needed
		        std::string            output;

			    };


				// CONSTRUCTOR, DESTRUCTOR

		  public:

				//! construct a master controller object with no supplied repository
				//! (one has to be provided in the command line arguments later)
				master_controller(boost::mpi::environment& e, boost::mpi::communicator& w,
				                  error_callback err, warning_callback warn, message_callback msg,
				                  unsigned int bcp = __CPP_TRANSPORT_DEFAULT_BATCHER_STORAGE,
				                  unsigned int pcp = __CPP_TRANSPORT_DEFAULT_PIPE_STORAGE,
				                  unsigned int zcp = __CPP_TRANSPORT_DEFAULT_ZETA_CACHE_SIZE);

				//! construct a master controller object with a supplied repository
				master_controller(boost::mpi::environment& e, boost::mpi::communicator& w,
				                  json_repository<number>* r,
				                  error_callback err, warning_callback warn, message_callback msg,
				                  unsigned int bcp = __CPP_TRANSPORT_DEFAULT_BATCHER_STORAGE,
				                  unsigned int pcp = __CPP_TRANSPORT_DEFAULT_PIPE_STORAGE,
				                  unsigned int zcp = __CPP_TRANSPORT_DEFAULT_ZETA_CACHE_SIZE);

				//! destroy a master manager object
				~master_controller();


				// INTERFACE

		  public:

				//! interpret command-line arguments
				void process_arguments(int argc, char* argv[], const typename instance_manager<number>::model_finder& finder);
				
		    //! execute any queued tasks
		    void execute_tasks(void);

				//! expose arguments
				const argument_cache& get_arguments(void) { return(this->arg_cache); }


		    // MPI FUNCTIONS

		  protected:

		    //! Get worker number
		    unsigned int worker_number() { return(static_cast<unsigned int>(this->world.rank()-1)); }

		    //! Return MPI rank of this process
		    unsigned int get_rank(void) const { return(static_cast<unsigned int>(this->world.rank())); }

		    //! Map worker number to communicator rank
		    constexpr unsigned int worker_rank(unsigned int worker_number) const { return(worker_number+1); }

		    //! Map communicator rank to worker number
		    constexpr unsigned int worker_number(unsigned int worker_rank) const { return(worker_rank-1); }


		    // MASTER JOB HANDLING

		  protected:

		    //! Master node: Process a 'get' job
		    void process_get(const job_descriptor& job);

		    //! Master node: Process a 'task' job.
		    //! Some tasks are integrations, others process the numerical output from an integration to product
		    //! a derived data product (like fNL).
		    //! This function schedules workers to process the task.
		    void process_task(const job_descriptor& job);


				// WORKER HANDLING

		  protected:

		    //! Master node: Terminate all worker processes
		    void terminate_workers(void);

		    //! Master node: Collect data on workers
		    void initialize_workers(void);

				//! Master node: set up workers in preparation for a new task
				void set_up_workers(boost::log::sources::severity_logger< base_writer::log_severity_level >& log);

				//! Master node: close down workers after completion of a task
				void close_down_workers(boost::log::sources::severity_logger< base_writer::log_severity_level >& log);

				//! Master node: main loop: poll workers for events
				bool poll_workers(integration_aggregator& int_agg, postintegration_aggregator& post_agg, derived_content_aggregator& derived_agg,
				                  integration_metadata& int_metadata, output_metadata& out_metadata,
				                  boost::log::sources::severity_logger< base_writer::log_severity_level >& log,
				                  slave_work_event::event_type begin_label, slave_work_event::event_type end_label);

				//! Master node: generate new work assignments for workers
				void assign_work_to_workers(boost::log::sources::severity_logger< base_writer::log_severity_level >& log);

        //! Master node: print progress update if it is required
        void check_for_progress_update(void);


		    // MASTER INTEGRATION TASKS

		  protected:

		    //! Master node: Dispatch an integration task to the worker processes.
		    //! Makes a queue then invokes master_dispatch_integration_queue()
		    void dispatch_integration_task(integration_task_record<number>* rec, const std::list<std::string>& tags);

		    //! Master node: Dispatch an integration queue to the worker processes.
		    template <typename TaskObject>
		    void schedule_integration(integration_task_record<number>* rec, TaskObject* tk, const std::list<std::string>& tags,
		                              slave_work_event::event_type begin_label, slave_work_event::event_type end_label);

		    //! Master node: Pass new integration task to the workers
		    bool integration_task_to_workers(std::shared_ptr< integration_writer<number>>& writer,
                                         integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                         slave_work_event::event_type begin_label, slave_work_event::event_type end_label);

        //! Master node: debrief after task completion
        void debrief_integration(std::shared_ptr< integration_writer<number> >& writer,
                                 integration_metadata& i_metadata, boost::timer::cpu_timer& wallclock_timer);

		    //! Master node: respond to an aggregation request
		    void aggregate_integration(std::shared_ptr<integration_writer<number> > &writer, MPI::data_ready_payload& payload, integration_metadata &metadata);

		    //! Master node: update integration metadata after a worker has finished its tasks
		    void update_integration_metadata(MPI::finished_integration_payload& payload, integration_metadata& metadata);


		    // MASTER POSTINTEGRATION TASKS

		  protected:

		    //! Master node: Dispatch a postintegration task to the worker processes.
		    void dispatch_postintegration_task(postintegration_task_record<number>* rec, const std::list<std::string>& tags);

		    //! Master node: Dispatch a postintegration queue to the worker processes
		    template <typename TaskObject>
		    void schedule_postintegration(postintegration_task_record<number>* rec, TaskObject* tk, const std::list<std::string>& tags,
		                                  slave_work_event::event_type begin_label, slave_work_event::event_type end_label);

        //! Master node: Dispatch a paired postintegration queue to the worker processes
        template <typename TaskObject, typename ParentTaskObject>
        void schedule_paired_postintegration(postintegration_task_record<number>* rec, TaskObject* tk,
                                             ParentTaskObject* ptk, const std::list<std::string>& tags,
                                             slave_work_event::event_type begin_label,
                                             slave_work_event::event_type end_label);

        //! Master node: Pass new postintegration task to workers
		    bool postintegration_task_to_workers(std::shared_ptr< postintegration_writer<number> >& writer, const std::list<std::string>& tags,
                                             integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                             slave_work_event::event_type begin_label, slave_work_event::event_type end_label);

        //! Master node: Pass new paired postintegration task to workers
        bool paired_postintegration_task_to_workers(std::shared_ptr< integration_writer<number> >& i_writer, std::shared_ptr< postintegration_writer<number> >& p_writer,
                                                    const std::list<std::string>& tags,
                                                    integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                                    slave_work_event::event_type begin_label, slave_work_event::event_type end_label);

        //! Master node: debrief after task completion
        void debrief_postintegration(std::shared_ptr< postintegration_writer<number> >& writer,
                                     output_metadata& i_metadata, boost::timer::cpu_timer& wallclock_timer);

		    //! Master node: respond to an aggregation request
		    void aggregate_postprocess(std::shared_ptr< postintegration_writer<number> >& writer, MPI::data_ready_payload& payload, output_metadata& metadata);


		    // MASTER OUTPUT TASKS

		  protected:

		    //! Master node: Dispatch an output 'task' (ie., generation of derived data products) to the worker processes
		    void dispatch_output_task(output_task_record<number>* rec, const std::list<std::string>& tags);

		    //! Master node: Pass new output task to the workers
		    bool output_task_to_workers(std::shared_ptr <derived_content_writer<number>>& writer, const std::list<std::string>& tags,
                                    integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                    slave_work_event::event_type begin_label, slave_work_event::event_type end_label);

        //! Master node: debrief after task completion
        void debrief_output(std::shared_ptr< derived_content_writer<number> >& writer,
                            output_metadata& i_metadata, boost::timer::cpu_timer& wallclock_timer);

		    //! Master node: respond to a notification of new derived content
		    bool aggregate_content(std::shared_ptr< derived_content_writer<number> >& writer, MPI::content_ready_payload& payload, output_metadata& metadata);

		    //! Master node: update output metadata after a worker has finished its tasks
		    template <typename PayloadObject>
		    void update_output_metadata(PayloadObject& payload, output_metadata& metadata);


		    // INTERNAL DATA

		  protected:


		    // MPI ENVIRONMENT

		    //! BOOST::MPI environment
		    boost::mpi::environment& environment;

		    //! BOOST::MPI world communicator
		    boost::mpi::communicator& world;


		    // RUNTIME AGENTS

		    //! Repository manager instance
		    json_repository<number>* repo;

		    //! Data manager instance
		    data_manager<number>* data_mgr;

				//! Event journal
				work_journal journal;

				//! Argument cache
				argument_cache arg_cache;


		    // DATA AND STATE

				//! scheduler
				master_scheduler work_scheduler;

		    //! Queue of tasks to process
		    std::list<job_descriptor> job_queue;

		    //! Storage capacity per batcher
		    unsigned int batcher_capacity;

		    //! Data cache capacity per datapipe
		    unsigned int pipe_data_capacity;

		    //! Zeta cache capacity per datapipe
		    unsigned int pipe_zeta_capacity;


		    // ERROR CALLBACKS

		    //! error callback
		    error_callback error_handler;

		    //! warning callback
		    warning_callback warning_handler;

		    //! message callback
		    message_callback message_handler;

			};


    template <typename number>
    master_controller<number>::master_controller(boost::mpi::environment& e, boost::mpi::communicator& w,
                                           error_callback err, warning_callback warn, message_callback msg,
                                           unsigned int bcp, unsigned int pcp, unsigned int zcp)
	    : environment(e),
	      world(w),
	      repo(nullptr),
	      data_mgr(data_manager_factory<number>(bcp, pcp, zcp)),
	      journal(w.size()-1),
	      batcher_capacity(bcp),
	      pipe_data_capacity(pcp),
	      pipe_zeta_capacity(zcp),
	      error_handler(err),
	      warning_handler(warn),
	      message_handler(msg)
	    {
	    }


    template <typename number>
    master_controller<number>::master_controller(boost::mpi::environment& e, boost::mpi::communicator& w,
                                           json_repository<number>* r,
                                           error_callback err, warning_callback warn, message_callback msg,
                                           unsigned int bcp, unsigned int pcp, unsigned int zcp)
	    : environment(e),
	      world(w),
	      repo(r),
	      data_mgr(data_manager_factory<number>(bcp, pcp, zcp)),
	      journal(w.size()-1),
	      batcher_capacity(bcp),
	      pipe_data_capacity(pcp),
	      pipe_zeta_capacity(zcp),
	      error_handler(err),
	      warning_handler(warn),
	      message_handler(msg)
	    {
		    assert(repo != nullptr);
	    }

		
    template <typename number>
    master_controller<number>::~master_controller()
	    {
		    // delete repository instance, if it is set
        if(this->repo != nullptr)
	        {
            delete this->repo;
	        }
	    }
		
		
		template <typename number>
		void master_controller<number>::process_arguments(int argc, char* argv[], const typename instance_manager<number>::model_finder& finder)
			{
		    bool multiple_repo_warn = false;

		    std::list<std::string> tags;

		    for(unsigned int i = 1; i < argc; i++)
			    {
		        if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_REPO)
			        {
		            if(repo != nullptr)
			            {
		                ++i;
		                if(!multiple_repo_warn)
			                {
		                    this->warning_handler(__CPP_TRANSPORT_MULTIPLE_SET_REPO);
		                    multiple_repo_warn = true;
			                }
			            }
		            else if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_REPO);
		            else
			            {
		                ++i;
		                std::string repo_path = std::string(argv[i]);
		                try
			                {
		                    repo = repository_factory<number>(repo_path, repository<number>::access_type::readwrite,
		                                                      this->error_handler, this->warning_handler, this->message_handler);
		                    this->repo->set_model_finder(finder);
			                }
		                catch (runtime_exception& xe)
			                {
		                    if(xe.get_exception_code() == runtime_exception::REPO_NOT_FOUND)
			                    {
		                        this->error_handler(xe.what());
		                        repo = nullptr;
			                    }
		                    else
			                    {
		                        throw xe;
			                    }
			                }
			            }
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_TAG)
			        {
		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_TAG);
		            else            tags.push_back(std::string(argv[++i]));
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_CAPACITY)
			        {
		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_CAPACITY);
		            else
			            {
		                ++i;
		                std::string capacity_str(argv[i]);
		                int capacity = boost::lexical_cast<int>(capacity_str);
		                capacity = capacity * 1024*1024;
		                if(capacity > 0)
			                {
		                    this->batcher_capacity = this->pipe_data_capacity = static_cast<unsigned int>(capacity);
		                    this->data_mgr->set_batcher_capacity(this->batcher_capacity);
		                    this->data_mgr->set_data_capacity(this->pipe_data_capacity);
			                }
		                else
			                {
		                    std::ostringstream msg;
		                    msg << __CPP_TRANSPORT_EXPECTED_POSITIVE << " " << __CPP_TRANSPORT_SWITCH_CAPACITY;
		                    this->error_handler(msg.str());
			                }
			            }
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_BATCHER_CAPACITY)
			        {
		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_CAPACITY);
		            else
			            {
		                ++i;
		                std::string capacity_str(argv[i]);
		                int capacity = boost::lexical_cast<int>(capacity_str);
		                capacity = capacity * 1024*1024;
		                if(capacity > 0)
			                {
		                    this->batcher_capacity = static_cast<unsigned int>(capacity);
		                    this->data_mgr->set_batcher_capacity(this->batcher_capacity);
			                }
		                else
			                {
		                    std::ostringstream msg;
		                    msg << __CPP_TRANSPORT_EXPECTED_POSITIVE << " " << __CPP_TRANSPORT_SWITCH_BATCHER_CAPACITY;
		                    this->error_handler(msg.str());
			                }
			            }
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_CACHE_CAPACITY)
			        {
		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_CAPACITY);
		            else
			            {
		                ++i;
		                std::string capacity_str(argv[i]);
		                int capacity = boost::lexical_cast<int>(capacity_str);
		                capacity = capacity * 1024*1024;
		                if(capacity > 0)
			                {
		                    this->pipe_data_capacity = static_cast<unsigned int>(capacity);
		                    this->data_mgr->set_data_capacity(this->pipe_data_capacity);
			                }
		                else
			                {
		                    std::ostringstream msg;
		                    msg << __CPP_TRANSPORT_EXPECTED_POSITIVE << " " << __CPP_TRANSPORT_SWITCH_CACHE_CAPACITY;
		                    this->error_handler(msg.str());
			                }
			            }
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_ZETA_CAPACITY)
			        {
		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_CAPACITY);
		            else
			            {
		                ++i;
		                std::string capacity_str(argv[i]);
		                int capacity = boost::lexical_cast<int>(capacity_str);
		                capacity = capacity * 1024*1024;
		                if(capacity > 0)
			                {
		                    this->pipe_zeta_capacity = static_cast<unsigned int>(capacity);
		                    this->data_mgr->set_zeta_capacity(this->pipe_zeta_capacity);
			                }
		                else
			                {
		                    std::ostringstream msg;
		                    msg << __CPP_TRANSPORT_EXPECTED_POSITIVE << " " << __CPP_TRANSPORT_SWITCH_ZETA_CAPACITY;
		                    this->error_handler(msg.str());
			                }
			            }
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_GANTT_CHART)
			        {
				        if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_GANTT_FILENAME);
				        else
					        {
						        ++i;
						        this->arg_cache.set_gantt_chart(true);
						        this->arg_cache.set_gantt_filename(argv[i]);
					        }
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_SWITCH_VERBOSE)
			        {
				        this->arg_cache.set_verbose(true);
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_VERB_TASK)
			        {
		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_TASK_ID);
		            else
			            {
		                ++i;
		                job_queue.push_back(job_descriptor(job_task, argv[i], tags));
		                tags.clear();
			            }
			        }
		        else if(std::string(argv[i]) == __CPP_TRANSPORT_VERB_GET)
			        {
		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_GET_TYPE);
		            ++i;

		            job_type type;
		            if(std::string(argv[i]) == __CPP_TRANSPORT_NOUN_PACKAGE)      type = job_get_package;
		            else if(std::string(argv[i]) == __CPP_TRANSPORT_NOUN_TASK)    type = job_get_task;
		            else if(std::string(argv[i]) == __CPP_TRANSPORT_NOUN_PRODUCT) type = job_get_product;
		            else if(std::string(argv[i]) == __CPP_TRANSPORT_NOUN_CONTENT) type = job_get_content;
		            else
			            {
		                std::ostringstream msg;
		                msg << __CPP_TRANSPORT_UNKNOWN_GET_TYPE << " '" << argv[i] << "'";
		                this->error_handler(msg.str());
			            }

		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_GET_NAME);
		            ++i;
		            std::string name = argv[i];

		            if(i+1 >= argc) this->error_handler(__CPP_TRANSPORT_EXPECTED_GET_OUTPUT);
		            ++i;
		            std::string output = argv[i];

		            job_queue.push_back(job_descriptor(type, name, tags, output));
		            tags.clear();
			        }
		        else
			        {
		            std::ostringstream msg;
		            msg << __CPP_TRANSPORT_UNKNOWN_SWITCH << " '" << argv[i] << "'";
		            this->error_handler(msg.str());
			        }
			    }
			}
		

    template <typename number>
    void master_controller<number>::execute_tasks(void)
	    {
        if(!(this->get_rank() == 0)) throw runtime_exception(runtime_exception::MPI_ERROR, __CPP_TRANSPORT_EXEC_SLAVE);

        if(this->repo == nullptr)
	        {
            this->error_handler(__CPP_TRANSPORT_REPO_NONE);
	        }
        else
	        {
            boost::timer::cpu_timer timer;

		        // set up workers
		        this->initialize_workers();

		        unsigned int database_tasks = 0;
            for(typename std::list<job_descriptor>::const_iterator t = this->job_queue.begin(); t != this->job_queue.end(); t++)
	            {
                switch((*t).get_type())
	                {
                    case job_task:
	                    {
                        this->process_task(*t);
		                    database_tasks++;
                        break;
	                    }

                    case job_get_product:
                    case job_get_task:
                    case job_get_package:
                    case job_get_content:
	                    {
                        this->process_get(*t);
                        break;
	                    }

                    default:
	                    throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_UNKNOWN_JOB_TYPE);
	                }
	            }

		        timer.stop();
		        if(database_tasks > 0)
			        {
		            std::ostringstream msg;
		            msg << __CPP_TRANSPORT_PROCESSED_TASKS_A << " " << database_tasks << " ";
				        if(database_tasks > 1)
					        {
						        msg << __CPP_TRANSPORT_PROCESSED_TASKS_B_PLURAL;
					        }
				        else
					        {
						        msg << __CPP_TRANSPORT_PROCESSED_TASKS_B_SINGULAR;
					        }
		            msg << " " << __CPP_TRANSPORT_PROCESSED_TASKS_C << " " << format_time(timer.elapsed().wall);
				        msg << " | " << __CPP_TRANSPORT_PROCESSED_TASKS_D << " ";

		            boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
				        msg << boost::posix_time::to_simple_string(now);

				        this->message_handler(msg.str());
			        }
	        }

        // there is no more work, so ask all workers to shut down
        // and then exit ourselves
        this->terminate_workers();

		    if(this->arg_cache.get_gantt_chart()) this->journal.make_gantt_chart(this->arg_cache.get_gantt_filename());
	    }


    template <typename number>
    void master_controller<number>::process_get(const job_descriptor& job)
	    {
        try
	        {
            std::string document;

            switch(job.get_type())
	            {
                case job_get_package:
	                {
                    document = this->repo->export_JSON_package_record(job.get_name());
                    break;
	                }

                case job_get_task:
	                {
                    document = this->repo->export_JSON_task_record(job.get_name());
                    break;
	                }

                case job_get_product:
	                {
                    document = this->repo->export_JSON_product_record(job.get_name());
                    break;
	                }

                case job_get_content:
	                {
                    document = this->repo->export_JSON_content_record(job.get_name());
                    break;
	                }

                default:
	                assert(false);
	            }

            std::ofstream out;
            out.open(job.get_output().c_str());
            if(out.is_open() && !out.fail())
	            {
                out << document;
	            }
            else
	            {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_OPEN_OUTPUT_FAIL << " '" << job.get_output() << "'";
                throw runtime_exception(runtime_exception::RUNTIME_ERROR, msg.str());
	            }
            out.close();
	        }
        catch(runtime_exception& xe)
	        {
            this->error_handler(xe.what());
	        }
	    }


    template <typename number>
    void master_controller<number>::process_task(const job_descriptor& job)
	    {
        try
	        {
            // query a task record with the name we're looking for from the database
            std::unique_ptr< task_record<number> > record(this->repo->query_task(job.get_name()));

            switch(record->get_type())
	            {
                case task_record<number>::integration:
	                {
                    integration_task_record<number>* int_rec = dynamic_cast< integration_task_record<number>* >(record.get());

                    assert(int_rec != nullptr);
                    if(int_rec == nullptr) throw runtime_exception(runtime_exception::REPOSITORY_ERROR, __CPP_TRANSPORT_REPO_RECORD_CAST_FAILED);

                    this->dispatch_integration_task(int_rec, job.get_tags());
                    break;
	                }

                case task_record<number>::output:
	                {
                    output_task_record<number>* out_rec = dynamic_cast< output_task_record<number>* >(record.get());

                    assert(out_rec != nullptr);
                    if(out_rec == nullptr) throw runtime_exception(runtime_exception::REPOSITORY_ERROR, __CPP_TRANSPORT_REPO_RECORD_CAST_FAILED);

                    this->dispatch_output_task(out_rec, job.get_tags());
                    break;
	                }

                case task_record<number>::postintegration:
	                {
                    postintegration_task_record<number>* pint_rec = dynamic_cast< postintegration_task_record<number>* >(record.get());

                    assert(pint_rec != nullptr);
                    if(pint_rec == nullptr) throw runtime_exception(runtime_exception::REPOSITORY_ERROR, __CPP_TRANSPORT_REPO_RECORD_CAST_FAILED);

                    this->dispatch_postintegration_task(pint_rec, job.get_tags());
                    break;
	                }

                default:
	                {
                    assert(false);

                    std::ostringstream msg;
                    msg << __CPP_TRANSPORT_REPO_UNKNOWN_RECORD_TYPE << " '" << job.get_name() << "'";
                    throw runtime_exception(runtime_exception::RUNTIME_ERROR, msg.str());
	                }
	            }
	        }
        catch (runtime_exception xe)
	        {
            if(xe.get_exception_code() == runtime_exception::RECORD_NOT_FOUND)
	            {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_REPO_MISSING_RECORD << " '" << xe.what() << "'" << __CPP_TRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else if(xe.get_exception_code() == runtime_exception::MISSING_MODEL_INSTANCE
	            || xe.get_exception_code() == runtime_exception::REPOSITORY_BACKEND_ERROR)
	            {
                std::ostringstream msg;
                msg << xe.what() << " " << __CPP_TRANSPORT_REPO_FOR_TASK << " '" << job.get_name() << "'" << __CPP_TRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else throw xe;
	        }
	    }


		// INTEGRATION TASKS


    template <typename number>
    void master_controller<number>::dispatch_integration_task(integration_task_record<number>* rec, const std::list<std::string>& tags)
	    {
        assert(rec != nullptr);

        // can't process a task if there are no workers
        if(this->world.size() == 1) throw runtime_exception(runtime_exception::MPI_ERROR, __CPP_TRANSPORT_TOO_FEW_WORKERS);

        integration_task<number>* tk = rec->get_task();
        model<number>* m = rec->get_task()->get_model();

        twopf_task<number>* tka = nullptr;
        threepf_task<number>* tkb = nullptr;

        if((tka = dynamic_cast< twopf_task<number>* >(tk)) != nullptr)
	        {
		        this->work_scheduler.set_state_size(m->backend_twopf_state_size());
		        this->work_scheduler.prepare_queue(*tka);
            this->schedule_integration(rec, tka, tags, slave_work_event::begin_twopf_assignment, slave_work_event::end_twopf_assignment);
	        }
        else if((tkb = dynamic_cast< threepf_task<number>* >(tk)) != nullptr)
	        {
		        this->work_scheduler.set_state_size(m->backend_threepf_state_size());
            this->work_scheduler.prepare_queue(*tkb);
            this->schedule_integration(rec, tkb, tags, slave_work_event::begin_threepf_assignment, slave_work_event::end_threepf_assignment);
	        }
        else
	        {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_UNKNOWN_DERIVED_TASK << " '" << rec->get_name() << "'";
            throw runtime_exception(runtime_exception::REPOSITORY_ERROR, msg.str());
	        }
	    }


    template <typename number>
    template <typename TaskObject>
    void master_controller<number>::schedule_integration(integration_task_record<number>* rec, TaskObject* tk, const std::list<std::string>& tags,
                                                         slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
	    {
        assert(rec != nullptr);

        // create an output writer to commit the result of this integration to the repository.
        // like all writers, it aborts (ie. executes a rollback if needed) when it goes out of scope unless
        // it is explicitly committed
        std::shared_ptr< integration_writer<number> > writer = this->repo->new_integration_task_content(rec, tags, this->get_rank());

        // initialize the writer
        this->data_mgr->initialize_writer(writer);

        // write the various tables needed in the database
        this->data_mgr->create_tables(writer, tk);

        // set up aggregators
        integration_aggregator     i_agg = std::bind(&master_controller<number>::aggregate_integration, this, writer, std::placeholders::_1, std::placeholders::_2);
        postintegration_aggregator p_agg;
        derived_content_aggregator d_agg;

        // write log header
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ NEW INTEGRATION TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << std::endl;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << *tk;

        // instruct workers to carry out the calculation
        // this call returns when all workers have signalled that their work is done
        bool success = this->integration_task_to_workers(writer, i_agg, p_agg, d_agg, begin_label, end_label);

        journal_instrument instrument(this->journal, master_work_event::database_begin, master_work_event::database_end);

        // close the writer
        this->data_mgr->close_writer(writer);

        // commit output if successful
        if(success) writer->commit();
	    }


    template <typename number>
    bool master_controller<number>::integration_task_to_workers(std::shared_ptr <integration_writer<number>>& writer,
                                                                integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                                                slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
	    {
        assert(this->repo != nullptr);

        // set up a timer to keep track of the total wallclock time used in this integration
        boost::timer::cpu_timer wallclock_timer;

        // aggregate integration times reported by worker processes
        integration_metadata i_metadata;
        output_metadata      o_metadata;  // unused

        // get paths the workers will need
        boost::filesystem::path tempdir_path = writer->get_abs_tempdir_path();
        boost::filesystem::path logdir_path  = writer->get_abs_logdir_path();

        {
	        // journal_instrument will log time spent doing MPI when it goes out of scope
	        journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

	        std::vector<boost::mpi::request> requests(this->world.size()-1);
	        MPI::new_integration_payload payload(writer->get_record()->get_name(), tempdir_path, logdir_path);

	        for(unsigned int i = 0; i < this->world.size()-1; i++)
		        {
	            requests[i] = this->world.isend(this->worker_rank(i), MPI::NEW_INTEGRATION, payload);
		        }

	        // wait for all messages to be received
	        boost::mpi::wait_all(requests.begin(), requests.end());
	        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ All workers received NEW_INTEGRATION instruction";
        }

		    bool success = this->poll_workers(i_agg, p_agg, d_agg, i_metadata, o_metadata, writer->get_log(), begin_label, end_label);

        // push task metadata we have collected to the writer
        writer->set_metadata(i_metadata);
        wallclock_timer.stop();
        this->debrief_integration(writer, i_metadata, wallclock_timer);

		    return(success);
	    }


    template <typename number>
    void master_controller<number>::debrief_integration(std::shared_ptr< integration_writer<number> >& writer,
                                                        integration_metadata& i_metadata, boost::timer::cpu_timer& wallclock_timer)
      {
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ TASK COMPLETE (at " << boost::posix_time::to_simple_string(now) << "): FINAL USAGE STATISTICS";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total wallclock time for task '" << writer->get_record()->get_name() << "' " << format_time(wallclock_timer.elapsed().wall);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total wallclock time required by worker processes = " << format_time(i_metadata.total_wallclock_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total aggregation time required by master process = " << format_time(i_metadata.total_aggregation_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ AGGREGATE CACHE STATISTICS";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Workers processed " << i_metadata.total_configurations << " individual integrations";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total integration time    = " << format_time(i_metadata.total_integration_time) << " | global mean integration time = " << format_time(i_metadata.total_integration_time/ i_metadata.total_configurations);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Min mean integration time = " << format_time(i_metadata.min_mean_integration_time) << " | global min integration time = " << format_time(i_metadata.global_min_integration_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Max mean integration time = " << format_time(i_metadata.max_mean_integration_time) << " | global max integration time = " << format_time(i_metadata.global_max_integration_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total batching time       = " << format_time(i_metadata.total_batching_time) << " | global mean batching time = " << format_time(i_metadata.total_batching_time/ i_metadata.total_configurations);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Min mean batching time    = " << format_time(i_metadata.min_mean_batching_time) << " | global min batching time = " << format_time(i_metadata.global_min_batching_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Max mean batching time    = " << format_time(i_metadata.max_mean_batching_time) << " | global max batching time = " << format_time(i_metadata.global_max_batching_time);
      }


    template <typename number>
    void master_controller<number>::aggregate_integration(std::shared_ptr<integration_writer<number> > &writer,
                                                          MPI::data_ready_payload& payload, integration_metadata &metadata)
	    {
        journal_instrument instrument(this->journal, master_work_event::aggregate_begin, master_work_event::aggregate_end);

        // set up a timer to measure how long we spend aggregating
        boost::timer::cpu_timer aggregate_timer;

        writer->aggregate(payload.get_container_path());

        aggregate_timer.stop();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ Aggregated temporary container '" << payload.get_container_path() << "' in time " << format_time(aggregate_timer.elapsed().wall);
        metadata.total_aggregation_time += aggregate_timer.elapsed().wall;

        // inform scheduler of a new aggregation
        this->work_scheduler.report_aggregation(aggregate_timer.elapsed().wall);

        // remove temporary container
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ Deleting temporary container '" << payload.get_container_path() << "'";
        if(!boost::filesystem::remove(payload.get_container_path()))
	        {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_DATACTR_REMOVE_TEMP << " '" << payload.get_container_path() << "'";
            this->error_handler(msg.str());
	        }
	    }


    template <typename number>
    void master_controller<number>::update_integration_metadata(MPI::finished_integration_payload& payload, integration_metadata& metadata)
	    {
        metadata.total_wallclock_time += payload.get_wallclock_time();

        metadata.total_integration_time += payload.get_integration_time();
        boost::timer::nanosecond_type mean_integration_time = payload.get_integration_time() / payload.get_num_integrations();
        if(metadata.max_mean_integration_time == 0 || mean_integration_time > metadata.max_mean_integration_time) metadata.max_mean_integration_time = mean_integration_time;
        if(metadata.min_mean_integration_time == 0 || mean_integration_time < metadata.min_mean_integration_time) metadata.min_mean_integration_time = mean_integration_time;

        if(metadata.global_max_integration_time == 0 || payload.get_max_integration_time() > metadata.global_max_integration_time) metadata.global_max_integration_time = payload.get_max_integration_time();
        if(metadata.global_min_integration_time == 0 || payload.get_min_integration_time() < metadata.global_min_integration_time) metadata.global_min_integration_time = payload.get_min_integration_time();

        metadata.total_batching_time += payload.get_batching_time();
        boost::timer::nanosecond_type mean_batching_time = payload.get_batching_time() / payload.get_num_integrations();
        if(metadata.max_mean_batching_time == 0 || mean_batching_time > metadata.max_mean_batching_time) metadata.max_mean_batching_time = mean_batching_time;
        if(metadata.min_mean_batching_time == 0 || mean_batching_time < metadata.min_mean_batching_time) metadata.min_mean_batching_time = mean_batching_time;

        if(metadata.global_max_batching_time == 0 || payload.get_max_batching_time() > metadata.global_max_batching_time) metadata.global_max_batching_time = payload.get_max_batching_time();
        if(metadata.global_min_batching_time == 0 || payload.get_min_batching_time() < metadata.global_min_batching_time) metadata.global_min_batching_time = payload.get_min_batching_time();

        metadata.total_configurations += payload.get_num_integrations();
	    }


		// OUTPUT TASKS


    template <typename number>
    void master_controller<number>::dispatch_output_task(output_task_record<number>* rec, const std::list<std::string>& tags)
	    {
		    assert(rec != nullptr);

        // can't process a task if there are no workers
        if(this->world.size() <= 1) throw runtime_exception(runtime_exception::MPI_ERROR, __CPP_TRANSPORT_TOO_FEW_WORKERS);

        output_task<number>* tk = rec->get_task();

		    this->work_scheduler.set_state_size(sizeof(number));
		    this->work_scheduler.prepare_queue(*tk);

        // set up an derived_content_writer object to coordinate logging, output destination and commits into the repository.
        // like all writers, it aborts (ie. executes a rollback if needed) when it goes out of scope unless
        // it is explicitly committed
        std::shared_ptr< derived_content_writer<number> > writer = this->repo->new_output_task_content(rec, tags, this->get_rank());

        // set up the writer for us
        this->data_mgr->initialize_writer(writer);

        // set up aggregators
        integration_aggregator     i_agg;
        postintegration_aggregator p_agg;
        derived_content_aggregator d_agg = std::bind(&master_controller<number>::aggregate_content, this, writer, std::placeholders::_1, std::placeholders::_2);

        // write log header
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ NEW OUTPUT TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << std::endl;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << *tk;

        // instruct workers to carry out their tasks
        bool success = this->output_task_to_workers(writer, tags, i_agg, p_agg, d_agg, slave_work_event::begin_output_assignment, slave_work_event::end_output_assignment);

        journal_instrument instrument(this->journal, master_work_event::database_begin, master_work_event::database_end);

        // close the writer
        this->data_mgr->close_writer(writer);

        // commit output to the database if successful
        if(success) writer->commit();
	    }


    template <typename number>
    bool master_controller<number>::output_task_to_workers(std::shared_ptr< derived_content_writer<number> >& writer, const std::list<std::string>& tags,
                                                           integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                                           slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
	    {
        assert(this->repo != nullptr);

        // set up a timer to keep track of the total wallclock time used in this task
        boost::timer::cpu_timer wallclock_timer;

        // aggregate cache information
        integration_metadata i_metadata;  // unused
        output_metadata      o_metadata;

        // get paths the workers will need
        boost::filesystem::path tempdir_path = writer->get_abs_tempdir_path();
        boost::filesystem::path logdir_path  = writer->get_abs_logdir_path();

        {
	        // journal_instrument will log time spent doing MPI when it goes out of scope
	        journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

	        std::vector<boost::mpi::request> requests(this->world.size()-1);
	        MPI::new_derived_content_payload payload(writer->get_record()->get_name(), tempdir_path, logdir_path, tags);

	        for(unsigned int i = 0; i < this->world.size()-1; i++)
		        {
	            requests[i] = this->world.isend(this->worker_rank(i), MPI::NEW_DERIVED_CONTENT, payload);
		        }

	        // wait for all messages to be received
	        boost::mpi::wait_all(requests.begin(), requests.end());
	        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ All workers received NEW_DERIVED_CONTENT instruction";
        }

		    bool success = this->poll_workers(i_agg, p_agg, d_agg, i_metadata, o_metadata, writer->get_log(), begin_label, end_label);

        writer->set_metadata(o_metadata);
        wallclock_timer.stop();
        this->debrief_output(writer, o_metadata, wallclock_timer);

        return(success);
	    }


    template <typename number>
    void master_controller<number>::debrief_output(std::shared_ptr< derived_content_writer<number> >& writer,
                                                   output_metadata& o_metadata, boost::timer::cpu_timer& wallclock_timer)
      {
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ TASK COMPLETE (at " << boost::posix_time::to_simple_string(now) << "): FINAL USAGE STATISTICS";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total wallclock time for task '" << writer->get_record()->get_name() << "' " << format_time(wallclock_timer.elapsed().wall);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ AGGREGATE CACHE STATISTICS";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total work time required by worker processes      = " << format_time(o_metadata.work_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total aggregation time required by master process = " << format_time(o_metadata.aggregation_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total time spent reading database                 = " << format_time(o_metadata.db_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total time-configuration cache hits               = " << o_metadata.time_config_hits << ", unloads = " << o_metadata.time_config_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total twopf k-config cache hits                   = " << o_metadata.twopf_kconfig_hits << ", unloads = " << o_metadata.twopf_kconfig_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total threepf k-config cache hits                 = " << o_metadata.threepf_kconfig_hits << ", unloads = " << o_metadata.threepf_kconfig_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total data cache hits                             = " << o_metadata.data_hits << ", unloads = " << o_metadata.data_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total zeta cache hits                             = " << o_metadata.zeta_hits << ", unloads = " << o_metadata.zeta_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total time config cache evictions                 = " << format_time(o_metadata.time_config_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total twopf k-config cache evictions              = " << format_time(o_metadata.twopf_kconfig_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total threepf k-config cache evictions            = " << format_time(o_metadata.threepf_kconfig_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total data cache evictions                        = " << format_time(o_metadata.data_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total zeta cache evictions                        = " << format_time(o_metadata.zeta_evictions);
      }


    template <typename number>
    bool master_controller<number>::aggregate_content(std::shared_ptr< derived_content_writer<number> >& writer,
                                                      MPI::content_ready_payload& payload, output_metadata& metadata)
	    {
        journal_instrument instrument(this->journal, master_work_event::aggregate_begin, master_work_event::aggregate_end);

        // set up a timer to measure how long we spend aggregating
        boost::timer::cpu_timer aggregate_timer;

        bool success = writer->aggregate(payload.get_product_name());

        aggregate_timer.stop();
        metadata.aggregation_time += aggregate_timer.elapsed().wall;

		    // inform scheduler of a new aggregation
		    this->work_scheduler.report_aggregation(aggregate_timer.elapsed().wall);

        return (success);
	    }


    template <typename number>
    template <typename PayloadObject>
    void master_controller<number>::update_output_metadata(PayloadObject& payload, output_metadata& metadata)
	    {
        metadata.work_time                 += payload.get_cpu_time();
        metadata.db_time                   += payload.get_database_time();
        metadata.time_config_hits          += payload.get_time_config_hits();
        metadata.time_config_unloads       += payload.get_time_config_unloads();
        metadata.time_config_evictions     += payload.get_time_config_evictions();
        metadata.twopf_kconfig_hits        += payload.get_twopf_kconfig_hits();
        metadata.twopf_kconfig_unloads     += payload.get_twopf_kconfig_unloads();
        metadata.twopf_kconfig_evictions   += payload.get_twopf_kconfig_evictions();
        metadata.threepf_kconfig_hits      += payload.get_threepf_kconfig_hits();
        metadata.threepf_kconfig_unloads   += payload.get_threepf_kconfig_unloads();
        metadata.threepf_kconfig_evictions += payload.get_threepf_kconfig_evictions();
        metadata.data_hits                 += payload.get_data_hits();
        metadata.data_unloads              += payload.get_data_unloads();
        metadata.data_evictions            += payload.get_data_evictions();
        metadata.zeta_hits                 += payload.get_zeta_hits();
        metadata.zeta_unloads              += payload.get_zeta_unloads();
        metadata.zeta_evictions            += payload.get_zeta_evictions();
	    }


		// POSTINTEGRATION TASKS


    template <typename number>
    void master_controller<number>::dispatch_postintegration_task(postintegration_task_record<number>* rec, const std::list<std::string>& tags)
	    {
        assert(rec != nullptr);

        // can't process a task if there are no workers
        if(this->world.size() <= 1) throw runtime_exception(runtime_exception::MPI_ERROR, __CPP_TRANSPORT_TOO_FEW_WORKERS);

        postintegration_task<number>* tk = rec->get_task();

        zeta_twopf_task<number>*   z2pf = nullptr;
        zeta_threepf_task<number>* z3pf = nullptr;
        fNL_task<number>*          zfNL = nullptr;

        if((z2pf = dynamic_cast< zeta_twopf_task<number>* >(tk)) != nullptr)
	        {
            twopf_task<number>* ptk = dynamic_cast<twopf_task<number>*>(z2pf->get_parent_task());

            assert(ptk != nullptr);
            if(ptk == nullptr)
	            {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_EXPECTED_TWOPF_TASK << " '" << z2pf->get_parent_task()->get_name() << "'";
                throw runtime_exception(runtime_exception::REPOSITORY_ERROR, msg.str());
	            }

            // is this 2pf task paired?
            if(z2pf->is_paired())
              {
                model<number>* m = ptk->get_model();
                this->work_scheduler.set_state_size(m->backend_twopf_state_size());
                this->work_scheduler.prepare_queue(*ptk);
                this->schedule_paired_postintegration(rec, z2pf, ptk, tags, slave_work_event::begin_twopf_assignment, slave_work_event::end_twopf_assignment);
              }
            else
              {
                this->work_scheduler.set_state_size(sizeof(number));
                this->work_scheduler.prepare_queue(*ptk);
                this->schedule_postintegration(rec, z2pf, tags, slave_work_event::begin_zeta_twopf_assignment, slave_work_event::end_zeta_twopf_assignment);
              }
	        }
        else if((z3pf = dynamic_cast< zeta_threepf_task<number>* >(tk)) != nullptr)
	        {
            threepf_task<number>* ptk = dynamic_cast<threepf_task<number>*>(z3pf->get_parent_task());

            assert(ptk != nullptr);
            if(ptk == nullptr)
	            {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_EXPECTED_THREEPF_TASK << " '" << z3pf->get_parent_task()->get_name() << "'";
                throw runtime_exception(runtime_exception::REPOSITORY_ERROR, msg.str());
	            }

            // is this 3pf task apired?
            if(z3pf->is_paired())
              {
                model<number>* m = ptk->get_model();
                this->work_scheduler.set_state_size(m->backend_threepf_state_size());
                this->work_scheduler.prepare_queue(*ptk);
                this->schedule_paired_postintegration(rec, z3pf, ptk, tags, slave_work_event::begin_threepf_assignment, slave_work_event::end_threepf_assignment);
              }
            else
              {
                this->work_scheduler.set_state_size(sizeof(number));
                this->work_scheduler.prepare_queue(*ptk);
                this->schedule_postintegration(rec, z3pf, tags, slave_work_event::begin_zeta_threepf_assignment, slave_work_event::end_zeta_threepf_assignment);
              }
	        }
        else if((zfNL = dynamic_cast< fNL_task<number>* >(tk)) != nullptr)
	        {
            zeta_threepf_task<number>* ptk = dynamic_cast<zeta_threepf_task<number>*>(zfNL->get_parent_task());

            assert(ptk != nullptr);
            if(ptk == nullptr)
	            {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_EXPECTED_THREEPF_TASK << " '" << zfNL->get_parent_task()->get_name() << "'";
                throw runtime_exception(runtime_exception::REPOSITORY_ERROR, msg.str());
	            }

		        this->work_scheduler.set_state_size(sizeof(number));
		        this->work_scheduler.prepare_queue(*ptk);
            this->schedule_postintegration(rec, zfNL, tags, slave_work_event::begin_fNL_assignment, slave_work_event::end_fNL_assignment);
	        }
        else
	        {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_UNKNOWN_DERIVED_TASK << " '" << rec->get_name() << "'";
            throw runtime_exception(runtime_exception::REPOSITORY_ERROR, msg.str());
	        }
	    }


    template <typename number>
    template <typename TaskObject>
    void master_controller<number>::schedule_postintegration(postintegration_task_record<number>* rec, TaskObject* tk, const std::list<std::string>& tags,
                                                             slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
	    {
        assert(rec != nullptr);

        // create an output writer to commit our results into the repository
        // like all writers, it aborts (ie. executes a rollback if needed) when it goes out of scope unless
        // it is explicitly committed
        std::shared_ptr< postintegration_writer<number> > writer = this->repo->new_postintegration_task_content(rec, tags, this->get_rank());

        // initialize the writer
        this->data_mgr->initialize_writer(writer);

        // create new tables needed in the database
        this->data_mgr->create_tables(writer, tk);

        // set up aggregators
        integration_aggregator     i_agg;
        postintegration_aggregator p_agg = std::bind(&master_controller<number>::aggregate_postprocess, this, writer, std::placeholders::_1, std::placeholders::_2);
        derived_content_aggregator d_agg;

        // write log header
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ NEW POSTINTEGRATION TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << std::endl;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << *tk;

        // instruct workers to carry out the calculation
        bool success = this->postintegration_task_to_workers(writer, tags, i_agg, p_agg, d_agg, begin_label, end_label);

        journal_instrument instrument(this->journal, master_work_event::database_begin, master_work_event::database_end);

        // close the writer
        this->data_mgr->close_writer(writer);

        // commit output if successful
        if(success) writer->commit();
	    }


    template <typename number>
    template <typename TaskObject, typename ParentTaskObject>
    void master_controller<number>::schedule_paired_postintegration(postintegration_task_record<number>* rec, TaskObject* tk, ParentTaskObject* ptk, const std::list<std::string>& tags,
                                                                    slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
      {
        assert(rec != nullptr);

        std::unique_ptr< task_record<number> > pre_prec(this->repo->query_task(ptk->get_name()));
        integration_task_record<number>* prec = dynamic_cast< integration_task_record<number>* >(pre_prec.get());

        assert(prec != nullptr);
        if(prec == nullptr) throw runtime_exception(runtime_exception::REPOSITORY_ERROR, __CPP_TRANSPORT_REPO_RECORD_CAST_FAILED);

        // create an output writer for the postintegration task
        std::shared_ptr< postintegration_writer<number> > p_writer = this->repo->new_postintegration_task_content(rec, tags, this->get_rank());
        this->data_mgr->initialize_writer(p_writer);
        this->data_mgr->create_tables(p_writer, tk);

        // create an output writer for the integration task; use suffix option to add "-paired" to distinguish the different output groups
        std::shared_ptr<integration_writer<number> > i_writer = this->repo->new_integration_task_content(prec, tags, this->get_rank(), "-paired");
        this->data_mgr->initialize_writer(i_writer);
        this->data_mgr->create_tables(i_writer, ptk);

        // set up aggregators
        integration_aggregator     i_agg = std::bind(&master_controller<number>::aggregate_integration, this, i_writer, std::placeholders::_1, std::placeholders::_2);
        postintegration_aggregator p_agg = std::bind(&master_controller<number>::aggregate_postprocess, this, p_writer, std::placeholders::_1, std::placeholders::_2);
        derived_content_aggregator d_agg;

        // write log header
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(i_writer->get_log(), base_writer::normal) << "++ NEW PAIRED POSTINTEGRATION TASKS '" << tk->get_name() << "' & '" << ptk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << std::endl;
        BOOST_LOG_SEV(i_writer->get_log(), base_writer::normal) << *ptk;

        // instruct workers to carry out the calculation
        bool success = this->paired_postintegration_task_to_workers(i_writer, p_writer, tags, i_agg, p_agg, d_agg, begin_label, end_label);

        journal_instrument instrument(this->journal, master_work_event::database_begin, master_work_event::database_end);

        // close both writers
        this->data_mgr->close_writer(i_writer);
        this->data_mgr->close_writer(p_writer);

        // commit output if successful
        if(success)
          {
            i_writer->commit();
            p_writer->commit();
          }
      }


    template <typename number>
    bool master_controller<number>::postintegration_task_to_workers(std::shared_ptr< postintegration_writer<number> >& writer, const std::list<std::string>& tags,
                                                                    integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                                                    slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
	    {
        assert(this->repo != nullptr);

        // set up a timer to keep track of the total wallclock time used
        boost::timer::cpu_timer wallclock_timer;

        // aggregate cache information
        integration_metadata i_metadata;  // unused
        output_metadata      o_metadata;

        // get paths the workers will need
        boost::filesystem::path tempdir_path = writer->get_abs_tempdir_path();
        boost::filesystem::path logdir_path  = writer->get_abs_logdir_path();

        {
	        // journal_instrument will log time spent doing MPI when it goes out of scope
	        journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

	        std::vector<boost::mpi::request> requests(this->world.size()-1);
	        MPI::new_postintegration_payload payload(writer->get_record()->get_name(), tempdir_path, logdir_path, tags);

	        for(unsigned int i = 0; i < this->world.size()-1; i++)
		        {
	            requests[i] = this->world.isend(this->worker_rank(i), MPI::NEW_POSTINTEGRATION, payload);
		        }

	        // wait for all messages to be received
	        boost::mpi::wait_all(requests.begin(), requests.end());
	        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ All workers received NEW_POSTINTEGRATION instruction";
        }

		    bool success = this->poll_workers(i_agg, p_agg, d_agg, i_metadata, o_metadata, writer->get_log(), begin_label, end_label);

        writer->set_metadata(o_metadata);
        wallclock_timer.stop();
        this->debrief_postintegration(writer, o_metadata, wallclock_timer);

        return(success);
	    }


    template <typename number>
    bool master_controller<number>::paired_postintegration_task_to_workers(std::shared_ptr< integration_writer<number> >& i_writer,
                                                                           std::shared_ptr< postintegration_writer<number> >& p_writer,
                                                                           const std::list<std::string>& tags,
                                                                           integration_aggregator& i_agg, postintegration_aggregator& p_agg, derived_content_aggregator& d_agg,
                                                                           slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
      {
        assert(this->repo != nullptr);

        // set up a timer to keep track of the total wallclock time used
        boost::timer::cpu_timer wallclock_timer;

        // aggregate cache information
        integration_metadata i_metadata;
        output_metadata      o_metadata;

        // get paths needed by different batchers on each worker
        boost::filesystem::path i_tempdir_path = i_writer->get_abs_tempdir_path();
        boost::filesystem::path i_logdir_path  = i_writer->get_abs_logdir_path();

        boost::filesystem::path p_tempdir_path = p_writer->get_abs_tempdir_path();
        boost::filesystem::path p_logdir_path  = p_writer->get_abs_logdir_path();

        {
          // journal instrument will log time spent doing MPI when it goes out of scope
          journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

          std::vector<boost::mpi::request> requests(this->world.size()-1);
          MPI::new_postintegration_payload payload(p_writer->get_record()->get_name(), p_tempdir_path, p_logdir_path, tags, i_tempdir_path, i_logdir_path);

          for(unsigned int i = 0; i < this->world.size()-1; i++)
            {
              requests[i] = this->world.isend(this->worker_rank(i), MPI::NEW_POSTINTEGRATION, payload);
            }

          // wait for all messages to be received
          boost::mpi::wait_all(requests.begin(), requests.end());
          BOOST_LOG_SEV(i_writer->get_log(), base_writer::normal) << "++ All workers received NEW_POSTINTEGRATION instruction";
        }

        bool success = this->poll_workers(i_agg, p_agg, d_agg, i_metadata, o_metadata, i_writer->get_log(), begin_label, end_label);

        i_writer->set_metadata(i_metadata);
        p_writer->set_metadata(o_metadata);

        wallclock_timer.stop();
        this->debrief_integration(i_writer, i_metadata, wallclock_timer);
//        this->debrief_postintegration(p_writer, o_metadata, wallclock_timer);

        return(success);
      }


    template <typename number>
    void master_controller<number>::debrief_postintegration(std::shared_ptr< postintegration_writer<number> >& writer,
    output_metadata& o_metadata, boost::timer::cpu_timer& wallclock_timer)
      {
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ TASK COMPLETE (at " << boost::posix_time::to_simple_string(now) << "): FINAL USAGE STATISTICS";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total wallclock time for task '" << writer->get_record()->get_name() << "' " << format_time(wallclock_timer.elapsed().wall);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ AGGREGATE CACHE STATISTICS";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total work time required by worker processes      = " << format_time(o_metadata.work_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total aggregation time required by master process = " << format_time(o_metadata.aggregation_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total time spent reading database                 = " << format_time(o_metadata.db_time);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total time-configuration cache hits               = " << o_metadata.time_config_hits << ", unloads = " << o_metadata.time_config_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total twopf k-config cache hits                   = " << o_metadata.twopf_kconfig_hits << ", unloads = " << o_metadata.twopf_kconfig_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total threepf k-config cache hits                 = " << o_metadata.threepf_kconfig_hits << ", unloads = " << o_metadata.threepf_kconfig_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total data cache hits                             = " << o_metadata.data_hits << ", unloads = " << o_metadata.data_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total zeta cache hits                             = " << o_metadata.zeta_hits << ", unloads = " << o_metadata.zeta_unloads;
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "";
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total time config cache evictions                 = " << format_time(o_metadata.time_config_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total twopf k-config cache evictions              = " << format_time(o_metadata.twopf_kconfig_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total threepf k-config cache evictions            = " << format_time(o_metadata.threepf_kconfig_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total data cache evictions                        = " << format_time(o_metadata.data_evictions);
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++   Total zeta cache evictions                        = " << format_time(o_metadata.zeta_evictions);
      }


    template <typename number>
    void master_controller<number>::aggregate_postprocess(std::shared_ptr< postintegration_writer<number> >& writer,
                                                          MPI::data_ready_payload& payload, output_metadata& metadata)
	    {
		    journal_instrument instrument(this->journal, master_work_event::aggregate_begin, master_work_event::aggregate_end);

        // set up a timer to measure how long we spend batching
        boost::timer::cpu_timer aggregate_timer;

        writer->aggregate(payload.get_container_path());

        aggregate_timer.stop();
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ Aggregated temporary container '" << payload.get_container_path() << "' in time " << format_time(aggregate_timer.elapsed().wall);
        metadata.aggregation_time += aggregate_timer.elapsed().wall;

        // inform scheduler of a new aggregation
        this->work_scheduler.report_aggregation(aggregate_timer.elapsed().wall);

        // remove temporary container
        BOOST_LOG_SEV(writer->get_log(), base_writer::normal) << "++ Deleting temporary container '" << payload.get_container_path() << "'";
        if(!boost::filesystem::remove(payload.get_container_path()))
	        {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_DATACTR_REMOVE_TEMP << " '" << payload.get_container_path() << "'";
            this->error_handler(msg.str());
	        }
	    }


		// WORKER HANDLING


    template <typename number>
    void master_controller<number>::initialize_workers(void)
	    {
        // set up instrument to journal the MPI communication if needed
        journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

        std::vector<boost::mpi::request> requests(this->world.size()-1);

        // we require this->repo not to be null
        assert(this->repo != nullptr);
		    if(this->repo == nullptr) throw std::runtime_error(__CPP_TRANSPORT_REPO_NOT_SET);

		    // request information from each worker, and pass all necessary setup details
        MPI::slave_setup_payload payload(this->repo->get_root_path(), this->batcher_capacity, this->pipe_data_capacity, this->pipe_zeta_capacity);

        for(unsigned int i = 0; i < this->world.size()-1; i++)
	        {
            requests[i] = this->world.isend(this->worker_rank(i), MPI::INFORMATION_REQUEST, payload);
	        }

        // wait for all messages to be received, then return
        boost::mpi::wait_all(requests.begin(), requests.end());
	    }


		template <typename number>
    void master_controller<number>::set_up_workers(boost::log::sources::severity_logger< base_writer::log_severity_level >& log)
			{
		    // set up instrument to journal the MPI communication if needed
		    journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

		    // rebuild information about our workers; this information
				// it is updated whenever we start a new task, because the details can vary
				// between model instances
				this->work_scheduler.reset(this->world.size()-1);

		    while(!this->work_scheduler.is_ready())
			    {
		        boost::mpi::status stat = this->world.probe();

				    switch(stat.tag())
					    {
				        case MPI::INFORMATION_RESPONSE:
					        {
				            MPI::slave_information_payload payload;
						        this->world.recv(stat.source(), MPI::INFORMATION_RESPONSE, payload);
						        this->work_scheduler.initialize_worker(log, this->worker_number(stat.source()), payload);
						        break;
					        }

				        default:
					        {
						        BOOST_LOG_SEV(log, base_writer::normal) << "!! Received unexpected MPI message " << stat.tag() << " from worker " << stat.source() << "; discarding";
						        this->world.recv(stat.source(), stat.tag());
						        break;
					        }
					    };
			    }

				// now all data is available, ask scheduler to fix maximum unit of allocation
		    this->work_scheduler.complete_queue_setup();
	    }


		template <typename number>
		void master_controller<number>::close_down_workers(boost::log::sources::severity_logger< base_writer::log_severity_level >& log)
			{
		    BOOST_LOG_SEV(log, base_writer::normal) << "++ Notifying workers of end-of-work";

				for(unsigned int i = 0; i < this->world.size()-1; i++)
					{
						this->world.isend(this->worker_rank(i), MPI::END_OF_WORK);
					}
			}


    template <typename number>
    void master_controller<number>::check_for_progress_update(void)
      {
        // emit update message giving current status if required
        std::string msg;
        bool print_msg = this->work_scheduler.generate_update_string(msg);
        if(print_msg)
          {
            std::ostringstream update_msg;
            update_msg << __CPP_TRANSPORT_TASK_MANAGER_LABEL << " " << msg;
            this->message_handler(update_msg.str());
          }
      }


		template <typename number>
		void master_controller<number>::assign_work_to_workers(boost::log::sources::severity_logger< base_writer::log_severity_level >& log)
			{
		    // set up instrument to journal the MPI communication if needed
		    journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

        // generate a list of work assignments
        std::list<master_scheduler::work_assignment> work = this->work_scheduler.assign_work(log);

        BOOST_LOG_SEV(log, base_writer::normal) << "++ Generating new work assignments for " << work.size() << " worker" << (work.size() > 1 ? "s" : "") << " (" << this->work_scheduler.get_queue_size() << " work items remain in queue)";

        // push assignments to workers
        std::vector<boost::mpi::request> msg_status(work.size());

        unsigned int c = 0;
        for(std::list<master_scheduler::work_assignment>::const_iterator t = work.begin(); t != work.end(); t++, c++)
	        {
            MPI::work_assignment_payload payload(t->get_items());

		        // send message to worker with new assignment information
            msg_status[c] = this->world.isend(this->worker_rank(t->get_worker()), MPI::NEW_WORK_ASSIGNMENT, payload);

		        // mark this worker, and these work items, as assigned
            this->work_scheduler.mark_assigned(*t);
		        BOOST_LOG_SEV(log, base_writer::normal) << "++ Assigned " << t->get_items().size() << " work items to worker " << t->get_worker() << " [MPI rank=" << this->worker_rank(t->get_worker()) << "]";
	        }

        // wait for all assignments to be received
        boost::mpi::wait_all(msg_status.begin(), msg_status.end());
		    BOOST_LOG_SEV(log, base_writer::normal) << "++ All workers accepted new assignments";
			}


    template <typename number>
    void master_controller<number>::terminate_workers(void)
	    {
		    // set up instrument to journal the MPI communication if needed
		    journal_instrument instrument(this->journal, master_work_event::MPI_begin, master_work_event::MPI_end);

        std::vector<boost::mpi::request> requests(this->world.size()-1);

        for(unsigned int i = 0; i < this->world.size()-1; i++)
	        {
            requests[i] = this->world.isend(this->worker_rank(i), MPI::TERMINATE);
	        }

        // wait for all messages to be received, then exit ourselves
        boost::mpi::wait_all(requests.begin(), requests.end());
	    }


    template <typename number>
    bool master_controller<number>::poll_workers(integration_aggregator& int_agg, postintegration_aggregator& post_agg, derived_content_aggregator& derived_agg,
                                                 integration_metadata& int_metadata, output_metadata& out_metadata,
                                                 boost::log::sources::severity_logger< base_writer::log_severity_level >& log,
                                                 slave_work_event::event_type begin_label, slave_work_event::event_type end_label)
	    {
		    bool success = true;

        // set up aggregation queues
        std::list<MPI::data_ready_payload>    int_agg_queue;
        std::list<MPI::data_ready_payload>    post_agg_queue;
        std::list<MPI::content_ready_payload> derived_agg_queue;

        // wait for workers to report their characteristics
        this->set_up_workers(log);

        // record time of last-received message, so we can determine for how long we have been idle
        boost::posix_time::ptime last_msg = boost::posix_time::second_clock::universal_time();
        bool emit_agg_queue_msg = true;

        // poll workers, scattering work and aggregating the results until work items are exhausted
        bool sent_closedown = false;
        while(!this->work_scheduler.all_inactive())
	        {
            // send closedown instruction if no more work
            if(this->work_scheduler.is_finished() && !sent_closedown)
	            {
                sent_closedown = true;
                this->close_down_workers(log);
	            }

            // generate new work assignments if needed, and push them to the workers
            if(this->work_scheduler.assignable())
              {
                this->check_for_progress_update();
                this->assign_work_to_workers(log);
              }

            // check whether any messages are waiting in the queue
            boost::optional<boost::mpi::status> stat = this->world.iprobe();

            while(stat) // consume messages until no more are available
              {
                // update time of last received message and reset flag which enables logging
                last_msg = boost::posix_time::second_clock::universal_time();
                emit_agg_queue_msg = true;

                switch(stat->tag())
                  {
                    case MPI::INTEGRATION_DATA_READY:
                      {
                        if(int_agg)
                          {
                            MPI::data_ready_payload payload;
                            this->world.recv(stat->source(), MPI::INTEGRATION_DATA_READY, payload);
                            int_agg_queue.push_back(payload);
                            BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " sent aggregation notification for container '" << payload.get_container_path() << "'";
                          }
                        else
                          {
                            BOOST_LOG_SEV(log, base_writer::normal) << "!! Received INTEGRATION_DATA_READY message from worker " << stat->source() << ", but no integration aggregator has been assigned";
                          }
                        break;
                      }

                    case MPI::DERIVED_CONTENT_READY:
                      {
                        if(derived_agg)
                          {
                            MPI::content_ready_payload payload;
                            this->world.recv(stat->source(), MPI::DERIVED_CONTENT_READY, payload);
                            derived_agg_queue.push_back(payload);
                            BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " sent content-ready notification";
                          }
                        else
                          {
                            BOOST_LOG_SEV(log, base_writer::normal) << "!! Received DERIVED_CONTENT_READY message from worker " << stat->source() << ", but no derived content aggregator has been assigned";
                          }
                        break;
                      }

                    case MPI::POSTINTEGRATION_DATA_READY:
                      {
                        if(post_agg)
                          {
                            MPI::data_ready_payload payload;
                            this->world.recv(stat->source(), MPI::POSTINTEGRATION_DATA_READY, payload);
                            post_agg_queue.push_back(payload);
                            BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " sent aggregation notification for container '" << payload.get_container_path() << "'";
                          }
                        else
                          {
                            BOOST_LOG_SEV(log, base_writer::normal) << "!! Received POSTINTEGRATION_DATA_READY message from worker " << stat->source() << ", but no postintegration aggregator has been assigned";
                          }
                        break;
                      }

                    case MPI::NEW_WORK_ACKNOWLEDGMENT:
                      {
                        MPI::work_acknowledgment_payload payload;
                        this->world.recv(stat->source(), MPI::NEW_WORK_ACKNOWLEDGMENT, payload);
                        this->journal.add_entry(slave_work_event(this->worker_number(stat->source()), begin_label, payload.get_timestamp()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising receipt of work assignment at time " << boost::posix_time::to_simple_string(payload.get_timestamp());

                        break;
                      }

                    case MPI::FINISHED_INTEGRATION:
                      {
                        MPI::finished_integration_payload payload;
                        this->world.recv(stat->source(), MPI::FINISHED_INTEGRATION, payload);
                        this->journal.add_entry(slave_work_event(this->worker_number(stat->source()), end_label, payload.get_timestamp()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising finished work assignment in wallclock time " << format_time(payload.get_wallclock_time());

                        // mark this worker as unassigned, and update its mean time per work item
                        this->work_scheduler.mark_unassigned(this->worker_number(stat->source()), payload.get_integration_time(), payload.get_num_integrations());
                        this->update_integration_metadata(payload, int_metadata);

                        break;
                      }

                    case MPI::INTEGRATION_FAIL:
                      {
                        MPI::finished_integration_payload payload;
                        this->world.recv(stat->source(), MPI::INTEGRATION_FAIL, payload);
                        this->journal.add_entry(slave_work_event(this->worker_number(stat->source()), end_label, payload.get_timestamp()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising failure of work assignment (successful work items consumed wallclock time " << format_time(payload.get_wallclock_time()) << ")";

                        this->work_scheduler.mark_unassigned(this->worker_number(stat->source()), payload.get_integration_time(), payload.get_num_integrations());
                        this->update_integration_metadata(payload, int_metadata);

                        success = false;
                        break;
                      }

                    case MPI::FINISHED_DERIVED_CONTENT:
                      {
                        MPI::finished_derived_payload payload;
                        this->world.recv(stat->source(), MPI::FINISHED_DERIVED_CONTENT, payload);
                        this->journal.add_entry(slave_work_event(this->worker_number(stat->source()), end_label, payload.get_timestamp()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising finished work assignment in CPU time " << format_time(payload.get_cpu_time());

                        // mark this scheduler as unassigned, and update its mean time per work item
                        this->work_scheduler.mark_unassigned(this->worker_number(stat->source()), payload.get_processing_time(), payload.get_items_processed());
                        this->update_output_metadata(payload, out_metadata);

                        break;
                      }

                    case MPI::DERIVED_CONTENT_FAIL:
                      {
                        MPI::finished_derived_payload payload;
                        this->world.recv(stat->source(), MPI::DERIVED_CONTENT_FAIL, payload);
                        this->journal.add_entry(slave_work_event(this->worker_number(stat->source()), end_label, payload.get_timestamp()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising failure of work assignment (successful work items consumed wallclock time " << format_time(payload.get_cpu_time()) << ")";

                        // mark this scheduler as unassigned, and update its mean time per work item
                        this->work_scheduler.mark_unassigned(this->worker_number(stat->source()), payload.get_processing_time(), payload.get_items_processed());
                        this->update_output_metadata(payload, out_metadata);

                        success = false;
                        break;
                      }

                    case MPI::FINISHED_POSTINTEGRATION:
                      {
                        MPI::finished_postintegration_payload payload;
                        this->world.recv(stat->source(), MPI::FINISHED_POSTINTEGRATION, payload);
                        this->journal.add_entry(slave_work_event(this->worker_number(stat->source()), end_label, payload.get_timestamp()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising finished work assignment in wallclock time " << format_time(payload.get_cpu_time());

                        // mark this worker as unassigned, and update its mean time per work item
                        this->work_scheduler.mark_unassigned(this->worker_number(stat->source()), payload.get_processing_time(), payload.get_items_processed());
                        this->update_output_metadata(payload, out_metadata);

                        break;
                      }

                    case MPI::POSTINTEGRATION_FAIL:
                      {
                        MPI::finished_postintegration_payload payload;
                        this->world.recv(stat->source(), MPI::POSTINTEGRATION_FAIL, payload);
                        this->journal.add_entry(slave_work_event(this->worker_number(stat->source()), end_label, payload.get_timestamp()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising failure of work assignment (successful work items consumed wallclock time " << format_time(payload.get_cpu_time()) << ")";

                        // mark this worker as unassigned, and update its mean time per work item
                        this->work_scheduler.mark_unassigned(this->worker_number(stat->source()), payload.get_processing_time(), payload.get_items_processed());
                        this->update_output_metadata(payload, out_metadata);

                        success = false;
                        break;
                      }

                    case MPI::WORKER_CLOSE_DOWN:
                      {
                        this->world.recv(stat->source(), MPI::WORKER_CLOSE_DOWN);
                        this->work_scheduler.mark_inactive(this->worker_number(stat->source()));
                        BOOST_LOG_SEV(log, base_writer::normal) << "++ Worker " << stat->source() << " advising close-down after end-of-work";
                        break;
                      }

                    default:
                      {
                        BOOST_LOG_SEV(log, base_writer::warning) << "!! Received unexpected message " << stat->tag() << " waiting in the queue; discarding";
                        this->world.recv(stat->source(), stat->tag());
                        break;
                      }
                  }

                stat = this->world.iprobe();  // check for another message
              }

            // we arrive at this point only when no more messages are available to be received

            // check whether any aggregations are in the queue, and process them if we have been idle for sufficiently long
            if(int_agg_queue.size() > 0 || post_agg_queue.size() > 0 || derived_agg_queue.size() > 0)
              {
                boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
                boost::posix_time::time_duration idle_time = now - last_msg;
                if(idle_time.total_seconds() > 5)
                  {
                    if(emit_agg_queue_msg)
                      {
                        BOOST_LOG_SEV(log, base_writer::warning) << "++ Idle for more than 5 seconds: processing queued aggregations";
                        emit_agg_queue_msg = false;
                      }

                    if(int_agg_queue.size() > 0)
                      {
                        if(int_agg) int_agg(int_agg_queue.front(), int_metadata);
                        int_agg_queue.pop_front();
                      }
                    else if(post_agg_queue.size() > 0)
                      {
                        if(post_agg) post_agg(post_agg_queue.front(), out_metadata);
                        post_agg_queue.pop_front();
                      }
                    else if(derived_agg_queue.size() > 0)
                      {
                        if(derived_agg) success = derived_agg(derived_agg_queue.front(), out_metadata);
                        derived_agg_queue.pop_front();
                      }
                  }
              }
	        }

        this->check_for_progress_update();

        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(log, base_writer::warning) << "++ Work completed at " << boost::posix_time::to_simple_string(now);

        // process any remaining aggregations
        while(int_agg_queue.size() > 0 || post_agg_queue.size() > 0 || derived_agg_queue.size() > 0)
          {
            if(int_agg_queue.size() > 0)
              {
                if(int_agg) int_agg(int_agg_queue.front(), int_metadata);
                int_agg_queue.pop_front();
              }
            else if(post_agg_queue.size() > 0)
              {
                if(post_agg) post_agg(post_agg_queue.front(), out_metadata);
                post_agg_queue.pop_front();
              }
            else if(derived_agg_queue.size() > 0)
              {
                if(derived_agg) success = derived_agg(derived_agg_queue.front(), out_metadata);
                derived_agg_queue.pop_front();
              }
          }

        return(success);
	    }

	}   // namespace transport


#endif //__master_controller_H_
