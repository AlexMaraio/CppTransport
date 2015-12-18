//
// Created by David Seery on 12/03/15.
// Copyright (c) 2015 University of Sussex. All rights reserved.
//


#ifndef __ginac_cache_H_
#define __ginac_cache_H_


#include <array>

#include "ginac/ginac.h"

#include "boost/timer/timer.hpp"


constexpr unsigned int DEFAULT_GINAC_CACHE_SIZE = 2048;


template <typename ExpressionType, unsigned int HashSize>
class ginac_cache
	{

  protected:

		class cache_element
			{

		  public:

				// constructor
				cache_element(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, const GiNaC::ex& e);

				cache_element(ExpressionType t, unsigned int i, const GiNaC::ex& e);

				// destructor
				~cache_element() = default;

				// INTERFACE

		  public:

				bool compare(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e) const;

		    bool compare(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e,
		                 GiNaC::ex& expected, bool& matched) const;

				bool compare(ExpressionType t, unsigned int i, GiNaC::ex& e) const;


		    // WRITE SELF DETAILS TO STREAM

		  public:

		    //! write to stream
		    template <typename Stream> void write(Stream& out) const;


				// INTERNAL DATA

		  protected:

		    ExpressionType type;

				unsigned int index;
				std::vector<GiNaC::ex> args;

				GiNaC::ex expr;

			};

  public:

		//! construct a cache object
		ginac_cache()
			: hits(0),
        misses(0)
			{
				// pause query timer
				query_timer.stop();
			}

		//! destroy a cache object
		~ginac_cache() = default;


		// INTERFACE - CACHING FUNCTIONS

  public:

		//! query for an element; returns true if it exists in the cache, in which case its value
		//! is copied to expr
		bool query(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e);

		//! convenience form without an argument list
		bool query(ExpressionType t, unsigned int i, GiNaC::ex& e);

    //! query for an element; returns true if it exists in the cache, in which case its value
    //! is copied to expr. This is a debugging version which checks that the
		//! returned value matches the expected expression
    bool query(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e, GiNaC::ex& expected);

		//! store an element in the cache. No collision checking is performed.
		void store(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, const GiNaC::ex& e);

		//! convenience form without an argument list
		void store(ExpressionType t, unsigned int i, const GiNaC::ex& e);


		// INTERFACE - CACHE STATISTICS

  public:

		//! get number of hits
		unsigned int get_hits() const { return(this->hits); }

		//! get number of misses
		unsigned int get_misses() const { return(this->misses); }

		//! get time spend performing queries
		boost::timer::nanosecond_type get_query_time() const { return(this->query_timer.elapsed().wall); }


		// INTERNAL API

  protected:

		//! hash
		unsigned int hash(unsigned int a, unsigned int b, unsigned int c) const { return((a + b*101 + c*463) % HashSize); }


		// INTERNAL DATA

  private:

		// hash table representing the cache
		std::array< std::vector<cache_element>, HashSize> cache;

		// cache statistics
		unsigned int hits;
		unsigned int misses;

		boost::timer::cpu_timer query_timer;

	};


template <typename ExpressionType, unsigned int HashSize>
ginac_cache<ExpressionType, HashSize>::cache_element::cache_element(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, const GiNaC::ex& e)
	: type(t),
		index(i),
		args(a),
		expr(e)
	{
	}


template <typename ExpressionType, unsigned int HashSize>
ginac_cache<ExpressionType, HashSize>::cache_element::cache_element(ExpressionType t, unsigned int i, const GiNaC::ex& e)
	: type(t),
	  index(i),
	  expr(e)
	{
	}


template <typename ExpressionType, unsigned int HashSize>
bool ginac_cache<ExpressionType, HashSize>::cache_element::compare(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e) const
	{
		if(this->type != t) return(false);
		if(this->index != i) return(false);
		if(this->args.size() != a.size()) return(false);

    std::vector<GiNaC::ex>::const_iterator m;
    std::vector<GiNaC::ex>::const_iterator n;
		for(m = this->args.begin(), n = a.begin(); m != this->args.end() && n != a.end(); ++m, ++n)
			{
		    GiNaC::ex test = ((*m) - (*n)).eval();
				bool equal = static_cast<bool>(test == 0);
				if(!equal) return(false);
			}

		if(m != this->args.end() || n != a.end()) return(false);

		e = this->expr;
		return(true);
	}


template <typename ExpressionType, unsigned int HashSize>
bool ginac_cache<ExpressionType, HashSize>::cache_element::compare(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e,
                                                                   GiNaC::ex& expected, bool& matched) const
	{
    if(this->type != t) return(false);
    if(this->index != i) return(false);
    if(this->args.size() != a.size()) return(false);

    std::vector<GiNaC::ex>::const_iterator m;
    std::vector<GiNaC::ex>::const_iterator n;
    for(m = this->args.begin(), n = a.begin(); m != this->args.end() && n != a.end(); ++m, ++n)
	    {
        GiNaC::ex test = ((*m) - (*n)).eval();
        bool equal = static_cast<bool>(test == 0);
        if(!equal) return(false);
	    }

    if(m != this->args.end() || n != a.end()) return(false);

    e = this->expr;


    GiNaC::ex diff = (expected - e).eval();
    bool equal = static_cast<bool>(diff == 0);
    if(!equal)
	    {
        std::cout << "Detected mismatching expression: type = " << t << ", index = " << index << '\n';
        std::cout << "  Lookup expression = " << e << '\n';
        std::cout << "  Expected expression = " << expected << '\n';
        std::cout << "  Difference = " << diff << '\n';

        unsigned int count = 0;
        std::cout << "  Lookup argument list:" << '\n';
        for(std::vector<GiNaC::ex>::const_iterator u = this->args.begin(); u != this->args.end(); ++u, ++count)
	        {
            std::cout << "  argument " << count << " = " << (*u) << '\n';
	        }

        count = 0;
        std::cout << "  Expected argument list:" << '\n';
        for(std::vector<GiNaC::ex>::const_iterator u = a.begin(); u != a.end(); ++u, ++count)
	        {
            std::cout << "  argument " << count << " = " << (*u) << '\n';
	        }

		    matched = false;
	    }
		else
	    {
		    matched = true;
	    }

    return(true);
	}


template <typename ExpressionType, unsigned int HashSize>
bool ginac_cache<ExpressionType, HashSize>::cache_element::compare(ExpressionType t, unsigned int i, GiNaC::ex& e) const
	{
    if(this->type != t) return(false);
    if(this->index != i) return(false);
    if(this->args.size() != 0) return(false);

    e = this->expr;
    return(true);
	}


template <typename ExpressionType, unsigned int HashSize>
template <typename Stream>
void ginac_cache<ExpressionType, HashSize>::cache_element::write(Stream& out) const
	{
		out << "== Cache element" << '\n';
		out << "   Type = " << this->type << '\n';
		out << "   Index = " << this->index << '\n';
		out << "   Number of arguments = " << this->args.size() << '\n';

		unsigned int count = 0;
		for(std::vector<GiNaC::ex>::const_iterator t = this->args.begin(); t != this->args.end(); ++t, ++count)
			{
				out << "   Argument " << count << " = " << (*t) << '\n';
			}
		out << "   Expression = " << this->expr << '\n';
	}


template <typename ExpressionType, unsigned int HashSize>
bool ginac_cache<ExpressionType, HashSize>::query(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e)
	{
		this->query_timer.resume();

		unsigned int hash = this->hash(static_cast<typename std::underlying_type<ExpressionType>::type>(t), i, a.size());

    for(const cache_element& elt : this->cache[hash])
			{
				if(elt.compare(t, i, a, e))
					{
				    hits++;
						this->query_timer.stop();
				    return(true);
					}
			}

		misses++;
		this->query_timer.stop();
		return(false);
	}


template <typename ExpressionType, unsigned int HashSize>
bool ginac_cache<ExpressionType, HashSize>::query(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, GiNaC::ex& e, GiNaC::ex& expected)
	{
		this->query_timer.resume();

    unsigned int hash = this->hash(t, i, a.size());

    for(const cache_element& elt : this->cache[hash])
	    {
		    bool matched = true;

        if(elt.compare(t, i, a, e, expected, matched))
	        {
		        if(!matched)
			        {
		            std::cout << '\n';
		            std::cout << "Detected matching failure -- dumping diagnostics" << '\n';
		            std::cout << "  index = " << i << ", hash = " << hash << ", number of entries in hash table at this value = " << this->cache[hash].size() << '\n';
                for(const cache_element& v : this->cache[hash])
			            {
		                std::cout << '\n';
		                v.write(std::cout);
		                std::cout << '\n';
		                std::cout << "Match to this cache element = " << v.compare(t, i, a, e) << '\n';
			            }
		            std::cout << '\n';
			        }

            hits++;
		        this->query_timer.stop();
            return(true);
	        }
	    }

    misses++;
		this->query_timer.stop();
    return(false);
	}


template <typename ExpressionType, unsigned int HashSize>
bool ginac_cache<ExpressionType, HashSize>::query(ExpressionType t, unsigned int i, GiNaC::ex& e)
	{
		this->query_timer.resume();

    unsigned int hash = this->hash(static_cast<typename std::underlying_type<ExpressionType>::type>(t), i, 0);

    for(const cache_element& elt : this->cache[hash])
	    {
        if(elt.compare(t, i, e))
	        {
		        hits++;
		        this->query_timer.stop();
            return(true);
	        }
	    }

		misses++;
		this->query_timer.stop();
    return(false);
	}


template <typename ExpressionType, unsigned int HashSize>
void ginac_cache<ExpressionType, HashSize>::store(ExpressionType t, unsigned int i, const std::vector<GiNaC::ex>& a, const GiNaC::ex& e)
	{
    unsigned int hash = this->hash(static_cast<typename std::underlying_type<ExpressionType>::type>(t), i, a.size());

		this->cache[hash].emplace_back(t, i, a, e);
	}


template <typename ExpressionType, unsigned int HashSize>
void ginac_cache<ExpressionType, HashSize>::store(ExpressionType t, unsigned int i, const GiNaC::ex& e)
	{
    unsigned int hash = this->hash(static_cast<typename std::underlying_type<ExpressionType>::type>(t), i, 0);

    this->cache[hash].emplace_back(t, i, e);
	}



#endif //__ginac_cache_H_
