//
// Created by David Seery on 05/12/2013.
// Copyright (c) 2013-15 University of Sussex. All rights reserved.
//


#include <string>
#include <sstream>

#include <assert.h>
#include <functional>

#include "cpp_steppers.h"


#define BIND(X) std::bind(&cpp_steppers::X, this, std::placeholders::_1)


namespace cpp
  {

    constexpr unsigned int BACKG_STEPPER_STATE_ARGUMENT = 0;
    constexpr unsigned int BACKG_STEPPER_TOTAL_ARGUMENTS = 1;

    constexpr unsigned int PERT_STEPPER_STATE_ARGUMENT = 0;
    constexpr unsigned int PERT_STEPPER_TOTAL_ARGUMENTS = 1;


    const std::vector<macro_packages::simple_rule> cpp_steppers::get_pre_rules()
      {
        std::vector<macro_packages::simple_rule> package;

        const std::vector<replacement_rule_simple> rules =
          { BIND(replace_backg_stepper),   BIND(replace_pert_stepper)
          };

        const std::vector<std::string> names =
          { "MAKE_BACKG_STEPPER",          "MAKE_PERT_STEPPER"
          };

        const std::vector<unsigned int> args =
          { BACKG_STEPPER_TOTAL_ARGUMENTS, PERT_STEPPER_TOTAL_ARGUMENTS
          };

        assert(rules.size() == names.size());
        assert(rules.size() == args.size());

        for(int i = 0; i < rules.size(); ++i)
          {
            package.emplace_back(names[i], rules[i], args[i]);
          }

        return(package);
      }


    const std::vector<macro_packages::simple_rule> cpp_steppers::get_post_rules()
      {
        std::vector<macro_packages::simple_rule> package;

        return(package);
      }


    const std::vector<macro_packages::index_rule> cpp_steppers::get_index_rules()
      {
        std::vector<macro_packages::index_rule> package;

        return(package);
      }


    // *******************************************************************


    std::string cpp_steppers::replace_stepper(const struct stepper& s, std::string state_name)
      {
        std::ostringstream out;

        // note that we need a generic stepper which works with an arbitrary state type; see
        // http://headmyshoulder.github.io/odeint-v2/doc/boost_numeric_odeint/concepts/system.html
        // we can't use things like rosenbrock4 or the implicit euler methods which work only with boost matrices

        // exactly when the steppers call the observer functor depends which stepper is in use; see
        // http://headmyshoulder.github.io/odeint-v2/doc/boost_numeric_odeint/odeint_in_detail/integrate_functions.html

        // to summarize the discussion there:
        //  ** If stepper is a Stepper or Error Stepper dt is the step size used for integration.
        //     However, whenever a time point from the sequence is approached the step size dt will
        //     be reduced to obtain the state x(t) exactly at the time point.
        //  ** If stepper is a Controlled Stepper then dt is the initial step size. The actual step
        //     size is adjusted during integration according to error control.
        //     However, if a time point from the sequence is approached the step size is
        //     reduced to obtain the state x(t) exactly at the time point. [runge_kutta_fehlberg78]
        //  ** If stepper is a Dense Output Stepper then dt is the initial step size. The actual step
        //     size is adjusted during integration according to error control. Dense output is used
        //     to obtain the states x(t) at the time points from the sequence. [runge_kutta_dopri5, bulirsch_stoer]

        if(s.name == "runge_kutta_dopri5")
          {
            out << "boost::numeric::odeint::make_dense_output< boost::numeric::odeint::runge_kutta_dopri5< " << state_name << " > >(" << s.abserr << ", " << s.relerr << ")";
          }
        else if(s.name == "bulirsch_stoer_dense_out")
          {
            out << "boost::numeric::odeint::bulirsch_stoer_dense_out< " << state_name << " >(" << s.abserr << ", " << s.relerr << ")";
          }
        else if(s.name == "bulirsch_stoer")
	        {
						out << "boost::numeric::odeint::bulirsch_stoer< " << state_name << " >(" << s.abserr << ", " << s.relerr << ")";
	        }
        else if(s.name == "runge_kutta_fehlberg78")
          {
            out << "boost::numeric::odeint::make_controlled< boost::numeric::odeint::runge_kutta_fehlberg78< " << state_name << " > >(" << s.abserr << ", " << s.relerr << ")";
          }
        else
          {
            std::ostringstream msg;
            msg << ERROR_UNKNOWN_STEPPER << " '" << s.name << "'";
            throw macro_packages::rule_apply_fail(msg.str());
          }

        return(out.str());
      }


    // ********************************************************************************


    std::string cpp_steppers::replace_backg_stepper(const macro_argument_list& args)
      {
        const struct stepper& s = this->data_payload.get_background_stepper();
        std::string state_name = args[BACKG_STEPPER_STATE_ARGUMENT];

        return(this->replace_stepper(s, state_name));
      }


    std::string cpp_steppers::replace_pert_stepper(const macro_argument_list& args)
      {
        const struct stepper& s = this->data_payload.get_perturbations_stepper();
        std::string state_name = args[PERT_STEPPER_STATE_ARGUMENT];

        return(this->replace_stepper(s, state_name));
      }


  } // namespace cpp
