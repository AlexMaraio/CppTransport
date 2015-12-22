//
// Created by David Seery on 15/12/2015.
// Copyright (c) 2015 University of Sussex. All rights reserved.
//


#include "replacement_rule_definitions.h"


namespace macro_packages
  {

    std::string replacement_rule_simple::operator()(const macro_argument_list& args)
      {
        // check that correct number of arguments have been supplied
        if(args.size() != this->num_args)
          {
            std::ostringstream msg;

            msg << ERROR_WRONG_ARGUMENT_COUNT << " '" << this->name << "'; " << ERROR_EXPECTED_ARGUMENT_COUNT << " " << this->num_args << ", " << ERROR_RECEIVED_ARGUMENT_COUNT << " " << args.size();
            throw macro_packages::rule_apply_fail(msg.str());
          }

        return this->evaluate(args);
      }


    std::string replacement_rule_index::operator()(const macro_argument_list& args, const assignment_list& indices)
      {
        // check that correct number of arguments have been supplied
        if(args.size() != this->num_args)
          {
            std::ostringstream msg;

            msg << ERROR_WRONG_ARGUMENT_COUNT << " '" << this->name << "'; " << ERROR_EXPECTED_ARGUMENT_COUNT << " " << this->num_args << ", " << ERROR_RECEIVED_ARGUMENT_COUNT << " " << args.size();
            throw rule_apply_fail(msg.str());
          }

        // check that correct number of indices have been supplied
        if(indices.size() != this->num_indices)
          {
            std::ostringstream msg;

            msg << ERROR_WRONG_INDEX_COUNT << " '" << this->name << "'; " << ERROR_EXPECTED_INDEX_COUNT << " " << this->num_indices << ", " << ERROR_RECEIVED_INDEX_COUNT << " " << indices.size();
            throw macro_packages::rule_apply_fail(msg.str());
          }

        // check that index types are compatible
        for(const assignment_record& rec : indices)
          {
            switch(this->idx_class)
              {
                case index_class::full:
                  {
                    // full is compatible with either full or field_only
                    if(rec.get_class() != index_class::full && rec.get_class() != index_class::field_only)
                      {
                        std::ostringstream msg;

                        msg << ERROR_WRONG_INDEX_CLASS << " '" << this->name << "' " << ERROR_WRONG_INDEX_LABEL << " '" << rec.get_label() << "'";
                        throw macro_packages::rule_apply_fail(msg.str());
                      }

                    break;
                  }

                case index_class::field_only:
                  {
                    // field_only is compatible only with field_only

                    if(rec.get_class() != index_class::field_only)
                      {
                        std::ostringstream msg;

                        msg << ERROR_WRONG_INDEX_CLASS << " '" << this->name << "' " << ERROR_WRONG_INDEX_LABEL << " '" << rec.get_label() << "'";
                        throw macro_packages::rule_apply_fail(msg.str());
                      }

                    break;
                  }

                case index_class::parameter:
                  {
                    // parameter is compatible only with parameter

                    if(rec.get_class() != index_class::parameter)
                      {
                        std::ostringstream msg;

                        msg << ERROR_WRONG_INDEX_CLASS << " '" << this->name << "' " << ERROR_WRONG_INDEX_LABEL << " '" << rec.get_label() << "'";
                        throw macro_packages::rule_apply_fail(msg.str());
                      }

                    break;
                  }
              }
          }

        return this->evaluate(args, indices);
      }


    void replacement_rule_index::pre(const macro_argument_list& args)
      {
        // check that correct number of arguments have been supplied
        if(args.size() != this->num_args)
          {
            std::ostringstream msg;

            msg << ERROR_WRONG_ARGUMENT_COUNT << " '" << this->name << "'; " << ERROR_EXPECTED_ARGUMENT_COUNT << " " << this->num_args << ", " << ERROR_RECEIVED_ARGUMENT_COUNT << " " << args.size();
            throw rule_apply_fail(msg.str());
          }

        this->pre_evaluate(args);
      }


    void replacement_rule_index::post(const macro_argument_list& args)
      {
        this->post_evaluate(args);
      }

  }   // namespace macro_packages
