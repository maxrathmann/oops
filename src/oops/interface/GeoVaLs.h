/*
 * (C) Copyright 2009-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef OOPS_INTERFACE_GEOVALS_H_
#define OOPS_INTERFACE_GEOVALS_H_

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "eckit/config/Configuration.h"
#include "oops/base/Variables.h"
#include "oops/interface/Locations.h"
#include "util/ObjectCounter.h"
#include "util/Printable.h"
#include "util/Timer.h"

namespace oops {

// -----------------------------------------------------------------------------
template <typename MODEL>
class GeoVaLs : public util::Printable,
                private util::ObjectCounter<GeoVaLs<MODEL> > {
  typedef typename MODEL::GeoVaLs          GeoVaLs_;
  typedef Locations<MODEL>                 Locations_;

 public:
  static const std::string classname() {return "oops::GeoVaLs";}

  GeoVaLs(const Locations_ &, const Variables &);
  GeoVaLs(const eckit::Configuration &, const Variables &);
  GeoVaLs(const Locations_ &, const Variables &, const eckit::Configuration &);
  GeoVaLs(const GeoVaLs &);

  ~GeoVaLs();

/// Interfacing
  const GeoVaLs_ & geovals() const {return *gvals_;}
  GeoVaLs_ & geovals() {return *gvals_;}

/// Linear algebra and utilities, mostly for writing tests
  void abs();
  void zero();
  void random();
  double norm() const;
  GeoVaLs & operator=(const GeoVaLs &);
  GeoVaLs & operator*=(const double &);
  GeoVaLs & operator+=(const GeoVaLs &);
  GeoVaLs & operator-=(const GeoVaLs &);
  GeoVaLs & operator/=(const GeoVaLs &);
  double dot_product_with(const GeoVaLs &) const;
  void read(const eckit::Configuration &);
  void write(const eckit::Configuration &) const;
  
 private:
  void print(std::ostream &) const;
  boost::scoped_ptr<GeoVaLs_> gvals_;
};

// -----------------------------------------------------------------------------

template <typename MODEL>
GeoVaLs<MODEL>::GeoVaLs(const Locations_ & locs, const Variables & vars) : gvals_() {
  Log::trace() << "GeoVaLs<MODEL>::GeoVaLs starting" << std::endl;
  util::Timer timer(classname(), "GeoVaLs");
  gvals_.reset(new GeoVaLs_(locs.locations(), vars));
  Log::trace() << "GeoVaLs<MODEL>::GeoVaLs done" << std::endl;
}

// -----------------------------------------------------------------------------
// We may want to eliminate this constructor eventually in favor of the
// following one

template <typename MODEL>
GeoVaLs<MODEL>::GeoVaLs(const eckit::Configuration & conf, const Variables & vars) : gvals_() {
  Log::trace() << "GeoVaLs<MODEL>::GeoVaLs read starting" << std::endl;
  util::Timer timer(classname(), "GeoVaLs");
  gvals_.reset(new GeoVaLs_(conf, vars));
  Log::trace() << "GeoVaLs<MODEL>::GeoVaLs read done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
  GeoVaLs<MODEL>::GeoVaLs(const Locations_ & locs, const Variables & vars,
			  const eckit::Configuration & conf) : gvals_() {
  Log::trace() << "GeoVaLs<MODEL>::GeoVaLs read starting" << std::endl;
  util::Timer timer(classname(), "GeoVaLs");
  gvals_.reset(new GeoVaLs_(locs.locations(), vars, conf));
  Log::trace() << "GeoVaLs<MODEL>::GeoVaLs read done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
GeoVaLs<MODEL>::GeoVaLs(const GeoVaLs & other): gvals_() {
  Log::trace() << "GeoVaLs<MODEL>::GeoVaLs starting" << std::endl;
  util::Timer timer(classname(), "GeoVaLs");
  gvals_.reset(new GeoVaLs_(*other.gvals_));
  Log::trace() << "ObsVector<MODEL>::ObsVector done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
GeoVaLs<MODEL>::~GeoVaLs() {
  Log::trace() << "GeoVaLs<MODEL>::~GeoVaLs starting" << std::endl;
  util::Timer timer(classname(), "~GeoVaLs");
  gvals_.reset();
  Log::trace() << "GeoVaLs<MODEL>::~GeoVaLs done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
double GeoVaLs<MODEL>::dot_product_with(const GeoVaLs & other) const {
  Log::trace() << "GeoVaLs<MODEL>::dot_product_with starting" << std::endl;
  util::Timer timer(classname(), "dot_product_with");
  double zz = gvals_->dot_product_with(*other.gvals_);
  Log::trace() << "GeoVaLs<MODEL>::dot_product_with done" << std::endl;
  return zz;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
GeoVaLs<MODEL> & GeoVaLs<MODEL>::operator=(const GeoVaLs & rhs) {
  Log::trace() << "GeoVaLs<MODEL>::operator= starting" << std::endl;
  util::Timer timer(classname(), "operator=");
  *gvals_ = *rhs.gvals_;
  Log::trace() << "GeovaLs<MODEL>::operator= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
GeoVaLs<MODEL> & GeoVaLs<MODEL>::operator+=(const GeoVaLs & rhs) {
  Log::trace() << "GeoVaLs<MODEL>::+=(GeoVaLs, GeoVaLs) starting" << std::endl;
  util::Timer timer(classname(), "operator+=");
  *gvals_ += *rhs.gvals_;
  Log::trace() << "GeoVaLs<MODEL>::+= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
GeoVaLs<MODEL> & GeoVaLs<MODEL>::operator-=(const GeoVaLs & rhs) {
  Log::trace() << "GeoVaLs<MODEL>::-=(GeoVaLs, GeoVaLs) starting" << std::endl;
  util::Timer timer(classname(), "operator-=");
  *gvals_ -= *rhs.gvals_;
  Log::trace() << "GeoVaLs<MODEL>::+= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------
/*! GeoVaLs Normalization Operator
 *
 * This is a normalization operator that first computes the normalization 
 * factor for each variable based on the rms amplitude of that variable across 
 * all locations in the reference GeoVaLs object (rhs).  Then each element of 
 * the input GeoVals object (*this) is divided by these normalization factors.
 */

template <typename MODEL>
GeoVaLs<MODEL> & GeoVaLs<MODEL>::operator/=(const GeoVaLs & rhs) {
  Log::trace() << "GeoVaLs<MODEL>::/=(GeoVaLs, GeoVaLs) starting" << std::endl;
  util::Timer timer(classname(), "operator/=");
  *gvals_ /= *rhs.gvals_;
  Log::trace() << "GeoVaLs<MODEL>::+= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

template<typename MODEL>
GeoVaLs<MODEL> & GeoVaLs<MODEL>::operator*=(const double & zz) {
  Log::trace() << "GeoVaLs<MODEL>::operator*= starting" << std::endl;
  util::Timer timer(classname(), "operator*=");
  *gvals_ *= zz;
  Log::trace() << "GeoVaLs<MODEL>::operator*= done" << std::endl;
  return *this;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
void GeoVaLs<MODEL>::abs() {
  Log::trace() << "GeoVaLs<MODEL>::abs starting" << std::endl;
  util::Timer timer(classname(), "abs");
  gvals_->abs();
  Log::trace() << "GeoVaLs<MODEL>::abs done" << std::endl;
}
// -----------------------------------------------------------------------------

template <typename MODEL>
void GeoVaLs<MODEL>::zero() {
  Log::trace() << "GeoVaLs<MODEL>::zero starting" << std::endl;
  util::Timer timer(classname(), "zero");
  gvals_->zero();
  Log::trace() << "GeoVaLs<MODEL>::zero done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
double GeoVaLs<MODEL>::norm() const {
  Log::trace() << "GeoVaLs<MODEL>::norm starting" << std::endl;
  util::Timer timer(classname(), "norm");
  double zz = gvals_->norm();
  Log::trace() << "GeoVaLs<MODEL>::norm done" << std::endl;
  return zz;
}

// -----------------------------------------------------------------------------

template <typename MODEL>
void GeoVaLs<MODEL>::random() {
  Log::trace() << "GeoVaLs<MODEL>::random starting" << std::endl;
  util::Timer timer(classname(), "random");
  gvals_->random();
  Log::trace() << "GeoVaLs<MODEL>::random done" << std::endl;
}

// -----------------------------------------------------------------------------

template<typename MODEL>
void GeoVaLs<MODEL>::read(const eckit::Configuration & conf) {
  Log::trace() << "GeoVaLs<MODEL>::read starting" << std::endl;
  util::Timer timer(classname(), "read");
  gvals_->read(conf);
  Log::trace() << "GeoVaLs<MODEL>::read done" << std::endl;
}

// -----------------------------------------------------------------------------

template<typename MODEL>
void GeoVaLs<MODEL>::write(const eckit::Configuration & conf) const {
  Log::trace() << "GeoVaLs<MODEL>::write starting" << std::endl;
  util::Timer timer(classname(), "write");
  gvals_->write(conf);
  Log::trace() << "GeoVaLs<MODEL>::write done" << std::endl;
}

// -----------------------------------------------------------------------------

template<typename MODEL>
void GeoVaLs<MODEL>::print(std::ostream & os) const {
  Log::trace() << "GeoVaLs<MODEL>::print starting" << std::endl;
  util::Timer timer(classname(), "print");
  os << *gvals_;
  Log::trace() << "GeoVaLs<MODEL>::print done" << std::endl;
}

// -----------------------------------------------------------------------------

}  // namespace oops

#endif  // OOPS_INTERFACE_GEOVALS_H_
