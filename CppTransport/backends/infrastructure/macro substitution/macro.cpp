//
// Created by David Seery on 27/06/2013.
// Copyright (c) 2013 University of Sussex. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include <assert.h>
#include <ctype.h>

#include <boost/algorithm/string.hpp>

#include "macro.h"

#include "package_group.h"


// **************************************************************************************


macro_package::macro_package(translation_unit* u, package_group* pkg, std::string pf, std::string sp, unsigned int dm)
  : unit(u), package(pkg), prefix(pf), split(sp),
    recursion_max(dm), recursion_depth(0),
    pre_rule_cache(pkg->get_pre_ruleset()), post_rule_cache(pkg->get_post_ruleset()), index_rule_cache(pkg->get_index_ruleset())
  {
    assert(unit != nullptr);
    assert(package != nullptr);
    assert(recursion_max > 0);

    if(recursion_max == 0) recursion_max = 1;

    fields     = unit->get_number_fields();
    parameters = unit->get_number_parameters();
    order      = unit->get_index_order();
  }


unsigned int macro_package::apply(std::string& line)
  {
    unsigned int rval = 0;

    if(++this->recursion_depth < this->recursion_max)
      {
        rval = this->apply_line(line);
        --this->recursion_depth;
      }
    else
      {
        std::ostringstream msg;
        msg << WARNING_RECURSION_DEPTH << "=" << this->recursion_max << ")";
        this->package->warn(msg.str());
      }

    return(rval);
  }


unsigned int macro_package::apply_line(std::string& line)
  {
    unsigned int rval = 0;
    std::string line_prefix = "";
    std::string new_line = "";

    // apply all pre- macros
    rval += this->apply_simple(line, this->pre_rule_cache);

    // first, find a split point if there is one
    size_t split_point;
    if((split_point = line.find(this->split)) != std::string::npos)
      {
        line_prefix = line.substr(0, split_point);
        line.replace(0, split_point + this->split.size(), "");
      }

    // trim trailing white space on 'line'
    boost::algorithm::trim_right(line);

    // check if the last component is a semicolon
    // note std:string::back() and std::string::pop_back() require C++11
    bool semicolon = false;
    if(line.back() == ';')
      {
        semicolon = true;
        line.pop_back();
      }

    // check if the last component is a comma
    bool comma = false;
    if(line.back() == ',')
      {
        comma = true;
        line.pop_back();
      }

    // extract a set of indices on the LHS, represented by here by the prefix 'line_prefix'
    // these 'lvalue' indices are excluded from the summation convention *within* each line
    // (of course, we eventually want to write out the whole system of equations;
    // we will have one equation for each assignment of indices on the LHS)
    std::vector<struct index_abstract> lhs_indices = this->get_lhs_indices(line_prefix);

    // work out the type of each lvalue index
    this->assign_lhs_index_types(line, lhs_indices, this->index_rule_cache);

    // generate an assignment for each lvalue index
    assignment_package lvalue_assigner(this->fields, this->parameters, this->order == indexorder_left ? index_left_order : index_right_order);
    std::vector< std::vector<struct index_assignment> > lvalue_assignments = lvalue_assigner.assign(lhs_indices);

    // for each index assignment, carry out replacement in 'line_prefix' and then
    // apply summation convention to the rest of the line
    for(int i = 0; i < lvalue_assignments.size(); i++)
      {
        assert(lvalue_assignments[i].size() == lhs_indices.size());

        std::string cur_line_prefix = line_prefix;
        std::string cur_line        = line;

        map_indices(cur_line_prefix, this->prefix, lvalue_assignments[i]);
        for(int j = 0; j < lhs_indices.size(); j++)
          {
            lhs_indices[j].assignment = (lvalue_assignments[i])[j];
          }

        rval += this->apply_index(cur_line, lhs_indices, semicolon, comma, line_prefix != "", this->index_rule_cache);

        if(i > 0) new_line += NEWLINE_CHAR;
        new_line += cur_line_prefix + cur_line;
      }

    line = new_line;

    rval += this->apply_simple(line, this->post_rule_cache);

    return(rval);
  }


unsigned int macro_package::apply_simple(std::string& line, std::vector<macro_packages::simple_rule>& ruleset, bool blank)
  {
    unsigned int rval = 0;

    while(line.find(this->prefix) != std::string::npos)
      {
        bool fail = true;

        for(int i = 0; i < ruleset.size(); i++)
          {
            size_t pos;
            macro_packages::simple_rule rule = ruleset[i];

            if((pos = line.find(this->prefix + rule.name)) != std::string::npos)
              {
                std::vector<std::string> arg_list = this->get_argument_list(line, pos + this->prefix.size() + rule.name.size(),
                                                                            rule.args, rule.name);

                // found a simple macro; replace it with its value
                std::string replacement = blank ? "" : (rule.rule)(arg_list);
                line.replace(pos, this->prefix.size() + rule.name.size(), replacement);
                rval++;
                fail = false;
              }
          }

        if(fail) break;    // no more simple macros to replace
      }

    return(rval);
  }


// TODO: this macro replacement implementation is fairly kludgy - would be nice to improve it
unsigned int macro_package::apply_index(std::string& line, const std::vector<struct index_abstract>& lhs_indices,
  const bool semicolon, const bool comma, const bool lhs_present,
  std::vector<macro_packages::index_rule>& ruleset)
  {
    unsigned int rval = 0;

    bool replaced = false;    // have we made any macro replacements? if not, might need to add a trailing semicolon or comma later

    // loop while there are still macros to replace, indicated by the presence of 'this->prefix'
    while(line.find(this->prefix) != std::string::npos)
      {
        bool fail = true;   // flag to break out of the loop, if instances of 'this->prefix' still exist but can't be rewritten
     
        // loop over all macros, testing whether we can make a replacement
        for(int i = 0; i < ruleset.size(); i++)
          {
            macro_packages::index_rule rule = ruleset[i];
            size_t pos;

            if((pos = line.find(this->prefix + rule.name)) != std::string::npos)
              {
                replaced = true;  // mark that we have done at least one macro replacement
                fail = false;     //
                rval++;

                std::string new_line = "";

                // found a macro -- strip out the index set associated with it
                // NOTE - this call leaves the index list intact in 'line'
                std::vector<struct index_abstract> indices =
                                                     this->get_index_set(line, pos + this->prefix.size() + rule.name.size(),
                                                                         rule.name, rule.indices, rule.range);

                // work through the index set, matching to lhs_indices if possible
                // these won't be replaced; instead, we use the current value provided to us
                // NOTE - this need not replace *all* instances of LHS indices (some might not appear
                // in association with this macro), so we cannot rely on that later
                for(int j = 0; j < indices.size(); j++)
                  {
                    for(int k = 0; k < lhs_indices.size(); k++)
                      {
                        if(indices[j].label == lhs_indices[k].label)
                          {
                            indices[j].assign     = false;
                            indices[j].assignment = lhs_indices[k].assignment;
                            break;
                          }
                      }
                  }

                // obtain a list of suitable index assignments
                assignment_package assign(this->fields, this->parameters, (this->order == indexorder_left) ? index_left_order : index_right_order);
                std::vector< std::vector<struct index_assignment> > assgn = assign.assign(indices);

                // get argument list for this macro
                // NOTE - this call *deletes* the argument list from 'line'
                std::vector<std::string> arg_list =
                                           get_argument_list(line,
                                                             pos + this->prefix.size() + rule.name.size() + 2 + indices.size(),
                                                             rule.args, rule.name);

                // set up state, if required
                void* state = (rule.pre != nullptr) ? (rule.pre)(arg_list) : nullptr;

                // for each index assignment, write out a replaced version
                bool endofline = false;
                for(int j = 0; j < assgn.size(); j++)
                  {
                    assert(assgn[j].size() == indices.size());    // make sure we have the correct number of of indices

                    if(j > 0 && endofline)
                      {
                        if(comma) new_line += ',';
                        new_line += NEWLINE_CHAR;
                      }
                    endofline = false;

                    // replace macro
                    std::string temp_line = line;

                    temp_line.replace(pos, this->prefix.size() + rule.name.size() + 2 + indices.size(),
                                      (rule.rule)(arg_list, assgn[j], state));

                    // map indices associated with this macro
                    map_indices(temp_line, this->prefix, assgn[j]);

                    // check whether there are any unresolved replacements after rewriting any remaining instances of LHS indices
                    // and removing any instances of post-macros

                    // if there are none, we want to consider adding a terminal comma or semicolon if that has been asked for
                    // we only do this if there is no LHS present, because only in that case is the result of
                    // each individual index replacement a complete statement;
                    // if a LHS is present, then it's the *sum* of all index replacements which is a complete statement

                    // if there are still indices to be replaced then we do nothing

                    std::string temp_temp_line = temp_line;
                    std::vector<struct index_assignment> lhs_assignments;
                    for(int k = 0; k < lhs_indices.size(); k++)
                      {
                        lhs_assignments.push_back(lhs_indices[k].assignment);
                      }
                    map_indices(temp_temp_line, this->prefix, lhs_assignments);
                    this->apply_simple(temp_temp_line, this->post_rule_cache, true);    // blank these remaining macros

                    if(temp_temp_line.find(this->prefix) == std::string::npos)
                      {
                        if(!lhs_present)
                          {
                            if(semicolon) temp_line += ';';
                            endofline = true;
                          }
                      }

                    // add this line
                    new_line += temp_line;
                  }

                // destroy state
                if(rule.post != nullptr) (rule.post)(state);

                line = new_line;
              }
          }

        if(fail) break;
      }

    // finally, rewrite any instances of LHS indices which have not yet been replaced
    // eg. they may not appear explicitly in any RHS macros
    std::vector<struct index_assignment> lhs_assignments;
    for(int i = 0; i < lhs_indices.size(); i++)
      {
        lhs_assignments.push_back(lhs_indices[i].assignment);
      }
    map_indices(line, this->prefix, lhs_assignments);

    // add semicolons and commas if ndeeded,
    // but don't add a comma if we made any index replacement.
    // In that case, presumably, the indices make a list which does not require a terminal comma
    if((lhs_present || !replaced) && semicolon) line += ';';
    if(!replaced && comma)                      line += ',';

    return(rval);
  }


// **************************************************************************************


std::vector<struct index_abstract> macro_package::get_lhs_indices(std::string lhs)
  {
    std::vector<struct index_abstract> rval;
    size_t                             pos;

    while((pos = lhs.find(this->prefix)) != std::string::npos)
      {
        if(pos + this->prefix.size() > lhs.size() + 1)
          {
            std::ostringstream msg;
            msg << ERROR_EXPECTED_INDEX << " '" << this->prefix << "'";
            this->package->error(msg.str());
            lhs.replace(pos, this->prefix.size()+1, "");
          }
        else
          {
            struct index_abstract index;
            index.label  = lhs[pos + this->prefix.size()];
            index.range  = INDEX_RANGE_UNKNOWN;            // this will be assigned later, when we know which object this index belongs to
            index.assign = true;                           // mark for inclusion when generating assignments
            rval.push_back(index);

            while((pos = lhs.find(this->prefix + index.label)) != std::string::npos)
              {
                lhs.replace(pos, this->prefix.size()+1, "");
              }
          }
      }

    return(rval);
  }


void macro_package::assign_lhs_index_types(std::string rhs, std::vector<struct index_abstract>& lhs_indices,
                                           std::vector<macro_packages::index_rule>& ruleset)
  {
    while(rhs.find(this->prefix) != std::string::npos)
      {
        bool fail = true;     // flag to break out of the loop, if instances of 'this->prefix' still exist but can't be recognized

        for(int i = 0; i < ruleset.size(); i++)
          {
            macro_packages::index_rule rule = ruleset[i];
            size_t pos;

            if((pos = rhs.find(this->prefix + rule.name)) != std::string::npos)
              {
                // found a macro -- strip out the index set
                std::vector<struct index_abstract> indices =
                  get_index_set(rhs, pos + this->prefix.size() + rule.name.size(),
                                rule.name, rule.indices, rule.range);

                // now try to match this index set with elements of lhs_indices
                for(int j = 0; j < indices.size(); j++)
                  {
                    for(int k = 0; k < lhs_indices.size(); k++)
                      {
                        if(indices[j].label == lhs_indices[k].label)
                          {
                            if(lhs_indices[k].range != INDEX_RANGE_UNKNOWN) // must be a duplicate
                              {
                                std::ostringstream msg;
                                msg << ERROR_DUPLICATE_INDEX << " '" << lhs_indices[k].label << "'";
                                this->package->error(msg.str());
                              }
                            lhs_indices[k].range = rule.range;

                            break;
                          }
                      }
                  }

                // remove this macro
                rhs.replace(pos, this->prefix.size() + rule.name.size() + 2 + rule.indices, "");
                fail = false;
              }
          }

        if(fail) break;
      }

    // pick sensible defaults for any LHS indices which remain unassigned
    this->assign_index_defaults(lhs_indices);
  }


void macro_package::assign_index_defaults(std::vector<struct index_abstract>& lhs_indices)
  {
    for(int i = 0; i < lhs_indices.size(); i++)
      {
        if(lhs_indices[i].range == INDEX_RANGE_UNKNOWN)
          {
            if(islower(lhs_indices[i].label))         // assume ranges only over fields if lower case Latin letter, otherwise fields+momenta
              {
                lhs_indices[i].range = 1;
              }
            else if (isdigit(lhs_indices[i].label))  // assume numeric labels range over parameters
              {
                lhs_indices[i].range = INDEX_RANGE_PARAMETER;
              }
            else
              {
                lhs_indices[i].range = 2;
              }
          }
      }
  }


// **************************************************************************************


std::vector<std::string> macro_package::get_argument_list(std::string& line, size_t pos,
  unsigned int num_args, std::string macro_name)
  {
    std::vector<std::string> rval;
    size_t                   length = 0;

    if(line[pos] == '{')   // an argument list follows
      {
        length++;
        std::string arg;

        while(pos + length < line.size() && line[pos + length] != '}')
          {
            if(line[pos + length] == ',')
              {
                rval.push_back(arg);
                arg = "";
              }
            else if(line[pos + length] == '"' && arg == "")
              {
                length++;
                while(pos + length < line.size() && line[pos + length] != '"')
                  {
                    arg += line[pos + length];
                    length++;
                  }
              }
            else
              {
                if(!isspace(line[pos + length]))
                  {
                    arg += line[pos + length];
                  }
              }
            length++;
          }
        if(pos + length >= line.size())
          {
            this->package->error(ERROR_EXPECTED_CLOSE_ARGL);
          }
        else
          {
            length++;
          }

        if(arg != "")
          {
            rval.push_back(arg);
          }
      }

    if(rval.size() != num_args)
      {
        std::ostringstream msg;
        msg << ERROR_WRONG_ARG_NUM << " '" << macro_name << "' (" << ERROR_WRONG_ARG_NUM_EXPECT << " " << num_args << ")";
        this->package->error(msg.str());
      }

    line.replace(pos, length, "");
    return(rval);
  }


std::vector<struct index_abstract> macro_package::get_index_set(std::string line, size_t pos, std::string name, unsigned int indices,
  unsigned int range)
  {
    std::vector<struct index_abstract> rhs_indices;

    if(line.size() < pos + 2 + indices)
      {
        std::ostringstream msg;
        msg << ERROR_TOO_FEW_INDICES << " '" << name << "'";
        this->package->error(msg.str());
      }
    if(line[pos] != '[')
      {
        std::ostringstream msg;
        msg << ERROR_EXPECTED_OPEN_IDXL << " '" << name << "'";
        this->package->error(msg.str());
      }
    else
      {
        pos++;
      }

    for(int i = 0; i < indices; i++)
      {
        if(line[pos] == ']')
          {
            std::ostringstream msg;
            msg << ERROR_TOO_FEW_INDICES << " '" << name << "'";
            this->package->error(msg.str());
            break;
          }
        else
          {
            struct index_abstract index;

            index.label  = line[pos];
            index.range  = range;             // index_assignment has the potential for different index types per object, but not currently used
            index.assign = true;              // default to assign; may be switched later if this turns out to be a LHS index

            rhs_indices.push_back(index);
            pos++;
          }
      }
    if(line[pos] != ']')
      {
        std::ostringstream msg;
        msg << ERROR_EXPECTED_CLOSE_IDXL << " '" << name << "'";
        this->package->error(msg.str());
      }

    return(rhs_indices);
  }


void macro_package::map_indices(std::string& line, std::string prefix, const std::vector<struct index_assignment>& assignment)
  {
    for(int k = 0; k < assignment.size(); k++)
      {
        size_t index_pos;

        while((index_pos = line.find(prefix + assignment[k].label)) != std::string::npos)
          {
            std::ostringstream replacement;
            replacement << index_numeric(assignment[k]);
            line.replace(index_pos, prefix.size() + 1, replacement.str());
          }
      }
  }