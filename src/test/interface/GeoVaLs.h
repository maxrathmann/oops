/*
 * (C) Copyright 2017-2018 UCAR
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#ifndef TEST_INTERFACE_GEOVALS_H_
#define TEST_INTERFACE_GEOVALS_H_

#include <cmath>
#include <string>
#include <vector>

#define ECKIT_TESTING_SELF_REGISTER_CASES 0

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "oops/base/ObsSpaces.h"
#include "oops/interface/GeoVaLs.h"
#include "oops/interface/Locations.h"
#include "oops/interface/ObsOperator.h"
#include "oops/runs/Test.h"
#include "oops/util/dot_product.h"
#include "test/TestEnvironment.h"

namespace test {

// -----------------------------------------------------------------------------

template <typename MODEL>
class GeoVaLsFixture : private boost::noncopyable {
  typedef oops::ObsSpaces<MODEL>  ObsSpaces_;

 public:
  static const util::DateTime  & tbgn() {return *getInstance().tbgn_;}
  static const util::DateTime  & tend() {return *getInstance().tend_;}
  static ObsSpaces_         & obspace() {return *getInstance().ospaces_;}

 private:
  static GeoVaLsFixture<MODEL>& getInstance() {
    static GeoVaLsFixture<MODEL> theGeoVaLsFixture;
    return theGeoVaLsFixture;
  }

  GeoVaLsFixture(): tbgn_(), tend_(), ospaces_() {
    tbgn_.reset(new util::DateTime(TestEnvironment::config().getString("window_begin")));
    tend_.reset(new util::DateTime(TestEnvironment::config().getString("window_end")));

    const eckit::LocalConfiguration conf(TestEnvironment::config(), "Observations");
    ospaces_.reset(new ObsSpaces_(conf, *tbgn_, *tend_));
  }

  ~GeoVaLsFixture() {}

  boost::scoped_ptr<const util::DateTime> tbgn_;
  boost::scoped_ptr<const util::DateTime> tend_;
  boost::scoped_ptr<ObsSpaces_> ospaces_;
};

// -----------------------------------------------------------------------------

template <typename MODEL> void testConstructor() {
  typedef GeoVaLsFixture<MODEL> Test_;
  typedef oops::GeoVaLs<MODEL>    GeoVaLs_;
  typedef oops::Locations<MODEL>  Locations_;
  typedef oops::ObsOperator<MODEL> ObsOperator_;

  const eckit::LocalConfiguration obsconf(TestEnvironment::config(), "Observations");
  std::vector<eckit::LocalConfiguration> conf;
  obsconf.get("ObsTypes", conf);

  for (std::size_t jj = 0; jj < Test_::obspace().size(); ++jj) {
    eckit::LocalConfiguration obsopconf(conf[jj], "ObsOperator");
    ObsOperator_ hop(Test_::obspace()[jj], obsopconf);

    Locations_ locs(hop.locations(Test_::tbgn(), Test_::tend()));
    boost::scoped_ptr<GeoVaLs_> ov(new GeoVaLs_(locs, hop.variables()));
    EXPECT(ov.get());

    ov.reset();
    EXPECT(!ov.get());
  }
}

// -----------------------------------------------------------------------------

template <typename MODEL> void testUtils() {
  typedef GeoVaLsFixture<MODEL> Test_;
  typedef oops::GeoVaLs<MODEL>    GeoVaLs_;
  typedef oops::Locations<MODEL>  Locations_;
  typedef oops::ObsOperator<MODEL> ObsOperator_;

  const eckit::LocalConfiguration obsconf(TestEnvironment::config(), "Observations");
  std::vector<eckit::LocalConfiguration> conf;
  obsconf.get("ObsTypes", conf);

  for (std::size_t jj = 0; jj < Test_::obspace().size(); ++jj) {
    eckit::LocalConfiguration obsopconf(conf[jj], "ObsOperator");
    ObsOperator_ hop(Test_::obspace()[jj], obsopconf);

    Locations_ locs(hop.locations(Test_::tbgn(), Test_::tend()));
    GeoVaLs_ gval(locs, hop.variables());

    gval.random();
    const double zz1 = dot_product(gval, gval);
    EXPECT(zz1 > 0.0);

    gval.zero();
    const double zz2 = dot_product(gval, gval);
    EXPECT(zz2 == 0.0);
  }
}

// -----------------------------------------------------------------------------

template <typename MODEL> void testRead() {
  typedef GeoVaLsFixture<MODEL> Test_;
  typedef oops::GeoVaLs<MODEL>  GeoVaLs_;
  typedef oops::Locations<MODEL>  Locations_;
  typedef oops::ObsOperator<MODEL> ObsOperator_;

  const eckit::LocalConfiguration obsconf(TestEnvironment::config(), "Observations");
  std::vector<eckit::LocalConfiguration> conf;
  obsconf.get("ObsTypes", conf);

  const double tol = 1.0e-9;
  for (std::size_t jj = 0; jj < Test_::obspace().size(); ++jj) {
    eckit::LocalConfiguration obsopconf(conf[jj], "ObsOperator");
    ObsOperator_ hop(Test_::obspace()[jj], obsopconf);

    eckit::LocalConfiguration gconf(conf[jj], "GeoVaLs");
    Locations_ locs(hop.locations(Test_::tbgn(), Test_::tend()));

    GeoVaLs_ gval(gconf, hop.variables());

    const double xx = gconf.getDouble("norm");
    const double zz = sqrt(dot_product(gval, gval));

    oops::Log::debug() << "xx: " << std::fixed << std::setprecision(8) << xx << std::endl;
    oops::Log::debug() << "zz: " << std::fixed << std::setprecision(8) << zz << std::endl;

    EXPECT(oops::is_close(xx, zz, tol));
  }
}

// -----------------------------------------------------------------------------

template <typename MODEL>
class GeoVaLs : public oops::Test {
 public:
  GeoVaLs() {}
  virtual ~GeoVaLs() {}
 private:
  std::string testid() const {return "test::GeoVaLs<" + MODEL::name() + ">";}

  void register_tests() const {
    std::vector<eckit::testing::Test>& ts = eckit::testing::specification();

    ts.emplace_back(CASE("interface/GeoVaLs/testConstructor")
      { testConstructor<MODEL>(); });
    ts.emplace_back(CASE("interface/GeoVaLs/testRead")
      { testRead<MODEL>(); });
  }
};

// =============================================================================

}  // namespace test

#endif  // TEST_INTERFACE_GEOVALS_H_
