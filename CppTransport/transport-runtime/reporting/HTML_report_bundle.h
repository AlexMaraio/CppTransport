//
// Created by David Seery on 31/03/2016.
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

#ifndef CPPTRANSPORT_REPORTING_HTML_REPORT_BUNDLE_H
#define CPPTRANSPORT_REPORTING_HTML_REPORT_BUNDLE_H


#include "transport-runtime/data/data_manager.h"

#include "transport-runtime/reporting/repository_cache.h"
#include "transport-runtime/reporting/unique_tags.h"

#include "transport-runtime/reporting/HTML_writer.h"
#include "transport-runtime/reporting/JavaScript_writer.h"

#include "transport-runtime/manager/environment.h"
#include "transport-runtime/manager/argument_cache.h"
#include "transport-runtime/manager/message_handlers.h"

#include "transport-runtime/repository/repository_toolkit.h"

#include "transport-runtime/messages.h"
#include "transport-runtime/defaults.h"
#include "transport-runtime/exceptions.h"

#include "boost/filesystem/operations.hpp"


namespace transport
  {

    namespace reporting
      {

        constexpr auto CPPTRANSPORT_HTML_INDEX = "index.html";
        constexpr auto CPPTRANSPORT_HTML_JAVASCRIPT = "script.js";

        constexpr auto CPPTRANSPORT_HTML_JAVASCRIPT_DIR = "js";
        constexpr auto CPPTRANSPORT_HTML_CSS_DIR = "css";
        constexpr auto CPPTRANSPORT_HTML_FONTS_DIR = "fonts";
        constexpr auto CPPTRANSPORT_HTML_ASSET_DIR = "assets";

        template <typename number>
        class HTML_report_bundle
          {

            // CONSTRUCTOR, DESTRUCTOR

          public:

            //! constructor
            HTML_report_bundle(local_environment& e, argument_cache& c,
                               repository<number>& rep, data_manager<number>& dmg, boost::filesystem::path r);

            //! destructor adds JavaScript item
            ~HTML_report_bundle();


            // EMPLACE ASSETS

          public:

            //! emplace CSS asset
            void emplace_CSS_asset(boost::filesystem::path source, boost::filesystem::path dest);

            //! empalce font asset
            void emplace_font_asset(boost::filesystem::path source, boost::filesystem::path dest);

            //! emplace JavaScript asset
            void emplace_JavaScript_asset(boost::filesystem::path source, boost::filesystem::path dest);

            //! make directory for assets belonging to named object
            //! returns relative path of directory
            boost::filesystem::path make_asset_directory(const std::string& name);

            //! emplace an asset for a named object, creating a suitable asset directory if one does not exist
            //! returns relative path of emplaced asset
            boost::filesystem::path emplace_asset(const boost::filesystem::path& source, const std::string& name,
                                                  const boost::filesystem::path& filename);

          protected:

            //! emplace an asset file
            void copy_asset(boost::filesystem::path source, boost::filesystem::path absolute_path, boost::filesystem::path dest_filename);


            // EMPLACE HTML CONTENT

          public:

            //! emplace an HTML element
            void emplace_HTML_body(HTML_element& body) { this->HTML->add_body(body); }

            //! emplace a modal dialog
            void emplace_modal(HTML_element& modal) { this->HTML->add_modal(modal); }


            // WRITE JAVASCRIPT CONTENT

          public:

            //! write a line to the JavaScript file
            void write_JavaScript(std::string line) { this->JavaScript->write(std::move(line)); }


            // FORWARD REPOSITORY_CACHE INTERFACE

          public:

            //! get package database
            typename package_db<number>::type& get_package_db() { return this->cache.get_package_db(); }

            //! get task database
            typename task_db<number>::type& get_task_db() { return this->cache.get_task_db(); }

            //! get derived product database
            typename derived_product_db<number>::type& get_derived_product_db() { return this->cache.get_derived_product_db(); }


            //! get integration content database
            integration_content_db& get_integration_content_db() { return this->cache.get_integration_content_db(); }

            //! get postintegration content database
            postintegration_content_db& get_postintegration_content_db() { return this->cache.get_postintegration_content_db(); }

            //! get output content database
            output_content_db& get_output_content_db() { return this->cache.get_output_content_db(); }


            // REPOSITORY FUNCTIONS

          public:

            //! get title
            std::string get_title() const { return(this->repo.get_name()); }

            //! get cache object
            repository_cache<number>& get_cache() { return this->cache; }

            //! get toolkit
            std::unique_ptr< repository_graphkit<number> > get_graphkit(error_handler e, warning_handler w, message_handler h) { return std::make_unique< repository_graphkit<number> >(this->repo, e, w, h); }


            // DATA MANAGER FUNCTIONS

          public:

            //! get worker table, given *relative* path of container
            worker_information_db read_worker_database(const boost::filesystem::path& ctr_path) const { return(this->dmgr.read_worker_information(this->repo.get_root_path() / ctr_path)); }

            //! get statistics table, given *relative* path of container
            timing_db read_timing_database(const boost::filesystem::path& ctr_path) const { return(this->dmgr.read_timing_information(this->repo.get_root_path() / ctr_path)); }


            // FORWARD UNIQUE_TAGS INTERFACE

          public:

            //! reset all caches
            void reset() { this->tags.reset(); }

            //! get unique ID for package
            std::string get_id(const package_record<number>& rec) { return this->tags.get_id(rec); }

            //! get unique ID for task
            std::string get_id(const task_record<number>& rec) { return this->tags.get_id(rec); }

            //! get unique ID for derived product
            std::string get_id(const derived_product_record<number>& rec) { return this->tags.get_id(rec); }

            //! get unique ID for content group
            template <typename Payload>
            std::string get_id(const content_group_record<Payload>& rec) { return this->tags.get_id(rec); }


            // ENVIRONMENT QUERIES

          public:

            //! check whether it is possible to produce plots
            bool can_produce_plots() const { return(this->env.has_python() && this->env.has_matplotlib()); }

            //! get environment object
            local_environment& get_environment() { return(this->env); }

            //! get argument cache objects
            argument_cache& get_argument_cache() { return(this->arg_cache); }


            // INTERNAL API

          protected:

            //! find asset on search paths; throws an exception if asset cannot be located
            //! accepts a relative path and returns a fully-qualified absolute path
            boost::filesystem::path find_asset(boost::filesystem::path asset);


            // INTERNAL DATA

          private:

            //! reference to repository
            repository<number>& repo;

            //! reference to data manager
            data_manager<number>& dmgr;

            //! reference to local environment policy class
            local_environment& env;

            //! reference to argument cache policy class
            argument_cache& arg_cache;

            //! repository cache
            repository_cache<number> cache;

            //! unique-tags factory
            unique_tags<number> tags;

            //! HTML writer
            std::unique_ptr< HTML_writer > HTML;

            //! JavaScript writer
            std::unique_ptr< JavaScript_writer > JavaScript;

            //! root directory for bundle
            boost::filesystem::path root;

            //! asset finder
            std::unique_ptr<finder> asset_find;

          };


        template <typename number>
        HTML_report_bundle<number>::HTML_report_bundle(local_environment& e, argument_cache& c,
                                                       repository<number>& rep, data_manager<number>& dmg,
                                                       boost::filesystem::path r)
          : repo(rep),
            dmgr(dmg),
            env(e),
            arg_cache(c),
            cache(rep),
            root(r),
            asset_find(env.make_resource_finder(CPPTRANSPORT_HTML_RESOURCE_DIRECTORY))
          {
            // create root directory if it does not already exist, taking care to catch any exceptions which occur
            try
              {
                boost::filesystem::create_directories(this->root);
              }
            catch(boost::filesystem::filesystem_error &xe)
              {
                std::ostringstream msg;
                msg << CPPTRANSPORT_HTML_CREATE_FAILURE << " '" << root.string() << "'";
                throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
              }

            // create writers for HTML, CSS and JavaScript files
            this->HTML = std::make_unique< HTML_writer >(r / CPPTRANSPORT_HTML_INDEX, repo.get_name());
            this->JavaScript = std::make_unique< JavaScript_writer >(r / CPPTRANSPORT_HTML_JAVASCRIPT);
            
            // add explicitly-specified paths to asset finder
            asset_find->add(arg_cache.get_search_paths(), CPPTRANSPORT_HTML_RESOURCE_DIRECTORY);
          }


        template <typename number>
        HTML_report_bundle<number>::~HTML_report_bundle()
          {
            this->HTML->add_JavaScript(CPPTRANSPORT_HTML_JAVASCRIPT);
          }


        template <typename number>
        void HTML_report_bundle<number>::copy_asset(boost::filesystem::path source, boost::filesystem::path absolute_path,
                                                    boost::filesystem::path dest_filename)
          {
            if(!boost::filesystem::exists(absolute_path))
              {
                try
                  {
                    boost::filesystem::create_directories(absolute_path);
                  }
                catch(boost::filesystem::filesystem_error &xe)
                  {
                    std::ostringstream msg;
                    msg << CPPTRANSPORT_HTML_ASSET_CREATE_FAILURE << " '" << absolute_path.string() << "'";
                    throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
                  }
              }

            // copy asset into destination directory hierarchy
            boost::filesystem::path asset_src = this->find_asset(source);
            boost::filesystem::path asset_dest = absolute_path / dest_filename;

            try
              {
                boost::filesystem::copy_file(asset_src, asset_dest);
              }
            catch(boost::filesystem::filesystem_error &xe)
              {
                std::ostringstream msg;
                msg << CPPTRANSPORT_HTML_ASSET_EMPLACE_FAILURE_A << " '" << asset_dest.string() << "' "
                    << CPPTRANSPORT_HTML_ASSET_EMPLACE_FAILURE_B << " '" << asset_src.string() << "'";
                throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
              }
          }


        template <typename number>
        void HTML_report_bundle<number>::emplace_CSS_asset(boost::filesystem::path source, boost::filesystem::path dest)
          {
            boost::filesystem::path css_relative_path = CPPTRANSPORT_HTML_CSS_DIR;
            boost::filesystem::path css_absolute_path = this->root / css_relative_path;

            this->copy_asset(source, css_absolute_path, dest);

            // add to HTML writer
            this->HTML->add_stylesheet(css_relative_path / dest);
          }


        template <typename number>
        void HTML_report_bundle<number>::emplace_font_asset(boost::filesystem::path source, boost::filesystem::path dest)
          {
            boost::filesystem::path font_relative_path = CPPTRANSPORT_HTML_FONTS_DIR;
            boost::filesystem::path font_absolute_path = this->root / font_relative_path;

            this->copy_asset(source, font_absolute_path, dest);
          }


        template <typename number>
        void HTML_report_bundle<number>::emplace_JavaScript_asset(boost::filesystem::path source, boost::filesystem::path dest)
          {
            boost::filesystem::path js_relative_path = CPPTRANSPORT_HTML_JAVASCRIPT_DIR;
            boost::filesystem::path js_absolute_path = this->root / js_relative_path;

            if(!boost::filesystem::exists(js_absolute_path))
              {
                try
                  {
                    boost::filesystem::create_directories(js_absolute_path);
                  }
                catch(boost::filesystem::filesystem_error &xe)
                  {
                    std::ostringstream msg;
                    msg << CPPTRANSPORT_HTML_ASSET_CREATE_FAILURE << " '" << js_absolute_path.string() << "'";
                    throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
                  }
              }

            // copy asset into destination directory hierarchy
            boost::filesystem::path asset_src = this->find_asset(source);
            boost::filesystem::path asset_dest = js_absolute_path / dest;

            try
              {
                boost::filesystem::copy_file(asset_src, asset_dest);
              }
            catch(boost::filesystem::filesystem_error &xe)
              {
                std::ostringstream msg;
                msg << CPPTRANSPORT_HTML_ASSET_EMPLACE_FAILURE_A << " '" << asset_dest.string() << "' "
                    << CPPTRANSPORT_HTML_ASSET_EMPLACE_FAILURE_B << " '" << asset_src.string() << "'";
                throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
              }

            // add to HTML writer
            this->HTML->add_JavaScript(js_relative_path / dest);
          }


        template <typename number>
        boost::filesystem::path HTML_report_bundle<number>::find_asset(boost::filesystem::path asset)
          {
            auto p = this->asset_find->find(asset);
            if(p) return *p;

            // if we arrive here, the asset was not found
            std::ostringstream msg;
            msg << CPPTRANSPORT_HTML_ASSET_NOT_FOUND << " '" << asset.string() << "'";
            throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
          }


        template <typename number>
        boost::filesystem::path HTML_report_bundle<number>::make_asset_directory(const std::string& name)
          {
            auto relative_path = boost::filesystem::path{CPPTRANSPORT_HTML_ASSET_DIR} / name;
            auto absolute_path = this->root / relative_path;

            if(boost::filesystem::exists(absolute_path))
              {
                if(!boost::filesystem::is_directory(absolute_path))
                  {
                    std::ostringstream msg;
                    msg << CPPTRANSPORT_HTML_ASSERT_DIRECTORY_EXISTS << " '" << absolute_path.string() << "'";
                    throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
                  }

                return relative_path;
              }

            // directory didn't exist, so try to create it
            try
              {
                boost::filesystem::create_directories(absolute_path);
              }
            catch(boost::filesystem::filesystem_error& xe)
              {
                std::ostringstream msg;
                msg << CPPTRANSPORT_HTML_ASSET_DIR_FAILURE << " '" << absolute_path.string() << "'";
                throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
              }

            return relative_path;
          }


        template <typename number>
        boost::filesystem::path HTML_report_bundle<number>::emplace_asset(const boost::filesystem::path& source, const std::string& name,
                                                                          const boost::filesystem::path& filename)
          {
            boost::filesystem::path relative_asset_dir = this->make_asset_directory(name);
            boost::filesystem::path relative_asset_location = relative_asset_dir / filename;
            boost::filesystem::path absolute_asset_location = this->root / relative_asset_location;

            try
              {
                boost::filesystem::copy(source, absolute_asset_location);
              }
            catch(boost::filesystem::filesystem_error& xe)
              {
                std::ostringstream msg;
                msg << CPPTRANSPORT_HTML_ASSET_EMPLACE_FAILURE_A << " '" << absolute_asset_location.string() << "' "
                    << CPPTRANSPORT_HTML_ASSET_EMPLACE_FAILURE_B << " '" << source.string() << "'";
                throw runtime_exception(exception_type::REPORTING_ERROR, msg.str());
              }

            return relative_asset_location;
          }


      }   // namespace reporting

  }   // namespace transport


#endif //CPPTRANSPORT_REPORTING_HTML_REPORT_BUNDLE_H
