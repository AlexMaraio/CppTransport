//
// Created by David Seery on 05/12/2013.
// Copyright (c) 2013-15 University of Sussex. All rights reserved.
//


#ifndef CPPTRANSPORT_MACROS_CPP_STEPPERS_H
#define CPPTRANSPORT_MACROS_CPP_STEPPERS_H


#include "replacement_rule_package.h"
#include "stepper.h"


namespace cpp
  {

    constexpr unsigned int BACKG_STEPPER_STATE_ARGUMENT = 0;
    constexpr unsigned int BACKG_STEPPER_TOTAL_ARGUMENTS = 1;

    constexpr unsigned int PERT_STEPPER_STATE_ARGUMENT = 0;
    constexpr unsigned int PERT_STEPPER_TOTAL_ARGUMENTS = 1;


    class replace_backg_stepper : public ::macro_packages::replacement_rule_simple
      {

        // CONSTRUCTOR, DESTRUCTOR

      public:

        //! constructor
        replace_backg_stepper(std::string n, translator_data& p, language_printer& prn)
          : ::macro_packages::replacement_rule_simple(std::move(n), BACKG_STEPPER_TOTAL_ARGUMENTS),
            data_payload(p),
            printer(prn)
          {
          }

        //! destructor
        virtual ~replace_backg_stepper() = default;


        // INTERNAL API

      protected:

        //! evaluate
        virtual std::string evaluate(const macro_argument_list& args) override;


        // INTERNAL DATA

      private:

        //! data payload
        translator_data& data_payload;

        //! language printer
        language_printer& printer;

      };


    class replace_pert_stepper : public ::macro_packages::replacement_rule_simple
      {

        // CONSTRUCTOR, DESTRUCTOR

      public:

        //! constructor
        replace_pert_stepper(std::string n, translator_data& p, language_printer& prn)
          : ::macro_packages::replacement_rule_simple(std::move(n), PERT_STEPPER_TOTAL_ARGUMENTS),
            data_payload(p),
            printer(prn)
          {
          }

        //! destructor
        virtual ~replace_pert_stepper() = default;


        // INTERNAL API

      protected:

        //! evaluate
        virtual std::string evaluate(const macro_argument_list& args) override;


        // INTERNAL DATA

      private:

        //! data payload
        translator_data& data_payload;

        //! language printer
        language_printer& printer;

      };


    class cpp_steppers: public ::macro_packages::replacement_rule_package
      {

        // CONSTRUCTOR, DESTRUCTOR

      public:

        //! constructor
        cpp_steppers(tensor_factory& f, cse& cw, lambda_manager& lm, translator_data& p, language_printer& prn);

        //! destructor is default
        virtual ~cpp_steppers() = default;

      };

  } // namespace cpp


#endif //CPPTRANSPORT_MACROS_CPP_STEPPERS_H
