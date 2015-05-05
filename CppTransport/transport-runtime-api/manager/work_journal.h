//
// Created by David Seery on 03/04/15.
// Copyright (c) 2015 University of Sussex. All rights reserved.
//


#ifndef __work_journal_H_
#define __work_journal_H_


#include <list>

#include "transport-runtime-api/utilities/python_finder.h"

#include "transport-runtime-api/defaults.h"
#include "transport-runtime-api/messages.h"
#include "transport-runtime-api/exceptions.h"

#include "boost/date_time.hpp"
#include "boost/filesystem/operations.hpp"


// set default minimum time interval for instruments to be 1 second
#define __CPP_TRANSPORT_JOURNAL_MINIMUM_TIMESPAN (1)


namespace transport
	{

		//! Policy object representing information about the environment while formatting bars
		class Gantt_environment
			{

				// CONSTRUCTOR, DESTRUCTOR

		  public:

				//! constructor
				Gantt_environment(std::string ao, std::string d_n, std::string dt_n, std::string du_n, double h)
		      : axes_object_str(ao),
		        date_namespace_str(d_n),
		        dt_namespace_str(dt_n),
		        dateutil_namespace_str(du_n),
						height_val(h)
					{
					}


				// INTERFACE

		  public:

				const std::string& axes_object() const { return(this->axes_object_str); }

				const std::string& date_namespace() const { return(this->date_namespace_str); }

				const std::string& dt_namespace() const { return(this->dt_namespace_str); }

				const std::string& dateutil_namespace() const { return(this->dateutil_namespace_str); }

				double height() const { return(this->height_val); }


				// INTERNAL DATA

		  protected:

				//! name of axes object
				std::string axes_object_str;

				//! name of date namespace
				std::string date_namespace_str;

				//! name of dt namespace
				std::string dt_namespace_str;

				//! name of dateutil namespace
				std::string dateutil_namespace_str;

				//! height of bars
				double height_val;

			};

		//! Gantt_bar represents a time duration on a Gantt chart
		class Gantt_bar
			{

				// CONSTRUCTOR, DESTRUCTOR

		  public:

				//! constructor
				Gantt_bar(boost::posix_time::ptime b, boost::posix_time::ptime e, std::string c)
					: begin_time(b),
		        end_time(e),
		        colour(c),
						alpha(0.6)
					{
					}


				// INTERFACE

		  public:

				//! Produce a std::string representing Python code to format the Gantt chart bar
				std::string format(unsigned int unique_id, double y, Gantt_environment& env) const;

		  private:

				//! colour of this bar
				std::string colour;

				//! start time
				boost::posix_time::ptime begin_time;

				//! end time
				boost::posix_time::ptime end_time;

		    //! alpha channel
				double alpha;

			};


    std::string Gantt_bar::format(unsigned int unique_id, double y, Gantt_environment& env) const
	    {
        std::ostringstream begin_label;
        std::ostringstream end_label;
        std::ostringstream insn;

		    begin_label << "time_begin" << unique_id;
		    end_label << "time_end" << unique_id;

		    insn << begin_label.str() << " = " << env.date_namespace() << ".date2num(" << env.dateutil_namespace() << ".parse('" << this->begin_time << "'))" << std::endl;
		    insn << end_label.str()   << " = " << env.date_namespace() << ".date2num(" << env.dateutil_namespace() << ".parse('" << this->end_time << "'))" << std::endl;

		    double bar_height = 0.8*env.height();
		    double bar_pad    = 0.1*env.height();

        insn << env.axes_object() << ".barh(" << y+bar_pad << ", " << end_label.str() << "-" << begin_label.str()
	        << ", height=" << bar_height << ", left=" << begin_label.str() << ", color='" << this->colour << "', alpha=" << this->alpha << ", align='edge', linewidth=0)";

		    return(insn.str());
	    }


		//! Gantt_milestone represents a time on a Gantt chart
		class Gantt_milestone
			{

				// CONSTRUCTOR, DESTRUCTOR

		  public:

				//! constructor
				Gantt_milestone(boost::posix_time::ptime m, std::string c)
		      : milestone_time(m),
						colour(c)
					{
					}


				// INTERFACE

		  public:

				//! Produce a std::string representing Python code to format the milestone
				std::string format(unsigned int unique_id, double y, Gantt_environment& env) const;

		  private:

				//! colour of this milestone
				std::string colour;

				//! milestone time
				boost::posix_time::ptime milestone_time;

			};


		std::string Gantt_milestone::format(unsigned int unique_id, double y, Gantt_environment& env) const
			{
		    std::ostringstream time_label;
		    std::ostringstream insn;

				time_label << "milestone" << unique_id;

				insn << time_label.str() << " = " << env.date_namespace() << ".date2num(" << env.dateutil_namespace() << ".parse('" << this->milestone_time << "'))" << std::endl;

				double bar_height = 0.8*env.height();
				double size       = bar_height/3.0;
				double bar_pad    = 0.1*env.height();

				insn << env.axes_object() << ".plot([" << time_label.str() << "], [" << y+bar_pad+bar_height/2.0 << "], marker='D')";

				return(insn.str());
			}


		//! work_events record events
		class work_event
			{

				// CONSTRUCTOR, DESTRUCTOR

		  public:

				//! Create a work event for a given worker
				work_event();

				//! Create a work event for a given number, and a given timestamp and id
				work_event(boost::posix_time::ptime& t, unsigned int i);

				virtual ~work_event() = default;


				// INTERFACE

		  public:

				//! Is this a master-record object?
				virtual bool is_master()                 const = 0;

				//! Get timestamp
				boost::posix_time::ptime get_timestamp() const { return(this->timestamp); }

				//! Set id
				void set_id(unsigned int i) { this->id = i; }

				//! Get id
				unsigned int get_id() const { return(this->id); }


				// CLONE

		  public:

				//! Clone
				virtual work_event* clone() const = 0;


				// PRIVATE DATA

		  private:

				//! cache time this work event was journaled
				boost::posix_time::ptime timestamp;

				//! internal ID number (interpretation depends on concrete class)
				unsigned int id;

			};


		work_event::work_event()
			: timestamp(boost::posix_time::second_clock::universal_time()),
				id(0)
			{
			}


		work_event::work_event(boost::posix_time::ptime& t, unsigned int i)
			: timestamp(t),
				id(i)
			{
			}


		//! work events, specialized to master events
		class master_work_event: public work_event
			{

		  public:

				typedef enum
					{
				    aggregate_begin, aggregate_end,
						MPI_begin, MPI_end,
            database_begin, database_end
					} event_type;

				// CONSTRUCTOR, DESTRUCTOR

		  public:

				master_work_event(event_type t);

				master_work_event(event_type t, boost::posix_time::ptime ts, unsigned int id=0);

				virtual ~master_work_event() = default;


				// INTERFACE

		  public:

				virtual bool is_master() const override { return(true); }

				event_type   get_type()  const { return(this->type); }


		    // CLONE

		  public:

		    //! Clone
		    virtual master_work_event* clone() const override { return new master_work_event(dynamic_cast< const master_work_event& >(*this)); }


				// PRIVATE DATA

		  private:

				//! cache event type
				event_type type;

			};


		master_work_event::master_work_event(typename master_work_event::event_type t)
			: work_event(),
			  type(t)
			{
			}


		master_work_event::master_work_event(typename master_work_event::event_type t, boost::posix_time::ptime ts, unsigned int id)
			: work_event(ts, id),
				type(t)
			{
			}


		inline bool is_duration_delimiter(master_work_event::event_type type)
			{
				if(type == master_work_event::aggregate_begin
					|| type == master_work_event::aggregate_end
					|| type == master_work_event::MPI_begin
					|| type == master_work_event::MPI_end
					|| type == master_work_event::database_begin
					|| type == master_work_event::database_end) return(true);

				return(false);
			}


		inline bool is_milestone(master_work_event::event_type type)
			{
				return(false);
			}


		//! work events, specialized to slave events
		class slave_work_event: public work_event
			{

		  public:

				typedef enum
					{
				    begin_twopf_assignment, end_twopf_assignment,
						begin_threepf_assignment, end_threepf_assignment,
						begin_zeta_twopf_assignment, end_zeta_twopf_assignment,
						begin_zeta_threepf_assignment, end_zeta_threepf_assignment,
						begin_fNL_assignment, end_fNL_assignment,
						begin_output_assignment, end_output_assignment,
						integration_aggregation, postintegration_aggregation, derived_content_aggregation
					} event_type;

				// CONSTRUCTOR, DESTRUCTOR

		  public:

				slave_work_event(unsigned int w, event_type t);

				slave_work_event(unsigned int w, event_type t, boost::posix_time::ptime ts, unsigned int id=0);

				virtual ~slave_work_event() = default;


				// INTERFACE

		    virtual bool is_master() const override { return(false); }

				//! return worker number
				unsigned int get_worker_number() const { return(this->worker_number); }

		    event_type   get_type()          const { return(this->type); }


		    // CLONE

		  public:

		    //! Clone
		    virtual slave_work_event* clone() const override { return new slave_work_event(dynamic_cast< const slave_work_event& >(*this)); }


				// PRIVATE DATA

		  private:

				//! cache event type
				event_type type;

		    //! cache worker number
		    unsigned int worker_number;

			};


		inline bool is_duration_delimiter(slave_work_event::event_type type)
			{
				if(type == slave_work_event::begin_twopf_assignment || type == slave_work_event::end_twopf_assignment) return(true);
		    if(type == slave_work_event::begin_threepf_assignment || type == slave_work_event::end_threepf_assignment) return(true);
		    if(type == slave_work_event::begin_zeta_twopf_assignment || type == slave_work_event::end_zeta_twopf_assignment) return(true);
		    if(type == slave_work_event::begin_zeta_threepf_assignment || type == slave_work_event::end_zeta_threepf_assignment) return(true);
		    if(type == slave_work_event::begin_fNL_assignment || type == slave_work_event::end_fNL_assignment) return(true);
		    if(type == slave_work_event::begin_output_assignment || type == slave_work_event::end_output_assignment) return(true);

				return(false);
			}


		inline bool is_milestone(slave_work_event::event_type type)
			{
				if(type == slave_work_event::integration_aggregation
					|| type == slave_work_event::postintegration_aggregation
					|| type == slave_work_event::derived_content_aggregation) return(true);

				return(false);
			}


		slave_work_event::slave_work_event(unsigned int w, typename slave_work_event::event_type t)
			: work_event(),
				type(t),
				worker_number(w)
			{
			}


    slave_work_event::slave_work_event(unsigned int w, typename slave_work_event::event_type t, boost::posix_time::ptime ts, unsigned int id)
	    : work_event(ts, id),
	      type(t),
	      worker_number(w)
	    {
	    }


    //! a work journal records events in the timeline of a process,
    //! at a worker-level granularity.
		class work_journal
			{

				// CONSTRUCTOR, DESTRUCTOR

		  public:

				//! set up a work journal for N_workers workers (plus the master process)
				work_journal(unsigned int N);

				//! destructor
				~work_journal();


				// INTERFACE -- JOURNAL ENTRIES

				//! push item to the journal

		  public:

				//! add a work event to the journal
				void add_entry(const work_event& w);


				// INTERFACE -- ANALYSIS

		  public:

				//! construct a Gantt chart from the journal entries
				void make_gantt_chart(const std::string& filename);

		  protected:

				//! bin journal entries into Gantt bars
				void bin_bars(std::list<std::list<Gantt_bar> >& list);

				//! bin journal entries for master events into Gantt bars
				void bin_master_bars(std::list<std::list<Gantt_bar> >& list);

				//! bin journal entries for slave events into Gantt bars
				void bin_slave_bars(std::list<std::list<Gantt_bar> >& list);

				//! bin journal entries into Gantt milestones
				void bin_milestones(std::list<std::list<Gantt_milestone> >& list);

				//! bin journal entries for master events into Gantt milestones
				void bin_master_milestones(std::list<std::list<Gantt_milestone> >& list);

				//! bin journal entries for slave events into Gantt milestones
				void bin_slave_milestones(std::list<std::list<Gantt_milestone> >& list);

				// PRIVATE DATA

		  private:

				//! cache number of workers
				unsigned int N_workers;

				//! journal
				std::list< work_event* > journal;

				//! Path to Python executable
				boost::filesystem::path python_path;

			};


		work_journal::work_journal(unsigned int N)
			: N_workers(N)
			{
				this->python_path = find_python();
			}


		work_journal::~work_journal()
			{
				for(std::list< work_event* >::iterator t = this->journal.begin(); t != this->journal.end(); ++t)
					{
						delete *t;
					}
			}


		void work_journal::add_entry(const work_event& w)
			{
				this->journal.push_back(w.clone());
			}


		void work_journal::bin_bars(std::list<std::list<Gantt_bar> >& list)
			{
		    list.clear();

		    this->bin_master_bars(list);
		    this->bin_slave_bars(list);
			}


		namespace work_journal_impl
			{

		    class WorkItemSorter
			    {
		      public:
		        bool operator()(const work_event* A, const work_event* B)
			        {
		            return(A->get_timestamp() < B->get_timestamp());
			        }
			    };

			}


		void work_journal::bin_master_bars(std::list<std::list<Gantt_bar> >& list)
			{
				// strip out master duration-bracketing events and sort them in order
		    std::list< master_work_event* > master_events;
				for(std::list< work_event* >::iterator t = this->journal.begin(); t != this->journal.end(); ++t)
					{
						if((*t)->is_master())
							{
								master_work_event* event = dynamic_cast< master_work_event* >(*t);
								if(is_duration_delimiter(event->get_type())) master_events.push_back(event);
							}
					}
				master_events.sort(work_journal_impl::WorkItemSorter());

				list.resize(list.size()+1);
		    std::list< std::list<Gantt_bar> >::iterator current_bin = --list.end();

				// work through master events, pairing them up
				for(std::list< master_work_event* >::const_iterator t = master_events.begin(); t != master_events.end(); ++t)
					{
						switch((*t)->get_type())
							{
						    case master_work_event::aggregate_begin:
							    {
						        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
						        t++;
						        if(t == master_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_AGGREGATE_TOO_FEW);
						        if((*t)->get_type() != master_work_event::aggregate_end) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_AGGREGATE_END_MISSING);
						        current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "black"));
						        break;
							    }

						    case master_work_event::MPI_begin:
							    {
						        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
								    t++;
								    if(t == master_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_MPI_TOO_FEW);
								    if((*t)->get_type() != master_work_event::MPI_end) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_MPI_END_MISSING);
								    current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "darkgoldenrod"));
								    break;
							    }

                case master_work_event::database_begin:
                  {
                    boost::posix_time::ptime begin_time = (*t)->get_timestamp();
                    t++;
                    if(t == master_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_DATABASE_TOO_FEW);
                    if((*t)->get_type() != master_work_event::database_end) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_DATABASE_END_MISSING);
                    current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "aquamarine"));
                    break;
                  }

						    default:
							    assert(false);
							    throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_UNEXPECTED_EVENT);
							};
					}
			}


		void work_journal::bin_slave_bars(std::list<std::list<Gantt_bar> >& list)
			{
				// strip out events for each worker
		    std::list< slave_work_event* > worker_events;
				for(unsigned int i = 0; i < this->N_workers; ++i)
					{
				    worker_events.clear();
						for(std::list< work_event* >::iterator t = this->journal.begin(); t != this->journal.end(); ++t)
							{
								if(!(*t)->is_master())
									{
										slave_work_event* event = dynamic_cast< slave_work_event* >(*t);
								    if(event->get_worker_number() == i && is_duration_delimiter(event->get_type())) worker_events.push_back(event);
									}
							}
						worker_events.sort(work_journal_impl::WorkItemSorter());

				    list.resize(list.size()+1);
				    std::list< std::list<Gantt_bar> >::iterator current_bin = --list.end();

						// work through events, pairing them up
						for(std::list< slave_work_event* >::const_iterator t = worker_events.begin(); t != worker_events.end(); ++t)
							{
								switch((*t)->get_type())
									{
								    case slave_work_event::begin_twopf_assignment:
									    {
								        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
								        t++;
								        if(t == worker_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_TWOPF_TOO_FEW);
								        if((*t)->get_type() != slave_work_event::end_twopf_assignment) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_TWOPF_END_MISSING);
								        current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "red"));
								        break;
									    }

								    case slave_work_event::begin_threepf_assignment:
									    {
								        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
								        t++;
								        if(t == worker_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_THREEPF_TOO_FEW);
								        if((*t)->get_type() != slave_work_event::end_threepf_assignment) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_THREEPF_END_MISSING);
										    current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "orangered"));
								        break;
									    }

								    case slave_work_event::begin_zeta_twopf_assignment:
									    {
								        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
										    t++;
										    if(t == worker_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_ZETA_TWOPF_TOO_FEW);
										    if((*t)->get_type() != slave_work_event::end_zeta_twopf_assignment) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_ZETA_TWOPF_END_MISSING);
										    current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "green"));
										    break;
									    }

								    case slave_work_event::begin_zeta_threepf_assignment:
									    {
								        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
										    t++;
										    if(t == worker_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_ZETA_THREEPF_TOO_FEW);
										    if((*t)->get_type() != slave_work_event::end_zeta_threepf_assignment) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_ZETA_THREEPF_END_MISSING);
										    current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "limegreen"));
										    break;
									    }

								    case slave_work_event::begin_fNL_assignment:
									    {
								        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
										    t++;
										    if(t == worker_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_FNL_TOO_FEW);
										    if((*t)->get_type() != slave_work_event::end_fNL_assignment) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_FNL_END_MISSING);
										    current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "royalblue"));
										    break;
									    }

								    case slave_work_event::begin_output_assignment:
									    {
								        boost::posix_time::ptime begin_time = (*t)->get_timestamp();
										    t++;
										    if(t == worker_events.end()) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_OUTPUT_TOO_FEW);
										    if((*t)->get_type() != slave_work_event::end_output_assignment) throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_OUTPUT_END_MISSING);
										    current_bin->push_back(Gantt_bar(begin_time, (*t)->get_timestamp(), "orchid"));
										    break;
									    }

								    default:
									    throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_UNEXPECTED_EVENT);
									};
							}
					}
			}


    void work_journal::bin_milestones(std::list<std::list<Gantt_milestone> >& list)
	    {
        list.clear();

        this->bin_master_milestones(list);
        this->bin_slave_milestones(list);
	    }


		void work_journal::bin_master_milestones(std::list< std::list<Gantt_milestone> >& list)
			{
				// strip out milestones from master list
		    std::list< master_work_event* > master_events;
				for(std::list< work_event* >::iterator t = this->journal.begin(); t != this->journal.end(); ++t)
					{
						if((*t)->is_master())
							{
						    master_work_event* event = dynamic_cast< master_work_event* >(*t);
						    if(is_milestone(event->get_type())) master_events.push_back(event);
							}
					}
		    master_events.sort(work_journal_impl::WorkItemSorter());

				list.resize(list.size()+1);
		    std::list< std::list<Gantt_milestone> >::iterator current_bin = --list.end();

				for(std::list< master_work_event* >::const_iterator t = master_events.begin(); t != master_events.end(); ++t)
					{
						switch((*t)->get_type())
							{
						    default:
							    assert(false);
							    throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_UNEXPECTED_EVENT);
							}
					}
			}


		void work_journal::bin_slave_milestones(std::list< std::list<Gantt_milestone> >& list)
			{
				// strip out events for each worker
		    std::list< slave_work_event* > worker_events;
				for(unsigned int i = 0; i < this->N_workers; ++i)
					{
						worker_events.clear();
						for(std::list< work_event* >::iterator t = this->journal.begin(); t != this->journal.end(); ++t)
							{
								if(!(*t)->is_master())
									{
										slave_work_event* event = dynamic_cast< slave_work_event* >(*t);
										if(event->get_worker_number() == i && is_milestone(event->get_type())) worker_events.push_back(event);
									}
							}
						worker_events.sort(work_journal_impl::WorkItemSorter());

						list.resize(list.size()+1);
				    std::list< std::list<Gantt_milestone> >::iterator current_bin = --list.end();

						for(std::list< slave_work_event* >::const_iterator t = worker_events.begin(); t != worker_events.end(); ++t)
							{
								switch((*t)->get_type())
									{
								    case slave_work_event::integration_aggregation:
									    current_bin->push_back(Gantt_milestone((*t)->get_timestamp(), "red"));
												break;

								    case slave_work_event::postintegration_aggregation:
									    current_bin->push_back(Gantt_milestone((*t)->get_timestamp(), "green"));
									    break;

								    case slave_work_event::derived_content_aggregation:
									    current_bin->push_back(Gantt_milestone((*t)->get_timestamp(), "orchid"));
									    break;

								    default:
									    assert(false);
									    throw runtime_exception(runtime_exception::JOURNAL_ERROR, __CPP_TRANSPORT_JOURNAL_UNEXPECTED_EVENT);
									};
							}
					}
			}


		void work_journal::make_gantt_chart(const std::string& filename)
			{
		    std::list< std::list<Gantt_bar> > bars_list;
		    this->bin_bars(bars_list);

		    std::list< std::list<Gantt_milestone> > milestones_list;
				this->bin_milestones(milestones_list);

				assert(bars_list.size() == this->N_workers+1);

				// set up BOOST path to output file
		    boost::filesystem::path out_file(filename);

				// check if out_file already exists; if so, rename it uniquely
		    if(boost::filesystem::exists(out_file))
			    {
		        boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();

		        boost::filesystem::path out_dir = out_file;
		        out_dir.remove_filename();

		        boost::filesystem::path out_leaf = out_file.filename();
		        boost::filesystem::path out_ext  = out_file.extension();
		        out_leaf.replace_extension("");

		        std::stringstream new_leaf;
		        new_leaf << out_leaf.string() << "-" << boost::posix_time::to_iso_string(now);

		        out_leaf = boost::filesystem::path(new_leaf.str());
		        out_leaf.replace_extension(out_ext);
		        out_file = out_dir / out_leaf;
			    }

		    boost::filesystem::path script_file = out_file;
				if(script_file.extension() != ".py")
					{
				    // obtain path for Python script output
				    script_file.replace_extension(".py");
					}

		    std::ofstream out;
				out.open(script_file.c_str(), std::ios_base::trunc | std::ios_base::out);

				if(!out.is_open() || out.fail())
					{
				    std::ostringstream msg;
						msg << __CPP_TRANSPORT_JOURNAL_OPEN_FAIL << " " << script_file;
						throw runtime_exception(runtime_exception::JOURNAL_ERROR, msg.str());
					}

				out << "import numpy as np" << std::endl;
				out << "import matplotlib.pyplot as plt" << std::endl;
				out << "import matplotlib.dates as mdt" << std::endl;
				out << "import datetime as dt" << std::endl;
				out << "import dateutil.parser" << std::endl;

				out << "fig = plt.figure()" << std::endl;
				out << "ax = plt.gca()" << std::endl;

				Gantt_environment env("ax", "mdt", "dt", "dateutil.parser", 1.0);

		    // output the bar-chart part of the gantt chart

		    std::list< std::list<Gantt_bar> >::const_iterator t = bars_list.begin();
				unsigned int process_count = 0;       // number of processes appearing on the chart
				unsigned int total_count = 0;         // number of individual bars appearing on the chart (each process may have several bars)

		    while(t != bars_list.end())
			    {
		        std::list<Gantt_bar>::const_iterator u = t->begin();
		        while(u != t->end())
			        {
		            out << u->format(total_count, static_cast<double>(process_count) * 1.0, env) << std::endl;
		            ++u;
		            ++total_count;
			        }

		        ++process_count;
		        ++t;
			    }

				// output milestones

		    std::list< std::list<Gantt_milestone> >::const_iterator v = milestones_list.begin();
				process_count = 0;
				total_count = 0;

				while(v != milestones_list.end())
					{
				    std::list<Gantt_milestone>::const_iterator w = v->begin();
						while(w != v->end())
							{
								out << w->format(total_count, static_cast<double>(process_count) * 1.0, env) << std::endl;
								++w;
								++total_count;
							}

						++process_count;
						++v;
					}

				// label y-axis by worker if there are not too many labels
				if(this->N_workers <= 20)
					{
				    out << "ylabels = []" << std::endl;
				    out << "ylabels.append('Master')" << std::endl;

				    for(unsigned int i = 0; i < this->N_workers; ++i)
					    {
				        out << "ylabels.append('Worker " << i+1 << "')" << std::endl;
					    }

				    out << "pos = np.arange(0.5, " << 0.5 + static_cast<double>(process_count)*1.0 << ", 1.0)" << std::endl;
				    out << "locsy, labelsy = plt.yticks(pos, ylabels)" << std::endl;
				    out << "plt.setp(labelsy, fontsize=12)" << std::endl;
					}

		    // format the x-axis
		    out << "ax.xaxis_date()" << std::endl;
				out << "ax.axis('tight')" << std::endl;

				out << "ax.invert_yaxis()" << std::endl;
				out << "fig.autofmt_xdate()" << std::endl;
				if(out_file.extension() != ".py")
					{
				    out << "plt.savefig(" << out_file << ")" << std::endl;
					}
				else
					{
				    boost::filesystem::path temp = out_file;
						temp.replace_extension(".pdf");
						out << "plt.savefig(" << temp << ")" << std::endl;
					}

				out.close();

				// if output format wasn't Python, try to execute this script
				if(out_file.extension() != ".py")
					{
				    std::ostringstream command;
				    command << "source ~/.profile; " << this->python_path.string() << " \"" << script_file.string() << "\"";
				    int rc = system(command.str().c_str());

				    // remove python script if worked ok
				    if(rc == 0)
					    {
				        boost::filesystem::remove(script_file);
					    }
					}
			}


		//! journal_instrument is a class representing an instrument which automatically journals the
		//! time consumed by a block of code.
		//! It stores its creation time, and on destruction it generates a pair of journal entries
		//! recording when it was created and when it was destroyed.
		//! An optional resolution can be supplied which is the minimum timespan that is actually journalled.
    class journal_instrument
	    {

      public:

        journal_instrument(work_journal& j,
                           master_work_event::event_type b, master_work_event::event_type,
                           unsigned int i = 0, unsigned int m = __CPP_TRANSPORT_JOURNAL_MINIMUM_TIMESPAN);

        ~journal_instrument();


		    // INTERFACE

      public:

		    void stop() { this->stop_time = boost::posix_time::second_clock::universal_time(); this->stopped = true; }


		    // INTERNAL DATA

      private:

		    //! record start time of instrument
		    boost::posix_time::ptime start_time;

		    //! record stop time, if needed
		    boost::posix_time::ptime stop_time;

		    //! stopped?
		    bool stopped;

		    //! reference to parent scheduler
		    work_journal& journal;

		    //! label for begin journal item
		    master_work_event::event_type begin_label;

		    //! label for end journal item
		    master_work_event::event_type end_label;

		    //! minimum interval which will be journalled, measured in seconds
		    unsigned int minimum_interval;

		    //! id of events
		    unsigned int id;

	    };


    journal_instrument::journal_instrument(work_journal& j, master_work_event::event_type b, master_work_event::event_type e, unsigned int i, unsigned int m)
	    : journal(j),
	      begin_label(b),
	      end_label(e),
	      id(i),
	      minimum_interval(m),
	      stopped(false)
	    {
        this->start_time = boost::posix_time::second_clock::universal_time();
        this->stop_time  = this->start_time;
	    }


		journal_instrument::~journal_instrument()
			{
				if(!this->stopped) this->stop_time = boost::posix_time::second_clock::universal_time();

		    boost::posix_time::time_duration duration = this->stop_time - this->start_time;

				if(duration.total_seconds() > this->minimum_interval)
					{
						this->journal.add_entry(master_work_event(this->begin_label, this->start_time, this->id));
						this->journal.add_entry(master_work_event(this->end_label, this->stop_time, this->id));
					}
			}


	}   // namespace transport


#endif //__work_journal_H_
