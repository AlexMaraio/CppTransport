//
// Created by David Seery on 30/12/2013.
// Copyright (c) 2013 University of Sussex. All rights reserved.
//


#ifndef __repository_H_
#define __repository_H_


#include <string>
#include <sstream>
#include <stdexcept>

#include "transport/models/model.h"
#include "transport/tasks/task.h"
#include "transport/messages_en.h"
#include "transport/exceptions.h"

#include "dbxml/db.h"
#include "dbxml/dbxml/DbXml.hpp"

#include "boost/filesystem/operations.hpp"
#include "exceptions.h"


#define __CPP_TRANSPORT_REPO_ENVIRONMENT_LEAF   "env"
#define __CPP_TRANSPORT_REPO_CONTAINERS_LEAF    "containers"
#define __CPP_TRANSPORT_CNTR_MODELS_LEAF        "models.dbxml"
#define __CPP_TRANSPORT_CNTR_INTEGRATIONS_LEAF  "integrations.dbxml"

#define __CPP_TRANSPORT_NODE_MODEL_SPEC         "model-specification"
#define __CPP_TRANSPORT_NODE_TWOPF_SPEC         "twopf-specification"
#define __CPP_TRANSPORT_NODE_THREEPF_SPEC       "threepf-specification"
#define __CPP_TRANSPORT_NODE_MODEL_SPECIFIER    "model-specifier"


namespace transport
  {
    // forward-declare 'key' class used to create repositories
    // the complete declaration is in a separate file,
    // which must be included to allow creation of repositories
    class repository_creation_key;

    template <typename number>
    class repository
      {
      public:
        typedef enum { node_storage, document_storage } storage_type;

        // CONSTRUCTOR, DESTRUCTOR

      public:
        //! Open a repository with a specific pathname
        repository(const std::string& path);
        //! Create a repository with a specific pathname
        repository(const std::string& path, const repository_creation_key& key, storage_type type=node_storage);

        //! Close a repository, including the corresponding containers and environment
        ~repository();

        // INTERFACE -- PUSH TASKS TO THE REPOSITORY DATABASE

        //! Write a model/initial conditions/parameters combination to the model database.
        //! No combination with the supplied name should already exist; if it does, this is considered an error.
        void write_model(const initial_conditions<number>& ics, const model<number>* m);

        //! Write a threepf integration task to the integration database.
        //! Delegates write_integration_task() to do the work
        void write_integration(const twopf_task<number>& t, const model<number>* m) { this->write_integration_task(t, m, __CPP_TRANSPORT_NODE_TWOPF_SPEC); }
        //! Write a twopf integration task to the integration database
        //! Delegates write_integration_task() to do the work
        void write_integration(const threepf_task<number>& t, const model<number>* m) { this->write_integration_task(t, m, __CPP_TRANSPORT_NODE_THREEPF_SPEC); }

      protected:
        //! Write a generic task to the integration database, using a supplied node tag
        void write_integration_task(const task<number>& t, const model<number>* m, const std::string& node);

        // INTERFACE -- PULL TASKS FROM THE REPOSITORY DATABASE

      public:
        //! Query the database for a named task, and reconstruct it if present
        task<number>& query_task(const std::string& name);

        // INTERNAL DATA

      protected:
        //! BOOST path to repository root directory
        boost::filesystem::path root_path;
        //! BOOST path to repository environment
        boost::filesystem::path env_path;
        //! BOOST path to containers
        boost::filesystem::path containers_path;
        //! BOOST path to model container
        boost::filesystem::path models_path;
        //! BOOST path to integrations container
        boost::filesystem::path integrations_path;

        // DATABASE ENVIRONMENT

        //! Berkeley DB XML environment object corresponding to the open repository
        DB_ENV* env;
        //! Berkeley DB XML XmlManager object corresponding to the open repository
        DbXml::XmlManager* mgr;
      };


    // Open a named repository
    template <typename number>
    repository<number>::repository(const std::string& path)
      : env(nullptr), mgr(nullptr)
      {
        root_path = path;
        if(!boost::filesystem::is_directory(root_path))
          {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_MISSING_ROOT << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        env_path = root_path / __CPP_TRANSPORT_REPO_ENVIRONMENT_LEAF;
        if(!boost::filesystem::is_directory(env_path))
          {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_MISSING_ENV << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        containers_path = root_path / __CPP_TRANSPORT_REPO_CONTAINERS_LEAF;
        if(!boost::filesystem::is_directory(containers_path))
          {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_MISSING_CNTR << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        models_path = containers_path / __CPP_TRANSPORT_CNTR_MODELS_LEAF;
        if(!boost::filesystem::is_regular_file(models_path))
          {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_MISSING_MODELS << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        integrations_path = containers_path / __CPP_TRANSPORT_CNTR_INTEGRATIONS_LEAF;
        if(!boost::filesystem::is_regular_file(integrations_path))
          {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_MISSING_INTGRTNS << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        int dberr;
        if((dberr = ::db_env_create(&env, 0)) > 0)
          {
            if(env != nullptr) env->close(env, 0);
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_FAIL_ENV << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        // set up environment to enable logging, transactional support
        // and locking (so multiple processes can access the repository safely)
        u_int32_t env_flags = DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
        env->open(env, env_path.string().c_str(), env_flags, 0);

        // set up XmlManager object
        mgr = new DbXml::XmlManager(env, 0);
      }


    // Create a named repository
    template <typename number>
    repository<number>::repository(const std::string& path, const repository_creation_key& key, storage_type type)
      : env(nullptr), mgr(nullptr)
      {
        // check whether root directory already exists
        root_path = path;
        if(boost::filesystem::exists(root_path))
          {
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_ROOT_EXISTS << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        env_path = root_path / __CPP_TRANSPORT_REPO_ENVIRONMENT_LEAF;
        containers_path = root_path / __CPP_TRANSPORT_REPO_CONTAINERS_LEAF;
        models_path = containers_path / __CPP_TRANSPORT_CNTR_MODELS_LEAF;
        integrations_path = containers_path / __CPP_TRANSPORT_CNTR_INTEGRATIONS_LEAF;

        // create directories
        boost::filesystem::create_directories(root_path);
        boost::filesystem::create_directories(env_path);
        boost::filesystem::create_directories(containers_path);

        int dberr;
        if((dberr = ::db_env_create(&env, 0)) > 0)
          {
            if(env != nullptr) env->close(env, 0);
            std::ostringstream msg;
            msg << __CPP_TRANSPORT_REPO_FAIL_ENV << " '" << path << "'";
            throw std::runtime_error(msg.str());
          }

        // set up environment to enable logging, transactional support
        // and locking (so multiple processes can access the repository safely)
        u_int32_t env_flags = DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
        env->open(env, env_path.string().c_str(), env_flags, 0);

        // set up XmlManager object
        mgr = new DbXml::XmlManager(env, DbXml::DBXML_ADOPT_DBENV);

        // create database containers
        switch(type)
          {
            case node_storage:
              mgr->setDefaultContainerType(DbXml::XmlContainer::NodeContainer);
              break;

            case document_storage:
              mgr->setDefaultContainerType(DbXml::XmlContainer::WholedocContainer);
              break;

            default:
              assert(false);
          }
        DbXml::XmlContainer models = this->mgr->createContainer(models_path.string().c_str());
        DbXml::XmlContainer integrations = this->mgr->createContainer(integrations_path.string().c_str());
      }


    // Destroy a respository object, closing the associated repository
    template <typename number>
    repository<number>::~repository()
      {
        assert(this->env != nullptr);
        assert(this->mgr != nullptr);

        delete this->mgr;
      }


    // Write a model/initial conditions/parameters combination to the repository
    template <typename number>
    void repository<number>::write_model(const initial_conditions<number>& ics, const model<number>* m)
      {
        assert(this->env != nullptr);
        assert(this->mgr != nullptr);
        assert(m != nullptr);

        if(m == nullptr) throw std::runtime_error(__CPP_TRANSPORT_REPO_NULL_MODEL);

        try
          {
            // open database container
            DbXml::XmlContainer models = this->mgr->openContainer(this->models_path.string().c_str());

            DbXml::XmlUpdateContext ctx = this->mgr->createUpdateContext();

            // begin XML document representing this model
            DbXml::XmlDocument doc = this->mgr->createDocument();
            doc.setName(ics.get_name());

            DbXml::XmlEventWriter& writer = models.putDocumentAsEventWriter(doc, ctx);
            writer.writeStartDocument(nullptr, nullptr, nullptr);

            // write root node
            writer.writeStartElement(__CPP_TRANSPORT_DBXML_STRING(__CPP_TRANSPORT_NODE_MODEL_SPEC), nullptr, nullptr, 0, false);
            m->serialize_xml(writer);
            ics.serialize_xml(writer);
            writer.writeEndElement(__CPP_TRANSPORT_DBXML_STRING(__CPP_TRANSPORT_NODE_MODEL_SPEC), nullptr, nullptr);

            // finalize XML document
            writer.writeEndDocument();
            writer.close();
          }
        catch (DbXml::XmlException& xe)
          {
            if(xe.getExceptionCode() == DbXml::XmlException::UNIQUE_ERROR)
              {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_REPO_MODEL_EXISTS << " '" << ics.get_name() << "'";
                throw std::runtime_error(msg.str());
              }
            else
              {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_REPO_INSERT_ERROR << xe.getExceptionCode() << ": '" << xe.what() << "')";
                throw std::runtime_error(msg.str());
              }
          }
      }


    // Write a task to the repository
    template <typename number>
    void repository<number>::write_integration_task(const task<number>& t, const model<number>* m, const std::string& node)
      {
        assert(this->env != nullptr);
        assert(this->mgr != nullptr);
        assert(m != nullptr);

        if(m == nullptr) throw std::runtime_error(__CPP_TRANSPORT_REPO_NULL_MODEL);

        try
          {
            // open database container
            DbXml::XmlContainer models = this->mgr->openContainer(this->models_path.string().c_str());
            DbXml::XmlContainer integrations = this->mgr->openContainer(this->integrations_path.string().c_str());

            // check whether XML document corresponding to our initial_conditions object is in the database
            try
              {
                DbXml::XmlDocument doc = models.getDocument(t.get_ics().get_name());
              }
            catch (DbXml::XmlException& xe)
              {
                if(xe.getExceptionCode() == DbXml::XmlException::DOCUMENT_NOT_FOUND)
                  {
                    this->write_model(t.get_ics(), m);
                  }
                else
                  {
                    std::ostringstream msg;
                    msg << __CPP_TRANSPORT_REPO_INSERT_ERROR << xe.getExceptionCode() << ": '" << xe.what() << "')";
                    throw std::runtime_error(msg.str());
                  }
              }

            // now insert task information into the integrations database
            DbXml::XmlUpdateContext ctx = this->mgr->createUpdateContext();

            // begin XML document representing this integration
            DbXml::XmlDocument doc = this->mgr->createDocument();
            doc.setName(t.get_name());

            DbXml::XmlEventWriter& writer = integrations.putDocumentAsEventWriter(doc, ctx);
            writer.writeStartDocument(nullptr, nullptr, nullptr);

            // write root node
            writer.writeStartElement(__CPP_TRANSPORT_DBXML_STRING(node.c_str()), nullptr, nullptr, 0, false);

            writer.writeStartElement(__CPP_TRANSPORT_DBXML_STRING(__CPP_TRANSPORT_NODE_MODEL_SPECIFIER), nullptr, nullptr, 0, false);
            writer.writeText(DbXml::XmlEventReader::Characters, __CPP_TRANSPORT_DBXML_STRING(t.get_ics().get_name().c_str()), t.get_ics().get_name().length());
            writer.writeEndElement(__CPP_TRANSPORT_DBXML_STRING(node.c_str()), nullptr, nullptr);

            t.serialize_xml(writer);

            writer.writeEndElement(__CPP_TRANSPORT_DBXML_STRING(__CPP_TRANSPORT_NODE_THREEPF_SPEC), nullptr, nullptr);

            // finalize XML document
            writer.writeEndDocument();
            writer.close();
          }
        catch (DbXml::XmlException& xe)
          {
            if(xe.getExceptionCode() == DbXml::XmlException::UNIQUE_ERROR)
              {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_REPO_INTGRTN_EXISTS << " '" << t.get_name() << "'";
                throw std::runtime_error(msg.str());
              }
            else
              {
                std::ostringstream msg;
                msg << __CPP_TRANSPORT_REPO_INSERT_ERROR << xe.getExceptionCode() << ": '" << xe.what() << "')";
                throw std::runtime_error(msg.str());
              }
          }
      }


    // Query the database for a named task
    template <typename number>
    task<number>& repository<number>::query_task(const std::string& name)
      {
        assert(this->env != nullptr);
        assert(this->mgr != nullptr);

        try
          {
            // open database containers
            DbXml::XmlContainer models = this->mgr->openContainer(this->models_path.string().c_str());
            DbXml::XmlContainer integrations = this->mgr->openContainer(this->integrations_path.string().c_str());

            DbXml::XmlDocument task_document;
            try
              {
                // find XML document corresponding to our task name; will throw a DOCUMENT_NOT_FOUND EXCEPTION if it does not exist
                task_document = integrations.getDocument(name);
              }
            catch (DbXml::XmlException& xe)
              {
                if(xe.getExceptionCode() == DbXml::XmlException::DOCUMENT_NOT_FOUND)
                  {
                    throw runtime_exception(runtime_exception::TASK_NOT_FOUND, name);
                  }
                else
                  {
                    std::ostringstream msg;
                    msg << __CPP_TRANSPORT_REPO_QUERY_ERROR << xe.getExceptionCode() << ": " << xe.what() << "')";
                    throw runtime_exception(runtime_exception::RUNTIME_ERROR, msg.str());
                  }
              }

            // convert task XML document to an XmlValue which can be queried
            DbXml::XmlValue task(task_document);

            // now run a query to find the XML record for the corresponding initial conditions/parameter set
            std::ostringstream model_query;
            model_query << "distinct-values(" << __CPP_TRANSPORT_XQUERY_SEPARATOR
              << __CPP_TRANSPORT_XQUERY_WILDCARD << __CPP_TRANSPORT_XQUERY_SEPARATOR
              << __CPP_TRANSPORT_NODE_MODEL_SPECIFIER << ")";

            // create a context for the query
            DbXml::XmlQueryContext model_ctx = this->mgr->createQueryContext();

            // compile the query
            DbXml::XmlQueryExpression model_expr = this->mgr->prepare(model_query.str(), model_ctx);

            // execute it and obtain a result set
            DbXml::XmlResults model_results = model_expr.execute(task, model_ctx);

            if(model_results.size() != 1) throw runtime_exception(runtime_exception::BADLY_FORMED_XML_INTEGRATION, name);

            DbXml::XmlValue model_node;
            model_results.next(model_node);

            std::string model_name = model_node.asString();

            DbXml::XmlDocument model_document;
            try
              {
                // find XML document corresponding to our model name; will throw a DOCUMENT_NOT_FOUND EXCEPTION if it does not exist
                model_document = models.getDocument(model_name);
              }
            catch (DbXml::XmlException& xe)
              {
                if(xe.getExceptionCode() == DbXml::XmlException::DOCUMENT_NOT_FOUND)
                  {
                    throw runtime_exception(runtime_exception::MODEL_NOT_FOUND, model_name);
                  }
                else
                  {
                    std::ostringstream msg;
                    msg << __CPP_TRANSPORT_REPO_QUERY_ERROR << xe.getExceptionCode() << ": " << xe.what() << "')";
                    throw runtime_exception(runtime_exception::RUNTIME_ERROR, msg.str());
                  }
              }
          }
        catch (DbXml::XmlException& xe)
          {
            throw runtime_exception(runtime_exception::RUNTIME_ERROR, xe.what());
          }
      }

  }   // namespace transport



#endif //__repository_H_
