//
// Created by David Seery on 08/01/2014.
// Copyright (c) 2014-15 University of Sussex. All rights reserved.
//


#ifndef __data_manager_sqlite3_H_
#define __data_manager_sqlite3_H_


#include <sstream>
#include <list>
#include <string>
#include <vector>

#include "transport-runtime-api/repository/repository.h"
#include "transport-runtime-api/data/data_manager.h"

#include "transport-runtime-api/models/model.h"

#include "transport-runtime-api/utilities/formatter.h"

#include "transport-runtime-api/messages.h"
#include "transport-runtime-api/exceptions.h"

#include "boost/filesystem/operations.hpp"
#include "boost/timer/timer.hpp"

#include "sqlite3.h"
#include "transport-runtime-api/sqlite3/sqlite3_operations.h"


#define __CPP_TRANSPORT_TEMPORARY_CONTAINER_STEM "worker"
#define __CPP_TRANSPORT_TEMPORARY_CONTAINER_XTN  ".sqlite"


namespace transport
  {

    namespace
      {

        const std::string default_excpt_a = __CPP_TRANSPORT_DATACTR_OPEN_A;
        const std::string default_excpt_b = __CPP_TRANSPORT_DATACTR_OPEN_B;

      }   // unnamed namespace


    // implements the data_manager interface using sqlite3
    template <typename number>
    class data_manager_sqlite3: public data_manager<number>
      {

        // CONSTRUCTOR, DESTRUCTOR

      public:

        //! Create a data_manager_sqlite3 instance
        data_manager_sqlite3(unsigned int bcap, unsigned int dcap, unsigned int zcap)
          : data_manager<number>(bcap, dcap, zcap), temporary_container_serial(0)
          {
          }

        //! Destroy a data_manager_sqlite3 instance
        ~data_manager_sqlite3();


        // WRITER HANDLONG -- implements a 'data_manager' interface

      public:

        //! Initialize a new integration_writer object, including the data container.
        //! Never overwrites existing data; if the container already exists, an exception is thrown
        virtual void initialize_writer(std::shared_ptr< integration_writer<number> >& writer) override;

        //! Close an open integration_writer object.

        //! Any open sqlite3 handles are closed, meaning that any integration_writer objects will be invalidated.
        //! After closing, attempting to use an integration_writer will lead to unsubtle errors.
        virtual void close_writer(std::shared_ptr< integration_writer<number> >& writer) override;

		    //! Initialize a new derived_content_writer object.
		    virtual void initialize_writer(std::shared_ptr< derived_content_writer<number> >& writer) override;

		    //! Close an open derived_content_writer object.

		    //! Any open sqlite3 handles are closed. Attempting to use the writer after closing
		    //! will lead to unsubtle errors.
		    virtual void close_writer(std::shared_ptr< derived_content_writer<number> >& writer) override;

        //! Initialize a new postintegration_writer object.
        virtual void initialize_writer(std::shared_ptr< postintegration_writer<number> >& writer) override;

        //! Close an open postintegration_writer object
        virtual void close_writer(std::shared_ptr< postintegration_writer<number> >& writer) override;


        // WRITE INDEX TABLES -- implements a 'data_manager' interface

      public:

        //! Create tables needed for a twopf container
        virtual void create_tables(std::shared_ptr< integration_writer<number> >& writer, twopf_task<number>* tk) override;

        //! Create tables needed for a threepf container
        virtual void create_tables(std::shared_ptr< integration_writer<number> >& writer, threepf_task<number>* tk) override;

        //! Create tables needed for a zeta twopf container
        virtual void create_tables(std::shared_ptr< postintegration_writer<number> >& writer, zeta_twopf_task<number>* tk) override;

        //! Create tables needed for a zeta threepf container
        virtual void create_tables(std::shared_ptr< postintegration_writer<number> >& writer, zeta_threepf_task<number>* tk) override;

        //! Create tables needed for an fNL container
        virtual void create_tables(std::shared_ptr< postintegration_writer<number> >& writer, fNL_task<number>* tk) override;


        // TEMPORARY CONTAINERS  -- implements a 'data_manager' interface

      public:

        //! Create a temporary container for twopf data. Returns a batcher which can be used for writing to the container.
        virtual typename data_manager<number>::twopf_batcher create_temp_twopf_container(const boost::filesystem::path& tempdir,
                                                                                         const boost::filesystem::path& logdir,
                                                                                         unsigned int worker, model<number>* m,
                                                                                         typename data_manager<number>::container_dispatch_function dispatcher) override;

        //! Create a temporary container for threepf data. Returns a batcher which can be used for writing to the container.
        virtual typename data_manager<number>::threepf_batcher create_temp_threepf_container(const boost::filesystem::path& tempdir,
                                                                                             const boost::filesystem::path& logdir,
                                                                                             unsigned int worker, model<number>* m,
                                                                                             typename data_manager<number>::container_dispatch_function dispatcher) override;

        //! Create a temporary container for zeta twopf data. Returns a batcher which can be used for writing to the container.
        virtual typename data_manager<number>::zeta_twopf_batcher create_temp_zeta_twopf_container(const boost::filesystem::path& tempdir,
                                                                                                   const boost::filesystem::path& logdir,
                                                                                                   unsigned int worker,
                                                                                                   typename data_manager<number>::container_dispatch_function dispatcher) override;

        //! Create a temporary container for zeta threepf data. Returns a batcher which can be used for writing to the container.
        virtual typename data_manager<number>::zeta_threepf_batcher create_temp_zeta_threepf_container(const boost::filesystem::path& tempdir,
                                                                                                       const boost::filesystem::path& logdir,
                                                                                                       unsigned int worker,
                                                                                                       typename data_manager<number>::container_dispatch_function dispatcher) override;

        //! Create a temporary container for fNL data. Returns a batcher which can be used for writing to the container.
        virtual typename data_manager<number>::fNL_batcher create_temp_fNL_container(const boost::filesystem::path& tempdir,
                                                                                     const boost::filesystem::path& logdir,
                                                                                     unsigned int worker,
                                                                                     typename data_manager<number>::container_dispatch_function dispatcher,
                                                                                     derived_data::template_type type) override;

      protected:

        //! Aggregate a temporary twopf container into a principal container
        bool aggregate_twopf_batch(base_writer& writer, const std::string& temp_ctr);

        //! Aggregate a temporary threepf container into a principal container
        bool aggregate_threepf_batch(base_writer& writer, const std::string& temp_ctr);

        //! Aggregate a derived product
        bool aggregate_derived_product(base_writer& writer, const std::string& temp_name);

        //! Aggregate a temporary zeta_twopf container
        bool aggregate_zeta_twopf_batch(base_writer& writer, const std::string& temp_ctr);

        //! Aggregate a temporary zeta_threepf container
        bool aggregate_zeta_threepf_batch(base_writer& writer, const std::string& temp_ctr);

        //! Aggregate a temporary fNL container
        bool aggregate_fNL_batch(base_writer& writer, const std::string& temp_ctr, derived_data::template_type type);


        // DATA PIPES -- implements a 'data_manager' interface

      public:

        //! Create a new datapipe
        virtual typename data_manager<number>::datapipe create_datapipe(const boost::filesystem::path& logdir,
                                                                        const boost::filesystem::path& tempdir,
                                                                        typename data_manager<number>::datapipe::output_group_finder finder,
                                                                        typename data_manager<number>::datapipe::dispatch_function dispatcher,
                                                                        unsigned int worker, bool no_log=false) override;

        //! Pull a set of time sample-points from a datapipe
        virtual void pull_time_config(typename data_manager<number>::datapipe* pipe,
                                      const std::vector<unsigned int>& serial_numbers, std::vector<double>& sample) override;

        //! Pull a set of 2pf k-configuration serial numbers from a datapipe
        void pull_kconfig_twopf(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& serial_numbers,
                                typename std::vector<typename data_manager<number>::twopf_configuration>& sample) override;

        //! Pull a set of 3pd k-configuration serial numbesr from a datapipe
        //! Simultaneously, populates three lists (k1, k2, k3) with serial numbers for the 2pf k-configurations
        //! corresponding to k1, k2, k3
        void pull_kconfig_threepf(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& serial_numbers,
                                  typename std::vector<typename data_manager<number>::threepf_configuration>& sample) override;

        //! Pull a time sample of a background field from a datapipe
        virtual void pull_background_time_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                                 const std::vector<unsigned int>& t_serials, std::vector<number>& sample) override;

        //! Pull a time sample of a twopf component at fixed k-configuration from a datapipe
        virtual void pull_twopf_time_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                            const std::vector<unsigned int>& t_serials, unsigned int k_serial,
                                            std::vector<number>& sample, typename data_manager<number>::datapipe::twopf_type type) override;

        //! Pull a sample of a threepf at fixed k-configuration from a datapipe
        virtual void pull_threepf_time_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                              const std::vector<unsigned int>& t_serials,
                                              unsigned int k_serial, std::vector<number>& sample) override;

        //! Pull a sample of a tensor twopf component at fixed k-configuration from a datapipe
        virtual void pull_tensor_twopf_time_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                                   const std::vector<unsigned int>& t_serials,
                                                   unsigned int k_serial, std::vector<number>& sample) override;

        //! Pull a sample of the zeta twopf at fixed k-configuration from a datapipe
        virtual void pull_zeta_twopf_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                 unsigned int k_serial, std::vector<number>& sample) override;

        //! Pull a sample of the zeta threepf at fixed k-configuration from a datapipe
        virtual void pull_zeta_threepf_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                   unsigned int k_serial, std::vector<number>& sample) override;

        //! Pull a sample of the zeta reduced bispectrum at fixed k-configuration from a datapipe
        virtual void pull_zeta_redbsp_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                  unsigned int k_serial, std::vector<number>& sample) override;

        //! Pull a sample of fNL from a datapipe
        virtual void pull_fNL_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                          std::vector<number>& sample, derived_data::template_type type) override;

        //! Pull a sample of bispectrum.template from a datapipe
        virtual void pull_BT_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                          std::vector<number>& sample, derived_data::template_type type) override;

        //! Pull a sample of template.template from a datapipe
        virtual void pull_TT_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                          std::vector<number>& sample, derived_data::template_type type) override;

        //! Pull a kconfig sample of a twopf component at fixed time from a datapipe
        virtual void pull_twopf_kconfig_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                               const std::vector<unsigned int>& k_serials, unsigned int t_serial,
                                               std::vector<number>& sample, typename data_manager<number>::datapipe::twopf_type type) override;

        //! Pull a kconfig sample of a threepf at fixed time from a datapipe
        virtual void pull_threepf_kconfig_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                                 const std::vector<unsigned int>& k_serials,
                                                 unsigned int t_serial, std::vector<number>& sample) override;

        //! Pull a kconfig sample of a tensor twopf component at fixed time from a datapipe
        virtual void pull_tensor_twopf_kconfig_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                                      const std::vector<unsigned int>& k_serials,
                                                      unsigned int t_serial, std::vector<number>& sample) override;

        //! Pull a kconfig sample of the zeta twopf at fixed time from a datapipe
        virtual void pull_zeta_twopf_kconfig_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& k_serials,
                                                    unsigned int t_serial, std::vector<number>& sample) override;

        //! Pull a kconfig sample of the zeta threepf at fixed time from a datapipe
        virtual void pull_zeta_threepf_kconfig_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& k_serials,
                                                      unsigned int t_serial, std::vector<number>& sample) override;

        //! Pull a kconfig sample of the zeta reduced bispectrum at fixed time from a datapipe
        virtual void pull_zeta_redbsp_kconfig_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& k_serials,
                                                     unsigned int t_serial, std::vector<number>& sample) override;

      protected:

        //! Attach an output_group_record to a pipe
        std::shared_ptr< output_group_record<integration_payload> >
          datapipe_attach(typename data_manager<number>::datapipe* pipe,
                          typename data_manager<number>::datapipe::output_group_finder& finder,
                          const std::string& name, const std::list<std::string>& tags);

        //! Detach an output_group_record from a pipe
        void datapipe_detach(typename data_manager<number>::datapipe* pipe);


		    // INTERNAL UTILITY FUNCTIONS

      protected:

        //! Replace a temporary twopf container with a new one
        void replace_temp_twopf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                          model<number>* m, typename data_manager<number>::generic_batcher* batcher,
                                          typename data_manager<number>::replacement_action action);

        //! Replace a temporary threepf container with a new one
        void replace_temp_threepf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                            model<number>* m, typename data_manager<number>::generic_batcher* batcher,
                                            typename data_manager<number>::replacement_action action);

        //! Replace a temporary zeta twopf container with a new one
        void replace_temp_zeta_twopf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                               typename data_manager<number>::generic_batcher* batcher,
                                               typename data_manager<number>::replacement_action action);

        //! Replace a temporary zeta threepf container with a new one
        void replace_temp_zeta_threepf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                                 typename data_manager<number>::generic_batcher* batcher,
                                                 typename data_manager<number>::replacement_action action);

        //! Replace a temporary fNL container with a new one
        void replace_temp_fNL_container(const boost::filesystem::path& tempdir, unsigned int worker, derived_data::template_type type,
                                        typename data_manager<number>::generic_batcher* batcher,
                                        typename data_manager<number>::replacement_action action);

        //! Generate the name for a temporary container
        boost::filesystem::path generate_temporary_container_path(const boost::filesystem::path& tempdir, unsigned int worker);


        // INTERNAL DATA

      private:

        //! List of open sqlite3 connexions
        std::list< sqlite3* > open_containers;

        //! Serial number used to distinguish
        //! the set of temporary containers associated with this worker.
		    //! Begins at zero and is incremented as temporary containers are generated.
        unsigned int          temporary_container_serial;

      };


    // CONSTRUCTOR, DESTRUCTOR

    template <typename number>
    data_manager_sqlite3<number>::~data_manager_sqlite3()
      {
        for(std::list<sqlite3*>::iterator t = this->open_containers.begin(); t != this->open_containers.end(); t++)
          {
            int status = sqlite3_close(*t);

            if(status != SQLITE_OK)
              {
                if(status == SQLITE_BUSY) throw runtime_exception(runtime_exception::DATA_CONTAINER_ERROR, __CPP_TRANSPORT_DATACTR_NOT_CLOSED);
                else
                  {
                    std::ostringstream msg;
                    msg << __CPP_TRANSPORT_DATACTR_CLOSE << status << ")";
                    throw runtime_exception(runtime_exception::DATA_CONTAINER_ERROR, msg.str());
                  }
              }
          }
      }


    // Create data files for a new integration_writer object
    template <typename number>
    void data_manager_sqlite3<number>::initialize_writer(std::shared_ptr< integration_writer<number> >& writer)
      {
        sqlite3* db = nullptr;

        // get paths of the data container
        boost::filesystem::path ctr_path = writer->get_abs_container_path();

        // open the main container
        int status = sqlite3_open_v2(ctr_path.string().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

        if(status != SQLITE_OK)
	        {
            std::ostringstream msg;
            if(db != nullptr)
	            {
                msg << __CPP_TRANSPORT_DATACTR_CREATE_A << " '" << ctr_path.string() << "' " << __CPP_TRANSPORT_DATACTR_CREATE_B << status << ": " << sqlite3_errmsg(db) << ")";
                sqlite3_close(db);
	            }
            else
	            {
                msg << __CPP_TRANSPORT_DATACTR_CREATE_A << " '" << ctr_path.string() << "' " << __CPP_TRANSPORT_DATACTR_CREATE_B << status << ")";
	            }
            throw runtime_exception(runtime_exception::DATA_CONTAINER_ERROR, msg.str());
	        }

        sqlite3_extended_result_codes(db, 1);

        // enable foreign key constraints
        char* errmsg;
        sqlite3_exec(db, "PRAGMA foreign_keys = ON", nullptr, nullptr, &errmsg);

        // remember this connexion
        this->open_containers.push_back(db);
        writer->set_data_manager_handle(db);

        // set up aggregation handlers
        integration_task_record<number>* rec = writer->get_record();
        assert(rec != nullptr);

        integration_task<number>* tk = rec->get_task();

        twopf_task<number>* tk2 = nullptr;
        threepf_task<number>* tk3 = nullptr;
        if((tk2 = dynamic_cast<twopf_task<number>*>(tk)) != nullptr)
          {
            writer->set_aggregation_handler(std::bind(&data_manager_sqlite3<number>::aggregate_twopf_batch, this, std::placeholders::_1, std::placeholders::_2));
          }
        else if((tk3 = dynamic_cast<threepf_task<number>*>(tk)) != nullptr)
          {
            writer->set_aggregation_handler(std::bind(&data_manager_sqlite3<number>::aggregate_threepf_batch, this, std::placeholders::_1, std::placeholders::_2));
          }
        else
          {
            assert(false);
            throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATACTR_AGGREGATION_HANDLER_NOT_SET);
          }
      }


    // Close data files associated with an integration_writer object
    template <typename number>
    void data_manager_sqlite3<number>::close_writer(std::shared_ptr< integration_writer<number> >& writer)
      {
        // close sqlite3 handle to principal database
        sqlite3* db = nullptr;
        writer->get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        this->open_containers.remove(db);
        sqlite3_close(db);

        // physically remove the tempfiles directory
        boost::filesystem::remove(writer->get_abs_tempdir_path());
      }


		// Create data files for a new derived_content_writer object
		template <typename number>
		void data_manager_sqlite3<number>::initialize_writer(std::shared_ptr< derived_content_writer<number> >& writer)
			{
        // set up aggregation handler
        writer->set_aggregation_handler(std::bind(&data_manager_sqlite3<number>::aggregate_derived_product, this, std::placeholders::_1, std::placeholders::_2));
			}


		// Close data files for a derived_content_writer object
		template <typename number>
		void data_manager_sqlite3<number>::close_writer(std::shared_ptr< derived_content_writer<number> >& writer)
			{
				// physically remove the tempfiles directory
		    boost::filesystem::remove(writer->get_abs_tempdir_path());
			}


    // Initialize a new postintegration_writer object
    template <typename number>
    void data_manager_sqlite3<number>::initialize_writer(std::shared_ptr< postintegration_writer<number> >& writer)
      {
        sqlite3* db = nullptr;

        // get paths to the data container
        boost::filesystem::path ctr_path = writer->get_abs_container_path();

        // open the main container
        int status = sqlite3_open_v2(ctr_path.string().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

        if(status != SQLITE_OK)
          {
            std::ostringstream msg;
            if(db != nullptr)
              {
                msg << __CPP_TRANSPORT_DATACTR_CREATE_A << " '" << ctr_path.string() << "' " << __CPP_TRANSPORT_DATACTR_CREATE_B << status << ": " << sqlite3_errmsg(db) << ")";
                sqlite3_close(db);
              }
            else
              {
                msg << __CPP_TRANSPORT_DATACTR_CREATE_A << " '" << ctr_path.string() << "' " << __CPP_TRANSPORT_DATACTR_CREATE_B << status << ")";
              }
            throw runtime_exception(runtime_exception::DATA_CONTAINER_ERROR, msg.str());
          }

        sqlite3_extended_result_codes(db, 1);
        // leave foreign keys disabled

        // remember this connexion
        this->open_containers.push_back(db);
        writer->set_data_manager_handle(db);

        // set aggregation handler
        postintegration_task_record<number> * rec = writer->get_record();
        assert(rec != nullptr);

        postintegration_task<number>* tk = rec->get_task();

        zeta_twopf_task<number>* z2pf = nullptr;
        zeta_threepf_task<number>* z3pf = nullptr;
        fNL_task<number>* zfNL = nullptr;
        if((z2pf = dynamic_cast<zeta_twopf_task<number>*>(tk)) != nullptr)
          {
            writer->set_aggregation_handler(std::bind(&data_manager_sqlite3<number>::aggregate_zeta_twopf_batch, this, std::placeholders::_1, std::placeholders::_2));
          }
        else if((z3pf = dynamic_cast<zeta_threepf_task<number>*>(tk)) != nullptr)
          {
            writer->set_aggregation_handler(std::bind(&data_manager_sqlite3<number>::aggregate_zeta_threepf_batch, this, std::placeholders::_1, std::placeholders::_2));
          }
        else if((zfNL = dynamic_cast<fNL_task<number>*>(tk)) != nullptr)
          {
            writer->set_aggregation_handler(std::bind(&data_manager_sqlite3<number>::aggregate_fNL_batch, this, std::placeholders::_1, std::placeholders::_2, zfNL->get_template()));
          }
        else
          {
            assert(false);
            throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATACTR_AGGREGATION_HANDLER_NOT_SET);
          }

		    typename postintegration_writer<number>::merge_group mergers;
		    mergers.zeta_twopf   = std::bind(&sqlite3_operations::merge_zeta_twopf, std::placeholders::_1, std::placeholders::_2);
		    mergers.zeta_threepf = std::bind(&sqlite3_operations::merge_zeta_threepf, std::placeholders::_1, std::placeholders::_2);
				mergers.zeta_redbsp  = std::bind(&sqlite3_operations::merge_zeta_redbsp, std::placeholders::_1, std::placeholders::_2);
		    mergers.fNL          = std::bind(&sqlite3_operations::merge_fNL, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		    writer->set_merge_handlers(mergers);
      }


    // Close a postintegration_writer object
    template <typename number>
    void data_manager_sqlite3<number>::close_writer(std::shared_ptr< postintegration_writer<number> >& writer)
      {
        // close sqlite3 handle to principal database
        sqlite3* db = nullptr;
        writer->get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        this->open_containers.remove(db);
        sqlite3_close(db);

        // physically remove the tempfiles directory
        boost::filesystem::remove(writer->get_abs_tempdir_path());
      }


    // INDEX TABLE MANAGEMENT

    template <typename number>
    void data_manager_sqlite3<number>::create_tables(std::shared_ptr< integration_writer<number> >& writer, twopf_task<number>* tk)
      {
        sqlite3* db = nullptr;
        writer->get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        unsigned int Nfields = tk->get_model()->get_N_fields();

        sqlite3_operations::create_time_sample_table(db, tk);
        sqlite3_operations::create_twopf_sample_table(db, tk);
        sqlite3_operations::create_backg_table(db, Nfields, sqlite3_operations::foreign_keys);
        sqlite3_operations::create_twopf_table(db, Nfields, sqlite3_operations::real_twopf, sqlite3_operations::foreign_keys);
        sqlite3_operations::create_tensor_twopf_table(db, sqlite3_operations::foreign_keys);

        sqlite3_operations::create_worker_info_table(db, sqlite3_operations::foreign_keys);
        if(writer->collect_statistics()) sqlite3_operations::create_stats_table(db, sqlite3_operations::foreign_keys, sqlite3_operations::twopf_configs);
      }


    template <typename number>
    void data_manager_sqlite3<number>::create_tables(std::shared_ptr< integration_writer<number> >& writer, threepf_task<number>* tk)
      {
        sqlite3* db = nullptr;
        writer->get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        unsigned int Nfields = tk->get_model()->get_N_fields();

        sqlite3_operations::create_time_sample_table(db, tk);
        sqlite3_operations::create_twopf_sample_table(db, tk);
        sqlite3_operations::create_threepf_sample_table(db, tk);
        sqlite3_operations::create_backg_table(db, Nfields, sqlite3_operations::foreign_keys);
        sqlite3_operations::create_twopf_table(db, Nfields, sqlite3_operations::real_twopf, sqlite3_operations::foreign_keys);
        sqlite3_operations::create_twopf_table(db, Nfields, sqlite3_operations::imag_twopf, sqlite3_operations::foreign_keys);
        sqlite3_operations::create_tensor_twopf_table(db, sqlite3_operations::foreign_keys);
        sqlite3_operations::create_threepf_table(db, Nfields, sqlite3_operations::foreign_keys);

        sqlite3_operations::create_worker_info_table(db, sqlite3_operations::foreign_keys);
        if(writer->collect_statistics()) sqlite3_operations::create_stats_table(db, sqlite3_operations::foreign_keys, sqlite3_operations::threepf_configs);
      }


    template <typename number>
    void data_manager_sqlite3<number>::create_tables(std::shared_ptr< postintegration_writer<number> >& writer, zeta_twopf_task<number>* tk)
      {
        sqlite3* db = nullptr;
        writer->get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::create_zeta_twopf_table(db, sqlite3_operations::no_foreign_keys);
      }


    template <typename number>
    void data_manager_sqlite3<number>::create_tables(std::shared_ptr< postintegration_writer<number> >& writer, zeta_threepf_task<number>* tk)
      {
        sqlite3* db = nullptr;
        writer->get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::create_zeta_twopf_table(db, sqlite3_operations::no_foreign_keys);
        sqlite3_operations::create_zeta_threepf_table(db, sqlite3_operations::no_foreign_keys);
        sqlite3_operations::create_zeta_reduced_bispectrum_table(db, sqlite3_operations::no_foreign_keys);
      }


    template <typename number>
    void data_manager_sqlite3<number>::create_tables(std::shared_ptr< postintegration_writer<number> >& writer, fNL_task<number>* tk)
      {
        sqlite3* db = nullptr;
        writer->get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::create_fNL_table(db, tk->get_template(), sqlite3_operations::no_foreign_keys);
      }


    // TEMPORARY CONTAINERS

    template <typename number>
    typename data_manager<number>::twopf_batcher
    data_manager_sqlite3<number>::create_temp_twopf_container(const boost::filesystem::path& tempdir, const boost::filesystem::path& logdir,
                                                              unsigned int worker, model<number>* m,
                                                              typename data_manager<number>::container_dispatch_function dispatcher)
      {
        boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

        sqlite3* db = sqlite3_operations::create_temp_twopf_container(container, m->get_N_fields(), m->supports_per_configuration_statistics());

        // set up writers
        typename data_manager<number>::twopf_batcher::writer_group writers;
        writers.host_info    = std::bind(&sqlite3_operations::write_host_info<number>, std::placeholders::_1);
        writers.stats        = std::bind(&sqlite3_operations::write_stats<number>, std::placeholders::_1, std::placeholders::_2);
        writers.backg        = std::bind(&sqlite3_operations::write_backg<number>, std::placeholders::_1, std::placeholders::_2);
        writers.twopf        = std::bind(&sqlite3_operations::write_twopf<number>, sqlite3_operations::real_twopf, std::placeholders::_1, std::placeholders::_2);
        writers.tensor_twopf = std::bind(&sqlite3_operations::write_tensor_twopf<number>, std::placeholders::_1, std::placeholders::_2);

        // set up a replacement function
        typename data_manager<number>::container_replacement_function replacer =
                                                                        std::bind(&data_manager_sqlite3<number>::replace_temp_twopf_container,
                                                                                  this, tempdir, worker, m,
                                                                                  std::placeholders::_1, std::placeholders::_2);

        // set up batcher
        typename data_manager<number>::twopf_batcher batcher(this->batcher_capacity, m->get_N_fields(), container, logdir, writers, dispatcher, replacer, db, worker, m->supports_per_configuration_statistics());

        BOOST_LOG_SEV(batcher.get_log(), data_manager<number>::normal) << "** Created new temporary twopf container " << container;

        // add this database to our list of open connections
        this->open_containers.push_back(db);

        return(batcher);
      }


    template <typename number>
    typename data_manager<number>::threepf_batcher
    data_manager_sqlite3<number>::create_temp_threepf_container(const boost::filesystem::path& tempdir, const boost::filesystem::path& logdir,
                                                                unsigned int worker, model<number>* m,
                                                                typename data_manager<number>::container_dispatch_function dispatcher)
      {
        boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

        sqlite3* db = sqlite3_operations::create_temp_threepf_container(container, m->get_N_fields(), m->supports_per_configuration_statistics());

        // set up writers
        typename data_manager<number>::threepf_batcher::writer_group writers;
		    writers.host_info    = std::bind(&sqlite3_operations::write_host_info<number>, std::placeholders::_1);
        writers.stats        = std::bind(&sqlite3_operations::write_stats<number>, std::placeholders::_1, std::placeholders::_2);
        writers.backg        = std::bind(&sqlite3_operations::write_backg<number>, std::placeholders::_1, std::placeholders::_2);
        writers.twopf_re     = std::bind(&sqlite3_operations::write_twopf<number>, sqlite3_operations::real_twopf, std::placeholders::_1, std::placeholders::_2);
        writers.twopf_im     = std::bind(&sqlite3_operations::write_twopf<number>, sqlite3_operations::imag_twopf, std::placeholders::_1, std::placeholders::_2);
        writers.tensor_twopf = std::bind(&sqlite3_operations::write_tensor_twopf<number>, std::placeholders::_1, std::placeholders::_2);
        writers.threepf      = std::bind(&sqlite3_operations::write_threepf<number>, std::placeholders::_1, std::placeholders::_2);

        // set up a replacement function
        typename data_manager<number>::container_replacement_function replacer =
                                                                        std::bind(&data_manager_sqlite3<number>::replace_temp_threepf_container,
                                                                                  this, tempdir, worker, m,
                                                                                  std::placeholders::_1, std::placeholders::_2);

        // set up batcher
        typename data_manager<number>::threepf_batcher batcher(this->batcher_capacity, m->get_N_fields(), container, logdir, writers, dispatcher, replacer, db, worker, m->supports_per_configuration_statistics());

        BOOST_LOG_SEV(batcher.get_log(), data_manager<number>::normal) << "** Created new temporary threepf container " << container;

        // add this database to our list of open connections
        this->open_containers.push_back(db);

        return(batcher);
      }


    template <typename number>
    typename data_manager<number>::zeta_twopf_batcher
    data_manager_sqlite3<number>::create_temp_zeta_twopf_container(const boost::filesystem::path& tempdir, const boost::filesystem::path& logdir, unsigned int worker,
                                                                   typename data_manager<number>::container_dispatch_function dispatcher)
      {
        boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

        sqlite3* db = sqlite3_operations::create_temp_zeta_twopf_container(container);

        // set up writers
        typename data_manager<number>::zeta_twopf_batcher::writer_group writers;
        writers.twopf = std::bind(&sqlite3_operations::write_zeta_twopf<number>, std::placeholders::_1, std::placeholders::_2);

        // set up replacement function
        typename data_manager<number>::container_replacement_function replacer =
                                                                        std::bind(&data_manager_sqlite3<number>::replace_temp_zeta_twopf_container,
                                                                                  this, tempdir, worker, std::placeholders::_1, std::placeholders::_2);

        // set up batcher
        typename data_manager<number>::zeta_twopf_batcher batcher(this->batcher_capacity, container, logdir, writers, dispatcher, replacer, db, worker);

        BOOST_LOG_SEV(batcher.get_log(), data_manager<number>::normal) << "** Created new temporary zeta twopf container " << container;

        // add this database to our list of open connections
        this->open_containers.push_back(db);

        return(batcher);
      }


    template <typename number>
    typename data_manager<number>::zeta_threepf_batcher
    data_manager_sqlite3<number>::create_temp_zeta_threepf_container(const boost::filesystem::path& tempdir, const boost::filesystem::path& logdir, unsigned int worker,
                                                                     typename data_manager<number>::container_dispatch_function dispatcher)
      {
        boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

        sqlite3* db = sqlite3_operations::create_temp_zeta_threepf_container(container);

        // set up writers
        typename data_manager<number>::zeta_threepf_batcher::writer_group writers;
        writers.twopf   = std::bind(&sqlite3_operations::write_zeta_twopf<number>, std::placeholders::_1, std::placeholders::_2);
        writers.threepf = std::bind(&sqlite3_operations::write_zeta_threepf<number>, std::placeholders::_1, std::placeholders::_2);
        writers.redbsp  = std::bind(&sqlite3_operations::write_zeta_redbsp<number>, std::placeholders::_1, std::placeholders::_2);

        // set up replacement function
        typename data_manager<number>::container_replacement_function replacer =
                                                                        std::bind(&data_manager_sqlite3<number>::replace_temp_zeta_threepf_container,
                                                                                  this, tempdir, worker, std::placeholders::_1, std::placeholders::_2);

        // set up batcher
        typename data_manager<number>::zeta_threepf_batcher batcher(this->batcher_capacity, container, logdir, writers, dispatcher, replacer, db, worker);

        BOOST_LOG_SEV(batcher.get_log(), data_manager<number>::normal) << "** Created new temporary zeta threepf container " << container;

        // add this database to our list of open connections
        this->open_containers.push_back(db);

        return(batcher);
      }


    template <typename number>
    typename data_manager<number>::fNL_batcher
    data_manager_sqlite3<number>::create_temp_fNL_container(const boost::filesystem::path& tempdir, const boost::filesystem::path& logdir, unsigned int worker,
                                                            typename data_manager<number>::container_dispatch_function dispatcher, derived_data::template_type type)
      {
        boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

        sqlite3* db = sqlite3_operations::create_temp_fNL_container(container, type);

        // set up writers
        typename data_manager<number>::fNL_batcher::writer_group writers;
        writers.fNL = std::bind(&sqlite3_operations::write_fNL<number>, std::placeholders::_1, std::placeholders::_2, type);

        // set up replacement function
        typename data_manager<number>::container_replacement_function replacer =
                                                                        std::bind(&data_manager_sqlite3<number>::replace_temp_fNL_container,
                                                                                  this, tempdir, worker, type, std::placeholders::_1, std::placeholders::_2);

        // set up batcher
        typename data_manager<number>::fNL_batcher batcher(this->batcher_capacity, container, logdir, writers, dispatcher, replacer, db, worker, type);

        BOOST_LOG_SEV(batcher.get_log(), data_manager<number>::normal) << "** Created new temporary " << derived_data::template_type(type) << " container " << container;

        // add this database to our list of open connections
        this->open_containers.push_back(db);

        return(batcher);
      }


    template <typename number>
    void data_manager_sqlite3<number>::replace_temp_twopf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                                                    model<number>* m, typename data_manager<number>::generic_batcher* batcher,
                                                                    typename data_manager<number>::replacement_action action)
      {
        sqlite3* db = nullptr;

        BOOST_LOG_SEV(batcher->get_log(), data_manager<number>::normal)
            << "** " << (action == data_manager<number>::action_replace ? "Replacing" : "Closing")
            << " temporary twopf container " << batcher->get_container_path();

        batcher->get_manager_handle(&db);
        this->open_containers.remove(db);
        sqlite3_close(db);

        if(action == data_manager<number>::action_replace)
          {
            boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

            sqlite3* new_db = sqlite3_operations::create_temp_twopf_container(container, m->get_N_fields(), m->supports_per_configuration_statistics());

            batcher->set_container_path(container);
            batcher->set_manager_handle(new_db);

            this->open_containers.push_back(new_db);
          }
      }


    template <typename number>
    void data_manager_sqlite3<number>::replace_temp_threepf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                                                      model<number>* m, typename data_manager<number>::generic_batcher* batcher,
                                                                      typename data_manager<number>::replacement_action action)
      {
        sqlite3* db = nullptr;

        BOOST_LOG_SEV(batcher->get_log(), data_manager<number>::normal)
            << "** " << (action == data_manager<number>::action_replace ? "Replacing" : "Closing")
            << " temporary threepf container " << batcher->get_container_path();

        batcher->get_manager_handle(&db);
        this->open_containers.remove(db);
        sqlite3_close(db);

        BOOST_LOG_SEV(batcher->get_log(), data_manager<number>::normal) << "** Closed sqlite3 handle for " << batcher->get_container_path();

        if(action == data_manager<number>::action_replace)
          {
            boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

            BOOST_LOG_SEV(batcher->get_log(), data_manager<number>::normal) << "** Opening new threepf container " << container;

            sqlite3* new_db = sqlite3_operations::create_temp_threepf_container(container, m->get_N_fields(), m->supports_per_configuration_statistics());

            batcher->set_container_path(container);
            batcher->set_manager_handle(new_db);

            this->open_containers.push_back(new_db);
          }
      }


    template <typename number>
    void data_manager_sqlite3<number>::replace_temp_zeta_twopf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                                                         typename data_manager<number>::generic_batcher* batcher,
                                                                         typename data_manager<number>::replacement_action action)
      {
        sqlite3* db = nullptr;

        BOOST_LOG_SEV(batcher->get_log(), data_manager<number>::normal)
            << "** " << (action == data_manager<number>::action_replace ? "Replacing" : "Closing")
            << " temporary zeta twopf container " << batcher->get_container_path();

        batcher->get_manager_handle(&db);
        this->open_containers.remove(db);
        sqlite3_close(db);

        if(action == data_manager<number>::action_replace)
          {
            boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

            sqlite3* new_db = sqlite3_operations::create_temp_zeta_twopf_container(container);

            batcher->set_container_path(container);
            batcher->set_manager_handle(new_db);

            this->open_containers.push_back(new_db);
          }
      }


    template <typename number>
    void data_manager_sqlite3<number>::replace_temp_zeta_threepf_container(const boost::filesystem::path& tempdir, unsigned int worker,
                                                                           typename data_manager<number>::generic_batcher* batcher,
                                                                           typename data_manager<number>::replacement_action action)
      {
        sqlite3* db = nullptr;

        BOOST_LOG_SEV(batcher->get_log(), data_manager<number>::normal)
            << "** " << (action == data_manager<number>::action_replace ? "Replacing" : "Closing")
            << " temporary zeta threepf container " << batcher->get_container_path();

        batcher->get_manager_handle(&db);
        this->open_containers.remove(db);
        sqlite3_close(db);

        if(action == data_manager<number>::action_replace)
          {
            boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

            sqlite3* new_db = sqlite3_operations::create_temp_zeta_threepf_container(container);

            batcher->set_container_path(container);
            batcher->set_manager_handle(new_db);

            this->open_containers.push_back(new_db);
          }
      }


    template <typename number>
    void data_manager_sqlite3<number>::replace_temp_fNL_container(const boost::filesystem::path& tempdir, unsigned int worker, derived_data::template_type type,
                                                                  typename data_manager<number>::generic_batcher* batcher,
                                                                  typename data_manager<number>::replacement_action action)
      {
        sqlite3* db = nullptr;

        BOOST_LOG_SEV(batcher->get_log(), data_manager<number>::normal)
            << "** " << (action == data_manager<number>::action_replace ? "Replacing" : "Closing")
            << " temporary " << derived_data::template_name(type) << " container " << batcher->get_container_path();

        batcher->get_manager_handle(&db);
        this->open_containers.remove(db);
        sqlite3_close(db);

        if(action == data_manager<number>::action_replace)
          {
            boost::filesystem::path container = this->generate_temporary_container_path(tempdir, worker);

            sqlite3* new_db = sqlite3_operations::create_temp_fNL_container(container, type);

            batcher->set_container_path(container);
            batcher->set_manager_handle(new_db);

            this->open_containers.push_back(new_db);
          }
      }


    template <typename number>
    boost::filesystem::path data_manager_sqlite3<number>::generate_temporary_container_path(const boost::filesystem::path& tempdir, unsigned int worker)
      {
        std::ostringstream container_name;
        container_name << __CPP_TRANSPORT_TEMPORARY_CONTAINER_STEM << worker << "_" << this->temporary_container_serial++ << __CPP_TRANSPORT_TEMPORARY_CONTAINER_XTN;

        boost::filesystem::path container = tempdir / container_name.str();

        return(container);
      }


    template <typename number>
    bool data_manager_sqlite3<number>::aggregate_twopf_batch(base_writer& gwriter, const std::string& temp_ctr)
      {
        integration_writer<number>& writer = dynamic_cast< integration_writer<number>& >(gwriter);

        sqlite3* db = nullptr;
        writer.get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::aggregate_backg<number>(db, writer, temp_ctr);
        sqlite3_operations::aggregate_twopf<number>(db, writer, temp_ctr, sqlite3_operations::real_twopf);
        sqlite3_operations::aggregate_tensor_twopf<number>(db, writer, temp_ctr);

        sqlite3_operations::aggregate_workers<number>(db, writer, temp_ctr);
        if(writer.collect_statistics()) sqlite3_operations::aggregate_statistics<number>(db, writer, temp_ctr);

        return(true);
      }


    template <typename number>
    bool data_manager_sqlite3<number>::aggregate_threepf_batch(base_writer& gwriter, const std::string& temp_ctr)
      {
        integration_writer<number>& writer = dynamic_cast< integration_writer<number>& >(gwriter);

        sqlite3* db = nullptr;
        writer.get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::aggregate_backg<number>(db, writer, temp_ctr);
        sqlite3_operations::aggregate_twopf<number>(db, writer, temp_ctr, sqlite3_operations::real_twopf);
        sqlite3_operations::aggregate_twopf<number>(db, writer, temp_ctr, sqlite3_operations::imag_twopf);
        sqlite3_operations::aggregate_tensor_twopf<number>(db, writer, temp_ctr);
        sqlite3_operations::aggregate_threepf<number>(db, writer, temp_ctr);

        sqlite3_operations::aggregate_workers<number>(db, writer, temp_ctr);
        if(writer.collect_statistics()) sqlite3_operations::aggregate_statistics<number>(db, writer, temp_ctr);

        return(true);
      }


    template <typename number>
    bool data_manager_sqlite3<number>::aggregate_zeta_twopf_batch(base_writer& gwriter, const std::string& temp_ctr)
      {
        postintegration_writer<number>& writer = dynamic_cast< postintegration_writer<number>& >(gwriter);

        sqlite3* db = nullptr;
        writer.get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::aggregate_zeta_twopf<number>(db, writer, temp_ctr);

        return(true);
      }


    template <typename number>
    bool data_manager_sqlite3<number>::aggregate_zeta_threepf_batch(base_writer& gwriter, const std::string& temp_ctr)
      {
        postintegration_writer<number>& writer = dynamic_cast< postintegration_writer<number>& >(gwriter);

        sqlite3* db = nullptr;
        writer.get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::aggregate_zeta_twopf<number>(db, writer, temp_ctr);
        sqlite3_operations::aggregate_zeta_threepf<number>(db, writer, temp_ctr);
        sqlite3_operations::aggregate_zeta_reduced_bispectrum<number>(db, writer, temp_ctr);

        return(true);
      }


    template <typename number>
    bool data_manager_sqlite3<number>::aggregate_fNL_batch(base_writer& gwriter, const std::string& temp_ctr, derived_data::template_type type)
      {
        postintegration_writer<number>& writer = dynamic_cast< postintegration_writer<number>& >(gwriter);

        sqlite3* db = nullptr;
        writer.get_data_manager_handle(&db); // throws an exception if handle is unset, so the return value is guaranteed not to be nullptr

        sqlite3_operations::aggregate_fNL<number>(db, writer, temp_ctr, type);

        return(true);
      }


    template <typename number>
    bool data_manager_sqlite3<number>::aggregate_derived_product(base_writer& gwriter, const std::string& temp_name)
      {
        derived_content_writer<number>& writer = dynamic_cast< derived_content_writer<number>& >(gwriter);

        bool success = true;

        // lookup derived product from output task
        output_task_record<number>* rec = writer.get_record();
        assert(rec != nullptr);

        output_task<number>* tk = rec->get_task();
        assert(tk != nullptr);

        derived_data::derived_product<number>* product = tk->lookup_derived_product(temp_name);

        if(product == nullptr)
          {
            BOOST_LOG_SEV(writer.get_log(), base_writer::error) << "!! Failed to lookup derived product '" << temp_name << "'; skipping this product";
            return(false);
          }

        // find output product in the temporary folder
        boost::filesystem::path temp_location = writer.get_abs_tempdir_path() / product->get_filename();
        boost::filesystem::path dest_location = writer.get_abs_output_path() / product->get_filename();

        if(!boost::filesystem::exists(temp_location))
          {
            BOOST_LOG_SEV(writer.get_log(), base_writer::error) << "!! Derived product " << temp_location << " missing; skipping this product";
            return(false);
          }

        if(boost::filesystem::exists(dest_location))
          {
            BOOST_LOG_SEV(writer.get_log(), base_writer::error) << "!! Destination " << dest_location << " for derived product " << temp_location << " already exists; skipping this product";
            return(false);
          }

        boost::filesystem::rename(temp_location, dest_location);

        BOOST_LOG_SEV(writer.get_log(), base_writer::normal) << "++ Emplaced derived product " << dest_location;

        // commit this product to the current output group
        writer.push_content(*product);

        return(success);
      }


		// DATAPIPES


		template <typename number>
		typename data_manager<number>::datapipe data_manager_sqlite3<number>::create_datapipe(const boost::filesystem::path& logdir,
		                                                                                      const boost::filesystem::path& tempdir,
                                                                                          typename data_manager<number>::datapipe::output_group_finder finder,
                                                                                          typename data_manager<number>::datapipe::dispatch_function dispatcher,
		                                                                                      unsigned int worker, bool no_log)
			{
		    // set up callback API

		    typename data_manager<number>::datapipe::utility_callbacks utilities;

		    utilities.attach = std::bind(&data_manager_sqlite3<number>::datapipe_attach, this,
		                                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    utilities.detach = std::bind(&data_manager_sqlite3<number>::datapipe_detach, this, std::placeholders::_1);

		    utilities.finder   = finder;
		    utilities.dispatch = dispatcher;

		    typename data_manager<number>::datapipe::config_cache config;

		    config.time = std::bind(&data_manager_sqlite3<number>::pull_time_config, this,
		                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

		    config.twopf = std::bind(&data_manager_sqlite3<number>::pull_kconfig_twopf, this,
		                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

		    config.threepf = std::bind(&data_manager_sqlite3<number>::pull_kconfig_threepf, this,
		                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

		    typename data_manager<number>::datapipe::timeslice_cache timeslice;

		    timeslice.background = std::bind(&data_manager_sqlite3<number>::pull_background_time_sample, this,
		                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    timeslice.twopf = std::bind(&data_manager_sqlite3<number>::pull_twopf_time_sample, this,
		                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
		                                std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);

		    timeslice.threepf = std::bind(&data_manager_sqlite3<number>::pull_threepf_time_sample, this,
		                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
		                                  std::placeholders::_4, std::placeholders::_5);

        timeslice.tensor_twopf = std::bind(&data_manager_sqlite3<number>::pull_tensor_twopf_time_sample, this,
                                           std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                           std::placeholders::_4, std::placeholders::_5);

		    timeslice.zeta_twopf = std::bind(&data_manager_sqlite3<number>::pull_zeta_twopf_time_sample, this,
		                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    timeslice.zeta_threepf = std::bind(&data_manager_sqlite3<number>::pull_zeta_threepf_time_sample, this,
		                                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    timeslice.zeta_redbsp = std::bind(&data_manager_sqlite3<number>::pull_zeta_redbsp_time_sample, this,
		                                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    timeslice.fNL = std::bind(&data_manager_sqlite3<number>::pull_fNL_time_sample, this,
		                              std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    timeslice.BT = std::bind(&data_manager_sqlite3<number>::pull_BT_time_sample, this,
		                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    timeslice.TT = std::bind(&data_manager_sqlite3<number>::pull_TT_time_sample, this,
		                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    typename data_manager<number>::datapipe::kslice_cache kslice;

		    kslice.twopf = std::bind(&data_manager_sqlite3<number>::pull_twopf_kconfig_sample, this,
		                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
		                             std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);

		    kslice.threepf = std::bind(&data_manager_sqlite3<number>::pull_threepf_kconfig_sample, this,
		                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
		                               std::placeholders::_4, std::placeholders::_5);

        kslice.tensor_twopf = std::bind(&data_manager_sqlite3<number>::pull_tensor_twopf_kconfig_sample, this,
                                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                        std::placeholders::_4, std::placeholders::_5);

		    kslice.zeta_twopf = std::bind(&data_manager_sqlite3<number>::pull_zeta_twopf_kconfig_sample, this,
		                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    kslice.zeta_threepf = std::bind(&data_manager_sqlite3<number>::pull_zeta_threepf_kconfig_sample, this,
		                                       std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    kslice.zeta_redbsp = std::bind(&data_manager_sqlite3<number>::pull_zeta_redbsp_kconfig_sample, this,
		                                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		    // set up datapipe
		    typename data_manager<number>::datapipe pipe(this->pipe_data_capacity, this->pipe_zeta_capacity,
                                                     logdir, tempdir, worker,
		                                                 utilities, config, timeslice, kslice, no_log);

				return(pipe);
			}


    template <typename number>
    void data_manager_sqlite3<number>::pull_time_config(typename data_manager<number>::datapipe* pipe,
                                                        const std::vector<unsigned int>& serial_numbers, std::vector<double>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_time_sample(db, serial_numbers, sample, pipe->get_worker_number());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_kconfig_twopf(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& serial_numbers,
                                                          typename std::vector<typename data_manager<number>::twopf_configuration>& sample)
			{
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

		    sqlite3_operations::pull_twopf_kconfig_sample<number>(db, serial_numbers, sample, pipe->get_worker_number());
			}


    template <typename number>
    void data_manager_sqlite3<number>::pull_kconfig_threepf(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& serial_numbers,
                                                            typename std::vector<typename data_manager<number>::threepf_configuration>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_threepf_kconfig_sample<number>(db, serial_numbers, sample, pipe->get_worker_number());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_background_time_sample(typename data_manager<number>::datapipe* pipe,
                                                                   unsigned int id, const std::vector<unsigned int>& t_serials,
                                                                   std::vector<number>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_background_time_sample(db, id, t_serials, sample, pipe->get_worker_number(), pipe->get_N_fields());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_twopf_time_sample(typename data_manager<number>::datapipe* pipe,
                                                              unsigned int id, const std::vector<unsigned int>& t_serials,
                                                              unsigned int k_serial, std::vector<number>& sample,
                                                              typename data_manager<number>::datapipe::twopf_type type)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_twopf_time_sample(db, id, t_serials, k_serial, sample,
                                                   (type == data_manager<number>::datapipe::twopf_real ? sqlite3_operations::real_twopf : sqlite3_operations::imag_twopf),
                                                   pipe->get_worker_number(), pipe->get_N_fields());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_threepf_time_sample(typename data_manager<number>::datapipe* pipe,
                                                                unsigned int id, const std::vector<unsigned int>& t_serials,
                                                                unsigned int k_serial, std::vector<number>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_threepf_time_sample(db, id, t_serials, k_serial, sample, pipe->get_worker_number(), pipe->get_N_fields());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_tensor_twopf_time_sample(typename data_manager<number>::datapipe* pipe,
                                                                     unsigned int id, const std::vector<unsigned int>& t_serials,
                                                                     unsigned int k_serial, std::vector<number>& sample)
      {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_tensor_twopf_time_sample(db, id, t_serials, k_serial, sample, pipe->get_worker_number());
      }


    template <typename number>
    void data_manager_sqlite3<number>::pull_zeta_twopf_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                                   unsigned int k_serial, std::vector<number>& sample)
	    {
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_zeta_twopf_time_sample(db, t_serials, k_serial, sample, pipe->get_worker_number());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_zeta_threepf_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                                     unsigned int k_serial, std::vector<number>& sample)
	    {
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_zeta_threepf_time_sample(db, t_serials, k_serial, sample, pipe->get_worker_number());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_zeta_redbsp_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                                    unsigned int k_serial, std::vector<number>& sample)
	    {
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_zeta_redbsp_time_sample(db, t_serials, k_serial, sample, pipe->get_worker_number());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_fNL_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                            std::vector<number>& sample, derived_data::template_type type)
	    {
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_fNL_time_sample(db, t_serials, sample, pipe->get_worker_number(), type);
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_BT_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                           std::vector<number>& sample, derived_data::template_type type)
	    {
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_BT_time_sample(db, t_serials, sample, pipe->get_worker_number(), type);
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_TT_time_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& t_serials,
                                                           std::vector<number>& sample, derived_data::template_type type)
	    {
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_TT_time_sample(db, t_serials, sample, pipe->get_worker_number(), type);
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_twopf_kconfig_sample(typename data_manager<number>::datapipe* pipe,
                                                                 unsigned int id, const std::vector<unsigned int>& k_serials,
                                                                 unsigned int t_serial, std::vector<number>& sample,
                                                                 typename data_manager<number>::datapipe::twopf_type type)
	    {
				assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

		    sqlite3* db = nullptr;
		    pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_twopf_kconfig_sample(db, id, k_serials, t_serial, sample,
                                                      (type == data_manager<number>::datapipe::twopf_real ? sqlite3_operations::real_twopf : sqlite3_operations::imag_twopf),
                                                      pipe->get_worker_number(), pipe->get_N_fields());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_threepf_kconfig_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                                                   const std::vector<unsigned int>& k_serials,
                                                                   unsigned int t_serial, std::vector<number>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_threepf_kconfig_sample(db, id, k_serials, t_serial, sample, pipe->get_worker_number(), pipe->get_N_fields());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_tensor_twopf_kconfig_sample(typename data_manager<number>::datapipe* pipe, unsigned int id,
                                                                        const std::vector<unsigned int>& k_serials,
                                                                        unsigned int t_serial, std::vector<number>& sample)
      {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_tensor_twopf_kconfig_sample(db, id, k_serials, t_serial, sample, pipe->get_worker_number());
      }


    template <typename number>
    void data_manager_sqlite3<number>::pull_zeta_twopf_kconfig_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& k_serials,
                                                                      unsigned int t_serial, std::vector<number>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_zeta_twopf_kconfig_sample(db, k_serials, t_serial, sample, pipe->get_worker_number());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_zeta_threepf_kconfig_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& k_serials,
                                                                        unsigned int t_serial, std::vector<number>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_zeta_threepf_kconfig_sample(db, k_serials, t_serial, sample, pipe->get_worker_number());
	    }


    template <typename number>
    void data_manager_sqlite3<number>::pull_zeta_redbsp_kconfig_sample(typename data_manager<number>::datapipe* pipe, const std::vector<unsigned int>& k_serials,
                                                                       unsigned int t_serial, std::vector<number>& sample)
	    {
        assert(pipe != nullptr);
        if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

        sqlite3* db = nullptr;
        pipe->get_manager_handle(&db);    // throws an exception if the handle is unset, so safe to proceed; we can't get nullptr back

        sqlite3_operations::pull_zeta_redbsp_kconfig_sample(db, k_serials, t_serial, sample, pipe->get_worker_number());
	    }


    template <typename number>
    std::shared_ptr< output_group_record<integration_payload> >
    data_manager_sqlite3<number>::datapipe_attach(typename data_manager<number>::datapipe* pipe,
                                                  typename data_manager<number>::datapipe::output_group_finder& finder,
                                                  const std::string& name, const std::list<std::string>& tags)
			{
				assert(pipe != nullptr);
				if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

				sqlite3* db = nullptr;

        // find a suitable output group for this task
        std::shared_ptr< output_group_record<integration_payload> > group = finder(name, tags);

        integration_payload& payload = group->get_payload();

				// get path to the output group data container
		    boost::filesystem::path ctr_path = group->get_abs_repo_path() / payload.get_container_path();

				int status = sqlite3_open_v2(ctr_path.string().c_str(), &db, SQLITE_OPEN_READONLY, nullptr);

				if(status != SQLITE_OK)
					{
				    std::ostringstream msg;
						if(db != nullptr)
							{
								msg << __CPP_TRANSPORT_DATACTR_OPEN_A << " '" << ctr_path.string() << "' " << __CPP_TRANSPORT_DATACTR_OPEN_B << status << ": " << sqlite3_errmsg(db) << ")";
								sqlite3_close(db);
							}
						else
							{
								msg << __CPP_TRANSPORT_DATACTR_OPEN_A << " '" << ctr_path.string() << "' " << __CPP_TRANSPORT_DATACTR_OPEN_B << status << ")";
							}
						throw runtime_exception(runtime_exception::DATA_CONTAINER_ERROR, msg.str());
					}
        sqlite3_extended_result_codes(db, 1);

        // enable foreign key constraints
        char* errmsg;
        sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errmsg);

		    // force temporary databases to be stored in memory, for speed
		    sqlite3_exec(db, "PRAGMA temp_store = 2;", nullptr, nullptr, &errmsg);

        // remember this connexion
				this->open_containers.push_back(db);
				pipe->set_manager_handle(db);

				BOOST_LOG_SEV(pipe->get_log(), data_manager<number>::normal) << "** Attached sqlite3 container '" << ctr_path.string() << "' to datapipe";

        return(group);
			}


		template <typename number>
		void data_manager_sqlite3<number>::datapipe_detach(typename data_manager<number>::datapipe* pipe)
			{
		    assert(pipe != nullptr);
		    if(pipe == nullptr) throw runtime_exception(runtime_exception::RUNTIME_ERROR, __CPP_TRANSPORT_DATAMGR_NULL_DATAPIPE);

				sqlite3* db = nullptr;
				pipe->get_manager_handle(&db);
				this->open_containers.remove(db);
				sqlite3_close(db);

				BOOST_LOG_SEV(pipe->get_log(), data_manager<number>::normal) << "** Detached sqlite3 container from datapipe";
			}


    // FACTORY FUNCTIONS TO BUILD A DATA_MANAGER

    template <typename number>
    data_manager<number>* data_manager_factory(unsigned int bcap, unsigned int dcap, unsigned int zcap)
      {
        return new data_manager_sqlite3<number>(bcap, dcap, zcap);
      }


  }   // namespace transport



#endif //__data_manager_sqlite3_H_
