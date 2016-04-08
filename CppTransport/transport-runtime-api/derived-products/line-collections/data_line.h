//
// Created by David Seery on 21/05/2014.
// Copyright (c) 2014-2016 University of Sussex. All rights reserved.
//


#ifndef __data_line_H_
#define __data_line_H_

#include <vector>
#include <string>
#include <algorithm>

#include "transport-runtime-api/messages.h"
#include "transport-runtime-api/exceptions.h"

#include "transport-runtime-api/derived-products/line-collections/line_values.h"

#include "transport-runtime-api/utilities/spline1d.h"


namespace transport
  {

    namespace derived_data
      {

        //! A data_line is a data line suitable for use by a derived output.
        //! data_lines are generated by the different content generators, eg field_time_data etc.

        template <typename number>
        class data_line
          {

          public:

		        //! Construct a dataline object from a sequence of axis and data points
            data_line(const std::list<std::string>& g, axis_value at, value_type vt, const std::vector<double>& a, const std::vector<number>& d,
                      const std::string& Ll, const std::string& nLl,
                      slave_message_buffer& msg, bool spectral_index=false);

            //! Construct a dataline object from a sequence of axis and data points
            data_line(const std::string& g, axis_value at, value_type vt, const std::vector<double>& a, const std::vector<number>& d,
                      const std::string& Ll, const std::string& nLl,
                      slave_message_buffer& msg, bool spectral_index=false)
              : data_line(std::list<std::string>{g}, at, vt, a, d, Ll, nLl, msg, spectral_index)
              {
              }

            ~data_line() = default;


            // GET DATA

          public:

            //! Get number of sample points
            unsigned int size() const { return(this->data.size()); }

            //! Get LaTeX label
            const std::string& get_LaTeX_label() const { return(this->LaTeX_label); }

		        //! Get non-LaTeX label
		        const std::string& get_non_LaTeX_label() const { return(this->non_LaTeX_label); }

		        //! Get data points
		        const std::vector< std::pair<double, number> >& get_data_points() const { return(this->data); }

		        //! Get axis type
		        axis_class get_axis_value() const { return(this->x_type); }

		        //! Get value type
		        value_type get_value_type() const { return(this->y_type); }

            //! Get type of data - scattered or continuous?
		        data_line_type get_data_line_type() const { return(this->data_type); }

		        //! Set type of data
		        void set_data_line_type(data_line_type t) { this->data_type = t; }

            //! Get parent content groups
            const std::list<std::string>& get_parent_groups() const { return(this->groups); }


            // INTERNAL API

          protected:

            void zip(const std::vector<double>& a, const std::vector<number>& d, std::vector< std::pair<double, number> >& zipped);


            // INTERNAL DATA

          protected:

            // ENVIRONMENT

            //! message buffer
            slave_message_buffer& messages;


            // DATA LINE

            //! source content group
            std::list<std::string> groups;

		        //! axis type
		        const axis_value x_type;

		        //! value type
		        const value_type y_type;

		        //! vector of data
		        std::vector< std::pair<double, number> > data;

		        //! type of data - is it scattered or continuous?
		        data_line_type data_type;


            // LABELS

            //! non-LaTeX label
            const std::string LaTeX_label;

		        //! non-LaTeX label
						const std::string non_LaTeX_label;

          };


        template <typename number>
        data_line<number>::data_line(const std::list<std::string>& g, axis_value at, value_type vt,
                                     const std::vector<double>& a, const std::vector<number>& d,
                                     const std::string& Ll, const std::string& nLl,
                                     slave_message_buffer& msg, bool spectral_index)
	        : groups(g),
            x_type(at),
	          y_type(spectral_index ? value_type::spectral_index_value : vt),
	          LaTeX_label(Ll),
	          non_LaTeX_label(nLl),
            data_type(data_line_type::continuous_data),
            messages(msg)
          {
            if(a.size() != d.size())
              {
                throw runtime_exception(exception_type::RUNTIME_ERROR, CPPTRANSPORT_PRODUCT_DATALINE_AXIS_MISMATCH);
              }

            if(!spectral_index)
              {
                this->zip(a, d, data);
              }
            else
              {
                std::vector< std::pair<double, number> > zipped;
                this->zip(a, d, zipped);

                std::vector<double> new_axis(a.size());
                std::vector<number> new_data(d.size());

                for(unsigned int i = 0; i < zipped.size(); ++i)
                  {
                    new_axis[i] = zipped[i].first;
                    new_data[i] = zipped[i].second;
                  }

		            try
			            {
		                spline1d<number> spline(new_axis, new_data);

		                // compute logarithmic derivative at each axis point
		                for(unsigned int i = 0; i < new_axis.size(); ++i)
			                {
		                    number value = spline.eval_diff(new_axis[i]) * (new_axis[i]/new_data[i]);
		                    new_data[i] = value;
			                }
		                this->zip(new_axis, new_data, data);
			            }
		            catch(runtime_exception& xe)
			            {
		                if(xe.get_exception_code() == exception_type::SPLINE_ERROR)
			                {
                        std::ostringstream msg;

                        msg << CPPTRANSPORT_PRODUCT_CANT_BUILD_SPLINE_A << " '" << nLl << "' "
                            << CPPTRANSPORT_PRODUCT_CANT_BUILD_SPLINE_B;
                        this->messages.push_back(msg.str());
//		                    std::cout << "** SPLINE ERROR: axis data" << '\n';
//				                for(unsigned int i = 0; i < a.size(); ++i)
//					                {
//				                    std::cout << i << ". x=" << a[i] << ", y=" << d[i] << '\n';
//					                }
//		                    std::cout << "** LaTeX label = " << Ll << ", non-LaTeX label = " << nLl << '\n';

				                this->zip(a, d, data);
			                }
		                else throw xe;
			            }
              }
          }


        namespace data_line_impl
          {

            template <typename number>
            struct AxisSorter
              {
                bool operator()(const std::pair<double, number>& a, const std::pair<double, number>& b)
                  {
                    return (a.first < b.first);
                  }
              };

          }   // namespace data_line_impl


        template <typename number>
        void data_line<number>::zip(const std::vector<double>& a, const std::vector<number>& d, std::vector< std::pair<double, number> >& zipped)
          {
            zipped.clear();
            zipped.reserve(a.size());

            // push data points onto the axis
            for(unsigned int i = 0; i < a.size(); ++i)
	            {
                zipped.push_back(std::make_pair(a[i], d[i]));
	            }

            // now sort axis into ascending order
            std::sort(zipped.begin(), zipped.end(), data_line_impl::AxisSorter<number>());
	        }


	    }   // namespace derived_data

  }   // namespace transport


#endif //__data_line_H_
