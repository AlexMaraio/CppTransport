//
// Created by David Seery on 20/12/2015.
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

#include "u3.h"


namespace nontrivial_metric
  {

    std::unique_ptr<flattened_tensor>
    u3::compute(const index_literal_list& indices, symbol_wrapper& k1, symbol_wrapper& k2, symbol_wrapper& k3,
                symbol_wrapper& a)
      {
        if(indices.size() != U3_TENSOR_INDICES) throw tensor_exception("U3 indices");

        auto result = std::make_unique<flattened_tensor>(this->fl.get_flattened_size<phase_index>(U3_TENSOR_INDICES));
    
        const phase_index max_i = this->shared.get_max_phase_index(indices[0]->get_variance());
        const phase_index max_j = this->shared.get_max_phase_index(indices[1]->get_variance());
        const phase_index max_k = this->shared.get_max_phase_index(indices[2]->get_variance());

        // set up a TensorJanitor to manage use of cache
        TensorJanitor J(*this, indices);

        for(phase_index i = phase_index(0, indices[0]->get_variance()); i < max_i; ++i)
          {
            for(phase_index j = phase_index(0, indices[1]->get_variance()); j < max_j; ++j)
              {
                for(phase_index k = phase_index(0, indices[2]->get_variance()); k < max_k; ++k)
                  {
                    (*result)[this->fl.flatten(i, j, k)] = this->compute_component(i, j, k, k1, k2, k3, a);
                  }
              }
          }

        return(result);
      }
    
    
    GiNaC::ex u3::compute_component(phase_index i, phase_index j, phase_index k,
                                    symbol_wrapper& k1, symbol_wrapper& k2, symbol_wrapper& k3, symbol_wrapper& a)
      {
        if(!this->cached) throw tensor_exception("U3 cache not ready");

        GiNaC::ex result;

        // we don't actively cache u3 results; the A, B, C agents individually cache their own results, so
        // we can just rely on these

        // note that we flip the sign of momenta attached to the i, j, k components compared to the
        // analytic definition
        // this accounts for integrating out the delta-functions when contracting u3 with something else

        // also, note factor of 2 from definition of 2nd order term in transport eq: dX/dN = u2.X + (1/2) u3.X.X + ...

        field_index species_i = this->traits.to_species(i);
        field_index species_j = this->traits.to_species(j);
        field_index species_k = this->traits.to_species(k);

        bool is_species_i = this->traits.is_species(i);
        bool is_species_j = this->traits.is_species(j);
        bool is_species_k = this->traits.is_species(k);

        bool is_momentum_i = !is_species_i;
        bool is_momentum_j = !is_species_j;
        bool is_momentum_k = !is_species_k;

        if(is_species_i && is_species_j && is_species_k)
          {
            result = - B_agent.compute_component(species_j, species_k, species_i, k2, k3, k1, a);
          }
        else if(is_species_i && is_species_j && is_momentum_k)
          {
            result = - C_agent.compute_component(species_i, species_k, species_j, k1, k3, k2, a);
          }
        else if(is_species_i && is_momentum_j && is_species_k)
          {
            result = - C_agent.compute_component(species_i, species_j, species_k, k1, k2, k3, a);
          }
        else if(is_species_i && is_momentum_j && is_momentum_k)
          {
            result = 0;
          }
        else if(is_momentum_i && is_species_j && is_species_k)
          {
            result = 3 * A_agent.compute_component(species_i, species_j, species_k, k1, k2, k3, a);
          }
        else if(is_momentum_i && is_species_j && is_momentum_k)
          {
            result = B_agent.compute_component(species_i, species_j, species_k, k1, k2, k3, a);
          }
        else if(is_momentum_i && is_momentum_j && is_species_k)
          {
            result = B_agent.compute_component(species_i, species_k, species_j, k1, k3, k2, a);
          }
        else if(is_momentum_i && is_momentum_j && is_momentum_k)
          {
            result = C_agent.compute_component(species_j, species_k, species_i, k2, k3, k1, a);
          }
        else throw tensor_exception("U3 index");

        return(result);
      }
    
    
    unroll_state u3::get_unroll(const index_literal_list& idx_list)
      {
        const std::array< variance, RESOURCE_INDICES::DV_INDICES > i = { idx_list[0]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DV_INDICES > j = { idx_list[1]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DV_INDICES > k = { idx_list[2]->get_variance() };
    
        const std::array< variance, RESOURCE_INDICES::DDV_INDICES > ij = { idx_list[0]->get_variance(), idx_list[1]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DDV_INDICES > jk = { idx_list[1]->get_variance(), idx_list[2]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DDV_INDICES > ik = { idx_list[0]->get_variance(), idx_list[2]->get_variance() };
        
        const std::array< variance, RESOURCE_INDICES::DDDV_INDICES > ijk = { idx_list[0]->get_variance(), idx_list[1]->get_variance(), idx_list[2]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DDDV_INDICES > ikj = { idx_list[0]->get_variance(), idx_list[2]->get_variance(), idx_list[1]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DDDV_INDICES > jik = { idx_list[1]->get_variance(), idx_list[0]->get_variance(), idx_list[2]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DDDV_INDICES > jki = { idx_list[1]->get_variance(), idx_list[2]->get_variance(), idx_list[0]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DDDV_INDICES > kij = { idx_list[2]->get_variance(), idx_list[0]->get_variance(), idx_list[1]->get_variance() };
        const std::array< variance, RESOURCE_INDICES::DDDV_INDICES > kji = { idx_list[2]->get_variance(), idx_list[1]->get_variance(), idx_list[0]->get_variance() };

        // if any index is covariant then we need the metric to pull down an index on the coordinate vector
        // if any index is contravariant then we need the inverse metric to push up an index on the potential derivatives
        bool has_G = true;
        bool has_Ginv = true;
        if(idx_list[0]->get_variance() == variance::covariant
           || idx_list[1]->get_variance() == variance::covariant
           || idx_list[2]->get_variance() == variance::covariant)
          {
            has_G = this->res.can_roll_metric();
          }

        if(idx_list[0]->get_variance() == variance::contravariant
           || idx_list[1]->get_variance() == variance::contravariant
           || idx_list[2]->get_variance() == variance::contravariant)
          {
            has_Ginv = this->res.can_roll_metric_inverse();
          }
    
        if(this->shared.can_roll_coordinates() && has_G && has_Ginv
           && this->res.can_roll_dV(i)
           && this->res.can_roll_dV(j)
           && this->res.can_roll_dV(k)
           && this->res.can_roll_ddV(ij)
           && this->res.can_roll_ddV(ik)
           && this->res.can_roll_ddV(jk)
           && this->res.can_roll_dddV(ijk)
           && this->res.can_roll_dddV(ikj)
           && this->res.can_roll_dddV(jik)
           && this->res.can_roll_dddV(jki)
           && this->res.can_roll_dddV(kij)
           && this->res.can_roll_dddV(kji))
          return unroll_state::allow;

        return unroll_state::force;   // can't roll-up
      }


    std::unique_ptr<map_lambda>
    u3::compute_lambda(const index_literal& i, const index_literal& j, const index_literal& k,
                       symbol_wrapper& k1, symbol_wrapper& k2, symbol_wrapper& k3, symbol_wrapper& a)
      {
        if(i.get_class() != index_class::full) throw tensor_exception("U3");
        if(j.get_class() != index_class::full) throw tensor_exception("U3");
        if(k.get_class() != index_class::full) throw tensor_exception("U3");

        auto idx_i = this->shared.generate_index<GiNaC::varidx>(i);
        auto idx_j = this->shared.generate_index<GiNaC::varidx>(j);
        auto idx_k = this->shared.generate_index<GiNaC::varidx>(k);

        auto args =
          this->res.generate_cache_arguments(use_dV | use_ddV | use_dddV,
                                             std::array<index_literal, 3>{i,j,k}, this->printer);
        args += { k1, k2, k3, a };
        args += { idx_i, idx_j, idx_k };

        // convert these indices to species-only indices
        const auto i_field_a = this->traits.species_to_species(i);
        const auto i_field_b = this->traits.momentum_to_species(i);
        const auto j_field_a = this->traits.species_to_species(j);
        const auto j_field_b = this->traits.momentum_to_species(j);
        const auto k_field_a = this->traits.species_to_species(k);
        const auto k_field_b = this->traits.momentum_to_species(k);

        map_lambda_table table(lambda_flattened_map_size(3));

        auto fff = B_agent.compute_lambda(*j_field_a.second, *k_field_a.second, *i_field_a.second, k2, k3, k1, a);
        auto ffm = C_agent.compute_lambda(*i_field_a.second, *k_field_b.second, *j_field_a.second, k1, k3, k2, a);
        auto fmf = C_agent.compute_lambda(*i_field_a.second, *j_field_b.second, *k_field_a.second, k1, k2, k3, a);
        auto mff = A_agent.compute_lambda(*i_field_b.second, *j_field_a.second, *k_field_a.second, k1, k2, k3, a);
        auto mfm = B_agent.compute_lambda(*i_field_b.second, *j_field_a.second, *k_field_b.second, k1, k2, k3, a);
        auto mmf = B_agent.compute_lambda(*i_field_b.second, *k_field_a.second, *j_field_b.second, k1, k3, k2, a);
        auto mmm = C_agent.compute_lambda(*j_field_b.second, *k_field_b.second, *i_field_b.second, k2, k3, k1, a);

        table[lambda_flatten(LAMBDA_FIELD, LAMBDA_FIELD, LAMBDA_FIELD)] = -(**fff);
        table[lambda_flatten(LAMBDA_FIELD, LAMBDA_FIELD, LAMBDA_MOMENTUM)] = -(**ffm);
        table[lambda_flatten(LAMBDA_FIELD, LAMBDA_MOMENTUM, LAMBDA_FIELD)] = -(**fmf);
        table[lambda_flatten(LAMBDA_FIELD, LAMBDA_MOMENTUM, LAMBDA_MOMENTUM)] = 0;
        table[lambda_flatten(LAMBDA_MOMENTUM, LAMBDA_FIELD, LAMBDA_FIELD)] = 3 * (**mff);
        table[lambda_flatten(LAMBDA_MOMENTUM, LAMBDA_FIELD, LAMBDA_MOMENTUM)] = **mfm;
        table[lambda_flatten(LAMBDA_MOMENTUM, LAMBDA_MOMENTUM, LAMBDA_FIELD)] = **mmf;
        table[lambda_flatten(LAMBDA_MOMENTUM, LAMBDA_MOMENTUM, LAMBDA_MOMENTUM)] = **mmm;

        return std::make_unique<map_lambda>(i, j, k, table, expression_item_types::U3_lambda, args, this->shared.generate_working_type());
      }
    
    
    u3::u3(language_printer& p, cse& cw, expression_cache& c, resources& r, shared_resources& s,
           boost::timer::cpu_timer& tm, index_flatten& f, index_traits& t)
      : ::u3(),
        A_agent(p, cw, c, r, s, tm, f, t),
        B_agent(p, cw, c, r, s, tm, f, t),
        C_agent(p, cw, c, r, s, tm, f, t),
        printer(p),
        cse_worker(cw),
        cache(c),
        shared(s),
        res(r),
        fl(f),
        traits(t),
        compute_timer(tm),
        cached(false)
      {
      }
    
    
    void u3::pre_explicit(const index_literal_list& indices)
      {
        if(cached) throw tensor_exception("U3 already cached");

        this->A_Janitor = std::make_unique<TensorJanitor>(this->A_agent, indices);
        this->B_Janitor = std::make_unique<TensorJanitor>(this->B_agent, indices);
        this->C_Janitor = std::make_unique<TensorJanitor>(this->C_agent, indices);

        this->cached = true;
      }
    
    
    void u3::post()
      {
        if(!this->cached) throw tensor_exception("U3 not cached");

        // invalidate cache
        this->A_Janitor.reset(nullptr);
        this->B_Janitor.reset(nullptr);
        this->C_Janitor.reset(nullptr);

        this->cached = false;
      }

  }   // namespace nontrivial_metric
