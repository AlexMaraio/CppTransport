//
// Created by David Seery on 23/03/2016.
// Copyright (c) 2016 University of Sussex. All rights reserved.
//

#ifndef CPPTRANSPORT_REPORTING_HTML_REPORT_H
#define CPPTRANSPORT_REPORTING_HTML_REPORT_H


#include "transport-runtime-api/repository/repository.h"

#include "transport-runtime-api/reporting/content_group_data.h"
#include "transport-runtime-api/reporting/HTML_report_bundle.h"

#include "transport-runtime-api/manager/environment.h"
#include "transport-runtime-api/manager/argument_cache.h"
#include "transport-runtime-api/manager/message_handlers.h"

#include "transport-runtime-api/utilities/formatter.h"


namespace transport
  {

    namespace reporting
      {

        constexpr auto CPPTRANSPORT_HTML_BUTTON_TAG_PREFIX = "button";

        class HTML_report
          {

            // CONSTRUCTOR, DESTRUCTOR

          public:

            //! constructor
            HTML_report(local_environment& e, argument_cache& c,
                        error_handler eh, warning_handler wh, message_handler mh)
              : env(e),
                arg_cache(c),
                err(eh),
                warn(wh),
                msg(mh),
                button_id(0)
              {
              }

            //! destructor is default
            ~HTML_report() = default;


            // INTERFACE

          public:

            //! set root directory
            void set_root(boost::filesystem::path p);

            //! produce report
            template <typename number>
            void report(repository<number>& repo);


            // INTERNAL API

          protected:

            //! generate HTML report
            template <typename number>
            void generate_report(HTML_report_bundle<number>& bundle);

            //! write HTML report
            template <typename number>
            void generate_report_body(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write package tab
            template <typename number>
            void write_packages(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write integration tasks tab
            template <typename number>
            void write_integration(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write postintegration tasks tab
            template <typename number>
            void write_postintegration(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write output tasks tab
            template <typename number>
            void write_output(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write derived products tab
            template <typename number>
            void write_derived_products(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write integration content tabs
            template <typename number>
            void write_integration_content(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write postintegration content tabs
            template <typename number>
            void write_postintegration_content(HTML_report_bundle<number>& bundle, HTML_node& parent);

            //! write output content tabs
            template <typename number>
            void write_output_content(HTML_report_bundle<number>& bundle, HTML_node& parent);


            // MAKE BUTTONS

          protected:

            //! write a link-to-task button
            template <typename number>
            void write_task_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent);

            //! write a link-to-group button
            template <typename number>
            void write_content_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent);

            //! write a link-to-group button
            template <typename number>
            void write_content_button(HTML_report_bundle<number>& bundle, const std::string& name, const std::string& pane, HTML_node& parent);

            //! write a link-to-package button
            template <typename number>
            void write_package_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent);

            //! write a link-to-package button
            template <typename number>
            void write_package_button(HTML_report_bundle<number>& bundle, const std::string& name, const std::string& element, HTML_node& parent);

            //! write a link-to-derived product button
            template <typename number>
            void write_derived_product_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent);

            //! write a link-to-derived product button
            template <typename number>
            void write_derived_product_button(HTML_report_bundle<number>& bundle, const std::string& name, const std::string& element, HTML_node& parent);


            // WRITE GENERIC RECORD DETAILS

          protected:

            //! write details for generic records
            template <typename RecordType>
            void write_generic_record(const RecordType& rec, HTML_node& parent);

            //! write details for generic output records
            template <typename number, typename Payload>
            void write_generic_output_record(HTML_report_bundle<number>& bundle, const output_group_record<Payload>& rec,
                                             HTML_node& parent);

            //! write a seeding label
            template <typename number, typename Payload>
            void write_seeded(HTML_report_bundle<number>& bundle, const Payload& payload, HTML_node& parent);

            //! write a collapsible notes section
            void write_notes_collapsible(const std::list<std::string>& notes, const std::string& tag, HTML_node& parent);


            // WRITE JAVASCRIPT HANDLERS

          protected:

            //! write JavaScript button handler which changes pane and scrolls to a given element
            template <typename number>
            void write_JavaScript_button_scroll(HTML_report_bundle<number>& bundle, std::string button, std::string pane, std::string element);

            //! write JavaScript button handler which simply changes pane
            template <typename number>
            void write_JavaScript_button(HTML_report_bundle<number>& bundle, std::string button, std::string pane);

            // MISCELLANEOUS

          protected:

            //! make a button tag
            std::string make_button_tag();

            //! build an ordinary menu tab
            HTML_node make_menu_tab(std::string pane, std::string name);

            //! build a content-group menu tab
            template <typename number, typename DatabaseType, typename PayloadType>
            HTML_node make_content_menu_tab(const DatabaseType& db, HTML_report_bundle<number>& bundle, std::string name);

            //! write a grid data element
            void make_data_element(std::string l, std::string v, HTML_node& parent, std::string type="label-default", std::string cls="col-md-4");

            //! write an empty grid data element
            void make_empty_data_element(HTML_node& parent, std::string cls="col-md-4");

            //! make a list item heading
            void make_list_item_label(std::string label, HTML_node& parent);

            //! make badged text
            void make_badged_text(std::string text, unsigned int number, HTML_node& parent);

            //! compose a tag list into a set of HTML labels
            std::string compose_tag_list(const std::list<std::string>& tags, HTML_node& parent);


            // INTERNAL DATA

          private:

            //! reference to local environment object
            local_environment& env;
            //! reference to argument cache
            argument_cache& arg_cache;


            // MESSAGE HANDLERS

            //! error handler
            error_handler err;

            //! warning handler
            warning_handler warn;

            //! message handler
            message_handler msg;


            // REPORT DATA

            //! root directory
            boost::filesystem::path root;

            //! current unique button id
            unsigned int button_id;

          };


        void HTML_report::set_root(boost::filesystem::path p)
          {
            // check whether root path is absolute, and if not make it relative to current working directory
            if(!p.is_absolute())
              {
                // not absolute, so use CWD
                p = boost::filesystem::absolute(p);
              }

            // check whether root directory already exists, and if it does, raise an error
            if(boost::filesystem::exists(p))
              {
                std::ostringstream msg;
                msg << CPPTRANSPORT_HTML_ROOT_EXISTS_A << " '" << p.string() << "' " << CPPTRANSPORT_HTML_ROOT_EXISTS_B;
                this->err(msg.str());
                this->root.clear();
                return;
              }

            this->root = p;
          }


        template <typename number>
        void HTML_report::report(repository<number>& repo)
          {
            // has a root directory been set? if not, no need to generate a report
            if(this->root.empty()) return;

            try
              {
                // reset button id
                this->button_id = 0;

                // set up a HTML bundle for this report; this encapsulates all the files needed
                // and also manages any assets
                HTML_report_bundle<number> bundle(this->env, this->arg_cache, repo, this->root);

                // write main index.html
                this->generate_report(bundle);
              }
            catch(runtime_exception& xe)
              {
                if(xe.get_exception_code() == exception_type::REPORTING_ERROR)
                  {
                    this->err(xe.what());
                    return;
                  }
                else throw;
              }
          }


        template <typename number>
        void HTML_report::generate_report(HTML_report_bundle<number>& bundle)
          {
            // add jQuery and bootstrap scripts; note jQuery must come first, before bootstrap is loaded
            bundle.emplace_JavaScript_asset("jQuery/jquery-2.2.2.min.js", "jquery-2.2.2.min.js");
            bundle.emplace_JavaScript_asset("bootstrap/js/bootstrap.min.js", "bootstrap.min.js");

            // add plugin to enable history on bootstrap tabs
            bundle.emplace_JavaScript_asset("bootstrap-tab-history/bootstrap-tab-history.js", "bootstrap-tab-history.js");

            // add our own JavaScript file utility.js which handles eg. resizing navigation bar on window resize
            bundle.emplace_JavaScript_asset("HTML_assets/utility.js", "utility.js");

            // add bootstrap CSS file
            bundle.emplace_CSS_asset("bootstrap/css/bootstrap.min.css", "bootstrap.min.css");

            // add our own CSS file
            bundle.emplace_CSS_asset("HTML_assets/cpptransport.css", "cpptransport.css");

            // now generate the main body of the report
            HTML_node body("div");
            body.add_attribute("class", "container-fluid");
            this->generate_report_body(bundle, body);

            // emplace the report body inside the bundle
            bundle.emplace_HTML_body(body);
          }


        template <typename number>
        void HTML_report::generate_report_body(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            // NAVIGATION BAR

            HTML_node navbar("nav");
            navbar.add_attribute("class", "navbar navbar-default navbar-fixed-top");

            HTML_node navbar_container("div");
            navbar_container.add_attribute("class", "container-fluid");

            HTML_node navbar_header("div");
            navbar_header.add_attribute("class", "navbar-header");

            HTML_node navbar_brand("a", bundle.get_title());
            navbar_brand.add_attribute("class", "navbar-brand").add_attribute("href", "#");

            navbar_header.add_element(navbar_brand);

            // Tablist
            HTML_node tablist("ul");
            tablist.add_attribute("id", "tabs").add_attribute("class", "nav navbar-nav").add_attribute("role", "tablist");

            // Package tab
            HTML_node package_tab = this->make_menu_tab("packages", "Packages");
            // mark this tab as active by default
            package_tab.add_attribute("class", "active");

            // Integration tasks tab
            HTML_node integration_tasks_tab = this->make_menu_tab("integration", "Integration tasks");

            // Integration content tab
            HTML_node integration_content_tab = this->make_content_menu_tab<number, integration_content_db, integration_payload>(
              bundle.get_integration_content_db(), bundle, "Integration content");

            // Postintegration tasks tab
            HTML_node postintegration_tasks_tab = this->make_menu_tab("postintegration", "Postintegration tasks");

            // Postintegration content tab
            HTML_node postintegration_content_tab = this->make_content_menu_tab<number, postintegration_content_db, postintegration_payload>(
              bundle.get_postintegration_content_db(), bundle, "Postintegration content");

            // Output tasks tab
            HTML_node output_tasks_tab = this->make_menu_tab("output", "Output tasks");

            // Output content tab
            HTML_node output_content_tab = this->make_content_menu_tab<number, output_content_db, output_payload>(
              bundle.get_output_content_db(), bundle, "Output_content");

            // Derived products tab
            HTML_node derived_products_tab = this->make_menu_tab("derived", "Derived products");

            tablist.add_element(package_tab);
            tablist.add_element(integration_tasks_tab);
            tablist.add_element(integration_content_tab);
            tablist.add_element(postintegration_tasks_tab);
            tablist.add_element(postintegration_content_tab);
            tablist.add_element(output_tasks_tab);
            tablist.add_element(output_content_tab);
            tablist.add_element(derived_products_tab);

            navbar_container.add_element(navbar_header).add_element(tablist);

            navbar.add_element(navbar_container);

            // WRITE PANE CONTENT

            HTML_node tab_panes("div");
            tab_panes.add_attribute("class", "tab-content");

            HTML_node package_pane("div");
            package_pane.add_attribute("id", "packages").add_attribute("class", "tab-pane fade in active");
            this->write_packages(bundle, package_pane);
            tab_panes.add_element(package_pane);

            HTML_node integration_task_pane("div");
            integration_task_pane.add_attribute("id", "integration").add_attribute("class", "tab-pane fade");
            this->write_integration(bundle, integration_task_pane);
            tab_panes.add_element(integration_task_pane);

            this->write_integration_content(bundle, tab_panes);

            HTML_node postintegration_task_pane("div");
            postintegration_task_pane.add_attribute("id", "postintegration").add_attribute("class", "tab-pane fade");
            this->write_postintegration(bundle, postintegration_task_pane);
            tab_panes.add_element(postintegration_task_pane);

            this->write_postintegration_content(bundle, tab_panes);

            HTML_node output_task_pane("div");
            output_task_pane.add_attribute("id", "output").add_attribute("class", "tab-pane fade");
            this->write_output(bundle, output_task_pane);
            tab_panes.add_element(output_task_pane);

            this->write_output_content(bundle, tab_panes);

            HTML_node derived_products_pane("div");
            derived_products_pane.add_attribute("id", "derived").add_attribute("class", "tab-pane fade");
            this->write_derived_products(bundle, derived_products_pane);
            tab_panes.add_element(derived_products_pane);

            parent.add_element(navbar);
            parent.add_element(tab_panes);
          }


        HTML_node HTML_report::make_menu_tab(std::string pane, std::string name)
          {
            HTML_node tab("li");
            HTML_node anchor("a", name);
            anchor.add_attribute("data-toggle", "tab").add_attribute("href", "#" + pane);
            anchor.add_attribute("data-tab-history", "true").add_attribute("data-tab-history-changer", "push").add_attribute("data-tab-history-update-url", "true");
            tab.add_element(anchor);

            return tab;
          }


        template <typename number, typename DatabaseType, typename PayloadType>
        HTML_node HTML_report::make_content_menu_tab(const DatabaseType& db, HTML_report_bundle<number>& bundle, std::string name)
          {
            HTML_node tab("li");
            tab.add_attribute("class", "dropdown");
            HTML_node anchor("a", name);
            anchor.add_attribute("href", "#").add_attribute("class", "dropdown-toggle").add_attribute("data-toggle", "dropdown");

            HTML_node caret("span");
            caret.add_attribute("class", "caret");
            anchor.add_element(caret);

            HTML_node dropdown_menu("ul");
            dropdown_menu.add_attribute("class", "dropdown-menu scrollable-menu").add_attribute("role", "menu");

            for(const typename DatabaseType::value_type& group : db)
              {
                const output_group_record<PayloadType>& rec = *group.second;
                std::string tag = bundle.get_id(rec);

                HTML_node menu_item("li");
                HTML_node menu_anchor("a", rec.get_name());
                menu_anchor.add_attribute("href", "#" + tag).add_attribute("data-toggle", "tab");
                menu_anchor.add_attribute("data-tab-history", "true").add_attribute("data-tab-history-changer", "push").add_attribute("data-tab-history-update-url", "true");

                menu_item.add_element(menu_anchor);
                dropdown_menu.add_element(menu_item);
              }

            tab.add_element(anchor);
            tab.add_element(dropdown_menu);

            return tab;
          }


        void HTML_report::make_data_element(std::string l, std::string v, HTML_node& parent, std::string type, std::string cls)
          {
            HTML_node item("div", l);
            item.add_attribute("class", cls);

            HTML_node value("span", v);
            value.add_attribute("class", "label " + type);
            item.add_element(value);

            parent.add_element(item);
          }


        void HTML_report::make_empty_data_element(HTML_node& parent, std::string cls)
          {
            HTML_node item("div");
            item.add_attribute("class", cls);
            parent.add_element(item);
          }


        void HTML_report::make_list_item_label(std::string label, HTML_node& parent)
          {
            HTML_node heading("h3", label);
            heading.add_attribute("class", "list-group-item-heading lead");

            parent.add_element(heading);
          }


        void HTML_report::make_badged_text(std::string text, unsigned int number, HTML_node& parent)
          {
            HTML_string text_label(text);

            HTML_node badge("span", boost::lexical_cast<std::string>(number));
            badge.add_attribute("class", "badge");

            parent.add_element(text_label);
            parent.add_element(badge);
          }


        template <typename number>
        void HTML_report::write_packages(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            typename package_db<number>::type& db = bundle.get_package_db();

            if(db.empty()) return;

            HTML_node list("div");
            list.add_attribute("class", "list-group");

            // step through all packages, writing them out as list elements elements
            for(const typename package_db<number>::value_type& pkg : db)
              {
                const package_record<number>& rec = *pkg.second;

                HTML_node item("a");
                item.add_attribute("href", "#" + bundle.get_id(rec)).add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");
                this->make_list_item_label(rec.get_name(), item);

                // write generic repository information for this record
                this->write_generic_record(rec, item);

                HTML_node row1("div");
                row1.add_attribute("class", "row");
                this->make_data_element("Model", rec.get_ics().get_model()->get_name(), row1);
                this->make_data_element("Authors", rec.get_ics().get_model()->get_author(), row1);
                this->make_data_element("Citation data", rec.get_ics().get_model()->get_tag(), row1);

                HTML_node row2("div");
                row2.add_attribute("class", "row");
                this->make_data_element("Initial time", boost::lexical_cast<std::string>(rec.get_ics().get_N_initial()), row2);
                this->make_data_element("Horizon-crossing time", boost::lexical_cast<std::string>(rec.get_ics().get_N_horizon_crossing()), row2);
                this->make_empty_data_element(row2);

                HTML_node row3("div");
                row3.add_attribute("class", "row");

                HTML_node params_column("div");
                params_column.add_attribute("class", "col-md-6");

                const parameters<number>& params = rec.get_ics().get_params();
                const std::vector<number>& param_vec = params.get_vector();
                const std::vector<std::string>& param_names = rec.get_ics().get_model()->get_param_names();

                HTML_node params_label("p");
                HTML_node params_label_heading("h4", "Parameter values");
                params_label_heading.add_attribute("class", "list-group-item-text topskip");
                params_label.add_element(params_label_heading);

                HTML_node params_table("table");
                params_table.add_attribute("class", "table table-striped");
                HTML_node params_table_body("tbody");

                // add separate row for value of Planck mass
                HTML_node Mp_row("tr");
                HTML_node Mp_label("td");
                HTML_string Mp_label_text("Planck mass");
                Mp_label_text.bold();
                Mp_label.add_element(Mp_label_text);
                HTML_node Mp_value("td", boost::lexical_cast<std::string>(params.get_Mp()));
                Mp_row.add_element(Mp_label).add_element(Mp_value);
                params_table_body.add_element(Mp_row);

                for(unsigned int i = 0; i < param_vec.size() && i < param_names.size(); ++i)
                  {
                    HTML_node row("tr");

                    HTML_node label("td");
                    HTML_string label_text(param_names[i]);
                    label_text.bold();
                    label.add_element(label_text);

                    HTML_node value("td", boost::lexical_cast<std::string>(param_vec[i]));

                    row.add_element(label).add_element(value);
                    params_table_body.add_element(row);
                  }

                params_table.add_element(params_table_body);
                params_column.add_element(params_label).add_element(params_table);

                HTML_node ics_column("div");
                ics_column.add_attribute("class", "col-md-6");

                const std::vector<number>& ics_vec = rec.get_ics().get_vector();
                const std::vector<std::string>& coord_names = rec.get_ics().get_model()->get_state_names();

                HTML_node ics_label("p");
                HTML_node ics_label_heading("h4", "Initial conditions");
                ics_label_heading.add_attribute("class", "list-group-item-text topskip");
                ics_label.add_element(ics_label_heading);

                HTML_node ics_table("table");
                ics_table.add_attribute("class", "table table-striped");
                HTML_node ics_table_body("tbody");

                for(unsigned int i = 0; i < ics_vec.size() && i < coord_names.size(); ++i)
                  {
                    HTML_node row("tr");

                    HTML_node label("td");
                    HTML_string label_text(coord_names[i]);
                    label_text.bold();
                    label.add_element(label_text);

                    HTML_node value("td", boost::lexical_cast<std::string>(ics_vec[i]));

                    row.add_element(label).add_element(value);
                    ics_table_body.add_element(row);
                  }

                ics_table.add_element(ics_table_body);
                ics_column.add_element(ics_label).add_element(ics_table);

                row3.add_element(params_column).add_element(ics_column);

                item.add_element(row1).add_element(row2).add_element(row3);

                list.add_element(item);
              }

            parent.add_element(list);
          }


        template <typename number>
        void HTML_report::write_integration(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            typename task_db<number>::type& db = bundle.get_task_db();

            if(db.empty()) return;

            HTML_node list("div");
            list.add_attribute("class", "list-group");

            for(const typename task_db<number>::value_type& task : db)
              {
                const task_record<number>& pre_rec = *task.second;

                if(pre_rec.get_type() == task_type::integration)
                  {
                    const integration_task_record<number>& rec = dynamic_cast< const integration_task_record<number>& >(pre_rec);
                    const std::string tag = bundle.get_id(rec);

                    HTML_node item("a");
                    item.add_attribute("href", "#" + tag).add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");
                    this->make_list_item_label(rec.get_name(), item);

                    // write generic repository information for this record
                    this->write_generic_record(rec, item);

                    HTML_node row1("div");
                    row1.add_attribute("class", "row");
                    this->make_data_element("Task type", task_type_to_string(rec.get_task_type()), row1);

                    HTML_node pkg("div", "Uses package");
                    pkg.add_attribute("class", "col-md-4");
                    this->write_package_button(bundle, rec.get_task()->get_ics().get_name(), pkg);
                    row1.add_element(pkg);

                    this->make_data_element("k-config database", rec.get_relative_kconfig_database_path().string(), row1);

                    item.add_element(row1);

                    // write table of output groups, if any are present
                    const std::list<std::string>& output_groups = rec.get_output_groups();
                    if(!output_groups.empty())
                      {
                        HTML_node button("button");
                        button.add_attribute("data-toggle", "collapse").add_attribute("data-target", "#" + tag + "groups");
                        button.add_attribute("type", "button").add_attribute("class", "btn btn-info topskip");
                        this->make_badged_text("Output groups", output_groups.size(), button);

                        HTML_node group_list("div");
                        group_list.add_attribute("id", tag + "groups").add_attribute("class", "collapse");

                        HTML_node table("table");
                        table.add_attribute("class", "table table-striped");

                        HTML_node head("thead");
                        HTML_node head_row("tr");

                        HTML_node name_label("th", "Name");
                        HTML_node created_label("th", "Created");
                        HTML_node edited_label("th", "Last updated");
                        HTML_node complete_label("th", "Complete");
                        HTML_node size_label("th", "Size");

                        head_row.add_element(name_label).add_element(created_label).add_element(edited_label).add_element(complete_label).add_element(size_label);
                        head.add_element(head_row);

                        HTML_node body("tbody");

                        integration_content_db& content_db = bundle.get_integration_content_db();
                        for(const std::string& group : output_groups)
                          {
                            integration_content_db::const_iterator t = content_db.find(group);

                            HTML_node row("tr");

                            if(t != content_db.end())
                              {
                                HTML_node name("td");
                                this->write_content_button(bundle, group, name);

                                HTML_node created("td", boost::posix_time::to_simple_string(t->second->get_creation_time()));
                                HTML_node edited("td", boost::posix_time::to_simple_string(t->second->get_last_edit_time()));
                                HTML_node complete("td", (t->second->get_payload().is_failed() ? "No" : "Yes"));
                                HTML_node size("td", format_memory(t->second->get_payload().get_size()));

                                row.add_element(name).add_element(created).add_element(edited).add_element(complete).add_element(size);
                              }
                            else
                              {
                                HTML_node name("td");
                                HTML_node label("span", group);
                                label.add_attribute("class", "label label-danger");
                                name.add_element(label);

                                HTML_node null("td");
                                HTML_node null_label("span", "NULL");
                                null_label.add_attribute("class", "label label-danger");
                                null.add_element(null_label);

                                row.add_element(name).add_element(null).add_element(null).add_element(null).add_element(null);
                              }

                            body.add_element(row);
                          }

                        table.add_element(head).add_element(body);
                        group_list.add_element(table);
                        item.add_element(button).add_element(group_list);
                      }

                    list.add_element(item);
                  }
              }

            parent.add_element(list);
          }


        template <typename number>
        void HTML_report::write_postintegration(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            typename task_db<number>::type& db = bundle.get_task_db();

            if(db.empty()) return;

            HTML_node list("div");
            list.add_attribute("class", "list-group");

            for(const typename task_db<number>::value_type& task : db)
              {
                const task_record<number>& pre_rec = *task.second;

                if(pre_rec.get_type() == task_type::postintegration)
                  {
                    const postintegration_task_record<number>& rec = dynamic_cast< const postintegration_task_record<number>& >(pre_rec);
                    const std::string tag = bundle.get_id(rec);

                    HTML_node item("a");
                    item.add_attribute("href", "#" + tag).add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");
                    this->make_list_item_label(rec.get_name(), item);

                    // write generic repository information for this record
                    this->write_generic_record(rec, item);

                    HTML_node row1("div");
                    row1.add_attribute("class", "row");
                    this->make_data_element("Task type", task_type_to_string(rec.get_task_type()), row1);

                    HTML_node parent_tk("div", "Parent task");
                    parent_tk.add_attribute("class", "col-md-4");
                    this->write_task_button(bundle, rec.get_task()->get_parent_task()->get_name(), parent_tk);
                    row1.add_element(parent_tk);

                    this->make_empty_data_element(row1);
                    item.add_element(row1);

                    // write table of output groups, if any are present
                    const std::list<std::string>& output_groups = rec.get_output_groups();
                    if(!output_groups.empty())
                      {
                        HTML_node button("button");
                        button.add_attribute("data-toggle", "collapse").add_attribute("data-target",
                                                                                      "#" + tag + "groups");
                        button.add_attribute("type", "button").add_attribute("class", "btn btn-info topskip");
                        this->make_badged_text("Output groups", output_groups.size(), button);

                        HTML_node group_list("div");
                        group_list.add_attribute("id", tag + "groups").add_attribute("class", "collapse");

                        HTML_node table("table");
                        table.add_attribute("class", "table table-striped");

                        HTML_node head("thead");
                        HTML_node head_row("tr");

                        HTML_node name_label("th", "Name");
                        HTML_node created_label("th", "Created");
                        HTML_node edited_label("th", "Last updated");
                        HTML_node complete_label("th", "Complete");
                        HTML_node size_label("th", "Size");

                        head_row.add_element(name_label).add_element(created_label).add_element(edited_label).add_element(complete_label).add_element(size_label);
                        head.add_element(head_row);

                        HTML_node body("tbody");

                        postintegration_content_db& content_db = bundle.get_postintegration_content_db();
                        for(const std::string& group : output_groups)
                          {
                            postintegration_content_db::const_iterator t = content_db.find(group);

                            HTML_node row("tr");

                            if(t != content_db.end())
                              {
                                HTML_node name("td");
                                this->write_content_button(bundle, group, name);

                                HTML_node created("td", boost::posix_time::to_simple_string(t->second->get_creation_time()));
                                HTML_node edited("td", boost::posix_time::to_simple_string(t->second->get_last_edit_time()));
                                HTML_node complete("td", (t->second->get_payload().is_failed() ? "No" : "Yes"));
                                HTML_node size("td", format_memory(t->second->get_payload().get_size()));

                                row.add_element(name).add_element(created).add_element(edited).add_element(complete).add_element(size);
                              }
                            else
                              {
                                HTML_node name("td");
                                HTML_node label("span", group);
                                label.add_attribute("class", "label label-danger");
                                name.add_element(label);

                                HTML_node null("td");
                                HTML_node null_label("span", "NULL");
                                null_label.add_attribute("class", "label label-danger");
                                null.add_element(null_label);

                                row.add_element(name).add_element(null).add_element(null).add_element(null).add_element(null);
                              }

                            body.add_element(row);
                          }

                        table.add_element(head).add_element(body);
                        group_list.add_element(table);
                        item.add_element(button).add_element(group_list);
                      }

                    list.add_element(item);
                  }
              }

            parent.add_element(list);
          }


        template <typename number>
        void HTML_report::write_output(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            typename task_db<number>::type& db = bundle.get_task_db();

            if(db.empty()) return;

            HTML_node list("div");
            list.add_attribute("class", "list-group");

            for(const typename task_db<number>::value_type& task : db)
              {
                const task_record<number>& pre_rec = *task.second;

                if(pre_rec.get_type() == task_type::output)
                  {
                    const output_task_record<number>& rec = dynamic_cast< const output_task_record<number>& >(pre_rec);
                    const std::string tag = bundle.get_id(rec);

                    HTML_node item("a");
                    item.add_attribute("href", "#" + tag).add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");
                    this->make_list_item_label(rec.get_name(), item);

                    // write generic repository information for this record
                    this->write_generic_record(rec, item);

                    // write elements
                    const typename std::vector< output_task_element<number> >& elements = rec.get_task()->get_elements();
                    if(!elements.empty())
                      {
                        HTML_node button("button");
                        button.add_attribute("data-toggle", "collapse").add_attribute("data-target", "#" + tag + "elements");
                        button.add_attribute("type", "button").add_attribute("class", "btn btn-info topskip");
                        this->make_badged_text("Derived products", elements.size(), button);

                        HTML_node group_list("div");
                        group_list.add_attribute("id", tag + "elements").add_attribute("class", "collapse");

                        HTML_node table("table");
                        table.add_attribute("class", "table table-striped");

                        HTML_node head("thead");
                        HTML_node head_row("tr");

                        HTML_node name_label("th", "Name");
                        HTML_node type_label("th", "Type");
                        HTML_node requires_label("th", "Requires tags");
                        HTML_node filename_label("th", "Filename");

                        head_row.add_element(name_label).add_element(type_label).add_element(requires_label).add_element(filename_label);
                        head.add_element(head_row);

                        HTML_node body("tbody");

                        for(const output_task_element<number>& element : elements)
                          {
                            HTML_node row("tr");

                            HTML_node name("td");
                            this->write_derived_product_button(bundle, element.get_product_name(), name);

                            HTML_node type("td", derived_data::derived_product_type_to_string(element.get_product().get_type()));
                            HTML_node requires("td");
                            this->compose_tag_list(element.get_tags(), requires);

                            HTML_node filename("td", element.get_product().get_filename().string());

                            row.add_element(name).add_element(type).add_element(requires).add_element(filename);
                            body.add_element(row);
                          }

                        table.add_element(head).add_element(body);
                        group_list.add_element(table);
                        item.add_element(button).add_element(group_list);
                      }

                    // write table of output groups
                    const std::list<std::string>& output_groups = rec.get_output_groups();
                    if(!output_groups.empty())
                      {
                        HTML_node button("button");
                        button.add_attribute("data-toggle", "collapse").add_attribute("data-target",
                                                                                      "#" + tag + "groups");
                        button.add_attribute("type", "button").add_attribute("class", "btn btn-info topskip");
                        this->make_badged_text("Output groups", output_groups.size(), button);

                        HTML_node group_list("div");
                        group_list.add_attribute("id", tag + "groups").add_attribute("class", "collapse");

                        HTML_node table("table");
                        table.add_attribute("class", "table table-striped");

                        HTML_node head("thead");
                        HTML_node head_row("tr");

                        HTML_node name_label("th", "Name");
                        HTML_node created_label("th", "Created");
                        HTML_node edited_label("th", "Last updated");

                        head_row.add_element(name_label).add_element(created_label).add_element(edited_label);
                        head.add_element(head_row);

                        HTML_node body("tbody");

                        output_content_db& content_db = bundle.get_output_content_db();
                        for(const std::string& group : output_groups)
                          {
                            output_content_db::const_iterator t = content_db.find(group);

                            HTML_node row("tr");

                            if(t != content_db.end())
                              {
                                HTML_node name("td");
                                this->write_content_button(bundle, group, name);

                                HTML_node created("td", boost::posix_time::to_simple_string(t->second->get_creation_time()));
                                HTML_node edited("td", boost::posix_time::to_simple_string(t->second->get_last_edit_time()));

                                row.add_element(name).add_element(created).add_element(edited);
                              }
                            else
                              {
                                HTML_node name("td");
                                HTML_node label("span", group);
                                label.add_attribute("class", "label label-danger");
                                name.add_element(label);

                                HTML_node null("td");
                                HTML_node null_label("span", "NULL");
                                null_label.add_attribute("class", "label label-danger");
                                null.add_element(null_label);

                                row.add_element(name).add_element(null).add_element(null);
                              }

                            body.add_element(row);
                          }

                        table.add_element(head).add_element(body);
                        group_list.add_element(table);
                        item.add_element(button).add_element(group_list);
                      }

                    list.add_element(item);
                  }
              }

            parent.add_element(list);
          }


        template <typename number>
        void HTML_report::write_derived_products(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            typename derived_product_db<number>::type& db = bundle.get_derived_product_db();

            if(db.empty()) return;

            HTML_node list("div");
            list.add_attribute("class", "list-group");

            for(const typename derived_product_db<number>::value_type& product : db)
              {
                const derived_product_record<number>& rec = *product.second;
                const std::string tag = bundle.get_id(rec);

                HTML_node item("a");
                item.add_attribute("href", "#" + tag).add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");
                this->make_list_item_label(rec.get_name(), item);

                // write generic repository information for this record
                this->write_generic_record(rec, item);

                HTML_node row1("div");
                row1.add_attribute("class", "row");
                this->make_data_element("Product type", derived_data::derived_product_type_to_string(rec.get_product()->get_type()), row1);
                this->make_data_element("Filename", rec.get_product()->get_filename().string(), row1);
                this->make_empty_data_element(row1);

                item.add_element(row1);

                typename std::list< derivable_task<number>* > task_list;
                rec.get_product()->get_task_list(task_list);

                if(!task_list.empty())
                  {
                    HTML_node button("button");
                    button.add_attribute("data-toggle", "collapse").add_attribute("data-target", "#" + tag + "tasks");
                    button.add_attribute("type", "button").add_attribute("class", "btn btn-info topskip");
                    this->make_badged_text("Depends on tasks", task_list.size(), button);

                    HTML_node group_list("div");
                    group_list.add_attribute("id", tag + "tasks").add_attribute("class", "collapse");

                    HTML_node table("table");
                    table.add_attribute("class", "table table-striped");

                    HTML_node head("thead");
                    HTML_node head_row("tr");

                    HTML_node name_label("th", "Name");

                    head_row.add_element(name_label);
                    head.add_element(head_row);

                    HTML_node body("tbody");

                    for(derivable_task<number>* tk : task_list)
                      {
                        HTML_node row("tr");

                        HTML_node name("td");
                        this->write_task_button(bundle, tk->get_name(), name);

                        row.add_element(name);
                        body.add_element(row);
                      }

                    table.add_element(head).add_element(body);
                    group_list.add_element(table);
                    item.add_element(button).add_element(group_list);
                  }

                list.add_element(item);
              }

            parent.add_element(list);
          }


        template <typename number>
        void HTML_report::write_integration_content(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            integration_content_db& db = bundle.get_integration_content_db();

            for(const integration_content_db::value_type& group : db)
              {
                const output_group_record<integration_payload>& rec = *group.second;
                std::string tag = bundle.get_id(rec);

                HTML_node pane("div");
                pane.add_attribute("id", tag).add_attribute("class", "tab-pane fade");

                HTML_node list("div");
                list.add_attribute("class", "list-group");

                HTML_node anchor("a");
                anchor.add_attribute("href", "#").add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");

                this->make_list_item_label(rec.get_name(), anchor);
                this->write_generic_output_record(bundle, rec, anchor);

                const integration_payload& payload = rec.get_payload();

                HTML_node properties_label("p");
                HTML_node properties_heading("h4", "Properties");
                properties_heading.add_attribute("class", "list-group-item-text topskip");
                properties_label.add_element(properties_heading);

                HTML_node row1("div");
                row1.add_attribute("class", "row");
                this->make_data_element("Container", payload.get_container_path().string(), row1, "label-default", "col-md-12");

                HTML_node row2("div");
                row2.add_attribute("class", "row");
                this->make_data_element("Complete", (payload.is_failed() ? "No" : "Yes"), row2);
                this->make_data_element("Workgroup", boost::lexical_cast<std::string>(payload.get_workgroup_number()), row2);
                HTML_node seeded("div", "Seeded");
                seeded.add_attribute("class", "col-md-4");
                this->write_seeded(bundle, payload, seeded);
                row2.add_element(seeded);

                HTML_node row3("div");
                row3.add_attribute("class", "row");
                this->make_data_element("Statistics", (payload.has_statistics() ? "Yes" : "No"), row3);
                this->make_data_element("Initial conditions", (payload.has_initial_conditions() ? "Yes" : "No"), row3);
                this->make_data_element("Size", format_memory(payload.get_size()), row3);

                const integration_metadata& metadata = payload.get_metadata();

                HTML_node row4("div");
                row4.add_attribute("class", "row");
                this->make_data_element("Wallclock time", format_time(metadata.total_wallclock_time), row4);
                this->make_data_element("Total time", format_time(metadata.total_integration_time), row4);
                this->make_data_element("Configurations", boost::lexical_cast<std::string>(metadata.total_configurations), row4);

                HTML_node row5("div");
                row5.add_attribute("class", "row");
                this->make_data_element("Mean integration time", format_time(metadata.total_integration_time/metadata.total_configurations), row5);
                this->make_data_element("Shortest integration time", format_time(metadata.global_min_integration_time), row5);
                this->make_data_element("Longest integration time", format_time(metadata.global_max_integration_time), row5);

                HTML_node row6("div");
                row6.add_attribute("class", "row");
                this->make_data_element("Failures", boost::lexical_cast<std::string>(metadata.total_failures), row6);
                this->make_empty_data_element(row6);
                this->make_empty_data_element(row6);

                anchor.add_element(properties_label).add_element(row1).add_element(row2).add_element(row3).add_element(row4).add_element(row5).add_element(row6);
                list.add_element(anchor);
                pane.add_element(list);
                parent.add_element(pane);
              }
          }


        template <typename number>
        void HTML_report::write_postintegration_content(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            postintegration_content_db& db = bundle.get_postintegration_content_db();

            for(const postintegration_content_db::value_type& group : db)
              {
                const output_group_record<postintegration_payload>& rec = *group.second;
                std::string tag = bundle.get_id(rec);

                HTML_node pane("div");
                pane.add_attribute("id", tag).add_attribute("class", "tab-pane fade");

                HTML_node list("div");
                list.add_attribute("class", "list-group");

                HTML_node anchor("a");
                anchor.add_attribute("href", "#").add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");

                this->make_list_item_label(rec.get_name(), anchor);
                this->write_generic_output_record(bundle, rec, anchor);

                const postintegration_payload& payload = rec.get_payload();

                HTML_node properties_label("p");
                HTML_node properties_heading("h4", "Properties");
                properties_heading.add_attribute("class", "list-group-item-text topskip");
                properties_label.add_element(properties_heading);

                HTML_node row1("div");
                row1.add_attribute("class", "row");
                this->make_data_element("Container", payload.get_container_path().string(), row1, "label-default", "col-md-12");

                HTML_node row2("div");
                row2.add_attribute("class", "row");
                this->make_data_element("Complete", (payload.is_failed() ? "No" : "Yes"), row2);
                HTML_node pt("div", "Parent");
                pt.add_attribute("class", "col-md-4");
                this->write_content_button(bundle, payload.get_parent_group(), pt);
                HTML_node seeded("div", "Seeded");
                seeded.add_attribute("class", "col-md-4");
                this->write_seeded(bundle, payload, seeded);
                row2.add_element(pt).add_element(seeded);

                HTML_node row3("div");
                row3.add_attribute("class", "row");
                this->make_data_element("Size", format_memory(payload.get_size()), row3);
                this->make_empty_data_element(row3);
                this->make_empty_data_element(row3);

                anchor.add_element(properties_label).add_element(row1).add_element(row2).add_element(row3);
                list.add_element(anchor);
                pane.add_element(list);
                parent.add_element(pane);
              }
          }


        template <typename number>
        void HTML_report::write_output_content(HTML_report_bundle<number>& bundle, HTML_node& parent)
          {
            output_content_db& db = bundle.get_output_content_db();
            typename derived_product_db<number>::type& product_db = bundle.get_derived_product_db();

            for(const output_content_db::value_type& group : db)
              {
                const output_group_record<output_payload>& rec = *group.second;
                std::string tag = bundle.get_id(rec);

                HTML_node pane("div");
                pane.add_attribute("id", tag).add_attribute("class", "tab-pane fade");

                HTML_node list("div");
                list.add_attribute("class", "list-group");

                HTML_node anchor("a");
                anchor.add_attribute("href", "#").add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");

                this->make_list_item_label(rec.get_name(), anchor);
                this->write_generic_output_record(bundle, rec, anchor);

                const output_payload& payload = rec.get_payload();
                const std::list<derived_content>& content = payload.get_derived_content();

                if(!content.empty())
                  {
                    // make_asset_directory() will throw an exception if it cannot succeed
                    boost::filesystem::path asset_directory = bundle.make_asset_directory(rec.get_name());

                    HTML_node content_list("ul");
                    content_list.add_attribute("class", "list-group topskip");

                    unsigned int count = 0;
                    for(const derived_content& item : content)
                      {
                        boost::filesystem::path filename = item.get_filename();
                        boost::filesystem::path extension = filename.extension();

                        HTML_node it("li");
                        it.add_attribute("href", "#").add_attribute("class", "list-group-item").add_attribute("onclick", "return false;");
                        HTML_node title("h4", filename.string());
                        title.add_attribute("class", "list-group-item-text topskip");
                        it.add_element(title);

                        boost::filesystem::path abs_product_location = rec.get_abs_output_path() / filename;
                        boost::filesystem::path relative_asset_location = bundle.emplace_asset(abs_product_location, rec.get_name(), filename);

                        if(extension.string() == ".pdf")
                          {
                            // Safari will display PDFs in an <img> tag, but Chrome and Firefox will not so we choose to emebed
                            // as PDF objects instead
                            HTML_node pdf("div");
                            pdf.add_attribute("class", "pdfproduct topskip");

                            HTML_node obj("object");
                            obj.add_attribute("data", relative_asset_location.string()).add_attribute("type", "application/pdf");
                            obj.add_attribute("width", "100%").add_attribute("height", "100%");

                            HTML_node fallback("p", "PDF embedding not supported in this browser");
                            obj.add_element(fallback);

                            pdf.add_element(obj);
                            it.add_element(pdf);
                          }
                        else if(extension.string() == ".png" || extension.string() == ".svg" || extension.string() == ".svgz")
                          {
                            HTML_node img("img", false);
                            img.add_attribute("class", "img-responsive imgproduct").add_attribute("src", relative_asset_location.string()).add_attribute("alt", filename.string());
                            it.add_element(img);
                          }
                        else if(extension.string() == ".txt")
                          {
                            HTML_node well("div");
                            well.add_attribute("class", "well topskip");
                            HTML_node frame("iframe", filename.string());
                            frame.add_attribute("src", relative_asset_location.string()).add_attribute("class", "iframeproduct");
                            well.add_element(frame);
                            it.add_element(well);
                          }
                        else
                          {
                            HTML_node well("div", "Cannot embed this content");
                            well.add_attribute("class", "well");
                            it.add_element(well);
                          }

                        typename derived_product_db<number>::type::const_iterator t = product_db.find(item.get_parent_product());

                        HTML_node row1("div");
                        row1.add_attribute("class", "row topskip");

                        if(t != product_db.end())
                          {
                            this->make_data_element("Type", derived_data::derived_product_type_to_string(t->second->get_product()->get_type()), row1);
                          }
                        else
                          {
                            HTML_node null_label("span", "NULL");
                            null_label.add_attribute("class", "label label-danger");
                            row1.add_element(null_label);
                          }
                        this->make_data_element("Created", boost::posix_time::to_simple_string(item.get_creation_time()), row1);

                        HTML_node tg("div", "Tags");
                        tg.add_attribute("class", "col-md-4");
                        this->compose_tag_list(item.get_tags(), tg);
                        row1.add_element(tg);

                        it.add_element(row1);

                        // Write out notes if present
                        this->write_notes_collapsible(item.get_notes(), tag, it);

                        const std::list<std::string>& groups = item.get_content_groups();
                        if(!groups.empty())
                          {
                            std::string group_tag = tag + "_" + boost::lexical_cast<std::string>(count);
                            ++count;

                            HTML_node button("button");
                            button.add_attribute("type", "button").add_attribute("class", "btn btn-info topskip");
                            button.add_attribute("data-toggle", "collapse").add_attribute("data-target", "#" + group_tag);
                            this->make_badged_text("Data provenance", groups.size(), button);

                            HTML_node data("div");
                            data.add_attribute("id", group_tag).add_attribute("class", "collapse");

                            HTML_node table("table");
                            table.add_attribute("class", "table table-striped");

                            HTML_node head("thead");
                            HTML_node head_row("tr");

                            HTML_node name_label("th", "Name");
                            HTML_node task_label("th", "Task");
                            HTML_node type_label("th", "Type");
                            HTML_node edited_label("th", "Last updated");

                            head_row.add_element(name_label).add_element(task_label).add_element(type_label).add_element(edited_label);
                            head.add_element(head_row);

                            HTML_node body("tbody");

                            for(const std::string& gp : groups)
                              {
                                HTML_node row("tr");

                                HTML_node name("td");
                                this->write_content_button(bundle, gp, name);

                                content_group_data<number> properties(gp, bundle.get_cache());

                                HTML_node task("td");
                                this->write_task_button(bundle, properties.get_task(), task);

                                HTML_node type("td", properties.get_type());
                                HTML_node edited("td", properties.get_last_edit());

                                row.add_element(name).add_element(task).add_element(type).add_element(edited);
                                body.add_element(row);
                              }

                            table.add_element(head).add_element(body);
                            data.add_element(table);
                            it.add_element(button).add_element(data);
                          }

                        content_list.add_element(it);
                      }

                    anchor.add_element(content_list);
                  }

                list.add_element(anchor);
                pane.add_element(list);
                parent.add_element(pane);
              }
          }


        std::string HTML_report::compose_tag_list(const std::list<std::string>& tags, HTML_node& parent)
          {
            std::ostringstream out;

            if(!tags.empty())
              {
                unsigned int count = 0;
                for(const std::string& tag : tags)
                  {
                    if(count > 0)
                      {
                        HTML_string pad("&nbsp;");
                        parent.add_element(pad);
                      }

                    HTML_node tg("span", tag);
                    tg.add_attribute("class", "label label-info");

                    parent.add_element(tg);
                    ++count;
                  }
              }
            else
              {
                HTML_node tg("span", "None");
                tg.add_attribute("class", "label label-default");

                parent.add_element(tg);
              }

            return(out.str());
          }


        template <typename RecordType>
        void HTML_report::write_generic_record(const RecordType& rec, HTML_node& parent)
          {
            boost::posix_time::ptime created = rec.get_creation_time();
            boost::posix_time::ptime edited = rec.get_last_edit_time();

            HTML_node row("div");
            row.add_attribute("class", "row");

            this->make_data_element("Created", boost::posix_time::to_simple_string(created), row, "label-primary");
            this->make_data_element("Last updated", boost::posix_time::to_simple_string(edited), row, "label-primary");
            this->make_data_element("Runtime version", boost::lexical_cast<std::string>(rec.get_runtime_API_version()), row, "label-primary");

            parent.add_element(row);
          }


        template <typename number, typename Payload>
        void HTML_report::write_generic_output_record(HTML_report_bundle<number>& bundle, const output_group_record<Payload>& rec,
                                                      HTML_node& parent)
          {
            boost::posix_time::ptime created = rec.get_creation_time();
            boost::posix_time::ptime edited = rec.get_last_edit_time();

            HTML_node row1("div");
            row1.add_attribute("class", "row");

            this->make_data_element("Created", boost::posix_time::to_simple_string(created), row1, "label-primary");
            this->make_data_element("Last updated", boost::posix_time::to_simple_string(edited), row1, "label-primary");
            this->make_data_element("Runtime version", boost::lexical_cast<std::string>(rec.get_runtime_API_version()), row1, "label-primary");

            HTML_node row2("div");
            row2.add_attribute("class", "row");

            HTML_node pt("div", "Parent task");
            pt.add_attribute("class", "col-md-4");
            this->write_task_button(bundle, rec.get_task_name(), pt);
            row2.add_element(pt);

            this->make_data_element("Locked", (rec.get_lock_status() ? "Yes" : "No"), row2, "label-primary");

            HTML_node tags("div", "Tags");
            tags.add_attribute("class", "col-md-4");
            this->compose_tag_list(rec.get_tags(), tags);
            row2.add_element(tags);

            parent.add_element(row1).add_element(row2);

            this->write_notes_collapsible(rec.get_notes(), bundle.get_id(rec), parent);
          }


        template <typename number, typename Payload>
        void HTML_report::write_seeded(HTML_report_bundle<number>& bundle, const Payload& payload, HTML_node& parent)
          {
            if(payload.is_seeded())
              {
                this->write_task_button(bundle, payload.get_seed_group(), parent);
              }
            else
              {
                HTML_node span("span", "No");
                span.add_attribute("class", "label label-default");
                parent.add_element(span);
              }
          }


        template <typename number>
        void HTML_report::write_JavaScript_button_scroll(HTML_report_bundle<number>& bundle, std::string button, std::string pane, std::string element)
          {
            // we need a Javascript function which causes this button to activate the appropriate task pane when clicked
            bundle.write_JavaScript("$(function(){");
            bundle.write_JavaScript("    $('#" + button + "').click(function(e){");
            bundle.write_JavaScript("        e.preventDefault();");
            bundle.write_JavaScript("        var tab_element = $('#tabs').find('a[href=\"#" + pane + "\"]');");
            bundle.write_JavaScript("        tab_element.tab('show');");
            bundle.write_JavaScript("        tab_element.one('shown.bs.tab', function(e2){");
            bundle.write_JavaScript("            var list_element = $('#" + pane + "').find('a[href=\"#" + element + "\"]');");
            bundle.write_JavaScript("            $('html, body').animate({ scrollTop: list_element.offset().top - $(\".navbar\").height() - 40 }, 1000);");
            bundle.write_JavaScript("        });");
            bundle.write_JavaScript("    });");
            bundle.write_JavaScript("});");
          }


        template <typename number>
        void HTML_report::write_JavaScript_button(HTML_report_bundle<number>& bundle, std::string button, std::string pane)
          {
            // we need a Javascript function which causes this button to activate the appropriate contents pane when clicked
            bundle.write_JavaScript("$(function(){");
            bundle.write_JavaScript("    $('#" + button + "').click(function(e){");
            bundle.write_JavaScript("        e.preventDefault();");
            bundle.write_JavaScript("        $('#tabs').find('a[href=\"#" + pane + "\"]').tab('show');");
            bundle.write_JavaScript("    })");
            bundle.write_JavaScript("});");
          }


        template <typename number>
        void HTML_report::write_task_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent)
          {
            typename task_db<number>::type& tk_db = bundle.get_task_db();
            typename task_db<number>::type::const_iterator t = tk_db.find(name);

            if(t != tk_db.end())
              {
                // find task tag
                std::string element_id = bundle.get_id(*t->second);
                std::string button_id = this->make_button_tag();

                HTML_node button("button");
                button.add_attribute("type", "button").add_attribute("class", "btn btn-link").add_attribute("id", button_id);
                this->make_badged_text(name, t->second->get_output_groups().size(), button);

                parent.add_element(button);

                std::string pane;
                switch(t->second->get_type())
                  {
                    case task_type::integration:
                      {
                        pane = "integration";
                        break;
                      }

                    case task_type::postintegration:
                      {
                        pane = "postintegration";
                        break;
                      }

                    case task_type::output:
                      {
                        pane = "output";
                        break;
                      }
                  }

                this->write_JavaScript_button_scroll(bundle, button_id, pane, element_id);
              }
            else
              {
                HTML_node fail("span", name);
                fail.add_attribute("class", "label label-danger");

                parent.add_element(fail);
              }
          }


        template <typename number>
        void HTML_report::write_content_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent)
          {
            integration_content_db& int_db = bundle.get_integration_content_db();
            postintegration_content_db& pint_db = bundle.get_postintegration_content_db();
            output_content_db& output_db = bundle.get_output_content_db();

            integration_content_db::const_iterator int_t = int_db.find(name);
            postintegration_content_db::const_iterator pint_t = pint_db.find(name);
            output_content_db::const_iterator out_t = output_db.find(name);

            std::string pane;
            if(int_t != int_db.end()) pane = bundle.get_id(*int_t->second);
            else if(pint_t != pint_db.end()) pane = bundle.get_id(*pint_t->second);
            else if(out_t != output_db.end()) pane = bundle.get_id(*out_t->second);

            this->write_content_button(bundle, name, pane, parent);
          }


        template <typename number>
        void HTML_report::write_content_button(HTML_report_bundle<number>& bundle, const std::string& name, const std::string& pane, HTML_node& parent)
          {
            if(!pane.empty())
              {
                std::string button_id = this->make_button_tag();

                HTML_node button("button", name);
                button.add_attribute("type", "button").add_attribute("class", "btn btn-link").add_attribute("id", button_id);

                parent.add_element(button);

                this->write_JavaScript_button(bundle, button_id, pane);
              }
            else
              {
                HTML_node fail("span", name);
                fail.add_attribute("class", "label label-danger");

                parent.add_element(fail);
              }
          }


        template <typename number>
        void HTML_report::write_package_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent)
          {
            typename package_db<number>::type& db = bundle.get_package_db();
            typename package_db<number>::type::const_iterator t = db.find(name);

            std::string element;
            if(t != db.end()) element = bundle.get_id(*t->second);

            this->write_package_button(bundle, name, element, parent);
          }


        template <typename number>
        void HTML_report::write_package_button(HTML_report_bundle<number>& bundle, const std::string& name, const std::string& element, HTML_node& parent)
          {
            if(!element.empty())
              {
                std::string button_id = this->make_button_tag();

                HTML_node button("button", name);
                button.add_attribute("type", "button").add_attribute("class", "btn btn-link").add_attribute("id", button_id);

                parent.add_element(button);

                this->write_JavaScript_button_scroll(bundle, button_id, "packages", element);
              }
            else
              {
                HTML_node fail("span", name);
                fail.add_attribute("class", "label label-danger");

                parent.add_element(fail);
              }
          }


        template <typename number>
        void HTML_report::write_derived_product_button(HTML_report_bundle<number>& bundle, const std::string& name, HTML_node& parent)
          {
            typename derived_product_db<number>::type& db = bundle.get_derived_product_db();
            typename derived_product_db<number>::type::const_iterator t = db.find(name);

            std::string element;
            if(t != db.end()) element = bundle.get_id(*t->second);

            this->write_derived_product_button(bundle, name, element, parent);
          }


        template <typename number>
        void HTML_report::write_derived_product_button(HTML_report_bundle<number>& bundle, const std::string& name, const std::string& element, HTML_node& parent)
          {
            if(!element.empty())
              {
                std::string button_id = this->make_button_tag();

                HTML_node button("button", name);
                button.add_attribute("type", "button").add_attribute("class", "btn btn-link").add_attribute("id", button_id);

                parent.add_element(button);

                this->write_JavaScript_button_scroll(bundle, button_id, "derived", element);
              }
            else
              {
                HTML_node fail("span", name);
                fail.add_attribute("class", "label label-danger");

                parent.add_element(fail);
              }
          }


        void HTML_report::write_notes_collapsible(const std::list<std::string>& notes, const std::string& tag, HTML_node& parent)
          {
            if(notes.empty()) return;

            HTML_node button("button");
            button.add_attribute("type", "button").add_attribute("class", "btn btn-info topbottomskip");
            button.add_attribute("data-toggle", "collapse").add_attribute("data-target", "#" + tag + "notes");
            this->make_badged_text("Notes", notes.size(), button);

            HTML_node content("div");
            content.add_attribute("id", tag + "notes").add_attribute("class", "collapse");

            HTML_node list("ol");
            list.add_attribute("class", "list-group");

            for(const std::string& note : notes)
              {
                HTML_node item("li", note);
                item.add_attribute("class", "list-group-item list-group-item-info");
                list.add_element(item);
              }

            content.add_element(list);
            parent.add_element(button).add_element(content);
          }


        std::string HTML_report::make_button_tag()
          {
            std::ostringstream tag;
            tag << CPPTRANSPORT_HTML_BUTTON_TAG_PREFIX << this->button_id++;
            return(tag.str());
          }


      }   // namespace reporting

  }   // namespace transport


#endif //CPPTRANSPORT_REPORTING_HTML_REPORT_H
