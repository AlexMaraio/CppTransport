//
// Created by David Seery on 19/03/15.
// Copyright (c) 2015-2016 University of Sussex. All rights reserved.
//


#ifndef CPPTRANSPORT_SLAVE_CONTROLLER_H
#define CPPTRANSPORT_SLAVE_CONTROLLER_H


#include <list>
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
#include "transport-runtime-api/manager/slave_work_handler.h"
#include "transport-runtime-api/manager/environment.h"

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
#include "boost/date_time/posix_time/posix_time.hpp"


namespace transport
	{

    // SLAVE FUNCTIONS

		template <typename number>
		class slave_controller
			{

		  public:

		    //! Error-reporting callback object
		    typedef std::function<void(const std::string&)> error_callback;

		    //! Warning callback object
		    typedef std::function<void(const std::string&)> warning_callback;

		    //! Message callback object
		    typedef std::function<void(const std::string&)> message_callback;


				// CONSTRUCTOR, DESTRUCTOR

		  public:

		    //! construct a slave controller object
		    //! unlike a master controller, there is no option to supply a repository;
		    //! one has to be provided by a master controller over MPI later
		    slave_controller(boost::mpi::environment& e, boost::mpi::communicator& w,
                         local_environment& le, argument_cache& ac,
		                     const model_finder<number>& f,
		                     error_callback err, warning_callback warn, message_callback msg);

		    //! destroy a slave manager object
		    ~slave_controller() = default;


				// INTERFACE

		  public:

		    //! poll for instructions to perform work
		    void wait_for_tasks(void);


				// MPI FUNCTIONS

		  protected:

		    //! Get worker number
		    unsigned int worker_number() { return(static_cast<unsigned int>(this->world.rank()-1)); }

		    //! Return MPI rank of this process
		    unsigned int get_rank(void) const { return(static_cast<unsigned int>(this->world.rank())); }


		    // SLAVE JOB HANDLING

		  protected:

		    //! Slave node: initialize ourselves
		    void initialize(const MPI::slave_setup_payload& payload);

				//! Slave node: Pass scheduling data to the master node
				void send_worker_data(model<number>* m);

				//! Slave node: Pass scheduling data to the master node
				void send_worker_data(void);


		    // SLAVE INTEGRATION TASKS

		  protected:

		    //! Slave node: Process a new integration task instruction
		    void process_task(const MPI::new_integration_payload& payload);

		    //! Slave node: process an integration task
		    void dispatch_integration_task(integration_task<number>* tk, const MPI::new_integration_payload& payload);

		    //! Slave node: process an integration queue
		    template <typename TaskObject, typename BatchObject>
		    void schedule_integration(TaskObject* tk, model<number>* m, BatchObject& batcher, unsigned int state_size);

		    //! Push a temporary container to the master process
		    void push_temp_container(generic_batcher* batcher, unsigned int message, std::string log_message);

				//! Construct a work item filter for a twopf task
				work_item_filter<twopf_kconfig> work_item_filter_factory(twopf_task<number>* tk, const std::list<unsigned int>& items) const { return work_item_filter<twopf_kconfig>(items); }

		    //! Construct a work item filter for a threepf task
		    work_item_filter<threepf_kconfig> work_item_filter_factory(threepf_task<number>* tk, const std::list<unsigned int>& items) const { return work_item_filter<threepf_kconfig>(items); }

				//! Construct a work item filter for a zeta threepf task
				work_item_filter<threepf_kconfig> work_item_filter_factory(zeta_threepf_task<number>* tk, const std::list<unsigned int>& items) const { return work_item_filter<threepf_kconfig>(items); }

				//! Construct a work item filter factory for an output task
				work_item_filter< output_task_element<number> > work_item_filter_factory(output_task<number>* tk, const std::list<unsigned int>& items) const { return work_item_filter< output_task_element<number> >(items); }


		    // SLAVE POSTINTEGRATION TASKS

		  protected:

		    //! Slave node: Process a new postintegration task instruction
		    void process_task(const MPI::new_postintegration_payload& payload);

		    //! Slave node: Process a postintegration task
		    void dispatch_postintegration_task(postintegration_task<number>* tk, const MPI::new_postintegration_payload& payload);

		    //! Slave node: process a postintegration queue
		    template <typename TaskObject, typename ParentTaskObject, typename BatchObject>
		    void schedule_postintegration(TaskObject* tk, ParentTaskObject* ptk,
		                                  const MPI::new_postintegration_payload& payload, BatchObject& batcher);

		    //! No-op push content for connexion to datapipe
		    void disallow_push_content(datapipe<number>* pipe, typename derived_data::derived_product<number>* product);


		    // SLAVE OUTPUT TASKS

		  protected:

		    //! Slave node: Process a new output task instruction
		    void process_task(const MPI::new_derived_content_payload& payload);

		    //! Slave node: Process an output task
		    void schedule_output(output_task<number>* tk, const MPI::new_derived_content_payload& payload);

		    //! Push new derived content to the master process
		    void push_derived_content(datapipe<number>* pipe, typename derived_data::derived_product<number>* product, const std::list<std::string>& used_groups);


		    // INTERNAL DATA

		  protected:


		    // MPI ENVIRONMENT

		    //! BOOST::MPI environment
		    boost::mpi::environment& environment;

		    //! BOOST::MPI world communicator
		    boost::mpi::communicator& world;


        // LOCAL ENVIRONMENT

        //! environment data
        local_environment& local_env;

        //! Argument cache
        argument_cache& arg_cache;


				// MODEL FINDER REFERENCE
		    const model_finder<number> finder;

		    // RUNTIME AGENTS

		    //! Repository manager instance
		    std::shared_ptr< json_repository<number> > repo;

		    //! Data manager instance
		    std::shared_ptr< data_manager<number> > data_mgr;

				//! Handler for postintegration and output tasks
				slave_work_handler<number> work_handler;


				// ERROR CALLBACKS

				//! error callback
				error_callback error_handler;

				//! warning callback
				warning_callback warning_handler;

				//! message callback
				message_callback message_handler;

			};


    template <typename number>
    slave_controller<number>::slave_controller(boost::mpi::environment& e, boost::mpi::communicator& w,
                                               local_environment& le, argument_cache& ac,
                                               const model_finder<number>& f,
                                               error_callback err, warning_callback warn, message_callback msg)
	    : environment(e),
	      world(w),
        local_env(le),
        arg_cache(ac),
	      finder(f),
	      data_mgr(data_manager_factory<number>(ac.get_batcher_capacity(), ac.get_datapipe_capacity(), ac.get_checkpoint_interval())),
	      error_handler(err),
	      warning_handler(warn),
	      message_handler(msg)
	    {
	    }


    template <typename number>
    void slave_controller<number>::wait_for_tasks(void)
	    {
        if(this->get_rank() == 0) throw runtime_exception(exception_type::MPI_ERROR, CPPTRANSPORT_WAIT_MASTER);

        bool finished = false;

        while(!finished)
	        {
            // wait until a message is available from master
            boost::mpi::status stat = this->world.probe(MPI::RANK_MASTER);

            switch(stat.tag())
	            {
                case MPI::NEW_INTEGRATION:
	                {
                    MPI::new_integration_payload payload;
                    this->world.recv(MPI::RANK_MASTER, MPI::NEW_INTEGRATION, payload);
                    this->process_task(payload);
                    break;
	                }

                case MPI::NEW_DERIVED_CONTENT:
	                {
                    MPI::new_derived_content_payload payload;
                    this->world.recv(MPI::RANK_MASTER, MPI::NEW_DERIVED_CONTENT, payload);
                    this->process_task(payload);
                    break;
	                }

                case MPI::NEW_POSTINTEGRATION:
	                {
                    MPI::new_postintegration_payload payload;
                    this->world.recv(MPI::RANK_MASTER, MPI::NEW_POSTINTEGRATION, payload);
                    this->process_task(payload);
                    break;
	                }

                case MPI::INFORMATION_REQUEST:
	                {
                    MPI::slave_setup_payload payload;
                    this->world.recv(MPI::RANK_MASTER, MPI::INFORMATION_REQUEST, payload);
                    this->initialize(payload);
                    break;
	                }

                case MPI::TERMINATE:
	                {
                    this->world.recv(MPI::RANK_MASTER, MPI::TERMINATE);
                    finished = true;
                    break;
	                }

                default:
	                throw runtime_exception(exception_type::MPI_ERROR, CPPTRANSPORT_UNEXPECTED_MPI);
	            }
	        }
	    }


    template <typename number>
    void slave_controller<number>::initialize(const MPI::slave_setup_payload& payload)
	    {
        try
	        {
            boost::filesystem::path repo_path = payload.get_repository_path();

            this->repo = repository_factory<number>(repo_path.string(), repository_mode::readonly,
                                                    this->error_handler, this->warning_handler, this->message_handler);
            this->repo->set_model_finder(this->finder);

            this->arg_cache = payload.get_argument_cache();

		        this->data_mgr->set_batcher_capacity(this->arg_cache.get_batcher_capacity());
            this->data_mgr->set_pipe_capacity(this->arg_cache.get_datapipe_capacity());
            this->data_mgr->set_checkpoint_interval(this->arg_cache.get_checkpoint_interval());
	        }
        catch (runtime_exception& xe)
	        {
            if(xe.get_exception_code() == exception_type::REPO_NOT_FOUND)
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


		template <typename number>
		void slave_controller<number>::send_worker_data(model<number>* m)
			{
				assert(m != nullptr);

		    MPI::slave_information_payload payload(m->get_backend_type(), m->get_backend_memory(), m->get_backend_priority());
				this->world.isend(MPI::RANK_MASTER, MPI::INFORMATION_RESPONSE, payload);
			}


		template <typename number>
		void slave_controller<number>::send_worker_data(void)
			{
		    MPI::slave_information_payload payload(worker_type::cpu, 0, 1);

		    this->world.isend(MPI::RANK_MASTER, MPI::INFORMATION_RESPONSE, payload);
			}


    template <typename number>
    void slave_controller<number>::process_task(const MPI::new_integration_payload& payload)
	    {
        // ensure that a valid repository object has been constructed
        if(!this->repo) throw runtime_exception(exception_type::RUNTIME_ERROR, CPPTRANSPORT_REPO_NOT_SET);

        // extract our task from the database
        // much of this is boiler-plate which is similar to master_process_task()
        // TODO: it would be nice to make this sharing more explicit, so the code isn't just duplicated
        try
	        {
            // query a task record with the name we're looking for from the database
            std::unique_ptr< task_record<number> > record(this->repo->query_task(payload.get_task_name()));

            switch(record->get_type())
	            {
                case task_record<number>::task_type::integration:
	                {
                    integration_task_record<number>* int_rec = dynamic_cast< integration_task_record<number>* >(record.get());

                    assert(int_rec != nullptr);
                    if(int_rec == nullptr) throw runtime_exception(exception_type::REPOSITORY_ERROR, CPPTRANSPORT_REPO_RECORD_CAST_FAILED);

                    integration_task<number>* tk = int_rec->get_task();
                    this->dispatch_integration_task(tk, payload);
                    break;
	                }

                case task_record<number>::task_type::output:
	                {
                    throw runtime_exception(exception_type::RECORD_NOT_FOUND, payload.get_task_name());    // RECORD_NOT_FOUND expects task name in message
	                }

                case task_record<number>::task_type::postintegration:
	                {
                    throw runtime_exception(exception_type::RECORD_NOT_FOUND, payload.get_task_name());    // RECORD_NOT_FOUND expects task name in message
	                }
	            }
	        }
        catch(runtime_exception xe)
	        {
            if(xe.get_exception_code() == exception_type::RECORD_NOT_FOUND)
	            {
                std::ostringstream msg;
                msg << CPPTRANSPORT_REPO_MISSING_RECORD << " '" << xe.what() << "'" << CPPTRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else if(xe.get_exception_code() == exception_type::MISSING_MODEL_INSTANCE
	                  || xe.get_exception_code() == exception_type::REPOSITORY_BACKEND_ERROR)
	            {
                std::ostringstream msg;
                msg << xe.what() << " " << CPPTRANSPORT_REPO_FOR_TASK << " '" << payload.get_task_name() << "'" << CPPTRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else
	            {
                throw xe;
	            }
	        }
	    }


    template <typename number>
    void slave_controller<number>::dispatch_integration_task(integration_task<number>* tk, const MPI::new_integration_payload& payload)
	    {
        assert(tk != nullptr);

        model<number>* m = tk->get_model();
        assert(m != nullptr);

		    // send scheduling information to the master process
		    this->send_worker_data(m);

        twopf_task<number>* tka = nullptr;
        threepf_task<number>* tkb = nullptr;

        if((tka = dynamic_cast<twopf_task<number>*>(tk)) != nullptr)
	        {
            // construct a callback for the integrator to push new batches to the master
            generic_batcher::container_dispatch_function dispatcher = std::bind(&slave_controller<number>::push_temp_container, this, std::placeholders::_1,
                                                                                MPI::INTEGRATION_DATA_READY, std::string("INTEGRATION_DATA_READY"));

            // construct a batcher to hold the output of the integration
            twopf_batcher<number> batcher = this->data_mgr->create_temp_twopf_container(tka, payload.get_tempdir_path(), payload.get_logdir_path(),
                                                                                        this->get_rank(), payload.get_workgroup_number(), m, dispatcher);

            // write log header
            boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
            BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW INTEGRATION TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
            BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << *tk;

            this->schedule_integration(tka, m, batcher, m->backend_twopf_state_size());
	        }
        else if((tkb = dynamic_cast<threepf_task<number>*>(tk)) != nullptr)
	        {
            // construct a callback for the integrator to push new batches to the master
            generic_batcher::container_dispatch_function dispatcher = std::bind(&slave_controller<number>::push_temp_container, this, std::placeholders::_1,
                                                                                MPI::INTEGRATION_DATA_READY, std::string("INTEGRATION_DATA_READY"));

            // construct a batcher to hold the output of the integration
            threepf_batcher<number> batcher = this->data_mgr->create_temp_threepf_container(tkb, payload.get_tempdir_path(), payload.get_logdir_path(),
                                                                                            this->get_rank(), payload.get_workgroup_number(), m, dispatcher);

            // write log header
            boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
            BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW INTEGRATION TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
            BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << *tk;

            this->schedule_integration(tkb, m, batcher, m->backend_threepf_state_size());
	        }
        else
	        {
            std::ostringstream msg;
            msg << CPPTRANSPORT_UNKNOWN_DERIVED_TASK << " '" << tk->get_name() << "'";
            throw runtime_exception(exception_type::REPOSITORY_ERROR, msg.str());
	        }
	    }


    template <typename number>
    template <typename TaskObject, typename BatchObject>
    void slave_controller<number>::schedule_integration(TaskObject* tk, model<number>* m, BatchObject& batcher, unsigned int state_size)
	    {
        // dispatch integration to the underlying model

        assert(tk != nullptr);  // should be guaranteed
        assert(m != nullptr);   // should be guaranteed

		    bool complete = false;
		    while(!complete)
			    {
				    // wait for messages from scheduler
		        boost::mpi::status stat = this->world.probe(MPI::RANK_MASTER);

				    switch(stat.tag())
					    {
				        case MPI::NEW_WORK_ASSIGNMENT:
					        {
				            MPI::work_assignment_payload payload;
						        this->world.recv(stat.source(), MPI::NEW_WORK_ASSIGNMENT, payload);

				            MPI::work_acknowledgment_payload ack_payload;
				            ack_payload.set_timestamp();
				            this->world.isend(MPI::RANK_MASTER, MPI::NEW_WORK_ACKNOWLEDGMENT, ack_payload);

				            const std::list<unsigned int>& work_items = payload.get_items();
						        auto filter = this->work_item_filter_factory(tk, work_items);

						        // create work queues based on whatever devices are relevant for our backend
						        context ctx = m->backend_get_context();
						        scheduler sch(ctx);
						        auto work = sch.make_queue(state_size, *tk, filter);

				            bool success = true;
						        batcher.begin_assignment();

				            // keep track of wallclock time
				            boost::timer::cpu_timer timer;

						        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW WORK ASSIGNMENT";

				            // perform the integration
				            try
					            {
				                m->backend_process_queue(work, tk, batcher, true);    // 'true' = work silently
					            }
				            catch(runtime_exception& xe)
					            {
				                success = false;
				                BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::error) << "-- Exception reported during integration: code=" << static_cast<int>(xe.get_exception_code()) << ": " << xe.what();
					            }

				            // all work is now done - stop the wallclock timer
						        batcher.end_assignment();
				            timer.stop();

				            // notify master process that all work has been finished (temporary containers will be deleted by the master node)
                    boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
				            if(success) BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- Worker sending FINISHED_INTEGRATION to master | finished at " << boost::posix_time::to_simple_string(now);
				            else        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::error)  << '\n' << "-- Worker reporting INTEGRATION_FAIL to master | finished at " << boost::posix_time::to_simple_string(now);

                    MPI::finished_integration_payload outgoing_payload(batcher.get_integration_time(),
                                                                       batcher.get_max_integration_time(), batcher.get_min_integration_time(),
                                                                       batcher.get_batching_time(),
                                                                       batcher.get_max_batching_time(), batcher.get_min_batching_time(),
                                                                       timer.elapsed().wall,
                                                                       batcher.get_reported_integrations(),
                                                                       batcher.get_reported_refinements(), batcher.get_reported_failures(),
                                                                       batcher.get_failed_serials());

				            this->world.isend(MPI::RANK_MASTER, success ? MPI::FINISHED_INTEGRATION : MPI::INTEGRATION_FAIL, outgoing_payload);

						        break;
					        };

				        case MPI::END_OF_WORK:
					        {
						        this->world.recv(stat.source(), MPI::END_OF_WORK);
						        complete = true;
						        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- Notified of end-of-work: preparing to shut down";

				            // close the batcher, flushing the current container to the master node if needed
				            batcher.close();

						        // send close-down acknowledgment to master
                    boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
						        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- Worker sending WORKER_CLOSE_DOWN to master | close down at " << boost::posix_time::to_simple_string(now);
						        this->world.isend(MPI::RANK_MASTER, MPI::WORKER_CLOSE_DOWN);

						        break;
					        };

				        default:
					        {
						        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << "!! Received unexpected MPI message " << stat.tag() << " from master node; discarding";
						        this->world.recv(stat.source(), stat.tag());
						        break;
					        }
					    }
			    }
	    }


    template <typename number>
    void slave_controller<number>::process_task(const MPI::new_derived_content_payload& payload)
	    {
        // ensure that a valid repository object has been constructed
        if(!this->repo) throw runtime_exception(exception_type::RUNTIME_ERROR, CPPTRANSPORT_REPO_NOT_SET);

        // extract our task from the database
        // much of this is boiler-plate which is similar to master_process_task()
        // TODO: it would be nice to make this sharing more explicit, so the code isn't just duplicated
        try
	        {
            // query a task record with the name we're looking for from the database
            std::unique_ptr< task_record<number> > record(this->repo->query_task(payload.get_task_name()));

            switch(record->get_type())
	            {
                case task_record<number>::task_type::integration:
	                {
                    throw runtime_exception(exception_type::RECORD_NOT_FOUND, payload.get_task_name());     // RECORD_NOT_FOUND expects task name in message
	                }

                case task_record<number>::task_type::output:
	                {
                    output_task_record<number>* out_rec = dynamic_cast< output_task_record<number>* >(record.get());

                    assert(out_rec != nullptr);
                    if(out_rec == nullptr) throw runtime_exception(exception_type::REPOSITORY_ERROR, CPPTRANSPORT_REPO_RECORD_CAST_FAILED);

                    output_task<number>* tk = out_rec->get_task();
                    this->schedule_output(tk, payload);
                    break;
	                }

                case task_record<number>::task_type::postintegration:
	                {
                    throw runtime_exception(exception_type::RECORD_NOT_FOUND, payload.get_task_name());    // RECORD_NOT_FOUND expects task name in message
	                }
	            }

	        }
        catch (runtime_exception xe)
	        {
            if(xe.get_exception_code() == exception_type::RECORD_NOT_FOUND)
	            {
                std::ostringstream msg;
                msg << CPPTRANSPORT_REPO_MISSING_RECORD << " '" << xe.what() << "'" << CPPTRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else if(xe.get_exception_code() == exception_type::MISSING_MODEL_INSTANCE
	                  || xe.get_exception_code() == exception_type::REPOSITORY_BACKEND_ERROR)
	            {
                std::ostringstream msg;
                msg << xe.what() << " " << CPPTRANSPORT_REPO_FOR_TASK << " '" << payload.get_task_name() << "'" << CPPTRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else
	            {
                throw xe;
	            }
	        }
	    }


    template <typename number>
    void slave_controller<number>::schedule_output(output_task<number>* tk, const MPI::new_derived_content_payload& payload)
	    {
        assert(tk != nullptr);  // should be guaranteed

        // send scheduling information to the master process; here, report ourselves as a CPU
        // since there is currently no capacity to process output tasks on a GPU
        this->send_worker_data();

        // set up output-group finder function
        typename datapipe<number>::integration_content_finder     i_finder = std::bind(&repository<number>::find_integration_task_output, this->repo, std::placeholders::_1, std::placeholders::_2);
        typename datapipe<number>::postintegration_content_finder p_finder = std::bind(&repository<number>::find_postintegration_task_output, this->repo, std::placeholders::_1, std::placeholders::_2);

        // set up content-dispatch function
        typename datapipe<number>::dispatch_function dispatcher = std::bind(&slave_controller<number>::push_derived_content, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        // acquire a datapipe which we can use to stream content from the databse
        std::unique_ptr< datapipe<number> > pipe = this->data_mgr->create_datapipe(payload.get_logdir_path(), payload.get_tempdir_path(), i_finder, p_finder, dispatcher, this->get_rank());

        // write log header
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << '\n' << "-- NEW OUTPUT TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
        BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << *tk;

		    bool complete = false;
		    while(!complete)
			    {
				    // wait for messages from scheduler
		        boost::mpi::status stat = this->world.probe(MPI::RANK_MASTER);

				    switch(stat.tag())
					    {
				        case MPI::NEW_WORK_ASSIGNMENT:
					        {
				            MPI::work_assignment_payload assignment_payload;
						        this->world.recv(stat.source(), MPI::NEW_WORK_ASSIGNMENT, assignment_payload);

				            MPI::work_acknowledgment_payload ack_payload;
						        ack_payload.set_timestamp();
						        this->world.isend(MPI::RANK_MASTER, MPI::NEW_WORK_ACKNOWLEDGMENT, ack_payload);

						        const std::list<unsigned int>& work_items = assignment_payload.get_items();
						        auto filter = this->work_item_filter_factory(tk, work_items);

						        // create work queues
						        context ctx;
						        ctx.add_device("CPU");
						        scheduler sch(ctx);
						        auto work = sch.make_queue(*tk, filter);

				            BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << '\n' << "-- NEW WORK ASSIGNMENT";

				            bool success = true;

                    // track output groups we use
                    std::list<std::string> content_groups;

				            // keep track of wallclock time
				            boost::timer::cpu_timer timer;
				            boost::timer::nanosecond_type processing_time = 0;
				            boost::timer::nanosecond_type min_processing_time = 0;
				            boost::timer::nanosecond_type max_processing_time = 0;

				            std::ostringstream work_msg;
				            work_msg << work;
				            BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << work_msg.str();

				            const typename work_queue< output_task_element<number> >::device_queue queues = work[0];
				            assert(queues.size() == 1);

				            const typename work_queue< output_task_element<number> >::device_work_list list = queues[0];

				            for(unsigned int i = 0; i < list.size(); ++i)
					            {
				                typename derived_data::derived_product<number>* product = list[i].get_product();

				                assert(product != nullptr);
				                if(product == nullptr)
					                {
				                    std::ostringstream msg;
				                    msg << CPPTRANSPORT_TASK_NULL_DERIVED_PRODUCT << " '" << tk->get_name() << "'";
				                    throw runtime_exception(exception_type::RUNTIME_ERROR, msg.str());
					                }

				                // merge command-line supplied tags with tags specified in the task
				                std::list<std::string> task_tags = list[i].get_tags();
				                std::list<std::string> command_line_tags = payload.get_tags();

				                task_tags.splice(task_tags.end(), command_line_tags);

				                BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << "-- Processing derived product '" << product->get_name() << "'";

                        std::list<std::string> this_groups;

				                try
					                {
				                    boost::timer::cpu_timer derive_timer;
				                    this_groups = product->derive(*pipe, task_tags, this->local_env);
                            content_groups.merge(this_groups);
						                derive_timer.stop();
						                processing_time += derive_timer.elapsed().wall;
						                if(max_processing_time == 0 || derive_timer.elapsed().wall > max_processing_time) max_processing_time = derive_timer.elapsed().wall;
						                if(min_processing_time == 0 || derive_timer.elapsed().wall < min_processing_time) min_processing_time = derive_timer.elapsed().wall;
					                }
				                catch(runtime_exception& xe)
					                {
				                    success = false;
				                    BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::error) << "!! Exception reported while processing: code=" << static_cast<int>(xe.get_exception_code()) << ": " << xe.what();
					                }

				                // check that the datapipe was correctly detached
				                if(pipe->is_attached())
					                {
				                    BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::error) << "!! Task manager detected that datapipe was not correctly detached after generating derived product '" << product->get_name() << "'";
				                    pipe->detach();
					                }

				                BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << "";
					            }

                    // collect content groups used during this derivation
                    content_groups.sort();
                    content_groups.unique();

				            // all work now done - stop the timer
				            timer.stop();

				            // notify master process that all work has been finished
				            now = boost::posix_time::second_clock::universal_time();
				            if(success) BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << '\n' << "-- Worker sending FINISHED_DERIVED_CONTENT to master | finished at " << boost::posix_time::to_simple_string(now);
				            else        BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::error)  << '\n' << "-- Worker reporting DERIVED_CONTENT_FAIL to master | finished at " << boost::posix_time::to_simple_string(now);

				            MPI::finished_derived_payload finish_payload(content_groups, pipe->get_database_time(), timer.elapsed().wall,
				                                                         list.size(), processing_time,
				                                                         min_processing_time, max_processing_time,
				                                                         pipe->get_time_config_cache_hits(), pipe->get_time_config_cache_unloads(),
				                                                         pipe->get_twopf_kconfig_cache_hits(), pipe->get_twopf_kconfig_cache_unloads(),
				                                                         pipe->get_threepf_kconfig_cache_hits(), pipe->get_threepf_kconfig_cache_unloads(),
                                                                 pipe->get_stats_cache_hits(), pipe->get_stats_cache_unloads(),
				                                                         pipe->get_data_cache_hits(), pipe->get_data_cache_unloads(),
				                                                         pipe->get_time_config_cache_evictions(), pipe->get_twopf_kconfig_cache_evictions(),
				                                                         pipe->get_threepf_kconfig_cache_evictions(), pipe->get_stats_cache_evictions(),
                                                                 pipe->get_data_cache_evictions());

				            this->world.isend(MPI::RANK_MASTER, success ? MPI::FINISHED_DERIVED_CONTENT : MPI::DERIVED_CONTENT_FAIL, finish_payload);

						        break;
					        }

				        case MPI::END_OF_WORK:
					        {
				            this->world.recv(stat.source(), MPI::END_OF_WORK);
				            complete = true;
				            BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << '\n' << "-- Notified of end-of-work: preparing to shut down";

				            // close the datapipe
				            pipe->close();

				            // send close-down acknowledgment to master
				            now = boost::posix_time::second_clock::universal_time();
				            BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << '\n' << "-- Worker sending WORKER_CLOSE_DOWN to master | close down at " << boost::posix_time::to_simple_string(now);
				            this->world.isend(MPI::RANK_MASTER, MPI::WORKER_CLOSE_DOWN);

				            break;
					        }

				        default:
					        {
				            BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << "!! Received unexpected MPI message " << stat.tag() << " from master node; discarding";
				            this->world.recv(stat.source(), stat.tag());
				            break;
					        }
					    };
			    }

	    }


    template <typename number>
    void slave_controller<number>::process_task(const MPI::new_postintegration_payload& payload)
	    {
        // ensure that a valid repository object has been constructed
        if(!this->repo) throw runtime_exception(exception_type::RUNTIME_ERROR, CPPTRANSPORT_REPO_NOT_SET);

        // extract our task from the database
        try
	        {
            // query a task record with the name we're looking for from the database
            std::unique_ptr< task_record<number> > record(this->repo->query_task(payload.get_task_name()));

            switch(record->get_type())
	            {
                case task_record<number>::task_type::integration:
	                {
//                    std::ostringstream msg;
//                    msg << CPPTRANSPORT_REPO_TASK_IS_INTEGRATION << " '" << payload.get_task_name() << "'";
                    throw runtime_exception(exception_type::RECORD_NOT_FOUND, payload.get_task_name());    // RECORD_NOT_FOUND expects task name in message
	                }

                case task_record<number>::task_type::output:
	                {
//                    std::ostringstream msg;
//                    msg << CPPTRANSPORT_REPO_TASK_IS_OUTPUT << " '" << payload.get_task_name() << "'";
                    throw runtime_exception(exception_type::RECORD_NOT_FOUND, payload.get_task_name());    // RECORD_NOT_FOUND expects task name in message
	                }

                case task_record<number>::task_type::postintegration:
	                {
                    postintegration_task_record<number>* pint_rec = dynamic_cast< postintegration_task_record<number>* >(record.get());

                    assert(pint_rec != nullptr);
                    if(pint_rec == nullptr) throw runtime_exception(exception_type::REPOSITORY_ERROR, CPPTRANSPORT_REPO_RECORD_CAST_FAILED);

                    postintegration_task<number>* tk = pint_rec->get_task();
                    this->dispatch_postintegration_task(tk, payload);
                    break;
	                }
	            }
	        }
        catch(runtime_exception xe)
	        {
            if(xe.get_exception_code() == exception_type::RECORD_NOT_FOUND)
	            {
                std::ostringstream msg;
                msg << CPPTRANSPORT_REPO_MISSING_RECORD << " '" << xe.what() << "'" << CPPTRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else if(xe.get_exception_code() == exception_type::MISSING_MODEL_INSTANCE
	                  || xe.get_exception_code() == exception_type::REPOSITORY_BACKEND_ERROR)
	            {
                std::ostringstream msg;
                msg << xe.what() << " " << CPPTRANSPORT_REPO_FOR_TASK << " '" << payload.get_task_name() << "'" << CPPTRANSPORT_REPO_SKIPPING_TASK;
                this->error_handler(msg.str());
	            }
            else
	            {
                throw xe;
	            }
	        }
	    }


    template <typename number>
    void slave_controller<number>::dispatch_postintegration_task(postintegration_task<number>* tk, const MPI::new_postintegration_payload& payload)
	    {
        assert(tk != nullptr);

        // send scheduling information to the master process; here, report ourselves as a CPU
		    // since there is currently no capacity to process postintegration tasks on a GPU
        this->send_worker_data();

        zeta_twopf_task<number>* z2pf = nullptr;
        zeta_threepf_task<number>* z3pf = nullptr;
        fNL_task<number>* zfNL = nullptr;

        if((z2pf = dynamic_cast<zeta_twopf_task<number>*>(tk)) != nullptr)
	        {
		        // get parent task
            twopf_task<number>* ptk = dynamic_cast<twopf_task<number>*>(z2pf->get_parent_task());

            assert(ptk != nullptr);
            if(ptk == nullptr)
	            {
                std::ostringstream msg;
                msg << CPPTRANSPORT_EXPECTED_TWOPF_TASK << " '" << z2pf->get_parent_task()->get_name() << "'";
                throw runtime_exception(exception_type::REPOSITORY_ERROR, msg.str());
	            }

            model<number>* m = ptk->get_model();

            // construct a callback for the postintegrator to push new batches to the master
            generic_batcher::container_dispatch_function dispatcher = std::bind(&slave_controller<number>::push_temp_container, this, std::placeholders::_1,
                                                                                MPI::POSTINTEGRATION_DATA_READY, std::string("POSTINTEGRATION_DATA_READY"));

            // construct batcher to hold postintegration output
            zeta_twopf_batcher<number> batcher = this->data_mgr->create_temp_zeta_twopf_container(payload.get_tempdir_path(), payload.get_logdir_path(), this->get_rank(), m, dispatcher);

            // is this 2pf task paired?
            if(z2pf->is_paired())
              {
                // also need a callback for the paired integrator
                generic_batcher::container_dispatch_function i_dispatcher = std::bind(&slave_controller<number>::push_temp_container, this, std::placeholders::_1,
                                                                                      MPI::INTEGRATION_DATA_READY, std::string("INTEGRATION_DATA_READY"));

                // construct a batcher to hold integration output
                twopf_batcher<number> i_batcher = this->data_mgr->create_temp_twopf_container(ptk, payload.get_paired_tempdir_path(), payload.get_paired_logdir_path(), this->get_rank(), payload.get_paired_workgroup_number(), m, i_dispatcher);

                // pair batchers
                i_batcher.pair(&batcher);

                // write log header
                boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
                BOOST_LOG_SEV(i_batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW PAIRED POSTINTEGRATION TASKS '" << tk->get_name() << "' & '" << ptk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
                BOOST_LOG_SEV(i_batcher.get_log(), generic_batcher::log_severity_level::normal) << *ptk;

                this->schedule_integration(ptk, m, i_batcher, m->backend_twopf_state_size());
              }
            else
              {
                // write log header
                boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
                BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW POSTINTEGRATION TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
                BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << *tk;

                this->schedule_postintegration(z2pf, ptk, payload, batcher);
              }
	        }
        else if((z3pf = dynamic_cast<zeta_threepf_task<number>*>(tk)) != nullptr)
	        {
		        // get parent task
            threepf_task<number>* ptk = dynamic_cast<threepf_task<number>*>(z3pf->get_parent_task());

            assert(ptk != nullptr);
            if(ptk == nullptr)
	            {
                std::ostringstream msg;
                msg << CPPTRANSPORT_EXPECTED_THREEPF_TASK << " '" << z3pf->get_parent_task()->get_name() << "'";
                throw runtime_exception(exception_type::REPOSITORY_ERROR, msg.str());
	            }

            model<number>* m = ptk->get_model();

            // construct a callback for the integrator to push new batches to the master
            generic_batcher::container_dispatch_function dispatcher = std::bind(&slave_controller<number>::push_temp_container, this, std::placeholders::_1,
                                                                                MPI::POSTINTEGRATION_DATA_READY, std::string("POSTINTEGRATION_DATA_READY"));

            // construct batcher to hold output
            zeta_threepf_batcher<number> batcher = this->data_mgr->create_temp_zeta_threepf_container(payload.get_tempdir_path(), payload.get_logdir_path(), this->get_rank(), m, dispatcher);

            if(z3pf->is_paired())
              {
                // also need a callback for the paired integrator
                generic_batcher::container_dispatch_function i_dispatcher = std::bind(&slave_controller<number>::push_temp_container, this, std::placeholders::_1,
                                                                                      MPI::INTEGRATION_DATA_READY, std::string("INTEGRATION_DATA_READY"));

                // construct a batcher to hold integration output
                threepf_batcher<number> i_batcher = this->data_mgr->create_temp_threepf_container(ptk, payload.get_paired_tempdir_path(), payload.get_paired_logdir_path(), this->get_rank(), payload.get_paired_workgroup_number(), m, i_dispatcher);

                // pair batchers
                i_batcher.pair(&batcher);

                // write log header
                boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
                BOOST_LOG_SEV(i_batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW PAIRED POSTINTEGRATION TASKS '" << tk->get_name() << "' & '" << ptk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
                BOOST_LOG_SEV(i_batcher.get_log(), generic_batcher::log_severity_level::normal) << *ptk;

                this->schedule_integration(ptk, m, i_batcher, m->backend_threepf_state_size());
              }
            else
              {
                // write log header
                boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
                BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW POSTINTEGRATION TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
                BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << *tk;

                this->schedule_postintegration(z3pf, ptk, payload, batcher);
              }
	        }
        else if((zfNL = dynamic_cast<fNL_task<number>*>(tk)) != nullptr)
	        {
		        // get parent task
            zeta_threepf_task<number>* ptk = dynamic_cast<zeta_threepf_task<number>*>(zfNL->get_parent_task());

            assert(ptk != nullptr);
            if(ptk == nullptr)
	            {
                std::ostringstream msg;
                msg << CPPTRANSPORT_EXPECTED_ZETA_THREEPF_TASK << " '" << zfNL->get_parent_task()->get_name() << "'";
                throw runtime_exception(exception_type::REPOSITORY_ERROR, msg.str());
	            }

            // get parent^2 task
            threepf_task<number>* pptk = dynamic_cast<threepf_task<number>*>(ptk->get_parent_task());

            assert(pptk != nullptr);
            if(pptk == nullptr)
              {
                std::ostringstream msg;
                msg << CPPTRANSPORT_EXPECTED_THREEPF_TASK << " '" << ptk->get_parent_task()->get_name() << "'";
                throw runtime_exception(exception_type::REPOSITORY_ERROR, msg.str());
              }

            model<number>* m = pptk->get_model();

            // construct a callback for the integrator to push new batches to the master
            generic_batcher::container_dispatch_function dispatcher = std::bind(&slave_controller<number>::push_temp_container, this, std::placeholders::_1,
                                                                                MPI::POSTINTEGRATION_DATA_READY, std::string("POSTINTEGRATION_DATA_READY"));

            // construct batcher to hold output
            fNL_batcher<number> batcher = this->data_mgr->create_temp_fNL_container(payload.get_tempdir_path(), payload.get_logdir_path(), this->get_rank(), m, dispatcher, zfNL->get_template());

            this->schedule_postintegration(zfNL, ptk, payload, batcher);
	        }
        else
	        {
            std::ostringstream msg;
            msg << CPPTRANSPORT_UNKNOWN_DERIVED_TASK << " '" << tk->get_name() << "'";
            throw runtime_exception(exception_type::REPOSITORY_ERROR, msg.str());
	        }
	    }


    template <typename number>
    template <typename TaskObject, typename ParentTaskObject, typename BatchObject>
    void slave_controller<number>::schedule_postintegration(TaskObject* tk, ParentTaskObject* ptk,
                                                            const MPI::new_postintegration_payload& payload, BatchObject& batcher)
	    {
        assert(tk != nullptr);    // should be guaranteed
        assert(ptk != nullptr);   // should be guaranteed

        // write log header
        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW POSTINTEGRATION TASK '" << tk->get_name() << "' | initiated at " << boost::posix_time::to_simple_string(now) << '\n';
        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << *tk;

        // set up output-group finder function
        typename datapipe<number>::integration_content_finder     i_finder = std::bind(&repository<number>::find_integration_task_output, this->repo, std::placeholders::_1, std::placeholders::_2);
        typename datapipe<number>::postintegration_content_finder p_finder = std::bind(&repository<number>::find_postintegration_task_output, this->repo, std::placeholders::_1, std::placeholders::_2);

        // set up empty content-dispatch function -- this datapipe is not used to produce content
        typename datapipe<number>::dispatch_function dispatcher = std::bind(&slave_controller<number>::disallow_push_content, this, std::placeholders::_1, std::placeholders::_2);

        // acquire a datapipe which we can use to stream content from the databse
        std::unique_ptr< datapipe<number> > pipe = this->data_mgr->create_datapipe(payload.get_logdir_path(), payload.get_tempdir_path(), i_finder, p_finder, dispatcher, this->get_rank(), true);

		    bool complete = false;
		    while(!complete)
			    {
				    // wait for messages from scheduler
		        boost::mpi::status stat = this->world.probe(MPI::RANK_MASTER);

				    switch(stat.tag())
					    {
				        case MPI::NEW_WORK_ASSIGNMENT:
					        {
				            MPI::work_assignment_payload assignment_payload;
						        this->world.recv(stat.source(), MPI::NEW_WORK_ASSIGNMENT, assignment_payload);

				            MPI::work_acknowledgment_payload ack_payload;
				            ack_payload.set_timestamp();
				            this->world.isend(MPI::RANK_MASTER, MPI::NEW_WORK_ACKNOWLEDGMENT, ack_payload);

				            const std::list<unsigned int>& work_items = assignment_payload.get_items();
						        auto filter = this->work_item_filter_factory(ptk, work_items);

						        // create work queues
						        context ctx;
				            ctx.add_device("CPU");
						        scheduler sch(ctx);
						        auto work = sch.make_queue(sizeof(number), *ptk, filter);

				            bool success = true;
						        batcher.begin_assignment();

				            // keep track of wallclock time
				            boost::timer::cpu_timer timer;

				            BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- NEW WORK ASSIGNMENT";

				            // perform the task
                    std::string group;
				            try
					            {
                        group = pipe->attach(ptk, payload.get_tags());
				                this->work_handler.postintegration_handler(tk, ptk, work, batcher, *pipe);
				                pipe->detach();
					            }
				            catch(runtime_exception& xe)
					            {
				                success = false;
				                BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::error) << "-- Exception reported during postintegration: code=" << static_cast<int>(xe.get_exception_code()) << ": " << xe.what();
					            }

				            // inform the batcher we are at the end of this assignment
				            batcher.end_assignment();

				            // all work is now done - stop the wallclock timer
				            timer.stop();

				            // notify master process that all work has been finished (temporary containers will be deleted by the master node)
				            now = boost::posix_time::second_clock::universal_time();
				            if(success) BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- Worker sending FINISHED_POSTINTEGRATION to master | finished at " << boost::posix_time::to_simple_string(now);
				            else        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::error)  << '\n' << "-- Worker reporting POSTINTEGRATION_FAIL to master | finished at " << boost::posix_time::to_simple_string(now);

				            MPI::finished_postintegration_payload outgoing_payload(group, pipe->get_database_time(), timer.elapsed().wall,
				                                                                   batcher.get_items_processed(), batcher.get_processing_time(),
				                                                                   batcher.get_max_processing_time(), batcher.get_min_processing_time(),
				                                                                   pipe->get_time_config_cache_hits(), pipe->get_time_config_cache_unloads(),
				                                                                   pipe->get_twopf_kconfig_cache_hits(), pipe->get_twopf_kconfig_cache_unloads(),
				                                                                   pipe->get_threepf_kconfig_cache_hits(), pipe->get_threepf_kconfig_cache_unloads(),
                                                                           pipe->get_stats_cache_hits(), pipe->get_stats_cache_unloads(),
				                                                                   pipe->get_data_cache_hits(), pipe->get_data_cache_unloads(),
				                                                                   pipe->get_time_config_cache_evictions(), pipe->get_twopf_kconfig_cache_evictions(),
				                                                                   pipe->get_threepf_kconfig_cache_evictions(), pipe->get_stats_cache_evictions(),
                                                                           pipe->get_data_cache_evictions());

				            this->world.isend(MPI::RANK_MASTER, success ? MPI::FINISHED_POSTINTEGRATION : MPI::POSTINTEGRATION_FAIL, outgoing_payload);

						        break;
					        }

				        case MPI::END_OF_WORK:
					        {
						        this->world.recv(stat.source(), MPI::END_OF_WORK);
						        complete = true;
						        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- Notified of end-of-work: preparing to shut down";

						        // close the batcher, flushing the current container to the master node if required
						        batcher.close();

						        // send close-down acknowledgment to master
						        now = boost::posix_time::second_clock::universal_time();
						        BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << '\n' << "-- Worker sending WORKER_CLOSE_DOWN to master | close down at " << boost::posix_time::to_simple_string(now);
						        this->world.isend(MPI::RANK_MASTER, MPI::WORKER_CLOSE_DOWN);

						        break;
					        }

				        default:
					        {
				            BOOST_LOG_SEV(batcher.get_log(), generic_batcher::log_severity_level::normal) << "!! Received unexpected MPI message " << stat.tag() << " from master node; discarding";
				            this->world.recv(stat.source(), stat.tag());
				            break;
					        }
					    };
			    }
	    }


    template <typename number>
    void slave_controller<number>::push_temp_container(generic_batcher* batcher, unsigned int message, std::string log_message)
	    {
        assert(batcher != nullptr);
        if(batcher == nullptr) throw runtime_exception(exception_type::DATA_CONTAINER_ERROR, CPPTRANSPORT_DATAMGR_NULL_BATCHER);

        BOOST_LOG_SEV(batcher->get_log(), generic_batcher::log_severity_level::normal) << "-- Sending " << log_message << " message for container " << batcher->get_container_path();

        MPI::data_ready_payload payload(batcher->get_container_path().string());

        // advise master process that data is available in the named container
        this->world.isend(MPI::RANK_MASTER, message, payload);
	    }


    template <typename number>
    void slave_controller<number>::push_derived_content(datapipe<number>* pipe, typename derived_data::derived_product<number>* product,
                                                        const std::list<std::string>& used_groups)
	    {
        assert(pipe != nullptr);
        assert(product != nullptr);

        // FIXME: error message tag is possibly in the wrong namespace (but error message namespaces are totally confused anyway)
        if(pipe == nullptr) throw runtime_exception(exception_type::DATAPIPE_ERROR, CPPTRANSPORT_DATAMGR_NULL_DATAPIPE);
        if(product == nullptr) throw runtime_exception(exception_type::DATAPIPE_ERROR, CPPTRANSPORT_DATAMGR_NULL_DERIVED_PRODUCT);

        BOOST_LOG_SEV(pipe->get_log(), datapipe<number>::log_severity_level::normal) << "-- Sending DERIVED_CONTENT_READY message for derived product '" << product->get_name() << "'";

        boost::filesystem::path product_filename = pipe->get_abs_tempdir_path() / product->get_filename();
        if(boost::filesystem::exists(product_filename))
	        {
            MPI::content_ready_payload payload(product->get_name(), used_groups);
            this->world.isend(MPI::RANK_MASTER, MPI::DERIVED_CONTENT_READY, payload);
	        }
        else
	        {
            std::ostringstream msg;
            msg << CPPTRANSPORT_DATAMGR_DERIVED_PRODUCT_MISSING << " " << product_filename;
            throw runtime_exception(exception_type::DATAPIPE_ERROR, msg.str());
	        }
	    }


    template <typename number>
    void slave_controller<number>::disallow_push_content(datapipe<number>* pipe, typename derived_data::derived_product<number>* product)
	    {
        assert(false);
	    }

	}


#endif //CPPTRANSPORT_SLAVE_CONTROLLER_H
