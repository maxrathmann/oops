/*
 * (C) Copyright 2019 UCAR
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#ifndef OOPS_BASE_OBSERVER_H_
#define OOPS_BASE_OBSERVER_H_

#include <memory>
#include <string>
#include <vector>

#include "oops/base/ObsFilters.h"
#include "oops/base/Variables.h"
#include "oops/interface/GeoVaLs.h"
#include "oops/interface/GetValues.h"
#include "oops/interface/Locations.h"
#include "oops/interface/ObsAuxControl.h"
#include "oops/interface/ObsDataVector.h"
#include "oops/interface/ObsDiagnostics.h"
#include "oops/interface/ObsOperator.h"
#include "oops/interface/ObsSpace.h"
#include "oops/interface/ObsVector.h"
#include "oops/interface/State.h"
#include "oops/util/DateTime.h"
#include "oops/util/Duration.h"
#include "oops/util/Logger.h"
#include "oops/util/parameters/Parameter.h"
#include "oops/util/parameters/Parameters.h"
#include "oops/util/parameters/RequiredParameter.h"
#include "oops/util/Printable.h"

namespace oops {

// -----------------------------------------------------------------------------

/// \brief Parameters controlling an Observer.

template <typename OBS>
class ObserverParameters : public Parameters {
  OOPS_CONCRETE_PARAMETERS(ObserverParameters, Parameters)

 public:
  oops::RequiredParameter<eckit::LocalConfiguration> obsOperator{"obs operator", this};
  oops::Parameter<std::vector<ObsFilterParametersWrapper<OBS>>> obsFilters{"obs filters", {}, this};
};

// -----------------------------------------------------------------------------

/// Computes observation equivalent for a single ObsType

template <typename MODEL, typename OBS>
class Observer : public util::Printable {
  typedef GeoVaLs<OBS>             GeoVaLs_;
  typedef Locations<OBS>           Locations_;
  typedef ObsDiagnostics<OBS>      ObsDiags_;
  typedef ObsSpace<OBS>            ObsSpace_;
  typedef GetValues<MODEL, OBS>    GetValues_;
  typedef ObsAuxControl<OBS>       ObsAuxCtrl_;
  typedef ObsFilters<OBS>          ObsFilters_;
  typedef ObsOperator<OBS>         ObsOperator_;
  typedef ObsVector<OBS>           ObsVector_;
  typedef State<MODEL>             State_;
  template <typename DATA> using ObsDataPtr_ = std::shared_ptr<ObsDataVector<OBS, DATA> >;

 public:
  Observer(const ObserverParameters<OBS> &, const ObsSpace_ &, const ObsAuxCtrl_ &,
           ObsVector_ &, ObsDataPtr_<int> qcflags, ObsDataPtr_<float> obserr,
           const int iteration = 0);
  ~Observer();

  void doInitialize(const State_ &, const util::DateTime &, const util::DateTime &);
  void doProcessing(const State_ &, const util::DateTime &, const util::DateTime &);
  void doFinalize();

 private:
  void print(std::ostream &) const override;

// Obs operator
  ObsOperator_ hop_;

// Data
  const ObsSpace_ & obsdb_;
  ObsVector_ & yobs_;
  const ObsAuxCtrl_ & ybias_;

  ObsFilters_ filters_;
  Variables geovars_;  // Variables needed from model (through geovals)
  Locations_ locs_;
  std::unique_ptr<GetValues_> getvals_;
  std::shared_ptr<GeoVaLs_> gvals_;
};

// -----------------------------------------------------------------------------

template <typename MODEL, typename OBS>
Observer<MODEL, OBS>::Observer(const ObserverParameters<OBS> & params, const ObsSpace_ & obsdb,
                          const ObsAuxCtrl_ & ybias, ObsVector_ & yobs,
                          ObsDataPtr_<int> qcflags, ObsDataPtr_<float> obserr,
                          const int iteration)
  : hop_(obsdb, params.obsOperator),
    obsdb_(obsdb), yobs_(yobs), ybias_(ybias),
    filters_(obsdb, params.obsFilters, qcflags, obserr, iteration),
    locs_(hop_.locations())
{
  Log::trace() << "Observer::Observer starting" << std::endl;
  geovars_ += hop_.requiredVars();
  geovars_ += ybias_.requiredVars();
  geovars_ += filters_.requiredVars();
  Log::trace() << "Observer::Observer done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL, typename OBS>
Observer<MODEL, OBS>::~Observer() {
  Log::trace() << "Observer::~Observer starting" << std::endl;
  gvals_.reset();
  Log::trace() << "Observer::~Observer done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL, typename OBS>
void Observer<MODEL, OBS>::doInitialize(const State_ & xx,
                                   const util::DateTime & begin,
                                   const util::DateTime & end) {
  Log::trace() << "Observer::doInitialize start" << std::endl;
  filters_.preProcess();
  getvals_.reset(new GetValues_(xx.geometry(), locs_));
  gvals_.reset(new GeoVaLs_(locs_, geovars_));
  Log::trace() << "Observer::doInitialize done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL, typename OBS>
void Observer<MODEL, OBS>::doProcessing(const State_ & xx,
                                   const util::DateTime & t1,
                                   const util::DateTime & t2) {
  Log::trace() << "Observer::doProcessing start" << std::endl;
// Get state variables at obs locations
  getvals_->fillGeoVaLs(xx, t1, t2, *gvals_);
  Log::trace() << "Observer::doProcessing done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL, typename OBS>
void Observer<MODEL, OBS>::doFinalize() {
  Log::trace() << "Observer::doFinalize start" << std::endl;
  filters_.priorFilter(*gvals_);
  oops::Variables vars;
  vars += filters_.requiredHdiagnostics();
  vars += ybias_.requiredHdiagnostics();
  ObsDiags_ ydiags(obsdb_, locs_, vars);
  hop_.simulateObs(*gvals_, yobs_, ybias_, ydiags);
  filters_.postFilter(yobs_, ydiags);
  Log::trace() << "Observer::doFinalize done" << std::endl;
}

// -----------------------------------------------------------------------------

template <typename MODEL, typename OBS>
void Observer<MODEL, OBS>::print(std::ostream &) const {}

// -----------------------------------------------------------------------------

}  // namespace oops

#endif  // OOPS_BASE_OBSERVER_H_
