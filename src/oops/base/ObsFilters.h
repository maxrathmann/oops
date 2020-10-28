/*
 * (C) Copyright 2017-2019 UCAR
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#ifndef OOPS_BASE_OBSFILTERS_H_
#define OOPS_BASE_OBSFILTERS_H_

#include <memory>
#include <set>
#include <vector>

#include <boost/noncopyable.hpp>

#include "eckit/config/LocalConfiguration.h"
#include "oops/base/ObsFilterBase.h"
#include "oops/base/Variables.h"
#include "oops/interface/GeoVaLs.h"
#include "oops/interface/ObsDataVector.h"
#include "oops/interface/ObsDiagnostics.h"
#include "oops/interface/ObsSpace.h"
#include "oops/interface/ObsVector.h"
#include "oops/util/IntSetParser.h"
#include "oops/util/Printable.h"

namespace oops {

/// Holds observation filters (usually QC) for one observation type

// -----------------------------------------------------------------------------

template <typename OBS>
class ObsFilters : public util::Printable,
                   private boost::noncopyable {
  typedef GeoVaLs<OBS>            GeoVaLs_;
  typedef ObsDiagnostics<OBS>     ObsDiags_;
  typedef ObsFilterBase<OBS>      ObsFilterBase_;
  typedef ObsSpace<OBS>           ObsSpace_;
  typedef ObsVector<OBS>          ObsVector_;
  typedef std::shared_ptr<ObsFilterBase<OBS> >  ObsFilterPtr_;
  template <typename DATA> using ObsDataPtr_ = std::shared_ptr<ObsDataVector<OBS, DATA> >;

 public:
  ObsFilters(const ObsSpace_ &, const eckit::Configuration &,
             ObsDataPtr_<int> qcflags = ObsDataPtr_<int>(),
             ObsDataPtr_<float> obserr = ObsDataPtr_<float>());
  ObsFilters();
  ~ObsFilters();

  void preProcess() const;
  void priorFilter(const GeoVaLs_ &) const;
  void postFilter(const ObsVector_ &, const ObsDiags_ &) const;

  Variables requiredVars() const {return geovars_;}
  Variables requiredHdiagnostics() const {return diagvars_;}

 private:
  void print(std::ostream &) const;

  std::vector<ObsFilterPtr_> filters_;
  Variables geovars_;
  Variables diagvars_;
};

// -----------------------------------------------------------------------------

template <typename OBS>
ObsFilters<OBS>::ObsFilters(const ObsSpace_ & os, const eckit::Configuration & conf,
                              ObsDataPtr_<int> qcflags, ObsDataPtr_<float> obserr)
  : filters_(), geovars_(), diagvars_() {
  Log::trace() << "ObsFilters::ObsFilters starting " << conf << std::endl;

// Get filters configuration
  std::vector<eckit::LocalConfiguration> confs;
  conf.get("obs filters", confs);

// Prepare QC handling and statistics if any filters are present
  if (confs.size() > 0) {
    eckit::LocalConfiguration preconf;
    preconf.set("filter", "QCmanager");
    filters_.push_back(FilterFactory<OBS>::create(os, preconf, qcflags, obserr));
  }

// Create the filters, only at 0-th iteration, or at iterations specified in "apply at iterations"
  for (std::size_t jj = 0; jj < confs.size(); ++jj) {
    // Only create filters for the 0-th iteration
    const int iter = conf.getInt("iteration");
    bool apply = (iter == 0);
    // If "apply at iterations" is set, check if this is the right iteration
    if (confs[jj].has("apply at iterations")) {
      std::set<int> iters = parseIntSet(confs[jj].getString("apply at iterations"));
      apply = contains(iters, iter);
    }
    if (apply) {
      ObsFilterPtr_ tmp(FilterFactory<OBS>::create(os, confs[jj], qcflags, obserr));
      geovars_ += tmp->requiredVars();
      diagvars_ += tmp->requiredHdiagnostics();
      filters_.push_back(tmp);
    }
  }

  Log::trace() << "ObsFilters::ObsFilters done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename OBS>
ObsFilters<OBS>::ObsFilters() : filters_(), geovars_(), diagvars_() {}

// -----------------------------------------------------------------------------

template <typename OBS>
ObsFilters<OBS>::~ObsFilters() {
  Log::trace() << "ObsFilters::~ObsFilters destructed" << std::endl;
}

// -----------------------------------------------------------------------------

template<typename OBS>
void ObsFilters<OBS>::preProcess() const {
  for (std::size_t jj = 0; jj < filters_.size(); ++jj) {
    filters_.at(jj)->preProcess();
  }
}

// -----------------------------------------------------------------------------

template<typename OBS>
void ObsFilters<OBS>::priorFilter(const GeoVaLs_ & gv) const {
  for (std::size_t jj = 0; jj < filters_.size(); ++jj) {
    filters_.at(jj)->priorFilter(gv);
  }
}

// -----------------------------------------------------------------------------

template<typename OBS>
void ObsFilters<OBS>::postFilter(const ObsVector_ & hofx, const ObsDiags_ & diags) const {
  for (std::size_t jj = 0; jj < filters_.size(); ++jj) {
    filters_.at(jj)->postFilter(hofx, diags);
  }
}

// -----------------------------------------------------------------------------

template <typename OBS>
void ObsFilters<OBS>::print(std::ostream & os) const {
  os << "ObsFilters: " << filters_.size() << " elements:" << std::endl;
  for (std::size_t jj = 0; jj < filters_.size(); ++jj) {
    os << *filters_.at(jj) << std::endl;
  }
}

// -----------------------------------------------------------------------------

}  // namespace oops

#endif  // OOPS_BASE_OBSFILTERS_H_
