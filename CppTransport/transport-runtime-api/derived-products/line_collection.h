//
// Created by David Seery on 25/05/2014.
// Copyright (c) 2014 University of Sussex. All rights reserved.
//


#ifndef __line_collection_H_
#define __line_collection_H_


#include <iostream>
#include <sstream>
#include <string>

#include "transport-runtime-api/derived-products/derived_product.h"
#include "transport-runtime-api/derived-products/derived-content/derived_line.h"
#include "transport-runtime-api/derived-products/derived-content/derived_line_helper.h"
#include "transport-runtime-api/derived-products/data_line.h"

#include "transport-runtime-api/messages.h"
#include "transport-runtime-api/exceptions.h"

#include "boost/filesystem/operations.hpp"


#define __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_MAX_VALUE_TYPES "max-value-types"
#define __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LOGX            "log-x"
#define __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LOGY            "log-y"
#define __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_ABSY            "abs-y"
#define __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LATEX           "latex"

#define __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LINE_ARRAY      "line-array"


namespace transport
	{

		namespace derived_data
			{

				//! A line-collection is a specialization of a derived product that produces
				//! derived data from a collection of 2d lines

				template <typename number>
				class line_collection: public derived_product<number>
					{

				  public:

				    class output_value
					    {

				      public:

				        output_value(double v)
					        : exists(true), value(v)
					        {
					        }

				        output_value()
					        : exists(false)
					        {
					        }

				        ~output_value() = default;


				        // FORMAT VALUE

				      public:

				        void format_python(std::ostream& out) const;


				        // INTERNAL DATA

				      private:

				        //! does this value exist?
				        bool exists;

				        //! numerical value, if exists
				        double value;
					    };


				    class output_line
					    {

				      public:

				        output_line(const std::string& l)
					        : label(l)
					        {
					        }

				        ~output_line() = default;


				        // INTERFACE

				      public:

				        //! Add a value at the back
				        void push_back(const output_value& v) { this->values.push_back(v); }
				        //! Add a value at the front
				        void push_front(const output_value& v) { this->values.push_front(v); }
				        //! Get values
				        const std::deque<output_value>& get_values() const { return(this->values); }

				        //! Get size
				        unsigned int size() const { return(this->values.size()); }

				        //! Get label
				        const std::string& get_label() const { return(this->label); }


				        // INTERNAL DATA

				      private:

				        //! this line's label
				        std::string label;

				        //! this line's data points
				        std::deque<output_value> values;
					    };


						// LINE_COLLECTION: CONSTRUCTOR, DESTRUCTOR

				  public:

						//! Basic user-facing constructor
						line_collection(const std::string& name, const boost::filesystem::path& filename, unsigned int mvt)
				      : max_value_types(mvt), derived_product<number>(name, filename)
							{
							}

						//! Override copy constructor to perform a deep copy on the derived_line<> objects we own
						line_collection(const line_collection<number>& obj);

						//! Deserialization constructor
						line_collection(const std::string& name, serialization_reader* reader, typename repository<number>::task_finder finder);

						virtual ~line_collection();


				    // LINE MANAGEMENT

				  public:

				    //! Add a line to the collection
				    void add_line(const derived_line<number>& line);

				  protected:

				    //! Merge axes and value data into a single series
				    void merge_lines(typename data_manager<number>::datapipe& pipe, const std::list< data_line<number> >& input, std::deque<double>& axis, std::vector<output_line>& data) const;

						//! Obtain output from our lines
				    void obtain_output(typename data_manager<number>::datapipe& pipe, const std::list<std::string>& tags, std::list< data_line<number> >& derived_lines) const;


						// GET AND SET BASIC LINE-HANDLING DATA

				    //! get logarithmic x-axis setting
				    bool get_log_x() const { return(this->log_x); }
				    //! set logarithmic x-axis setting
				    void set_log_x(bool g) { this->log_x = g; }

				    //! get logarithmic y-axis setting
				    bool get_log_y() const { return(this->log_y); }
				    //! set logarithmic y-axis setting
				    void set_log_y(bool g) { this->log_y = g; }

				    //! get abs-y-axis setting
				    bool get_abs_y() const { return(this->abs_y); }
				    //! set abs-y-axis setting
				    void set_abs_y(bool g) { this->abs_y = g; }

				    //! get default LaTeX labels setting
				    bool get_use_LaTeX() const { return(this->use_LaTeX); }
				    //! set default LaTeX labels setting
				    void set_use_LaTeX(bool g) { this->use_LaTeX = g; }


						// SERIALIZATION -- implements a 'serializable' interface

				  public:

						virtual void serialize(serialization_writer& writer) const override;


						// WRITE SELF TO A STANDARD STREAM

				  public:

						void write(std::ostream& out);


						// INTERNAL DATA

				  protected:

				    // PLOT DATA

				    //! List of data_line objects to be plotted on the graph.
				    std::list< derived_line<number>* > lines;


						// ADMIN

				    //! Maximum number of different value-types to allow
				    unsigned int max_value_types;


						// HANDLING DATA

						// LINE HANDLING ATTRIBUTES

				    //! logarithmic x-axis?
				    bool log_x;

				    //! logarithmic y-axis?
				    bool log_y;

				    //! take absolute value on y-axis? (mostly useful with log-y data)
				    bool abs_y;

				    //! use LaTeX default labels
				    bool use_LaTeX;

					};


				template <typename number>
				line_collection<number>::line_collection(const std::string& name, serialization_reader* reader, typename repository<number>::task_finder finder)
					: derived_product<number>(name, reader, finder)
					{
						// extract data from reader
						assert(reader != nullptr);

						// read in line management attributes
						reader->read_value(__CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_MAX_VALUE_TYPES, max_value_types);
				    reader->read_value(__CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LOGX, log_x);
				    reader->read_value(__CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LOGY, log_y);
				    reader->read_value(__CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_ABSY, abs_y);
				    reader->read_value(__CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LATEX, use_LaTeX);

						// read in line specifications
				    unsigned int num_lines = reader->start_array(__CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LINE_ARRAY);

				    lines.clear();
				    for(unsigned int i = 0; i < num_lines; i++)
					    {
				        reader->start_array_element();

				        derived_line<number>* data = derived_line_helper::deserialize<number>(reader, finder);
				        lines.push_back(data);

				        reader->end_array_element();
					    }

				    reader->end_element(__CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LINE_ARRAY);
					}


		    template <typename number>
		    line_collection<number>::line_collection(const line_collection<number>& obj)
			    : derived_product<number>(obj), max_value_types(obj.max_value_types),
		        log_x(obj.log_x), log_y(obj.log_y),
		        abs_y(obj.abs_y), use_LaTeX(obj.use_LaTeX)
			    {
		        lines.clear();

		        for(typename std::list< derived_line<number>* >::const_iterator t = obj.lines.begin(); t != obj.lines.end(); t++)
			        {
		            lines.push_back((*t)->clone());
			        }
			    }


		    template <typename number>
		    void line_collection<number>::add_line(const derived_line<number>& line)
			    {
				    // check that the axis-type used by this line is compatible with any existing lines
				    if(this->lines.size() > 0)
					    {
						    if(line.get_axis_type() != this->lines.front()->get_axis_type())
							    throw runtime_exception(runtime_exception::DERIVED_PRODUCT_ERROR, __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_AXIS_MISMATCH);
					    }

				    // check that adding this line won't introduce too many value types
		        typename std::list< typename derived_line<number>::value_type > value_list;
				    value_list.push_back(line.get_value_type());

				    for(typename std::list< derived_line<number>* >::iterator t = this->lines.begin(); t != this->lines.end(); t++)
					    {
						    typename derived_line<number>::value_type value = (*t)->get_value_type();
						    if(std::find(value_list.begin(), value_list.end(), value) == value_list.end())
							    {
								    value_list.push_back(value);
							    }
					    }

				    if(value_list.size() > this->max_value_types)
					    throw runtime_exception(runtime_exception::DERIVED_PRODUCT_ERROR, __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_TOO_MANY_VALUES);

		        this->lines.push_back(line.clone());
			    }


		    template <typename number>
		    line_collection<number>::~line_collection()
			    {
		        for(typename std::list< derived_line<number>* >::const_iterator t = this->lines.begin(); t != this->lines.end(); t++)
			        {
		            delete *t;
			        }
			    }


				template <typename number>
				void line_collection<number>::obtain_output(typename data_manager<number>::datapipe& pipe, const std::list<std::string>& tags, std::list< data_line<number> >& derived_lines) const
					{
				    for(typename std::list< derived_line<number>* >::const_iterator t = this->lines.begin(); t != this->lines.end(); t++)
					    {
				        (*t)->derive_lines(pipe, derived_lines, tags);
					    }
					}


		    template <typename number>
		    void line_collection<number>::merge_lines(typename data_manager<number>::datapipe& pipe,
		                                              const std::list< data_line<number> >& input, std::deque<double>& axis, std::vector<output_line>& output) const
			    {
		        // step through our plot lines, merging axis data and excluding any lines which are unplottable

		        // FIRST, build a list of plottable lines in 'data' (and work out whether we need to take the absolute value)
		        output.clear();

		        std::vector< std::vector<double> > axis_data;
		        std::vector< std::vector<number> > plot_data;
		        std::vector< bool >                data_absy;

		        for(typename std::list< data_line<number> >::const_iterator t = input.begin(); t != input.end(); t++)
			        {
		            const std::vector<number>& line_data = (*t).get_data_points();

		            bool need_abs_y = false;
		            bool nonzero_values = false;

		            if(this->log_y)
			            {
		                for(typename std::vector<number>::const_iterator u = line_data.begin(); (!need_abs_y || !nonzero_values) && u != line_data.end(); u++)
			                {
		                    if((*u) <= 0.0) need_abs_y = true;
		                    if((*u) > 0.0 || (*u) < 0.0) nonzero_values = true;
			                }

		                // warn if absolute values are needed, but suppress warning if the line won't plot - would be confusing
		                if(need_abs_y && !this->abs_y && nonzero_values)
			                BOOST_LOG_SEV(pipe.get_log(), data_manager<number>::normal) << ":: Warning: data line '" << (*t).get_non_LaTeX_label() << "' contains negative or zero values; plotting absolute values instead because of logarithmic y-axis";
			            }

		            // we can't plot the line if it is logarithmic but has no nonzero values, so check
		            if(!this->log_y || (this->log_y && nonzero_values))   // can plot the line
			            {
		                output.push_back(output_line(this->use_LaTeX ? (*t).get_LaTeX_label() : (*t).get_non_LaTeX_label()));
		                data_absy.push_back(this->abs_y || need_abs_y);

		                axis_data.push_back((*t).get_axis_points());
		                plot_data.push_back((*t).get_data_points());

		                // if we are logging the x-axis, check all points are strictly positive
		                bool ok = true;
		                if(this->log_x)
			                {
		                    for(std::vector<double>::const_iterator t = axis_data.back().begin(); ok && t != axis_data.back().end(); t++)
			                    {
		                        if((*t) <= 0.0) ok = false;
			                    }
			                }
		                if(!ok) throw runtime_exception(runtime_exception::DERIVED_PRODUCT_ERROR, __CPP_TRANSPORT_PRODUCT_LINE_PLOT2D_NEGATIVE_AXIS_POINT);
			            }
		            else    // can't plot the line
			            BOOST_LOG_SEV(pipe.get_log(), data_manager<number>::normal) << ":: Warning: data line '" << (*t).get_non_LaTeX_label() << "' contains no nonzero values and can't be plotted on a logarithmic axis -- skipping this line";
			        }

		        // SECOND work through each axis, populating the single merged axis
		        axis.clear();
		        bool finished = false;

		        while(!finished)
			        {
		            finished = true;

		            // any work left to do?
		            for(unsigned int i = 0; finished && i < output.size(); i++)
			            {
		                if(axis_data[i].size() > 0) finished = false;
			            }

		            if(!finished)
			            {
		                // find next point to add to merged x-axis (we work from the far right because std::vector can only pop from the end)
		                double next_axis_point = -DBL_MAX;
		                for(unsigned int i = 0; i < output.size(); i++)
			                {
		                    if(axis_data[i].size() > 0)
			                    {
		                        if(axis_data[i].back() > next_axis_point) next_axis_point = axis_data[i].back();
			                    }
			                }

		                if(next_axis_point != -DBL_MAX)
			                {
		                    // push point to merged axis
		                    axis.push_front(next_axis_point);

		                    // find data points on each line, if they exist, corresponding to this axis point
		                    for(unsigned int i = 0; i < output.size(); i++)
			                    {
		                        if(axis_data[i].size() > 0)
			                        {
		                            if(axis_data[i].back() == next_axis_point)   // yes, this line has a match
			                            {
		                                output[i].push_front(output_value(data_absy[i] ? fabs(plot_data[i].back()) : plot_data[i].back()));

		                                // remove point from this line
		                                axis_data[i].pop_back();
		                                plot_data[i].pop_back();
			                            }
		                            else
			                            {
		                                output[i].push_front(output_value());
			                            }
			                        }
		                        else  // no match, add an empty component
			                        {
		                            output[i].push_front(output_value());
			                        }
			                    }
			                }
		                else
			                {
		                    BOOST_LOG_SEV(pipe.get_log(), data_manager<number>::error) << ":: Error: failed to find new axis point to merge; giving up";
		                    finished = true;
			                }
			            }
			        }
			    }


				template <typename number>
				void line_collection<number>::serialize(serialization_writer& writer) const
					{
						this->write_value_node(writer, __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_MAX_VALUE_TYPES, this->max_value_types);
				    this->write_value_node(writer, __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LOGX, this->log_x);
				    this->write_value_node(writer, __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LOGY, this->log_y);
				    this->write_value_node(writer, __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_ABSY, this->abs_y);
				    this->write_value_node(writer, __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LATEX, this->use_LaTeX);

				    this->begin_array(writer, __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LINE_ARRAY, this->lines.size() == 0);
				    for(typename std::list< derived_line<number>* >::const_iterator t = this->lines.begin(); t != this->lines.end(); t++)
					    {
				        this->begin_node(writer, "arrayelt", false);    // node name ignored in array
				        (*t)->serialize(writer);
				        this->end_element(writer, "arrayelt");
					    }
				    this->end_element(writer, __CPP_TRANSPORT_NODE_PRODUCT_LINE_COLLECTION_LINE_ARRAY);

				    // call next serialization
				    this->derived_product<number>::serialize(writer);
					}


				template <typename number>
				void line_collection<number>::write(std::ostream& out)
					{
						// call next writer
						this->derived_product<number>::write(out);

				    unsigned int count = 0;

				    out << __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_LABEL_TITLE_A << " '" << this->get_name() << "', " << __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_LABEL_TITLE_B << std::endl;

				    this->wrapper.wrap_list_item(out, this->log_x, __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_LABEL_LOGX, count);
				    this->wrapper.wrap_list_item(out, this->log_y, __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_LABEL_LOGY, count);
				    this->wrapper.wrap_list_item(out, this->abs_y, __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_LABEL_ABSY, count);
				    this->wrapper.wrap_list_item(out, this->use_LaTeX, __CPP_TRANSPORT_PRODUCT_LINE_COLLECTION_LABEL_LATEX, count);

						this->wrapper.wrap_newline(out);

				    for(typename std::list< derived_line<number>* >::iterator t = this->lines.begin(); t != this->lines.end(); t++)
					    {
				        (*t)->write(out);
				        this->wrapper.wrap_newline(out);
					    }
					}


		    template <typename number>
		    void line_collection<number>::output_value::format_python(std::ostream& out) const
			    {
		        if(this->exists)
			        {
		            out << this->value;
			        }
		        else
			        {
		            out << "np.nan";
			        }
			    }

			}   // namespace derived_data

	}   // namespace transport

class line_collection
	{

	};


#endif //__line_collection_H_
