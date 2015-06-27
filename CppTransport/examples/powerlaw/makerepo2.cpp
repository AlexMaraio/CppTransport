//
// Created by David Seery on 6/6/2014.
// Copyright (c) 2014-15 University of Sussex. All rights reserved.
//


#include "powerlaw_basic_unrolled.h"

#include "transport-runtime-api/repository/repository_creation_key.h"


// ****************************************************************************


// set up parameters and initial conditions
// we work in units where M_p=1, but that's up to us;
// we could choose something different

const double M_Planck = 1.0;
const double m        = 1e-5;
const double Lambda   = pow(1.125E-8 / 100, 1.0/4.0) * M_Planck;

const double phi_init = 10.0  * M_Planck;
const double chi_init = 0.001 * M_Planck;


// ****************************************************************************


int main(int argc, char* argv[])
  {
		if(argc != 2)
			{
		    std::cerr << "makerepo: Too few arguments. Expected repository name" << std::endl;
		    exit(EXIT_FAILURE);
			}

    transport::repository_creation_key key;

    std::shared_ptr< transport::json_repository<double> > repo = transport::repository_factory<double>(argv[1], key);

    // set up an instance of a manager
    std::shared_ptr< transport::task_manager<double> > mgr = std::make_shared< transport::task_manager<double> >(0, nullptr, repo);

    // set up an instance of the axion-quadratic model,
    // using doubles, with given parameter choices
    transport::powerlaw_basic<double>* model = new transport::powerlaw_basic<double>(mgr);

    // set up parameter choices
    const std::vector<double>     init_params = { m, Lambda };
    transport::parameters<double> params(M_Planck, init_params, model);

    const std::vector<double> init_values = { phi_init, chi_init };

    const double Ninit  = 0.0;  // start counting from N=0 at the beginning of the integration
    const double Ncross = 9.5;  // horizon-crossing occurs at N=13
    const double Npre   = 9.5;  // number of e-folds of subhorizon evolution
    const double Nsplit = 1.5;  // split point between early and late
    const double Nmax   = 11.5; // how many e-folds to integrate after horizon crossing

    // set up initial conditions with the specified horizon-crossing time Ncross and Npre
    // e-folds of subhorizon evolution.
    // The resulting initial conditions apply at time Ncross-Npre
    transport::initial_conditions<double> ics("powerlaw-1", model, params, init_values, Ninit, Ncross, Npre);

    const unsigned int early_t_samples = 200;
    const unsigned int late_t_samples  = 100;

    transport::stepping_range<double> early_times(Ncross-Npre, Ncross+Nsplit, early_t_samples, transport::logarithmic_bottom_stepping);
    transport::stepping_range<double> late_times(Ncross+Nsplit, Ncross+Nmax, late_t_samples, transport::linear_stepping);
    transport::aggregation_range<double> times(early_times, late_times);

    // the conventions for k-numbers are as follows:
    // k=1 is the mode which crosses the horizon at time N*,
    // where N* is the 'offset' we pass to the integration method (see below)
    const double        ktmin         = exp(3.0);
    const double        ktmax         = exp(7.0);
    const unsigned int  k_samples     = 40;

		const double        alphamin      = 0.0;
		const double        alphamax      = 1.0/2.0;
		const unsigned int  a_samples     = 5;

		const double        betamin       = 0.0;
		const double        betamid       = 0.98;
		const double        betamax       = 0.999;
		const unsigned int  lo_b_samples  = 150;
		const unsigned int  hi_b_samples  = 200;

		struct ThreepfStoragePolicy
			{
		  public:
				bool operator() (const transport::threepf_kconfig& data) { return(true); }
			};


    // SET UP TASKS

    transport::stepping_range<double> kts   (ktmin, ktmax, k_samples, transport::linear_stepping);
    transport::stepping_range<double> alphas(alphamin, alphamax, a_samples, transport::linear_stepping);

//    transport::stepping_range<double> betas_lo(betamin, betamid, lo_b_samples, transport::linear_stepping);
//    transport::stepping_range<double> betas_hi(betamid, betamax, hi_b_samples, transport::logarithmic_top_stepping);
//    transport::aggregation_range<double> betas(betas_lo, betas_hi);
    transport::stepping_range<double> betas_equi(1.0/3.0, 1.0/3.0, 0, transport::linear_stepping);    // add dedicated equilateral configuration
    transport::stepping_range<double> betas_lo(0.0, 0.9, 5, transport::linear_stepping);
    transport::stepping_range<double> betas_mid(0.9, 0.99, 5, transport::logarithmic_top_stepping);
    transport::stepping_range<double> betas_hi(0.99, 0.999, 5, transport::logarithmic_top_stepping);
    transport::aggregation_range<double> betas = betas_lo + betas_mid + betas_hi + betas_equi;

    // construct a threepf task
    transport::threepf_fls_task<double> tk3("powerlaw.threepf-1", ics, times, kts, alphas, betas, ThreepfStoragePolicy(), false);
		tk3.set_fast_forward_efolds(4.0);
		tk3.set_collect_initial_conditions(true);

		// construct a zeta threepf task, paired with the primary integration task
    transport::zeta_threepf_task<double> ztk3("powerlaw.threepf-1.zeta", tk3);
    ztk3.set_paired(true);


    // SET UP SQL QUERIES

    // filter for all times
    transport::derived_data::SQL_time_config_query all_times("1=1");

		// filter: latest time
    transport::derived_data::SQL_time_config_query last_time("time IN (SELECT MAX(time) FROM time_samples)");

    // filter for all twopf wavenumbers
    transport::derived_data::SQL_twopf_kconfig_query all_twopfs("1=1");

    // filter: twopf with largest k
    transport::derived_data::SQL_twopf_kconfig_query largest_twopf("conventional IN (SELECT MAX(conventional) FROM twopf_samples)");

    // filter: (closest to) equilateral threepf with smallest k_t
    transport::derived_data::SQL_threepf_kconfig_query equilateral_smallest_threepf("ABS(alpha) < 0.01 AND ABS(beta-1.0/3.0) < 0.01 AND kt_conventional IN (SELECT MIN(kt_conventional) FROM threepf_samples)");

    // filter: squeezed, isosceles threepf
    transport::derived_data::SQL_threepf_kconfig_query isosceles_squeezed_threepf("ABS(beta-0.999)<0.0001 AND ABS(alpha)<0.01");

		// filter: equilateral with high k_t
    std::ostringstream hi_kt_query;
		hi_kt_query << std::setprecision(10) << "ABS(alpha) < 0.001 AND ABS(kt_conventional-" << exp(3.0) << ") < 0.01";
    transport::derived_data::SQL_threepf_kconfig_query isosceles_hi_kt(hi_kt_query.str());

    // filter: equilateral with lo k_t
    std::ostringstream lo_kt_query;
		lo_kt_query << std::setprecision(10) << "ABS(alpha) < 0.001 AND ABS(kt_conventional-" << exp(7.0) << ") < 0.001";
    transport::derived_data::SQL_threepf_kconfig_query isosceles_lo_kt(lo_kt_query.str());


    // PLOTS


    // a. TIME EVOLUTION OF THE FIELD-SPACE TWOPF, THREEPF


    transport::index_selector<2> twopf_fields(model->get_N_fields());
    twopf_fields.none();
		twopf_fields.set_on(std::array<unsigned int, 2>{ 0, 0 });
		twopf_fields.set_on(std::array<unsigned int, 2>{ 0, 1 });
		twopf_fields.set_on(std::array<unsigned int, 2>{ 1, 1 });

    transport::derived_data::twopf_time_series<double> tk3_twopf_group(tk3, twopf_fields, all_times, largest_twopf);
    tk3_twopf_group.set_klabel_meaning(transport::derived_data::conventional);

    transport::derived_data::time_series_plot<double> tk3_twopf_plot("powerlaw.threepf-1.twopf-time", "twopf-time.pdf");
    tk3_twopf_plot.add_line(tk3_twopf_group);
    tk3_twopf_plot.set_title_text("Time evolution of two-point function");
    tk3_twopf_plot.set_legend_position(transport::derived_data::bottom_left);

    transport::index_selector<3> threepf_fields(model->get_N_fields());
    threepf_fields.none();
    threepf_fields.set_on(std::array<unsigned int, 3>{ 0, 0, 0 });
	  threepf_fields.set_on(std::array<unsigned int, 3>{ 0, 1, 0 });
	  threepf_fields.set_on(std::array<unsigned int, 3>{ 1, 1, 0 });
	  threepf_fields.set_on(std::array<unsigned int, 3>{ 0, 0, 1 });
	  threepf_fields.set_on(std::array<unsigned int, 3>{ 0, 1, 1 });
	  threepf_fields.set_on(std::array<unsigned int, 3>{ 1, 1, 1 });

    transport::derived_data::threepf_time_series<double> tk3_threepf_group(tk3, threepf_fields, all_times, equilateral_smallest_threepf);
    tk3_twopf_group.set_klabel_meaning(transport::derived_data::conventional);

    transport::derived_data::time_series_plot<double> tk3_threepf_plot("powerlaw.threepf-1.threepf-time", "threepf-time.pdf");
    tk3_threepf_plot.add_line(tk3_threepf_group);
    tk3_threepf_plot.set_title_text("Time evolution of three-point function");
    tk3_threepf_plot.set_legend_position(transport::derived_data::bottom_left);


		// b. TIME EVOLUTION OF BACKGROUND QUANTITIES


    transport::derived_data::background_line<double> tk3_epsilon(tk3, all_times, transport::derived_data::epsilon);
    transport::derived_data::largest_u2_line<double> tk3_largest_u2(tk3, all_times, largest_twopf);
    transport::derived_data::largest_u3_line<double> tk3_largest_u3(tk3, all_times, equilateral_smallest_threepf);

    transport::derived_data::time_series_plot<double> tk3_SR_objects_plot("powerlaw.threepf-1.SR_objects", "SR_objects.pdf");
    tk3_SR_objects_plot.add_line(tk3_epsilon);
    tk3_SR_objects_plot.add_line(tk3_largest_u2);
    tk3_SR_objects_plot.add_line(tk3_largest_u3);
    tk3_SR_objects_plot.set_log_y(true);


    transport::derived_data::background_line<double> tk3_Hubble(tk3, all_times, transport::derived_data::Hubble);
    transport::derived_data::background_line<double> tk3_aHubble(tk3, all_times, transport::derived_data::aHubble);

    transport::derived_data::time_series_plot<double> tk3_Hubble_plot("powerlaw.threepf-1.Hubble", "Hubble.pdf");
		tk3_Hubble_plot.add_line(tk3_Hubble);
		tk3_Hubble_plot.add_line(tk3_aHubble);
		tk3_Hubble_plot.set_log_y(true);


    transport::index_selector<2> twopf_mf(model->get_N_fields());
    twopf_mf.none();
    twopf_mf.set_on(std::array<unsigned int, 2>{ 2, 0 });
    twopf_mf.set_on(std::array<unsigned int, 2>{ 2, 1 });
    twopf_mf.set_on(std::array<unsigned int, 2>{ 3, 0 });
    twopf_mf.set_on(std::array<unsigned int, 2>{ 3, 1 });
    transport::derived_data::u2_line<double> tk3_u2(tk3, twopf_mf, all_times, largest_twopf);

    transport::derived_data::time_series_plot<double> tk3_u2_plot("powerlaw.threepf-1.u2", "u2.pdf");
    tk3_u2_plot.add_line(tk3_u2);
    tk3_u2_plot.set_log_y(true);


    transport::derived_data::u3_line<double> tk3_u3(tk3, threepf_fields, all_times, equilateral_smallest_threepf);

    transport::derived_data::time_series_plot<double> tk3_u3_plot("powerlaw.threepf-1.u3", "u3.pdf");
    tk3_u3_plot.add_line(tk3_u3);
    tk3_u3_plot.set_log_y(true);


    // 1. TIME EVOLUTION OF THE ZETA TWOPF

    // check the zeta twopf
    transport::derived_data::zeta_twopf_time_series<double> tk3_zeta_twopf_group(ztk3, all_times, largest_twopf);
    tk3_zeta_twopf_group.set_dimensionless(true);

    transport::derived_data::time_series_plot<double> tk3_zeta_twopf("powerlaw.threepf-1.zeta-twopf", "zeta-twopf-time.pdf");

    tk3_zeta_twopf.add_line(tk3_zeta_twopf_group);
    tk3_zeta_twopf.set_title_text("$\\zeta$ two-point function");
    tk3_zeta_twopf.set_legend_position(transport::derived_data::bottom_left);


    // 2. TIME EVOLUTION OF THE ZETA THREEPF

    // check the zeta threepf
    transport::derived_data::zeta_threepf_time_series<double> tk3_zeta_sq_group(ztk3, all_times, isosceles_squeezed_threepf);
    tk3_zeta_sq_group.set_klabel_meaning(transport::derived_data::comoving);
    tk3_zeta_sq_group.set_use_beta_label(true);

    transport::derived_data::time_series_plot<double> tk3_zeta_sq("powerlaw.threepf-1.zeta-sq", "zeta-sq.pdf");
    tk3_zeta_sq.add_line(tk3_zeta_sq_group);
    tk3_zeta_sq.set_title_text("3pf of $\\zeta$ near squeezed configurations");

		// set up a table too
    transport::derived_data::time_series_table<double> tk3_zeta_sq_table("powerlaw.threepf-1.zeta-sq.table", "zeta-sq-table.txt");
		tk3_zeta_sq_table.add_line(tk3_zeta_sq_group);


    // 3. TIME EVOLUTION OF THE REDUCED BISPECTRUM

    // compute the reduced bispectrum in a few squeezed configurations
    transport::derived_data::zeta_reduced_bispectrum_time_series<double> tk3_zeta_redbsp(ztk3, all_times, isosceles_squeezed_threepf);
    tk3_zeta_redbsp.set_klabel_meaning(transport::derived_data::comoving);
    tk3_zeta_redbsp.set_use_beta_label(true);

    transport::derived_data::time_series_plot<double> tk3_redbsp("powerlaw.threepf-1.redbsp-sq", "redbsp-sq.pdf");
    tk3_redbsp.set_log_y(false);
    tk3_redbsp.set_abs_y(false);
    tk3_redbsp.add_line(tk3_zeta_redbsp);
    tk3_redbsp.set_legend_position(transport::derived_data::bottom_right);
    tk3_redbsp.set_title_text("Reduced bispectrum near squeezed configurations");

    transport::derived_data::time_series_table<double> tk3_redbsp_table = transport::derived_data::time_series_table<double>("powerlaw.threepf-1.redbsp-sq.table", "redbsp-sq-table.txt");
    tk3_redbsp_table.add_line(tk3_zeta_redbsp);


    // 4. LATE-TIME ZETA TWO POINT FUNCTION

    transport::derived_data::zeta_twopf_wavenumber_series<double> tk3_zeta_2spec(ztk3, last_time, all_twopfs);
    tk3_zeta_2spec.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_2spec.set_dimensionless(true);

    transport::derived_data::wavenumber_series_plot<double> tk3_zeta_2spec_plot("powerlaw.threepf-1.zeta-2spec", "zeta-2spec.pdf");
    tk3_zeta_2spec_plot.add_line(tk3_zeta_2spec);
		tk3_zeta_2spec_plot.set_typeset_with_LaTeX(true);
    tk3_zeta_2spec_plot.set_log_x(true);
    tk3_zeta_2spec_plot.set_title_text("$\\langle \\zeta \\zeta \\rangle$ power spectrum");

    transport::derived_data::wavenumber_series_table<double> tk3_zeta_2spec_table("powerlaw.threepf-1.zeta-2spec.table", "zeta-2spec-table.txt");
    tk3_zeta_2spec_table.add_line(tk3_zeta_2spec);


    // 5. LATE-TIME ZETA REDUCED BISPECTRUM -- FIXED k3/k_t, ISOSCELES TRIANGLES, VARYING k_t

    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_spec(ztk3, last_time, isosceles_squeezed_threepf);
		tk3_zeta_redbsp_spec.set_klabel_meaning(transport::derived_data::conventional);
		tk3_zeta_redbsp_spec.set_label_text("$k_3/k_t = 0.999$", "k3/k_t = 0.999");

    transport::derived_data::wavenumber_series_plot<double> tk3_redbsp_spec_plot("powerlaw.threepf-1.redbsp-spec", "redbsp-spec.pdf");
		tk3_redbsp_spec_plot.add_line(tk3_zeta_redbsp_spec);
		tk3_redbsp_spec_plot.set_typeset_with_LaTeX(true);
		tk3_redbsp_spec_plot.set_title_text("Reduced bispectrum at fixed $k_3/k_t$");
		tk3_redbsp_spec_plot.set_x_label_text("$k_t$");
		tk3_redbsp_spec_plot.set_log_x(true);
		tk3_redbsp_spec_plot.set_log_y(false);
    tk3_redbsp_spec_plot.set_abs_y(false);

    transport::derived_data::wavenumber_series_table<double> tk3_redbsp_spec_table("powerlaw.threepf-1.redbsp-spec-table", "redbsp-spec-table.txt");
		tk3_redbsp_spec_table.add_line(tk3_zeta_redbsp_spec);


    // 6. LATE TIME ZETA REDUCED BISPECTRUM -- FIXED k_t, ISOSCELES TRIANGLES, VARYING k3/k_t
    // x-axis is beta

    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_beta_lo(ztk3, last_time, isosceles_lo_kt);
    tk3_zeta_redbsp_beta_lo.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_redbsp_beta_lo.set_current_x_axis_value(transport::derived_data::beta_axis);
		tk3_zeta_redbsp_beta_lo.set_label_text("$k_t/k_\\star = \\mathrm{e}^3$", "k_t/k* = exp(3)");
    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_beta_hi(ztk3, last_time, isosceles_hi_kt);
    tk3_zeta_redbsp_beta_hi.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_redbsp_beta_hi.set_current_x_axis_value(transport::derived_data::beta_axis);
		tk3_zeta_redbsp_beta_hi.set_label_text("$k_t/k_\\star = \\mathrm{e}^7$", "k_t/k* = exp(7)");

    transport::derived_data::wavenumber_series_plot<double> tk3_redbsp_beta_plot("powerlaw.threepf-1.redbsp-beta", "redbsp-beta.pdf");
		tk3_redbsp_beta_plot.add_line(tk3_zeta_redbsp_beta_lo);
    tk3_redbsp_beta_plot.add_line(tk3_zeta_redbsp_beta_hi);
		tk3_redbsp_beta_plot.set_typeset_with_LaTeX(true);
		tk3_redbsp_beta_plot.set_title_text("Shape-dependence of reduced bispectrum on isosceles triangles at fixed $k_t$");
		tk3_redbsp_beta_plot.set_log_y(false);
    tk3_redbsp_beta_plot.set_abs_y(false);
		tk3_redbsp_beta_plot.set_legend_position(transport::derived_data::centre_left);


    // 7. LATE TIME ZETA REDUCED BISPECTRUM -- FIXED k_t, ISOSCELES TRIANGLES, VARYING k3/k_t (SAME AS 6)
    // x-axis is squeezing ratio k3/k_t

    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_sqk3_lo(ztk3, last_time, isosceles_lo_kt);
    tk3_zeta_redbsp_sqk3_lo.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_redbsp_sqk3_lo.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
		tk3_zeta_redbsp_sqk3_lo.set_label_text("$k_t/k_\\star = \\mathrm{e}^3$", "k_t/k* = exp(3)");

    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_sqk3_hi(ztk3, last_time, isosceles_hi_kt);
    tk3_zeta_redbsp_sqk3_hi.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_redbsp_sqk3_hi.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
    tk3_zeta_redbsp_sqk3_hi.set_label_text("$k_t/k_\\star = \\mathrm{e}^7$", "k_t/k* = exp(7)");

    transport::derived_data::wavenumber_series_plot<double> tk3_redbsp_sqk3_plot("powerlaw.threepf-1.redbsp-sqk3", "redbsp-sqk3.pdf");
    tk3_redbsp_sqk3_plot.add_line(tk3_zeta_redbsp_sqk3_lo);
    tk3_redbsp_sqk3_plot.add_line(tk3_zeta_redbsp_sqk3_hi);
    tk3_redbsp_sqk3_plot.set_typeset_with_LaTeX(true);
    tk3_redbsp_sqk3_plot.set_title_text("Shape-dependence of reduced bispectrum on isosceles triangles at fixed $k_t$");
		tk3_redbsp_sqk3_plot.set_log_x(true);
		tk3_redbsp_sqk3_plot.set_log_y(false);
    tk3_redbsp_sqk3_plot.set_abs_y(false);
		tk3_redbsp_sqk3_plot.set_legend_position(transport::derived_data::centre_right);


    // 8. SPECTRAL INDEX OF LATE TIME REDUCED BISPECTRUM -- FIXED k_t, ISOSCELES TRIANGLES, VARYING k3/k_t (SAME AS 6,7)
		// spectral index of 7

    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_sqk3_lo_index(ztk3, last_time, isosceles_lo_kt);
    tk3_zeta_redbsp_sqk3_lo_index.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_redbsp_sqk3_lo_index.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
    tk3_zeta_redbsp_sqk3_lo_index.set_spectral_index(true);
    tk3_zeta_redbsp_sqk3_lo_index.set_label_text("$n_{f_{\\mathrm{NL}}} \\;\\; k_t/k_\\star = \\mathrm{e}^3$", "n_fNL k_t/k* = exp(3)");

    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_sqk3_hi_index(ztk3, last_time, isosceles_hi_kt);
    tk3_zeta_redbsp_sqk3_hi_index.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_redbsp_sqk3_hi_index.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
    tk3_zeta_redbsp_sqk3_hi_index.set_spectral_index(true);
    tk3_zeta_redbsp_sqk3_hi_index.set_label_text("$n_{f_{\\mathrm{NL}}} \\;\\; k_t/k_\\star = \\mathrm{e}^7$", "n_fNL k_t/k* = exp(7)");

    transport::derived_data::wavenumber_series_plot<double> tk3_redbsp_sqk3_index_plot("powerlaw.threepf-1.redbsp-sqk3-index", "redbsp-sqk3-index.pdf");
    tk3_redbsp_sqk3_index_plot.add_line(tk3_zeta_redbsp_sqk3_lo_index);
    tk3_redbsp_sqk3_index_plot.add_line(tk3_zeta_redbsp_sqk3_hi_index);
    tk3_redbsp_sqk3_index_plot.set_typeset_with_LaTeX(true);
    tk3_redbsp_sqk3_index_plot.set_title_text("Spectral index of reduced bispectrum on isosceles triangles at fixed $k_t$");
    tk3_redbsp_sqk3_index_plot.set_log_x(true);
    tk3_redbsp_sqk3_index_plot.set_log_y(false);
    tk3_redbsp_sqk3_index_plot.set_abs_y(false);
    tk3_redbsp_sqk3_index_plot.set_legend_position(transport::derived_data::bottom_left);


    // 9. SPECTRAL INDEX OF LATE-TIME ZETA REDUCED BISPECTRUM -- FIXED k3/k_t, ISOSCELES TRIANGLES, VARYING k_t

    transport::derived_data::zeta_reduced_bispectrum_wavenumber_series<double> tk3_zeta_redbsp_spec_index(ztk3, last_time, isosceles_squeezed_threepf);
    tk3_zeta_redbsp_spec_index.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_redbsp_spec_index.set_label_text("$n_{f_{\\mathrm{NL}}} \\;\\; k_3/k_t = 0.999$", "k3/k_t = 0.999");
    tk3_zeta_redbsp_spec_index.set_spectral_index(true);

    transport::derived_data::wavenumber_series_plot<double> tk3_redbsp_spec_index_plot("powerlaw.threepf-1.redbsp-spec-index", "redbsp-spec-index.pdf");
    tk3_redbsp_spec_index_plot.add_line(tk3_zeta_redbsp_spec_index);
    tk3_redbsp_spec_index_plot.set_typeset_with_LaTeX(true);
    tk3_redbsp_spec_index_plot.set_title_text("Spectral index of reduced bispectrum at fixed $k_3/k_t$");
    tk3_redbsp_spec_index_plot.set_x_label_text("$k_t$");
    tk3_redbsp_spec_index_plot.set_log_x(true);
    tk3_redbsp_spec_index_plot.set_log_y(false);
    tk3_redbsp_spec_index_plot.set_abs_y(false);


    // 10. SPECTRAL INDEX OF LATE-TIME ZETA TWO POINT FUNCTION

    transport::derived_data::zeta_twopf_wavenumber_series<double> tk3_zeta_2spec_index(ztk3, last_time, all_twopfs);
    tk3_zeta_2spec_index.set_klabel_meaning(transport::derived_data::conventional);
    tk3_zeta_2spec_index.set_dimensionless(true);
    tk3_zeta_2spec_index.set_spectral_index(true);

    transport::derived_data::wavenumber_series_plot<double> tk3_zeta_2spec_index_plot("powerlaw.threepf-1.zeta-2spec-index", "zeta-2spec-index.pdf");
    tk3_zeta_2spec_index_plot.add_line(tk3_zeta_2spec_index);
    tk3_zeta_2spec_index_plot.set_typeset_with_LaTeX(true);
    tk3_zeta_2spec_index_plot.set_title_text("$\\langle \\zeta \\zeta \\rangle$ spectral index");
    tk3_zeta_2spec_index_plot.set_log_x(true);
    tk3_zeta_2spec_index_plot.set_abs_y(false);


		// 11. INTEGRATION COST ANALYSIS

    transport::derived_data::cost_wavenumber<double> tk3_hi_cost(tk3, isosceles_hi_kt);
		tk3_hi_cost.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
		tk3_hi_cost.set_label_text("$k_t = \\mathrm{e}^7$", "k_t = exp(7)");

    transport::derived_data::cost_wavenumber<double> tk3_lo_cost(tk3, isosceles_lo_kt);
		tk3_lo_cost.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
		tk3_lo_cost.set_label_text("$k_t = \\mathrm{e}^3$", "k_t = exp(3)");

    transport::derived_data::wavenumber_series_plot<double> tk3_cost_plot("powerlaw.threepf-1.sqk3-cost", "sqk3-cost.pdf");
		tk3_cost_plot.add_line(tk3_lo_cost);
    tk3_cost_plot.add_line(tk3_hi_cost);
		tk3_cost_plot.set_typeset_with_LaTeX(true);
		tk3_cost_plot.set_log_x(true);
		tk3_cost_plot.set_log_y(true);


		// 11. LATE TIME SIGMA SPECTRUM

    transport::index_selector<2> sigma_sigma(2);
    sigma_sigma.none();
    sigma_sigma.set_on(std::array<unsigned int, 2>{1,1});

    transport::derived_data::twopf_wavenumber_series<double> tk3_sigma_spec(tk3, sigma_sigma, last_time, all_twopfs);
		tk3_sigma_spec.set_klabel_meaning(transport::derived_data::conventional);

    transport::derived_data::wavenumber_series_plot<double> tk3_sigma_spec_plot("powerlaw.threepf-1.sigma-spec", "sigma-spec.pdf");
		tk3_sigma_spec_plot.add_line(tk3_sigma_spec);
    tk3_sigma_spec_plot.set_typeset_with_LaTeX(true);
    tk3_sigma_spec_plot.set_log_x(true);
    tk3_sigma_spec_plot.set_log_y(true);
    tk3_sigma_spec_plot.set_abs_y(true);


		// 12. LATE TIME SIGMA BISPECTRUM -- FIXED k3/k_t, ISOSCELES TRIANGLES, VARYING k_t

    transport::index_selector<3> sigma_sigma_sigma(2);
    sigma_sigma_sigma.none();
    sigma_sigma_sigma.set_on(std::array<unsigned int, 3>{1,1,1});

    transport::derived_data::threepf_wavenumber_series<double> tk3_sigma_bsp_spec(tk3, sigma_sigma_sigma, last_time, isosceles_squeezed_threepf);
		tk3_sigma_bsp_spec.set_klabel_meaning(transport::derived_data::conventional);
		tk3_sigma_bsp_spec.set_label_text("$k_3/k_t = 0.999$", "k3/k_t = 0.999");

    transport::derived_data::wavenumber_series_plot<double> tk3_sigma_bsp_spec_plot("powerlaw.threepf-1.sigma-bsp-spec", "sigma-bsp-spec.pdf");
		tk3_sigma_bsp_spec_plot.add_line(tk3_sigma_bsp_spec);
		tk3_sigma_bsp_spec_plot.set_typeset_with_LaTeX(true);
		tk3_sigma_bsp_spec_plot.set_x_label_text("$k_t$");
		tk3_sigma_bsp_spec_plot.set_title_text("Scale-dependence of $\\sigma$ 3pf on isosceles triangles at fixed $k_3/k_t$");
		tk3_sigma_bsp_spec_plot.set_log_x(true);
		tk3_sigma_bsp_spec_plot.set_log_y(true);
		tk3_sigma_bsp_spec_plot.set_abs_y(true);


		// 13. LATE TIME SIGMA BISPECTRUM -- FIXED k_t, ISOSCELES TRIANGLES, VARYING k_3/k_t

    transport::derived_data::threepf_wavenumber_series<double> tk3_sigma_bsp_sqk3spec_lo(tk3, sigma_sigma_sigma, last_time, isosceles_lo_kt);
		tk3_sigma_bsp_sqk3spec_lo.set_klabel_meaning(transport::derived_data::conventional);
		tk3_sigma_bsp_sqk3spec_lo.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
		tk3_sigma_bsp_sqk3spec_lo.set_label_text("$k_3/k_\\star = \\mathrm{e}^3$", "k_t/k* = exp(3)");

    transport::derived_data::threepf_wavenumber_series<double> tk3_sigma_bsp_sqk3spec_hi(tk3, sigma_sigma_sigma, last_time, isosceles_hi_kt);
    tk3_sigma_bsp_sqk3spec_hi.set_klabel_meaning(transport::derived_data::conventional);
    tk3_sigma_bsp_sqk3spec_hi.set_current_x_axis_value(transport::derived_data::squeezing_fraction_k3_axis);
    tk3_sigma_bsp_sqk3spec_hi.set_label_text("$k_3/k_\\star = \\mathrm{e}^7$", "k_t/k* = exp(7)");

    transport::derived_data::wavenumber_series_plot<double> tk3_sigma_bsp_sqk3_plot("powerlaw.threepf-1.sigma-bsp-sqk3", "sigma-bsp-sqk3.pdf");
		tk3_sigma_bsp_sqk3_plot.add_line(tk3_sigma_bsp_sqk3spec_lo);
    tk3_sigma_bsp_sqk3_plot.add_line(tk3_sigma_bsp_sqk3spec_hi);
		tk3_sigma_bsp_sqk3_plot.set_typeset_with_LaTeX(true);
    tk3_sigma_bsp_sqk3_plot.set_title_text("Shape-dependence of $\\sigma$ 3pf on isosceles triangles at fixed $k_t$");
		tk3_sigma_bsp_sqk3_plot.set_log_x(true);
    tk3_sigma_bsp_sqk3_plot.set_log_y(true);
    tk3_sigma_bsp_sqk3_plot.set_abs_y(true);


    // 14. LATE TIME SIGMA SPECTRAL INDEX

    transport::derived_data::twopf_wavenumber_series<double> tk3_sigma_spec_index(tk3, sigma_sigma, last_time, all_twopfs);
    tk3_sigma_spec_index.set_klabel_meaning(transport::derived_data::conventional);
    tk3_sigma_spec_index.set_spectral_index(true);

    transport::derived_data::wavenumber_series_plot<double> tk3_sigma_spec_index_plot("powerlaw.threepf-1.sigma-spec-index", "sigma-spec-index.pdf");
    tk3_sigma_spec_index_plot.add_line(tk3_sigma_spec_index);
    tk3_sigma_spec_index_plot.set_typeset_with_LaTeX(true);
    tk3_sigma_spec_index_plot.set_log_x(true);
    tk3_sigma_spec_index_plot.set_log_y(false);
    tk3_sigma_spec_index_plot.set_abs_y(false);


    // 15. LATE TIME SIGMA BISPECTRUM INDEX - FIXED k3/kt, ISOSCELES TRIANGLES, VARYING k_t


    transport::derived_data::threepf_wavenumber_series<double> tk3_sigma_bsp_spec_index(tk3, sigma_sigma_sigma, last_time, isosceles_squeezed_threepf);
    tk3_sigma_bsp_spec_index.set_klabel_meaning(transport::derived_data::conventional);
    tk3_sigma_bsp_spec_index.set_label_text("$k_3/k_t = 0.999$", "k3/k_t = 0.999");
    tk3_sigma_bsp_spec_index.set_spectral_index(true);

    transport::derived_data::wavenumber_series_plot<double> tk3_sigma_bsp_spec_index_plot("powerlaw.threepf-1.sigma-bsp-spec-index", "sigma-bsp-spec-index.pdf");
    tk3_sigma_bsp_spec_index_plot.add_line(tk3_sigma_bsp_spec_index);
    tk3_sigma_bsp_spec_index_plot.set_typeset_with_LaTeX(true);
    tk3_sigma_bsp_spec_index_plot.set_x_label_text("$k_t$");
    tk3_sigma_bsp_spec_index_plot.set_title_text("Spectral index of $\\sigma$ 3pf on isosceles triangles at fixed $k_3/k_t$");
    tk3_sigma_bsp_spec_index_plot.set_log_x(true);
    tk3_sigma_bsp_spec_index_plot.set_log_y(false);
    tk3_sigma_bsp_spec_index_plot.set_abs_y(false);


    // OUTPUT TASKS


    transport::output_task<double> threepf_output = transport::output_task<double>("powerlaw.threepf-1.output", tk3_twopf_plot);
		threepf_output = threepf_output + tk3_threepf_plot
																		+ tk3_SR_objects_plot
                                    + tk3_Hubble_plot
                                    + tk3_u2_plot
                                    + tk3_u3_plot
                                    + tk3_zeta_twopf
                                    + tk3_zeta_sq
                                    + tk3_redbsp
                                    + tk3_zeta_2spec_plot
																		+ tk3_redbsp_spec_plot
																		+ tk3_redbsp_beta_plot
                                    + tk3_redbsp_sqk3_plot
                                    + tk3_redbsp_sqk3_index_plot
                                    + tk3_redbsp_spec_index_plot
                                    + tk3_zeta_2spec_index_plot
																		+ tk3_cost_plot
																		+ tk3_sigma_spec_plot
																		+ tk3_sigma_bsp_spec_plot
																		+ tk3_sigma_bsp_sqk3_plot
                                    + tk3_sigma_spec_index_plot
                                    + tk3_sigma_bsp_spec_index_plot;

    std::cout << "powerlaw.threepf-1 output task:" << std::endl << threepf_output << std::endl;

		// write output tasks to the database
    repo->commit_task(threepf_output);

    return(EXIT_SUCCESS);
  }