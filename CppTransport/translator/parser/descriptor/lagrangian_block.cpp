//
// Created by David Seery on 24/05/2017.
// --@@
// Copyright (c) 2017 University of Sussex. All rights reserved.
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


#include "lagrangian_block.h"
#include "generics_descriptor.h"


const auto DERIV_PREFIX = "__d";


// CONSTRUCTOR


std::vector<std::string>     fake_keyword_table;
std::vector<keyword_type>    fake_keyword_map;
std::vector<std::string>     fake_character_table;
std::vector<character_type>  fake_character_map;
std::vector<bool>            fake_context_table;


lagrangian_block::lagrangian_block(symbol_factory& s, version_policy& vp, error_context err_ctx)
  : sym_factory(s),
    policy(vp)
  {
    // set up reserved symbol for Planck mass
    M_Planck = sym_factory.get_symbol(MPLANCK_SYMBOL, MPLANCK_LATEX_SYMBOL);

    // manufacture fake lexeme representing 'location' of Planck mass decalaration
    lexeme::lexeme_buffer MPlanck_buffer(MPLANCK_TEXT_NAME, lexeme::lexeme_buffer::type::string_literal, nullptr, 0);
    y::lexeme_type::minus_context mctx = y::lexeme_type::minus_context::unary;

    y::lexeme_type fake_MPlanck_lexeme(MPlanck_buffer, mctx, 0, err_ctx,
                                       fake_keyword_table, fake_keyword_map, fake_character_table,
                                       fake_character_map, fake_context_table);

    // set up attributes for Planck mass symbol
    attributes Mp_attrs;
    Mp_attrs.set_latex(MPLANCK_LATEX_SYMBOL, fake_MPlanck_lexeme);

    // emplace faked symbol table entry
    reserved.emplace(std::make_pair(MPLANCK_TEXT_NAME, std::make_unique<parameter_declaration>(MPLANCK_TEXT_NAME, M_Planck, fake_MPlanck_lexeme, Mp_attrs)));
  }


// SYMBOL SERVICES


bool lagrangian_block::add_field(const std::string& n, GiNaC::symbol& s, const y::lexeme_type& l, attributes& a)
  {
    if(this->symbols_frozen) return this->report_frozen(l);
    
    auto check = [&](auto& name) -> auto { return this->check_symbol_exists(name); };
    auto insert = [&](auto& name, auto& symbol, auto& lexeme, auto& attr) -> auto
      {
        // add declaration to list
        this->fields.emplace(std::make_pair(name, std::make_unique<field_declaration>(name, symbol, lexeme, attr)));

        // also need to generate a symbol for the momentum corresponding to this field
        GiNaC::symbol deriv_symbol(DERIV_PREFIX + symbol.get_name());
        this->deriv_symbols.push_back(deriv_symbol);
      };

    return GenericInsertSymbol(check, insert, n, s, l, a, ERROR_SYMBOL_EXISTS, NOTIFY_DUPLICATION_DEFINITION_WAS);
  }


bool lagrangian_block::add_parameter(const std::string& n, GiNaC::symbol& s, const y::lexeme_type& l, attributes& a)
  {
    if(this->symbols_frozen) return this->report_frozen(l);

    auto check = [&](auto& name) -> auto { return this->check_symbol_exists(name); };
    auto insert = [&](auto& name, auto& symbol, auto& lexeme, auto& attr) -> auto
      {
        // add declaration to list
        this->parameters.emplace(std::make_pair(name, std::make_unique<parameter_declaration>(name, symbol, lexeme, attr)));
      };

    return GenericInsertSymbol(check, insert, n, s, l, a, ERROR_SYMBOL_EXISTS, NOTIFY_DUPLICATION_DEFINITION_WAS);
  }


bool lagrangian_block::add_subexpr(const std::string& n, GiNaC::symbol& s, const y::lexeme_type& l, subexpr& e)
  {
    if(this->symbols_frozen) return this->report_frozen(l);

    auto check = [&](auto& name) -> auto { return this->check_symbol_exists(name); };
    auto insert = [&](auto& name, auto& symbol, auto& lexeme, auto& expr) -> auto
      {
        // add declaration to list
        this->subexprs.emplace(std::make_pair(name, std::make_unique<subexpr_declaration>(name, symbol, lexeme, expr)));
      };

    return GenericInsertSymbol(check, insert, n, s, l, e, ERROR_SYMBOL_EXISTS, NOTIFY_DUPLICATION_DEFINITION_WAS);
  }


boost::optional<declaration&> lagrangian_block::check_symbol_exists(const std::string& nm) const
  {
    // check user-defined symbols

    auto f_it = this->fields.find(nm);
    if(f_it != this->fields.end()) return *f_it->second;

    auto p_it = this->parameters.find(nm);
    if(p_it != this->parameters.end()) return *p_it->second;

    p_it = this->reserved.find(nm);
    if(p_it != this->reserved.end()) return *p_it->second;

    auto s_it = this->subexprs.find(nm);
    if(s_it != this->subexprs.end()) return *s_it->second;

    // didn't find anything -- assume symbol does not exist
    return(boost::none);
  }


// LAGRANGIAN MANAGEMENT


bool lagrangian_block::set_potential(GiNaC::ex& V, const y::lexeme_type& l)
  {
    // symbol tables should be frozen whenever we start to specify the model
    this->freeze_tables(l);

    return SetContextedValue(this->potential, V, l, ERROR_POTENTIAL_REDECLARATION);
  }


boost::optional< contexted_value<GiNaC::ex>& > lagrangian_block::get_potential() const
  {
    if(this->potential) return *this->potential; else return boost::none;
  }


// FIELDS AND PARAMETERS


unsigned int lagrangian_block::get_number_fields() const
  {
    return(static_cast<unsigned int>(this->fields.size()));
  }


unsigned int lagrangian_block::get_number_params() const
  {
    return(static_cast<unsigned int>(this->parameters.size()));
  }


std::vector<std::string> lagrangian_block::get_field_name_list() const
  {
    auto Get = [&](const field_symbol_table::mapped_type::element_type& v) -> auto { return v.get_name(); };
    return UnzipSortedZipList(this->fields, Get);
  }


std::vector<std::string> lagrangian_block::get_field_latex_list() const
  {
    auto Get = [&](const field_symbol_table::mapped_type::element_type& v) -> auto { return v.get_latex_name(); };
    return UnzipSortedZipList(this->fields, Get);
  }


std::vector<std::string> lagrangian_block::get_param_name_list() const
  {
    auto Get = [&](const parameter_symbol_table::mapped_type::element_type& v) -> auto { return v.get_name(); };
    return UnzipSortedZipList(this->parameters, Get);
  }


std::vector<std::string> lagrangian_block::get_param_latex_list() const
  {
    auto Get = [&](const parameter_symbol_table::mapped_type::element_type& v) -> auto { return v.get_latex_name(); };
    return UnzipSortedZipList(this->parameters, Get);
  }


symbol_list lagrangian_block::get_field_symbols() const
  {
    auto Get = [&](const field_symbol_table::mapped_type::element_type& v) -> auto { return v.get_ginac_symbol(); };
    return UnzipSortedZipList(this->fields, Get);
  }


symbol_list lagrangian_block::get_deriv_symbols() const
  {
    return(this->deriv_symbols);
  }


symbol_list lagrangian_block::get_param_symbols() const
  {
    auto Get = [&](const parameter_symbol_table::mapped_type::element_type& v) -> auto { return v.get_ginac_symbol(); };
    return UnzipSortedZipList(this->parameters, Get);
  }


const GiNaC::symbol& lagrangian_block::get_Mp_symbol() const
  {
    return(this->M_Planck);
  }


validation_exceptions lagrangian_block::validate() const
  {
    validation_exceptions list;
    
    if(!this->potential) list.push_back(std::make_unique<validation_message>(true, ERROR_NO_POTENTIAL));
    
    return list;
  }


void lagrangian_block::set_lagrangian_type(model_type t, const y::lexeme_type& l)
  {
    SetContextedValue(this->type, t, l, ERROR_LAGRANGIAN_TYPE_REDECLARATION);
  }


void lagrangian_block::freeze_tables(const y::lexeme_type& l)
  {
    // nothing to do if already frozen
    if(this->symbols_frozen) return;
    
    // set token contexted value to remember where freezing-point occurred
    SetContextedValue(this->symbols_frozen, 1u, l, "");
  }


bool lagrangian_block::report_frozen(const y::lexeme_type& l)
  {
    l.get_error_context().error(ERROR_MODEL_SPECIFICATION_STARTED);
    if(this->symbols_frozen)
      {
        auto& v = *this->symbols_frozen;
        v.get_declaration_point().warn(WARNING_SPECIFICATION_START_WAS);
      }

    throw parse_error(ERROR_MODEL_SPECIFICATION_STARTED);
  }
