/*
 * Copyright (c) 2003-2009, John Wiegley.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of New Artisans LLC nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @addtogroup data
 */

/**
 * @file   xact.h
 * @author John Wiegley
 *
 * @ingroup data
 */
#ifndef _XACT_H
#define _XACT_H

#include "item.h"
#include "predicate.h"

namespace ledger {

class post_t;
class journal_t;

typedef std::list<post_t *> posts_list;

class xact_base_t : public item_t
{
public:
  journal_t * journal;
  posts_list  posts;

  xact_base_t() : item_t(), journal(NULL) {
    TRACE_CTOR(xact_base_t, "");
  }
  xact_base_t(const xact_base_t& e);  

  virtual ~xact_base_t();

  virtual void add_post(post_t * post);
  virtual bool remove_post(post_t * post);

  posts_list::iterator posts_begin() {
    return posts.begin();
  }
  posts_list::iterator posts_end() {
    return posts.end();
  }

  value_t magnitude() const;

  bool finalize();
  bool verify();

  bool has_xdata();
  void clear_xdata();

  virtual bool valid() const {
    return true;
  }

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & boost::serialization::base_object<item_t>(*this);
    ar & journal;
    ar & posts;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

class xact_t : public xact_base_t
{
public:
  optional<string> code;
  string	   payee;

  xact_t() {
    TRACE_CTOR(xact_t, "");
  }
  xact_t(const xact_t& e);

  virtual ~xact_t() {
    TRACE_DTOR(xact_t);
  }

  virtual void add_post(post_t * post);

  string idstring() const;
  string id() const;

  virtual expr_t::ptr_op_t lookup(const symbol_t::kind_t kind,
				  const string& name);

  virtual bool valid() const;

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & boost::serialization::base_object<xact_base_t>(*this);
    ar & code;
    ar & payee;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

class auto_xact_t : public xact_base_t
{
public:
  predicate_t predicate;
  bool        try_quick_match;

  std::map<string, bool> memoized_results;

  auto_xact_t() : try_quick_match(true) {
    TRACE_CTOR(auto_xact_t, "");
  }
  auto_xact_t(const auto_xact_t& other)
    : xact_base_t(), predicate(other.predicate),
      try_quick_match(other.try_quick_match) {
    TRACE_CTOR(auto_xact_t, "copy");
  }
  auto_xact_t(const predicate_t& _predicate)
    : predicate(_predicate), try_quick_match(true)
  {
    TRACE_CTOR(auto_xact_t, "const predicate_t&");
  }

  virtual ~auto_xact_t() {
    TRACE_DTOR(auto_xact_t);
  }

  virtual void extend_xact(xact_base_t& xact);

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & boost::serialization::base_object<xact_base_t>(*this);
    ar & predicate;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

class period_xact_t : public xact_base_t
{
 public:
  date_interval_t period;
  string	  period_string;

  period_xact_t() {
    TRACE_CTOR(period_xact_t, "");
  }
  period_xact_t(const period_xact_t& e)
    : xact_base_t(e), period(e.period), period_string(e.period_string) {
    TRACE_CTOR(period_xact_t, "copy");
  }
  period_xact_t(const string& _period)
    : period(_period), period_string(_period) {
    TRACE_CTOR(period_xact_t, "const string&");
  }

  virtual ~period_xact_t() {
    TRACE_DTOR(period_xact_t);
  }

#if defined(HAVE_BOOST_SERIALIZATION)
private:
  /** Serialization. */

  friend class boost::serialization::access;

  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & boost::serialization::base_object<xact_base_t>(*this);
    ar & period;
    ar & period_string;
  }
#endif // HAVE_BOOST_SERIALIZATION
};

typedef std::list<xact_t *>	   xacts_list;
typedef std::list<auto_xact_t *>   auto_xacts_list;
typedef std::list<period_xact_t *> period_xacts_list;

void to_xml(std::ostream& out, const xact_t& xact);

} // namespace ledger

#endif // _XACT_H