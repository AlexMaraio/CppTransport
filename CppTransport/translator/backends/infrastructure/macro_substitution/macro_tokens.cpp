//
// Created by David Seery on 06/06/2017.
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


#include "macro_tokens.h"

#include "msg_en.h"


namespace token_list_impl
  {


    constexpr unsigned int MAX_TOKEN_ERRORS = 4;


    generic_token::generic_token(const std::string& c, error_context ec)
      : conversion(c),
        err_ctx(std::move(ec)),
        num_errors(0),
        silent(false)
      {
      }


    void generic_token::error(const std::string& msg)
      {
        if(!this->silent)
          {
            this->err_ctx.error(msg);
          }

        ++this->num_errors;

        if(!this->silent && this->num_errors >= MAX_TOKEN_ERRORS)
          {
            this->err_ctx.warn(ERROR_TOKENIZE_TOO_MANY_ERRORS);
            this->silent = true;
          }
      }


    text_token::text_token(const std::string& l, error_context ec)
      : generic_token(l, std::move(ec))
      {
      }


    index_literal_token::index_literal_token(abstract_index_database::const_iterator& it, error_context ec)
      : generic_token(std::string(1, it->get_label()), std::move(ec)),
        index(*it)
      {
      }


    void index_literal_token::evaluate(const assignment_list& a)
      {
        auto t = a.find(this->index.get_label());

        if(t == a.end())
          {
            std::ostringstream msg;
            msg << ERROR_MISSING_INDEX_ASSIGNMENT << " '" << this->index.get_label() << "'";

            throw macro_packages::rule_apply_fail(msg.str());
          }

        std::ostringstream cnv;
        cnv << t->get_numeric_value();
        this->conversion = cnv.str();
      }


    void index_literal_token::evaluate()
      {
        this->conversion = this->index.get_loop_variable();
      }


    void index_literal_token::evaluate(const index_remap_rule& rule)
      {
        // find substitution for this index
        index_remap_rule::const_iterator t = rule.find(this->index);

        if(t == rule.end())
          {
            std::ostringstream msg;
            msg << ERROR_INDEX_SUBSTITUTION << " '" << this->index.get_label() << "'";
            throw macro_packages::rule_apply_fail(msg.str());
          }

        this->conversion = t->second.get_loop_variable();
      }


    simple_macro_token::simple_macro_token(const std::string& m, const macro_argument_list& a,
                                           macro_packages::replacement_rule_simple& r, simple_macro_type t,
                                           error_context ec)
      : generic_token(m, std::move(ec)),
        name(m),
        args(a),
        rule(r),
        type(t),
        argument_error(false)
      {
        try
          {
            this->rule.post_tokenize_hook(a);
          }
        catch(macro_packages::argument_mismatch& xe)
          {
            this->error(xe.what());
            this->argument_error = true;
          }
        catch(macro_packages::rule_apply_fail& xe)
          {
            this->error(xe.what());
          }
      }


    void simple_macro_token::evaluate()
      {
        // evaluate the macro, and cache the result
        try
          {
            this->conversion = this->rule(this->args);
          }
        catch(macro_packages::argument_mismatch& xe)
          {
            if(!this->argument_error)
              {
                this->error(xe.what());
                this->argument_error = true;
              }
          }
        catch(macro_packages::rule_apply_fail& xe)
          {
            this->error(xe.what());
          }
      }


    index_macro_token::index_macro_token(const std::string& m, const abstract_index_database i,
                                         const macro_argument_list& a, macro_packages::replacement_rule_index& r,
                                         error_context ec)
      : generic_token(m, std::move(ec)),
        name(m),
        args(a),
        indices(std::move(i)),
        rule(r),
        initialized(false),
        argument_error(false),
        index_error(false)
      {
      }


    index_macro_token::~index_macro_token()
      {
        try
          {
            if(this->initialized) this->rule.post(this->args);
          }
        catch(macro_packages::rule_apply_fail& xe)
          {
            this->error(xe.what());
          }
      }


    void index_macro_token::evaluate_unroll(const assignment_list& a)
      {
        // call 'pre'-handler if it has not already been invoked
        if(!initialized)
          {
            try
              {
                this->rule.pre(args);
              }
            catch(macro_packages::argument_mismatch& xe)
              {
                if(!this->argument_error)
                  {
                    this->error(xe.what());
                    this->argument_error = true;
                  }
              }
            catch(macro_packages::index_mismatch& xe)
              {
                if(!this->index_error)
                  {
                    this->error(xe.what());
                    this->index_error = true;
                  }
              }
            catch(macro_packages::rule_apply_fail& xe)
              {
                this->error(xe.what());
              }

            initialized = true;
          }

        // strip out the index assignment -- just for the indices this macro requires;
        // preserves ordering
        assignment_list index_values;

        for(const abstract_index& idx : this->indices)
          {
            assignment_list::const_iterator it = a.find(idx.get_label());
            if(it == a.end())
              {
                std::ostringstream msg;
                msg << ERROR_MISSING_INDEX_ASSIGNMENT << " '" << idx.get_label() << "'";

                throw macro_packages::rule_apply_fail(msg.str());
              }

            index_values.emplace_back(std::make_pair(idx.get_label(), std::make_shared<assignment_record>(*it)));
          }

        try
          {
            this->conversion = this->rule.evaluate_unroll(this->args, index_values);
          }
        catch(macro_packages::argument_mismatch& xe)
          {
            if(!this->argument_error)
              {
                this->error(xe.what());
                this->argument_error = true;
              }
          }
        catch(macro_packages::index_mismatch& xe)
          {
            if(!this->index_error)
              {
                this->error(xe.what());
                this->index_error = true;
              }
          }
        catch(macro_packages::rule_apply_fail& xe)
          {
            this->error(xe.what());
          }
      }


    void index_macro_token::evaluate_roll()
      {
        // as a performance optimization, 'pre' handler is not called for roll-up evaluation;
        // it just results in lots of CSE being performed which is unnecessary for roll-up
        // cases

        try
          {
            this->conversion = this->rule.evaluate_roll(this->args, this->indices);
          }
        catch(macro_packages::rule_apply_fail& xe)
          {
            this->error(xe.what());
          }
      }


    void index_macro_token::evaluate_roll(const index_remap_rule& rule)
      {
        // build index set using substitution rules
        abstract_index_database list;

        for(const abstract_index& idx : this->indices)
          {
            index_remap_rule::const_iterator t = rule.find(idx);

            if(t == rule.end())
              {
                std::ostringstream msg;
                msg << ERROR_INDEX_SUBSTITUTION << " '" << idx.get_label() << "'";
                throw macro_packages::rule_apply_fail(msg.str());
              }

            const abstract_index& subs = t->second;

            list.emplace_back(std::make_pair(subs.get_label(), std::make_shared<abstract_index>(subs)));
          }

        try
          {
            this->conversion = this->rule.evaluate_roll(this->args, list);
          }
        catch(macro_packages::rule_apply_fail& xe)
          {
            this->error(xe.what());
          }
      }


    void index_macro_token::reset()
      {
        try
          {
            if(this->initialized) this->rule.post(this->args);
            this->initialized = false;
          }
        catch(macro_packages::rule_apply_fail& xe)
          {
            this->error(xe.what());
          }
      }


  }   // namespace token_list_impl
