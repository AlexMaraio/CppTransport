//
// DO NOT EDIT: GENERATED AUTOMATICALLY BY $$__TOOL $$__VERSION
//
// '$$__HEADER' generated from '$$__SOURCE'
// processed on $$__DATE

#ifndef $$__GUARD   // avoid multiple inclusion
#define $$__GUARD

#include <math.h>

#include "transport/transport.h"

namespace transport
  {
      static std::vector<std::string> $$__MODEL_field_names = $$__FIELD_NAME_LIST;
      static std::vector<std::string> $$__MODEL_latex_names = $$__LATEX_NAME_LIST;
      static std::vector<std::string> $$__MODEL_param_names = $$__PARAM_NAME_LIST;
      static std::vector<std::string> $$__MODEL_platx_names = $$__PLATX_NAME_LIST;
      static std::vector<std::string> $$__MODEL_state_names = $$__STATE_NAME_LIST;

      // set up a state type for GPU integration
      typedef vex::multivector<double, 2*$$__NUMBER_FIELDS + (2*$$__NUMBER_FIELDS)*(2*$$__NUMBER_FIELDS)> twopf_state;

      template <typename number>
      class $$__MODEL : public canonical_model<number>
        {
          public:
            $$__MODEL(number Mp, const std::vector<number>& ps);
            ~$$__MODEL();

            // Functions inherited from canonical_model
            number
              V(std::vector<number> fields);

            // Integration of the model
            // ========================

            // Integrate the background on the CPU
            transport::background<number>
              background(const std::vector<number>& ics, const std::vector<double>& times);

            // Integrate background and 2-point function on the GPU

            transport::twopf<number>
              twopf(vex::Context& ctx, const std::vector<double>& ks, const std::vector<number>& ics, const std::vector<double>& times);

          protected:
            void
              fix_initial_conditions(const std::vector<number>& ics, std::vector<number>& rics);
            void
              write_initial_conditions(const std::vector<number>& rics, std::ostream& stream);
        };


      // integration - background functor
      template <typename number>
      class $$__MODEL_background_functor
        {
          public:
            $$__MODEL_background_functor(const std::vector<number>& p, number Mp) : parameters(p), M_Planck(Mp) {}
            void operator()(const std::vector<number>& x, std::vector<number>& dxdt, double t);

          private:
            const number              M_Planck;
            const std::vector<number> parameters;
        };


      // integration - observer object for background only
      template <typename number>
      class $$__MODEL_background_observer
        {
          public:
            $$__MODEL_background_observer(std::vector<double>& s, std::vector< std::vector<number> >& h) : slices(s), history(h) {}
            void operator()(const std::vector<number>& x, double t);

          private:
            std::vector<double>&                slices;
            std::vector< std::vector<number> >& history;
        };


      // integration - 2pf functor
      template <typename number>
      class $$__MODEL_twopf_functor
        {
          public:
            $$__MODEL_twopf_functor(const vex::vector<number>& p, const vex::vector<number>& Mp, const vex::vector<double>& ks)
            : parameters(p), M_Planck(Mp), k_list(ks) {}
            void operator()(const twopf_state& x, twopf_state& dxdt, double t);

          private:
            const vex::vector<number>& M_Planck;
            const vex::vector<number>& parameters;
            const vex::vector<double>& k_list;
        };


      // integration - observer object for 2pf
      template <typename number>
      class $$__MODEL_twopf_observer
        {
          public:
            $$__MODEL_twopf_observer(std::vector<double>& s,
              std::vector< std::vector<number> >& bh,
              std::vector< std::vector< std::vector<number> > >& tpfh) : slices(s), background_history(bh), twopf_history(tpfh) {}
            void operator()(const twopf_state& x, double t);

          private:
            std::vector<double>&                               slices;
            std::vector< std::vector<number> >&                background_history;
            std::vector< std::vector< std::vector<number> > >& twopf_history;
        };


      // IMPLEMENTATION -- CLASS $$__MODEL
      // ==============


      template <typename number>
      $$__MODEL<number>::$$__MODEL(number Mp, const std::vector<number>& ps)
        : canonical_model<number>("$$__NAME", "$$__AUTHOR", "$$__TAG", Mp,
                                  $$__NUMBER_FIELDS, $$__NUMBER_PARAMS,
                                  $$__MODEL_field_names, $$__MODEL_latex_names,
                                  $$__MODEL_param_names, $$__MODEL_platx_names, ps)
        {
          return;
        }


      template <typename number>
      $$__MODEL<number>::~$$__MODEL()
        {
          return;
        }


      template <typename number>
      number $$__MODEL<number>::V(std::vector<number> fields)
        {
          assert(fields.size() == $$__NUMBER_FIELDS);
          $$__SET_PARAMETERS{number, this->parameters}
          $$__SET_FIELDS{number, fields}

          number rval = $$__V;

          return(rval);
        }


      // Integrate the background - on the CPU


      template <typename number>
      transport::background<number> $$__MODEL<number>::background(const std::vector<number>& ics, const std::vector<double>& times)
        {
          using namespace boost::numeric::odeint;

          // validate initial conditions (or set up ics for momenta if necessary)
          std::vector<number> x = ics;
          fix_initial_conditions(ics, x);
          write_initial_conditions(x, std::cout);

          // set up an observer which writes to this history vector
          // I'd prefer to encapsulate the history within the observer object, but for some reason
          // that doesn't seem to work (maybe related to the way odeint uses templates?)
          std::vector<double>                slices;
          std::vector< std::vector<number> > history;
          $$__MODEL_background_observer<number> obs(slices, history);

          // set up a functor to evolve this system
          $$__MODEL_background_functor<number>  system(this->parameters, this->M_Planck);

          integrate_times( make_dense_output< $$__BACKG_STEPPER< std::vector<number> > >($$__BACKG_ABS_ERR, $$__BACKG_REL_ERR),
            system, x, times.begin(), times.end(), $$__BACKG_STEP_SIZE, obs);

          transport::background<number> backg(2*$$__NUMBER_FIELDS, $$__MODEL_state_names, slices, history);

          return(backg);
        }


      // Integrate the 2pf - on the GPU


      template <typename number>
      transport::twopf<number> $$__MODEL<number>::twopf(vex::Context& ctx, const std::vector<double>& ks,
        const std::vector<number>& ics, const std::vector<double>& times)
        {
          using namespace boost::numeric::odeint;

          // validate initial conditions (or set up ics for momenta if necessary)
          std::vector<number> hst_x = ics;
          fix_initial_conditions(ics, hst_x);
          write_initial_conditions(hst_x, std::cout);

          // initialize device copy of Planck masses
          std::vector<number> hst_M_Planck(1, this->M_Planck);
          vex::vector<number> dev_M_Planck(ctx, hst_M_Planck);

          // initialize device copy of parameter vector
          vex::vector<number> dev_params(ctx, this->parameters);

          // initialize device copy of k list
          vex::vector<double> dev_ks(ctx, ks);

          // set up a functor to evolve this system
          $$__MODEL_twopf_functor<number> system(dev_params, dev_M_Planck, dev_ks);

          // work out how much space we need to store background + 2pf
          const unsigned int state_size = 2*$$__NUMBER_FIELDS + (2*$$__NUMBER_FIELDS)*(2*$$__NUMBER_FIELDS);
          const unsigned int num_ks     = ks.size();

          twopf_state dev_x(ctx, num_ks);

          // initialize the initial state
          for(int i = 0; i < 2*$$__NUMBER_FIELDS; i++)
            {
              dev_x(i) = hst_x[i];
            }
          for(int i = 2*$$__NUMBER_FIELDS; i < state_size; i++)
            {
              dev_x(i) = 0;
            }

          // set up functor to observe the integration
          std::vector<double>                               slices;
          std::vector< std::vector<number> >                background_history;
          std::vector< std::vector< std::vector<number> > > twopf_history;
          $$__MODEL_twopf_observer<number> obs(slices, background_history, twopf_history);

          integrate_times( make_dense_output< $$__PERT_STEPPER< std::vector<number> > >($$__PERT_ABS_ERR, $$__PERT_REL_ERR),
            system, dev_x, times.begin(), times.end(), $$__PERT_STEP_SIZE, obs);

          transport::twopf<number> tpf(2*$$__NUMBER_FIELDS, $$__MODEL_state_names, slices, background_history, twopf_history);

          return(tpf);
        }


      // Handle initial conditions


      template <typename number>
      void $$__MODEL<number>::fix_initial_conditions(const std::vector<number>& ics, std::vector<number>& rics)
        {
          if(ics.size() == this->N_fields)  // initial conditions for momenta *were not* supplied -- need to compute them
            {
              // supply the missing initial conditions using a slow-roll approximation
              $$__SET_PARAMETERS{number, this->parameters}
              $$__SET_FIELDS{number,ics}
              $$__SET_SR_VELOCITY{number}
              rics.push_back($$__SR_VELOCITY[a]);
            }
          else if(ics.size() == 2*this->N_fields)  // initial conditions for momenta *were* supplied
            {
              // need do nothing
            }
          else
            {
              std::cerr << __CPP_TRANSPORT_WRONG_ICS_A << ics.size()
                        << __CPP_TRANSPORT_WRONG_ICS_B << $$__NUMBER_FIELDS
                        << __CPP_TRANSPORT_WRONG_ICS_C << 2*$$__NUMBER_FIELDS << ")" << std::endl;
              exit(EXIT_FAILURE);
            }
        }


      template <typename number>
      void $$__MODEL<number>::write_initial_conditions(const std::vector<number>& ics, std::ostream& stream)
        {
          stream << __CPP_TRANSPORT_SOLVING_ICS_MESSAGE << std::endl;
          stream << __CPP_TRANSPORT_STEPPER_MESSAGE    << " '"  << "$$__BACKG_STEPPER"
                 << "', " << __CPP_TRANSPORT_ABS_ERR   << " = " << $$__BACKG_ABS_ERR
                 << ", "  << __CPP_TRANSPORT_REL_ERR   << " = " << $$__BACKG_REL_ERR
                 << ", "  << __CPP_TRANSPORT_STEP_SIZE << " = " << $$__BACKG_STEP_SIZE << std::endl;

          assert(ics.size() == 2*this->N_fields);

          for(int i = 0; i < this->N_fields; i++)
            {
              stream << "  " << this->field_names[i] << " = " << ics[i]
                     << "; d(" << this->field_names[i] << ")/dN = " << ics[this->N_fields+i] << std::endl;
            }

          stream << std::endl;
        }


      // IMPLEMENTATION - FUNCTOR FOR BACKGROUND INTEGRATION


      template <typename number>
      void $$__MODEL_background_functor<number>::operator()(const std::vector<number>& x, std::vector<number>& dxdt, double t)
        {
          $$__SET_PARAMETERS{number, this->parameters}
          $$__SET_ALL{number, x}
          $$__SET_U1_TENSOR{number}
          dxdt[$$__A] = $$__U1_TENSOR[A];
        }


      // IMPLEMENTATION - FUNCTOR FOR BACKGROUND OBSERVATION


      template <typename number>
      void $$__MODEL_background_observer<number>::operator()(const std::vector<number>& x, double t)
        {
          this->slices.push_back(t);
          this->history.push_back(x);
        }


      // IMPLEMENTATION - FUNCTOR FOR 2PF INTEGRATION


      template <typename number>
      void $$__MODEL_twopf_functor<number>::operator()(const twopf_state& x, twopf_state& dxdt, double t)
        {
          $$__SET_PARAMETERS{number, this->parameters}
          $$__SET_ALL{number, x}
          $$__SET_U1_TENSOR{number}
          dxdt($$__A) = $$__U1_TENSOR[A];
        }


      // IMPLEMENTATION - FUNCTOR FOR 2PF OBSERVATION


      template <typename number>
      void $$__MODEL_twopf_observer<number>::operator()(const twopf_state& x, double t)
        {
          const unsigned int background_state_size = 2*$$__NUMBER_FIELDS;
          const unsigned int twopf_state_size      = (2*$$__NUMBER_FIELDS)*(2*$$__NUMBER_FIELDS);

          this->slices.push_back(t);

          // allocate storage for state
          std::vector<number>                hst_background_state(background_state_size);
          std::vector< std::vector<number> > hst_state(twopf_state_size);

          // copy device state into local storage, and then push it into the history
          // (** TODO how slow is this?)

          // first, background
          for(int i = 0; i < background_state_size; i++)
            {
              hst_background_state.push_back(x(i)[0]);
            }
          this->background_history.push_back(hst_background_state);

          // then, two pf
          for(int i = 0; i < twopf_state_size; i++)
            {
              vex::copy(x(background_state_size + i), hst_state[i]);
            }

          this->twopf_history.push_back(hst_state);
        }


  }   // namespace transport


#endif  // $$__GUARD