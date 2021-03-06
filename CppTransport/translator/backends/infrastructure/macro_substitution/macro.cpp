//
// Created by David Seery on 27/06/2013.
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


#include <assert.h>
#include <cctype>

#include "macro.h"
#include "timing_instrument.h"

#include "boost/algorithm/string.hpp"


// **************************************************************************************


macro_agent::macro_agent(translator_data& p, package_group& pkg, std::string pf, std::string speq, std::string spsumeq, unsigned int dm)
  : data_payload(p),
    prefix(std::move(pf)),
    split_equal(std::move(speq)),
    split_sum_equal(std::move(spsumeq)),
    recursion_max(dm),
    recursion_depth(0),
    package(pkg),
    output_enabled(true)
  {
    assert(recursion_max > 0);

		// pause timers
		macro_apply_timer.stop();
		tokenization_timer.stop();

    if(recursion_max == 0) recursion_max = 1;
    
    fields = data_payload.model.get_number_fields();
    parameters = data_payload.model.get_number_params();
    order = data_payload.misc.get_indexorder();
  }


std::unique_ptr< std::list<std::string> > macro_agent::apply(const std::string& line, unsigned int& replacements)
  {
		// if timer is stopped, restart it
    timing_instrument timer(this->macro_apply_timer);

		// the result of macro substitution is potentially large, and we'd rather not copy
		// a very large array of strings while moving the result around.
		// So, use a std::unique_ptr<> to manage the result object
    std::unique_ptr< std::list<std::string> > r_list;

    // check whether it is still safe
    if(++this->recursion_depth < this->recursion_max)
      {
        r_list = this->apply_line(line, replacements);
        --this->recursion_depth;
      }
    else
      {
        std::ostringstream msg;
        msg << WARNING_RECURSION_DEPTH << "=" << this->recursion_max << ")";
    
        error_context err_context = this->data_payload.make_error_context();
        err_context.warn(msg.str());
      }

    return(r_list);
  }


std::unique_ptr<token_list>
macro_agent::tokenize(const std::string& line, boost::optional<index_literal_database&> validate_db, bool strict)
  {
    return std::make_unique<token_list>(line, this->prefix, this->fields, this->parameters, this->package,
                                        this->local_index_rules, this->data_payload, validate_db, strict);
  }


std::unique_ptr< std::list<std::string> > macro_agent::apply_line(const std::string& line, unsigned int& replacements)
  {
    std::unique_ptr< std::list<std::string> > r_list = std::make_unique<std::list<std::string> >();

    // break the line at the split point, if one exists, to get a 'left-hand' side and a 'right-hand' side
    macro_impl::split_string split_result(line, this->split_equal, this->split_sum_equal);

    // tokenize
    std::unique_ptr<token_list> left_tokens;
    std::unique_ptr<token_list> right_tokens;
    std::tie(left_tokens, right_tokens) = this->perform_tokenization(split_result);

    // if output is disabled then there is nothing to do, so we should return as quickly as possible after tokenization
    // (tokenization always happens so we can process directives such as eg. $ELSE, $ENDIF)
    if(!this->output_enabled) return r_list;
    
    // generate error context, contexted at split-point if one is available
    error_context ctx = this->make_split_point_context(line, split_result);

    // check whether any directives were recognized in this line;
    // if so, the line should be simple -- it shouldn't have a left-hand side and right-hand side,
    // so flag an error if it does
    if(split_result.get_split_type() != macro_impl::split_type::none
       && (right_tokens->is_directive() || left_tokens->is_directive()))
      {
        ctx.error(ERROR_DIRECTIVE_ON_RHS);
        return r_list;
      }

    // if the line contains a directive then handle quickly and return
    if(right_tokens->is_directive())
      {
        // the line is a directive, so won't contain any replacement rules
        language_printer& prn = this->package.get_language_printer();
        r_list->push_back(prn.comment(right_tokens->to_string()));
        return r_list;
      }
    
    // at this stage we know the line wasn't a directive, and instead contains possible
    // replacement rules or index sets that should be expanded
    
    // running total of number of macro replacements
    unsigned int counter = 0;

    // evaluate pre macros and cache the results
    // we'd like to do this only once if possible, because evaluation may be expensive
    counter += left_tokens->evaluate_macros(simple_macro_type::pre);
    counter += right_tokens->evaluate_macros(simple_macro_type::pre);
    
    // generate assignments for the LHS indices
    assignment_set LHS_assignments(left_tokens->get_index_database());

    // generate an assignment for each RHS index; first get RHS indices that are not also LHS indices
    abstract_index_database RHS_indices;

    try
      {
        RHS_indices = right_tokens->get_index_database() - left_tokens->get_index_database();
      }
    catch(index_exception& xe)
      {
        std::ostringstream msg;
        msg << ERROR_MACRO_LHS_RHS_MISMATCH << " '" << xe.what() << "'";
        ctx.error(msg.str());
      }

    assignment_set RHS_assignments(RHS_indices);
    
    bool unroll = this->apply_unroll_policy(*left_tokens, *right_tokens, LHS_assignments, RHS_assignments, ctx);
    
    if(unroll)
      {
        this->unroll_index_assignment(*left_tokens, *right_tokens, LHS_assignments, RHS_assignments, counter,
                                      split_result, ctx, *r_list);
      }
    else
      {
        this->forloop_index_assignment(*left_tokens, *right_tokens, LHS_assignments, RHS_assignments, counter,
                                       split_result, ctx, *r_list);
      }

    replacements = counter;

    return(r_list);
  }


bool macro_agent::apply_unroll_policy(const token_list& left_tokens, const token_list& right_tokens,
                                      const assignment_set& LHS_assignments, const assignment_set& RHS_assignments,
                                      const error_context& ctx) const
  {
    // determine total size of assignment set and apply policy for unrolling
    unsigned int total_assignment_size = LHS_assignments.size() * RHS_assignments.size();

    // if the total assignment size is smaller than the unroll policy size, then we would typically unroll
    // unless it is prevented
    bool unroll_by_policy = total_assignment_size <= data_payload.unroll_policy();
    
    // determine whether the LHS and RHS force or prevent unrolling
    unsigned int prevent = 0;
    unsigned int force = 0;

    if(left_tokens.unroll_status() == unroll_state::prevent) ++prevent;
    if(right_tokens.unroll_status() == unroll_state::prevent) ++prevent;
    if(left_tokens.unroll_status() == unroll_state::force) ++force;
    if(right_tokens.unroll_status() == unroll_state::force) ++force;
    
    // if the assignment set is nonempty and we have both force and prevent conditions, then we cannot
    // resolve the situation
    if(total_assignment_size > 1 && force > 0 && prevent > 0)
      {
        ctx.error(ERROR_LHS_RHS_INCOMPATIBLE_UNROLL);
        this->notify_unroll_outcome(left_tokens, right_tokens);
    
        return false;
      }

    // if the assignment set is nonempty, '--fast' is in effect and a rule is trying to prevent unrolling
    // then we have an error
    if(total_assignment_size > 1 && data_payload.fast() && prevent > 0)
      {
        ctx.error(ERROR_PREVENT_INCOMPATIBLE_FAST);
        this->notify_unroll_outcome(left_tokens, right_tokens);
        
        return false;
      }

    // emit warnings if there is a policy violation that was not mandated by an explicit specifier
    bool emit = false;
    if(unroll_by_policy && prevent > 0 && !left_tokens.has_explicit_prevent() && !right_tokens.has_explicit_prevent())
      {
        // issue notification that unrolling has been prevented, if we have been asked to do so
        ctx.warn(WARN_POLICY_WOULD_UNROLL);
        emit = true;
      }
    if(!unroll_by_policy && force > 0 && !left_tokens.has_explicit_force() && !right_tokens.has_explicit_force())
      {
        ctx.warn(WARN_POLICY_WOULD_ROLLUP);
        emit = true;
      }
    if(emit) this->notify_unroll_outcome(left_tokens, right_tokens);
    
    if(force > 0 || ((unroll_by_policy || data_payload.fast()) && prevent == 0)) return true;
    
    return false;
  }


void macro_agent::notify_unroll_outcome(const token_list& left_tokens, const token_list& right_tokens) const
  {
    if(!this->data_payload.get_argument_cache().report_unroll_warnings()) return;
    
    const bool RHS_only = left_tokens.empty();
    
    const auto LHS_force = left_tokens.get_force_rules();
    for(const auto& rule : LHS_force)
      {
        const error_context& pctx = rule.get_declaration_point();
        pctx.warn(NOTIFY_LHS_RULE_FORCES_UNROLL);
      }
    
    const auto RHS_force = right_tokens.get_force_rules();
    for(const auto& rule : RHS_force)
      {
        const error_context& pctx = rule.get_declaration_point();
        pctx.warn(RHS_only ? NOTIFY_RULE_FORCES_UNROLL : NOTIFY_RHS_RULE_FORCES_UNROLL);
      }
    
    const auto LHS_prevent = left_tokens.get_prevent_rules();
    for(const auto& rule : LHS_prevent)
      {
        const error_context& pctx = rule.get_declaration_point();
        pctx.warn(NOTIFY_LHS_RULE_PREVENTS_UNROLL);
      }
    
    const auto RHS_prevent = right_tokens.get_prevent_rules();
    for(const auto& rule : RHS_prevent)
      {
        const error_context& pctx = rule.get_declaration_point();
        pctx.warn(RHS_only ? NOTIFY_RULE_PREVENTS_UNROLL : NOTIFY_RHS_RULE_PREVENTS_UNROLL);
      }
  }


void macro_agent::unroll_index_assignment(token_list& left_tokens, token_list& right_tokens,
                                          assignment_set& LHS_assignments, assignment_set& RHS_assignments,
                                          unsigned int& counter, macro_impl::split_string& split_result,
                                          error_context& ctx, std::list<std::string>& r_list)
  {
    // LHS_assignments may be *empty* (ie size=0) if there are no valid assignments
    // (probably one of the index ranges is empty).
    // If there are no LHS indices then it will report size=1, ie. the trivial index assignment
    // It's important to distinguish these two cases!

		if(LHS_assignments.size() == 0)
      {
        language_printer& prn = this->package.get_language_printer();
        r_list.push_back(prn.comment("Skipped: empty index range (LHS index set is empty)"));
        return;
      }

    // compute raw indent for LHS
    std::string raw_indent = this->compute_prefix(split_result);

    for(std::unique_ptr<indices_assignment> LHS_assign : LHS_assignments)
      {
        // evaluate LHS macros on this index assignment;
        // only need index- and post-macros; pre-macros were evaluated earlier
        counter += left_tokens.evaluate_macros(*LHS_assign);
        counter += left_tokens.evaluate_macros(simple_macro_type::post);

        if(RHS_assignments.size() > 1)   // multiple RHS assignments
          {
            // push the LHS evaluated on this assignment into the output list, if it is non-empty
            if(left_tokens.size() > 0)
              {
                std::string lhs = left_tokens.to_string();
                if(split_result.get_split_type() == macro_impl::split_type::sum) lhs += " = ";
                if(split_result.get_split_type() == macro_impl::split_type::sum_equal) lhs += " += ";
                r_list.push_back(lhs);
              }

            // now generate a set of RHS evaluations for this LHS evaluation
            for(std::unique_ptr<indices_assignment> RHS_assign : RHS_assignments)
              {
                indices_assignment total_assignment;
                try
                  {
                    total_assignment = *LHS_assign + *RHS_assign;
                  }
                catch(index_exception& xe)
                  {
                    std::ostringstream msg;
                    msg << ERROR_MACRO_LHS_RHS_MISMATCH << " '" << xe.what() << "'";
                    ctx.error(msg.str());
                  }

                counter += right_tokens.evaluate_macros(total_assignment);
                counter += right_tokens.evaluate_macros(simple_macro_type::post);

                // set up replacement right hand side; add trailing ; and , only if the LHS is empty
                std::string this_line = right_tokens.to_string() +
                                        (left_tokens.size() == 0 && split_result.has_trailing_semicolon() ? ";" : "") +
                                        (left_tokens.size() == 0 && split_result.has_trailing_comma() ? "," : "");

                if(left_tokens.size() ==
                   0)   // no need to format for indentation if no LHS; RHS will already include indentation
                  {
                    r_list.push_back(this_line);
                  }
                else
                  {
                    r_list.push_back(this->dress(this_line, raw_indent, 3));
                  }
              }

            // add a trailing ; and , if the LHS is nonempty
            if(left_tokens.size() > 0 && r_list.size() > 0)
              {
                if(split_result.has_trailing_semicolon())
                  {
                    r_list.back() += ";";
                  }
                if(split_result.has_trailing_comma())
                  {
                    r_list.back() += ",";
                  }
              }
          }
        else if(RHS_assignments.size() == 1)  // just one RHS assignment, so coalesce with LHS
          {
            std::unique_ptr<indices_assignment> RHS_assign = *RHS_assignments.begin();

            indices_assignment total_assignment;
            try
              {
                total_assignment = *LHS_assign + *RHS_assign;
              }
            catch(index_exception& xe)
              {
                std::ostringstream msg;
                msg << ERROR_MACRO_LHS_RHS_MISMATCH << " '" << xe.what() << "'";
                ctx.error(msg.str());
              }

            counter += right_tokens.evaluate_macros(total_assignment);
            counter += right_tokens.evaluate_macros(simple_macro_type::post);

            // set up line with macro replacements, and add trailing ; and , if necessary;
            // since the line includes the full LHS it needs no special formatting to account for indentation
            std::string full_line = left_tokens.to_string();
            if(split_result.get_split_type() == macro_impl::split_type::sum) full_line += " =";
            if(split_result.get_split_type() == macro_impl::split_type::sum_equal) full_line += " +=";

            full_line += (left_tokens.size() > 0 ? " " : "") + right_tokens.to_string() +
                         (split_result.has_trailing_semicolon() ? ";" : "") +
                         (split_result.has_trailing_comma() ? "," : "");
            r_list.push_back(full_line);
          }
      }
	}


void macro_agent::forloop_index_assignment(token_list& left_tokens, token_list& right_tokens,
                                           assignment_set& LHS_assignments, assignment_set& RHS_assignments,
                                           unsigned int& counter, macro_impl::split_string& split_result,
                                           error_context& ctx, std::list<std::string>& r_list)
  {
    std::string raw_indent = this->compute_prefix(split_result);
    unsigned int current_indent = 0;

    if(LHS_assignments.size() == 0)
      {
        language_printer& prn = this->package.get_language_printer();
        r_list.push_back(prn.comment("Skipped: empty index range (LHS index set is empty)"));
      return;
      }

    this->plant_LHS_forloop(LHS_assignments.idx_set_begin(), LHS_assignments.idx_set_end(),
                            RHS_assignments, left_tokens, right_tokens, counter, split_result,
                            ctx, r_list, raw_indent, current_indent);
  }


std::string macro_agent::compute_prefix(macro_impl::split_string& split_string)
  {
    std::string prefix;
    size_t pos = 0;
    
    auto& left = split_string.get_left();
    auto& right = split_string.get_right();

    std::string raw = (left.length() > 0) ? left : right;

    while(pos < raw.length() && std::isspace(raw[pos]))
      {
        prefix += raw[pos];
        ++pos;
      }

    return(prefix);
  }


void macro_agent::plant_LHS_forloop(index_database<abstract_index>::const_iterator current,
                                    index_database<abstract_index>::const_iterator end,
                                    assignment_set& RHS_assignments, token_list& left_tokens, token_list& right_tokens,
                                    unsigned int& counter, macro_impl::split_string& split_result, error_context& ctx,
                                    std::list<std::string>& r_list, const std::string& raw_indent,
                                    unsigned int current_indent)
  {
    if(current == end)
      {
        // evaluate macros, converting indices to abstract 'for' loop variables
        counter += left_tokens.evaluate_macros();
        counter += left_tokens.evaluate_macros(simple_macro_type::post);

        // if this is an assignment statement then plant code to zero the accumulator variable,
        // unless there are no RHS assignments.
        // In that case we can simply coalesce the assignment with the RHS.
        if(left_tokens.size() > 0 && RHS_assignments.size() > 1 && split_result.get_split_type() == macro_impl::split_type::sum)
          {
            std::string zero_stmt = left_tokens.to_string() + " = 0;";
            boost::algorithm::trim_left(zero_stmt);
            r_list.push_back(this->dress(zero_stmt, raw_indent, current_indent));
          }

        this->plant_RHS_forloop(RHS_assignments.idx_set_begin(), RHS_assignments.idx_set_end(), left_tokens,
                                right_tokens, counter, split_result, ctx, r_list, raw_indent, current_indent, RHS_assignments.size() == 1);
      }
    else
      {
        language_printer& printer = this->package.get_language_printer();
        boost::optional< std::string > start_delimiter = printer.get_start_block_delimiter();
        boost::optional< std::string > end_delimiter = printer.get_end_block_delimiter();

        r_list.push_back(this->dress(printer.for_loop(current->get_loop_variable(), 0, current->numeric_range()), raw_indent, current_indent));

        std::ostringstream brace_indent;
        for(unsigned int i = 0; i < printer.get_block_delimiter_indent(); ++i) brace_indent << " ";

        if(start_delimiter) r_list.push_back(this->dress(brace_indent.str() + *start_delimiter, raw_indent, current_indent));
        this->plant_LHS_forloop(++current, end, RHS_assignments, left_tokens, right_tokens, counter, split_result, ctx,
                                r_list, raw_indent, current_indent + printer.get_block_indent());
        if(end_delimiter) r_list.push_back(this->dress(brace_indent.str() + *end_delimiter, raw_indent, current_indent));
      }
  }


void macro_agent::plant_RHS_forloop(index_database<abstract_index>::const_iterator current,
                                    index_database<abstract_index>::const_iterator end,
                                    token_list& left_tokens, token_list& right_tokens, unsigned int& counter,
                                    macro_impl::split_string& split_result, error_context& ctx,
                                    std::list<std::string>& r_list, const std::string& raw_indent,
                                    unsigned int current_indent, bool coalesce)
  {
    if(current == end)
      {
        // evaluate macros, converting indices to abstract 'for' loop variables
        counter += right_tokens.evaluate_macros();
        counter += right_tokens.evaluate_macros(simple_macro_type::post);

        std::string left_line = left_tokens.to_string();
        std::string right_line = right_tokens.to_string() + (split_result.has_trailing_semicolon() ? ";" : "") + (split_result.has_trailing_comma() ? "," : "");

        boost::algorithm::trim_left(left_line);
        std::string total_line = left_line;

        if(split_result.get_split_type() == macro_impl::split_type::sum_equal)
          {
            total_line += " += ";
          }
        else if(split_result.get_split_type() == macro_impl::split_type::sum)
          {
            if(coalesce) total_line += " = ";
            else         total_line += " += ";
          }
        // else split_result.type == macro_impl::splt_type::none and there is no need for an assignment operator

        total_line += right_line;
        if(left_tokens.size() == 0) boost::algorithm::trim_left(total_line);

        r_list.push_back(this->dress(total_line, raw_indent, current_indent));
      }
    else
      {
        language_printer& printer = this->package.get_language_printer();
        boost::optional< std::string > start_delimiter = printer.get_start_block_delimiter();
        boost::optional< std::string > end_delimiter = printer.get_end_block_delimiter();

        r_list.push_back(this->dress(printer.for_loop(current->get_loop_variable(), 0, current->numeric_range()), raw_indent, current_indent));

        std::ostringstream brace_indent;
        for(unsigned int i = 0; i < printer.get_block_delimiter_indent(); ++i) brace_indent << " ";

        if(start_delimiter) r_list.push_back(this->dress(brace_indent.str() + *start_delimiter, raw_indent, current_indent));
        this->plant_RHS_forloop(++current, end, left_tokens, right_tokens, counter, split_result, ctx, r_list,
                                raw_indent, current_indent + printer.get_block_indent(), coalesce);
        if(end_delimiter) r_list.push_back(this->dress(brace_indent.str() + *end_delimiter, raw_indent, current_indent));
      }
  }


std::string macro_agent::dress(std::string out_str, const std::string& raw_indent, unsigned int current_indent)
  {
    std::ostringstream out;

    out << raw_indent;
    for(unsigned int i = 0; i < current_indent; ++i) out << " ";

    out << out_str;
    return(out.str());
  }


void macro_agent::inject_macro(std::reference_wrapper< macro_packages::replacement_rule_index > rule)
  {
    this->local_index_rules.push_back(std::move(rule));
  }


std::pair< std::unique_ptr<token_list>, std::unique_ptr<token_list> >
macro_agent::perform_tokenization(const macro_impl::split_string& split_result)
  {
    // keep track of the time required for tokenization
    timing_instrument tok_timer(this->tokenization_timer);

    // tokenize LHS if it is present
    auto left_tokens = this->tokenize(split_result.get_left());

    // collect indices defined on LHS and convert to a validation database
    const auto& left_index_decls = left_tokens->get_index_declarations();
    std::unique_ptr<index_literal_database> validation_db;
    try
      {
        validation_db = to_database(left_index_decls);
      }
    catch(duplicate_index& xe)
      {
        const error_context& ctx = xe.get_error_point();

        std::ostringstream msg;
        msg << ERROR_LHS_INDEX_DUPLICATE << " '" << xe.what() << "'";
        ctx.error(msg.str());
      }

    // tokenize RHS, enforcing that indices shared with LHS have the same properties
    boost::optional<index_literal_database&> vdb;
    if(validation_db) vdb = *validation_db;
    auto right_tokens = this->tokenize(split_result.get_right(), vdb);

    // validate RHS index instances
    if(split_result.get_split_type() != macro_impl::split_type::none)
      this->validate_RHS_indices(*left_tokens, *right_tokens);

    // stop tokenization timer
    tok_timer.stop();

    return std::make_pair(std::move(left_tokens), std::move(right_tokens));
  }


error_context macro_agent::make_split_point_context(const std::string& line, const macro_impl::split_string& split_result)
  {
    error_context ctx = this->data_payload.make_error_context();

    switch(split_result.get_split_type())
      {
        case macro_impl::split_type::none:
          {
            ctx = this->data_payload.make_error_context();
            break;
          }

        case macro_impl::split_type::sum:
          {
            unsigned int p = split_result.get_split_point();
            ctx = this->data_payload.make_error_context(std::make_shared<std::string>(line),
                                                        p, p+this->split_equal.size());
            break;
          }

        case macro_impl::split_type::sum_equal:
          {
            unsigned int p = split_result.get_split_point();
            ctx = this->data_payload.make_error_context(std::make_shared<std::string>(line),
                                                        p, p+this->split_sum_equal.size());
            break;
          }
      }

    return ctx;
  }


void macro_agent::validate_RHS_indices(token_list& left_tokens, token_list& right_tokens)
  {
    // get RHS index declarations, and erase any indices occurring on the LHS
    index_literal_list decls = right_tokens.get_index_declarations();
    const abstract_index_database& LHS_db = left_tokens.get_index_database();
    
    auto T = decls.begin();
    while(T != decls.end())
      {
        // does this index occur on the LHS?
        auto t = LHS_db.find(T->get()->get().get_label());
        if(t != LHS_db.end())
          {
            T = decls.erase(T);
          }
        else
          {
            ++T;
          }
      }
    
    // check how many times each RHS index occurs
    this->validate_RHS_count(decls);

    // for nontrivial metric models, perform rudimentary checks on index position
    if(this->data_payload.model.get_lagrangian_type() == model_type::nontrivial_metric)
      this->validate_RHS_variances(decls);
  }


void macro_agent::validate_RHS_count(const index_literal_list& decls)
  {
    // count number of times each RHS index occurs
    std::map< char, unsigned int > count;
    std::map< char, std::reference_wrapper<const error_context> > ctxs;
    
    for(const auto& T : decls)
      {
        const index_literal& l = *T;
        char label = l.get().get_label();
        
        if(ctxs.count(label) == 0) ctxs.emplace(std::make_pair(label, std::cref(l.get_declaration_point())));
        count[label] = count[label] + 1;
      }
    
    // search for indices that occurred only once
    for(const auto& T : count)
      {
        if(T.second == 1)
          {
            auto t = ctxs.find(T.first);
            if(t != ctxs.end())
              {
                const error_context& ctx = t->second.get();
                ctx.warn(NOTIFY_RHS_INDEX_SINGLE_OCCURRENCE);
              }
          }
      }
  }


void macro_agent::validate_RHS_variances(const index_literal_list& decls)
  {
    // search for any indices which don't occur in both co- and contra-variant forms
    std::map< char, std::pair<bool, bool> > flags;
    std::map< char, std::reference_wrapper<const error_context> > ctxs;
    
    for(const auto& T : decls)
      {
        const index_literal& l = *T;
        char label = l.get().get_label();
        variance v = T.get()->get_variance();

        if(ctxs.count(label) == 0) ctxs.emplace(std::make_pair(label, std::cref(l.get_declaration_point())));
    
        if(v == variance::covariant) flags[label].first = true;
        if(v == variance::contravariant) flags[label].second = true;
      }
    
    // search for indices that didn't occur both ways
    for(const auto& T : flags)
      {
        const auto& F = T.second;
        if(!F.first || !F.second)
          {
            auto t = ctxs.find(T.first);
            if(t != ctxs.end())
              {
                const error_context& ctx = t->second.get();
                ctx.warn(NOTIFY_RHS_INDEX_SINGLE_VARIANCE);
              }
          }
      }
  }


macro_impl::split_string::split_string(const std::string& line, const std::string& split_equal,
                                       const std::string& split_sum_equal)
  : split_point(0),
    comma(false),
    semicolon(false),
    type(split_type::none)
  {
    // check for sum split point
    if((split_point = line.find(split_equal)) != std::string::npos)
      {
        left  = line.substr(0, split_point);
        right = line.substr(split_point + split_equal.length());
        type  = split_type::sum;
      }
    else    // not sum, but possibly sum-equals?
      {
        if((split_point = line.find(split_sum_equal)) != std::string::npos)
          {
            left  = line.substr(0, split_point);
            right = line.substr(split_point + split_sum_equal.length());
            type  = split_type::sum_equal;
          }
        else    // no split-point; everything counts as the right-hand side
          {
            right = line;
            type  = split_type::none;
          }
      }
    
    // trim trailing white space on the right-hand side,
    // and leading white space if there is a nontrivial left-hand side
    boost::algorithm::trim_right(right);
    if(left.length() > 0) boost::algorithm::trim_left(right);
    
    // check if the last component is a semicolon
    // note std:string::back() and std::string::pop_back() require C++11
    if(right.size() > 0 && right.back() == ';')
      {
        semicolon = true;
        right.pop_back();
      }
    
    // check if the last component is a comma
    if(right.size() > 0 && right.back() == ',')
      {
        comma = true;
        right.pop_back();
      }
  }
