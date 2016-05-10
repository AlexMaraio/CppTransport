//
// Created by David Seery on 01/05/2016.
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

// include implementation header generated by translator
#include "dquad_mpi.h"

int main(int argc, char* argv[]);

void write_tasks(transport::repository<>& repo, transport::dquad_mpi<>* model);


void write_tasks(transport::repository<>& repo, transport::dquad_mpi<>* model)
  {
    const double Mp   = 1.0;
    const double Mphi = 9E-5 * Mp;
    const double Mchi = 1E-5 * Mp;

    transport::parameters<> params(Mp, {Mphi, Mchi}, model);

    const double phi_init = 10.0 * Mp;
    const double chi_init = 12.9 * Mp;

    const double N_init = 0.0;
    const double N_pre  = 12.0;
    const double N_end  = 60.0;

    transport::initial_conditions<> ics("dquad", params, {phi_init, chi_init}, N_init, N_pre);

    transport::basic_range<> ts(N_init, N_end, 300, transport::spacing::linear);

    const double kt_lo = std::exp(3.0);
    const double kt_hi = std::exp(9.0);

    transport::basic_range<> ks(kt_lo, kt_hi, 50, transport::spacing::log_bottom);

    transport::twopf_task<> tk2("dquad.twopf", ics, ts, ks);
    tk2.set_adaptive_ics_efolds(4.0);
    tk2.set_description("Compute time history of the 2-point function from k ~ e^3 to k ~ e^9");

    transport::threepf_cubic_task<> tk3("dquad.threepf", ics, ts, ks);
    tk3.set_adaptive_ics_efolds(4.0);
    tk3.set_description("Compute time history of the 3-point function on a cubic lattice from k ~ e^3 to k ~ e^9");

    transport::zeta_twopf_task<> ztk2("dquad.twopf-zeta", tk2);
    ztk2.set_description("Convert the output from dquad.twopf into a zeta 2-point function");

    transport::zeta_threepf_task<> ztk3("dquad.threepf-zeta", tk3);
    ztk3.set_description("Convert the output from dquad.threepf into zeta 2- and 3-point functions");

    repo.commit(ztk2);
    repo.commit(ztk3);
  }


int main(int argc, char* argv[])
  {
    // set up a task_manager instance to control this process
    transport::task_manager<> mgr(argc, argv);

    // set up an instance of the double quadratic model
    std::shared_ptr< transport::dquad_mpi<> > model = mgr.create_model< transport::dquad_mpi<> >();

    // register task writer
    mgr.add_generator([=](transport::repository<>& repo) -> void { write_tasks(repo, model.get()); });

    // hand off control to the task manager
    mgr.process();

    return(EXIT_SUCCESS);
  }
