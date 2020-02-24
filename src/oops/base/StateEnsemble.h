/*
 * (C) Copyright 2019-2020 UCAR.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#ifndef OOPS_BASE_STATEENSEMBLE_H_
#define OOPS_BASE_STATEENSEMBLE_H_

#include <utility>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "oops/assimilation/State4D.h"
#include "oops/base/Accumulator.h"
#include "oops/base/Variables.h"
#include "oops/interface/Geometry.h"
#include "oops/util/Logger.h"

namespace oops {

// -----------------------------------------------------------------------------

/// \brief Ensemble of 4D states
template<typename MODEL> class StateEnsemble {
  typedef Geometry<MODEL>      Geometry_;
  typedef State4D<MODEL>       State_;

 public:
  /// Create ensemble of 4D states
  StateEnsemble(const Geometry_ &, const Variables &, const eckit::Configuration &);

  /// calculate ensemble mean
  State_ mean() const;

  /// Accessors
  unsigned int size() const { return states_.size(); }
  State_ & operator[](const int ii) { return states_[ii]; }
  const State_ & operator[](const int ii) const { return states_[ii]; }

 private:
  std::vector<State_> states_;
};

// ====================================================================================

template<typename MODEL>
StateEnsemble<MODEL>::StateEnsemble(const Geometry_ & resol,
                                    const Variables & vars,
                                    const eckit::Configuration & config)
  : states_() {
  std::vector<eckit::LocalConfiguration> memberConfig;
  config.get("members", memberConfig);
  states_.reserve(memberConfig.size());
  // Loop over all ensemble members
  for (size_t jj = 0; jj < memberConfig.size(); ++jj) {
    states_.emplace_back(resol, vars, memberConfig[jj]);
  }
  Log::trace() << "StateEnsemble:contructor done" << std::endl;
}

// -----------------------------------------------------------------------------

template<typename MODEL>
State4D<MODEL> StateEnsemble<MODEL>::mean() const {
  // Compute ensemble mean
  Accumulator<MODEL, State_, State_> ensmean(states_[0]);

  const double rr = 1.0/static_cast<double>(states_.size());
  for (size_t iens = 0; iens < states_.size(); ++iens) {
    ensmean.accumul(rr, states_[iens]);
  }

  Log::trace() << "StateEnsemble::mean done" << std::endl;
  return std::move(ensmean);
}

// -----------------------------------------------------------------------------

}  // namespace oops

#endif  // OOPS_BASE_STATEENSEMBLE_H_
