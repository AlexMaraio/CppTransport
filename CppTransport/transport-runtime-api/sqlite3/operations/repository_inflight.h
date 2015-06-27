//
// Created by David Seery on 22/06/15.
// Copyright (c) 2015 University of Sussex. All rights reserved.
//

#ifndef CPPTRANSPORT_REPOSITORY_INFLIGHT_H
#define CPPTRANSPORT_REPOSITORY_INFLIGHT_H


#include "transport-runtime-api/sqlite3/operations/repository_common.h"


namespace transport
  {

    namespace sqlite3_operations
      {

        class inflight_group
          {
          public:
            std::string             name;
            std::string             task_name;
            boost::filesystem::path path;
            std::string             posix_time;
          };


        class inflight_integration
          {
          public:
            std::string             content_group;
            std::string             task_name;
            boost::filesystem::path output;
            boost::filesystem::path container;
            boost::filesystem::path logdir;
            boost::filesystem::path tempdir;
            unsigned int            workgroup_number;
            bool                    is_seeded;
            std::string             seed_group;
            bool                    is_collecting_stats;
            bool                    is_collecting_ics;
          };


        class inflight_postintegration
          {
          public:
            std::string             content_group;
            std::string             task_name;
            boost::filesystem::path output;
            boost::filesystem::path container;
            boost::filesystem::path logdir;
            boost::filesystem::path tempdir;
            bool                    is_paired;
            std::string             parent_group;
            bool                    is_seeded;
            std::string             seed_group;
          };


        class inflight_derived_content
          {
          public:
            std::string             content_group;
            std::string             task_name;
            boost::filesystem::path output;
            boost::filesystem::path logdir;
            boost::filesystem::path tempdir;
          };


        std::string reserve_content_name(transaction_manager& mgr, sqlite3* db, const std::string& tk, boost::filesystem::path& parent_path,
                                         const std::string& posix_time_string, const std::string& suffix)
          {
            std::string filename = posix_time_string;
            if(suffix.length() > 0) filename += "-" + suffix;

            // check if a content group with this filename already exists
            unsigned int count = internal_count(db, filename, CPPTRANSPORT_SQLITE_RESERVED_CONTENT_NAMES_TABLE, "name");

            if(count > 0)
              {
                // a content group has already been reserved, so build a new unique name
                count = internal_count(db, posix_time_string, CPPTRANSPORT_SQLITE_RESERVED_CONTENT_NAMES_TABLE, "posix_time");

                filename = posix_time_string;
                if(count > 0) filename += "-" + boost::lexical_cast<std::string>(count);
                if(suffix.length() > 0) filename += "-" + suffix;
              }

            boost::filesystem::path output_path = parent_path / filename;

            std::stringstream store_stmt;
            store_stmt << "INSERT INTO " << CPPTRANSPORT_SQLITE_RESERVED_CONTENT_NAMES_TABLE << " VALUES (@name, @task, @path, @posix_time)";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, store_stmt.str().c_str(), store_stmt.str().length()+1, &stmt, nullptr));

            check_stmt(db, sqlite3_bind_text(stmt, 1, filename.c_str(), filename.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 2, tk.c_str(), tk.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 3, output_path.string().c_str(), output_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 4, posix_time_string.c_str(), posix_time_string.length(), SQLITE_STATIC));

            check_stmt(db, sqlite3_step(stmt), CPPTRANSPORT_REPO_STORE_RESERVE_FAIL, SQLITE_DONE);

            check_stmt(db, sqlite3_clear_bindings(stmt));
            check_stmt(db, sqlite3_finalize(stmt));

            return(filename);
          }


        void enumerate_inflight_groups(sqlite3* db, std::list<inflight_group>& groups)
          {
            std::stringstream find_stmt;
            find_stmt << "SELECT name, task, path, posix_time FROM " << CPPTRANSPORT_SQLITE_RESERVED_CONTENT_NAMES_TABLE << ";";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, find_stmt.str().c_str(), find_stmt.str().length()+1, &stmt, nullptr));

            int status;
            groups.clear();
            while((status = sqlite3_step(stmt)) != SQLITE_DONE)
              {
                if(status == SQLITE_ROW)
                  {
                    inflight_group gp;

                    const char* sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                    gp.name = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    gp.task_name = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    gp.path = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    gp.posix_time = std::string(sqlite_str);

                    groups.push_back(gp);
                  }
              }

            check_stmt(db, sqlite3_finalize(stmt));
          }


        void register_integration_writer(transaction_manager& mgr, sqlite3* db, const std::string& name, const std::string& task,
                                         const boost::filesystem::path& output_path, const boost::filesystem::path& sql_path,
                                         const boost::filesystem::path& logdir_path, const boost::filesystem::path& tempdir_path,
                                         unsigned int workgroup_number, bool is_seeded, const std::string& seed_group,
                                         bool is_collecting_stats, bool is_collecting_ics)
          {
            std::stringstream store_stmt;
            store_stmt << "INSERT INTO " << CPPTRANSPORT_SQLITE_INTEGRATION_WRITERS_TABLE << " VALUES (@content_group, @task, @output, @container, @logdir, @tempdir, @workgroup_number, @seeded, @seed_group, @collect_stats, @collect_ics)";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, store_stmt.str().c_str(), store_stmt.str().length()+1, &stmt, nullptr));

            check_stmt(db, sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 2, task.c_str(), task.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 3, output_path.string().c_str(), output_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 4, sql_path.string().c_str(), sql_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 5, logdir_path.string().c_str(), logdir_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 6, tempdir_path.string().c_str(), tempdir_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_int(stmt, 7, workgroup_number));
            check_stmt(db, sqlite3_bind_int(stmt, 8, static_cast<int>(is_seeded)));
            check_stmt(db, sqlite3_bind_text(stmt, 9, seed_group.c_str(), seed_group.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_int(stmt, 10, static_cast<int>(is_collecting_stats)));
            check_stmt(db, sqlite3_bind_int(stmt, 11, static_cast<int>(is_collecting_ics)));

            check_stmt(db, sqlite3_step(stmt), CPPTRANSPORT_REPO_STORE_INTEGRATION_WRITER_FAIL, SQLITE_DONE);

            check_stmt(db, sqlite3_clear_bindings(stmt));
            check_stmt(db, sqlite3_finalize(stmt));
          }


        void register_postintegration_writer(transaction_manager& mgr, sqlite3* db, const std::string& name, const std::string& task,
                                             const boost::filesystem::path& output_path, const boost::filesystem::path& container,
                                             const boost::filesystem::path& logdir_path, const boost::filesystem::path& tempdir_path,
                                             bool is_paired, const std::string& parent_group,
                                             bool is_seeded, const std::string& seed_group)
          {
            std::stringstream store_stmt;
            store_stmt << "INSERT INTO " << CPPTRANSPORT_SQLITE_POSTINTEGRATION_WRITERS_TABLE << " VALUES (@content_group, @task, @output, @container, @logdir, @tempdir, @paired, @parent, @seeded, @seed_group)";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, store_stmt.str().c_str(), store_stmt.str().length()+1, &stmt, nullptr));

            check_stmt(db, sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 2, task.c_str(), task.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 3, output_path.string().c_str(), output_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 4, container.string().c_str(), container.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 5, logdir_path.string().c_str(), logdir_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 6, tempdir_path.string().c_str(), tempdir_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_int(stmt, 7, static_cast<int>(is_paired)));
            check_stmt(db, sqlite3_bind_text(stmt, 8, parent_group.c_str(), parent_group.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_int(stmt, 9, static_cast<int>(is_seeded)));
            check_stmt(db, sqlite3_bind_text(stmt, 10, seed_group.c_str(), seed_group.length(), SQLITE_STATIC));

            check_stmt(db, sqlite3_step(stmt), CPPTRANSPORT_REPO_STORE_POSTINTEGRATION_WRITER_FAIL, SQLITE_DONE);

            check_stmt(db, sqlite3_clear_bindings(stmt));
            check_stmt(db, sqlite3_finalize(stmt));
          }


        void register_derived_content_writer(transaction_manager& mgr, sqlite3* db, const std::string& name, const std::string& task,
                                             const boost::filesystem::path& output_path, const boost::filesystem::path& logdir_path, const boost::filesystem::path& tempdir_path)
          {
            std::stringstream store_stmt;
            store_stmt << "INSERT INTO " << CPPTRANSPORT_SQLITE_DERIVED_WRITERS_TABLE << " VALUES (@content_group, @task, @output, @logdir, @tempdir)";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, store_stmt.str().c_str(), store_stmt.str().length()+1, &stmt, nullptr));

            check_stmt(db, sqlite3_bind_text(stmt, 1, name.c_str(), name.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 2, task.c_str(), task.length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 3, output_path.string().c_str(), output_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 4, logdir_path.string().c_str(), logdir_path.string().length(), SQLITE_STATIC));
            check_stmt(db, sqlite3_bind_text(stmt, 5, tempdir_path.string().c_str(), tempdir_path.string().length(), SQLITE_STATIC));

            check_stmt(db, sqlite3_step(stmt), CPPTRANSPORT_REPO_STORE_DERIVED_CONTENT_WRITER_FAIL, SQLITE_DONE);

            check_stmt(db, sqlite3_clear_bindings(stmt));
            check_stmt(db, sqlite3_finalize(stmt));
          }


        void enumerate_inflight_integrations(sqlite3* db, std::list<inflight_integration>& groups)
          {
            std::stringstream find_stmt;
            find_stmt << "SELECT content_group, task, output, container, logdir, tempdir, workgroup_number, seeded, seed_group, collect_stats, collect_ics FROM " << CPPTRANSPORT_SQLITE_INTEGRATION_WRITERS_TABLE << ";";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, find_stmt.str().c_str(), find_stmt.str().length()+1, &stmt, nullptr));

            int status;
            groups.clear();
            while((status = sqlite3_step(stmt)) != SQLITE_DONE)
              {
                if(status == SQLITE_ROW)
                  {
                    inflight_integration gp;

                    const char* sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                    gp.content_group       = std::string(sqlite_str);

                    sqlite_str   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    gp.task_name = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    gp.output  = std::string(sqlite_str);

                    sqlite_str   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    gp.container = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                    gp.logdir  = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                    gp.tempdir = std::string(sqlite_str);

                    gp.workgroup_number = static_cast<unsigned int>(sqlite3_column_int(stmt, 6));

                    gp.is_seeded = static_cast<bool>(sqlite3_column_int(stmt, 7));

                    sqlite_str    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
                    gp.seed_group = std::string(sqlite_str);

                    gp.is_collecting_ics   = static_cast<bool>(sqlite3_column_int(stmt, 9));
                    gp.is_collecting_stats = static_cast<bool>(sqlite3_column_int(stmt, 10));

                    groups.push_back(gp);
                  }
              }

            check_stmt(db, sqlite3_finalize(stmt));
          }


        void enumerate_inflight_postintegrations(sqlite3* db, std::list<inflight_postintegration>& groups)
          {
            std::stringstream find_stmt;
            find_stmt << "SELECT content_group, task, output, container, logdir, tempdir, paired, parent, seeded, seed_group FROM " << CPPTRANSPORT_SQLITE_POSTINTEGRATION_WRITERS_TABLE << ";";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, find_stmt.str().c_str(), find_stmt.str().length()+1, &stmt, nullptr));

            int status;
            groups.clear();
            while((status = sqlite3_step(stmt)) != SQLITE_DONE)
              {
                if(status == SQLITE_ROW)
                  {
                    inflight_postintegration gp;

                    const char* sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                    gp.content_group       = std::string(sqlite_str);

                    sqlite_str   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    gp.task_name = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    gp.output  = std::string(sqlite_str);

                    sqlite_str   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    gp.container = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                    gp.logdir  = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
                    gp.tempdir = std::string(sqlite_str);

                    gp.is_paired = static_cast<bool>(sqlite3_column_int(stmt, 6));

                    sqlite_str      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
                    gp.parent_group = std::string(sqlite_str);

                    gp.is_seeded = static_cast<bool>(sqlite3_column_int(stmt, 8));

                    sqlite_str    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
                    gp.seed_group = std::string(sqlite_str);

                    groups.push_back(gp);
                  }
              }

            check_stmt(db, sqlite3_finalize(stmt));
          }


        void enumerate_inflight_derived_content(sqlite3* db, std::list<inflight_derived_content>& groups)
          {
            std::stringstream find_stmt;
            find_stmt << "SELECT content_group, task, output, logdir, tempdir FROM " << CPPTRANSPORT_SQLITE_DERIVED_WRITERS_TABLE << ";";

            sqlite3_stmt* stmt;
            check_stmt(db, sqlite3_prepare_v2(db, find_stmt.str().c_str(), find_stmt.str().length()+1, &stmt, nullptr));

            int status;
            groups.clear();
            while((status = sqlite3_step(stmt)) != SQLITE_DONE)
              {
                if(status == SQLITE_ROW)
                  {
                    inflight_derived_content gp;

                    const char* sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                    gp.content_group       = std::string(sqlite_str);

                    sqlite_str   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                    gp.task_name = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                    gp.output  = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
                    gp.logdir  = std::string(sqlite_str);

                    sqlite_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
                    gp.tempdir = std::string(sqlite_str);

                    groups.push_back(gp);
                  }
              }

            check_stmt(db, sqlite3_finalize(stmt));
          }


      }   // namespace sqlite3_operations

  }   // namespace transport


#endif //CPPTRANSPORT_REPOSITORY_INFLIGHT_H