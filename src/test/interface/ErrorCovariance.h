/*
 * (C) Copyright 2009-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef TEST_INTERFACE_ERRORCOVARIANCE_H_
#define TEST_INTERFACE_ERRORCOVARIANCE_H_

#include <cmath>
#include <iostream>
#include <string>

#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#define BOOST_TEST_DYN_LINK

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include "eckit/config/Configuration.h"
#include "oops/base/instantiateCovarFactory.h"
#include "oops/base/ModelSpaceCovarianceBase.h"
#include "oops/base/Variables.h"
#include "oops/interface/Geometry.h"
#include "oops/interface/Increment.h"
#include "oops/interface/State.h"
#include "oops/runs/Test.h"
#include "oops/util/DateTime.h"
#include "oops/util/dot_product.h"
#include "oops/util/Logger.h"
#include "test/TestEnvironment.h"

namespace test {

// =============================================================================

template <typename MODEL> class ErrorCovarianceFixture : private boost::noncopyable {
  typedef oops::ModelSpaceCovarianceBase<MODEL> Covariance_;
  typedef oops::Geometry<MODEL>       Geometry_;

 public:
  static const eckit::Configuration   & test()       {return *getInstance().test_;}
  static const Geometry_       & resol()      {return *getInstance().resol_;}
  static const oops::Variables & ctlvars()    {return *getInstance().ctlvars_;}
  static const util::DateTime  & time()       {return *getInstance().time_;}
  static const Covariance_     & covariance() {return *getInstance().B_;}

 private:
  static ErrorCovarianceFixture<MODEL>& getInstance() {
    static ErrorCovarianceFixture<MODEL> theErrorCovarianceFixture;
    return theErrorCovarianceFixture;
  }

  ErrorCovarianceFixture<MODEL>() {
    oops::instantiateCovarFactory<MODEL>();

    test_.reset(new eckit::LocalConfiguration(TestEnvironment::config(), "CovarianceTest"));

    const eckit::LocalConfiguration resolConfig(TestEnvironment::config(), "Geometry");
    resol_.reset(new Geometry_(resolConfig));

    const eckit::LocalConfiguration varConfig(TestEnvironment::config(), "Variables");
    ctlvars_.reset(new oops::Variables(varConfig));

    const eckit::LocalConfiguration fgconf(TestEnvironment::config(), "State");
    oops::State<MODEL> xx(*resol_, fgconf);

    time_.reset(new util::DateTime(xx.validTime()));

//  Setup the B matrix
    const eckit::LocalConfiguration covar(TestEnvironment::config(), "Covariance");
    B_.reset(oops::CovarianceFactory<MODEL>::create(covar, *resol_, *ctlvars_, xx, xx));
  }

  ~ErrorCovarianceFixture<MODEL>() {}

  boost::scoped_ptr<const eckit::LocalConfiguration>   test_;
  boost::scoped_ptr<const Geometry_>       resol_;
  boost::scoped_ptr<const oops::Variables> ctlvars_;
  boost::scoped_ptr<const util::DateTime>  time_;
  boost::scoped_ptr<Covariance_>           B_;
};

// =============================================================================

template <typename MODEL> void testErrorCovarianceZero() {
  typedef ErrorCovarianceFixture<MODEL>   Test_;
  typedef oops::Increment<MODEL>    Increment_;

  Increment_ dx1(Test_::resol(), Test_::ctlvars(), Test_::time());
  Increment_ dx2(Test_::resol(), Test_::ctlvars(), Test_::time());

  Test_::covariance().randomize(dx2);
  BOOST_CHECK_EQUAL(dx1.norm(), 0.0);
  BOOST_CHECK(dx2.norm() > 0.0);
  Test_::covariance().multiply(dx1, dx2);
  BOOST_CHECK_EQUAL(dx2.norm(), 0.0);

  dx1.zero();
  Test_::covariance().randomize(dx2);
  BOOST_CHECK_EQUAL(dx1.norm(), 0.0);
  BOOST_CHECK(dx2.norm() > 0.0);
  Test_::covariance().inverseMultiply(dx1, dx2);
  BOOST_CHECK_EQUAL(dx2.norm(), 0.0);
}

// -----------------------------------------------------------------------------

template <typename MODEL> void testErrorCovarianceInverse() {
  typedef ErrorCovarianceFixture<MODEL>   Test_;
  typedef oops::Increment<MODEL>    Increment_;

  Increment_ dx1(Test_::resol(), Test_::ctlvars(), Test_::time());
  Increment_ dx2(Test_::resol(), Test_::ctlvars(), Test_::time());
  Increment_ dx3(Test_::resol(), Test_::ctlvars(), Test_::time());
  Test_::covariance().randomize(dx1);
  BOOST_CHECK(dx1.norm() > 0.0);

  Test_::covariance().multiply(dx1, dx2);
  Test_::covariance().inverseMultiply(dx2, dx3);

  BOOST_CHECK(dx2.norm() > 0.0);
  BOOST_CHECK(dx3.norm() > 0.0);
  dx3 -= dx1;
  const double tol = Test_::test().getDouble("tolerance");
  BOOST_CHECK_SMALL(dx3.norm(), tol);
}

// -----------------------------------------------------------------------------

template <typename MODEL> void testErrorCovarianceSym() {
  typedef ErrorCovarianceFixture<MODEL>   Test_;
  typedef oops::Increment<MODEL>    Increment_;

  Increment_ dx(Test_::resol(), Test_::ctlvars(), Test_::time());
  Increment_ Bdx(Test_::resol(), Test_::ctlvars(), Test_::time());
  Increment_ dy(Test_::resol(), Test_::ctlvars(), Test_::time());
  Increment_ Bdy(Test_::resol(), Test_::ctlvars(), Test_::time());

  dx.random();
  dy.random();

  Test_::covariance().multiply(dx, Bdx);
  Test_::covariance().multiply(dy, Bdy);
  const double zz1 = dot_product(dx, Bdy);
  const double zz2 = dot_product(Bdx, dy);
  oops::Log::info() << "<dx,Bdy>-<Bdx,dy>/<dx,Bdy>="
                    <<  (zz1-zz2)/zz1 << std::endl;
  oops::Log::info() << "<dx,Bdy>-<Bdx,dy>/<Bdx,dy>="
                    <<  (zz1-zz2)/zz2 << std::endl;
  const double tol = Test_::test().getDouble("tolerance");
  BOOST_CHECK_CLOSE(zz1, zz2, tol);
}

// =============================================================================

template <typename MODEL> class ErrorCovariance : public oops::Test {
 public:
  ErrorCovariance() {}
  virtual ~ErrorCovariance() {}
 private:
  std::string testid() const {return "test::ErrorCovariance<" + MODEL::name() + ">";}

  void register_tests() const {
    boost::unit_test::test_suite * ts = BOOST_TEST_SUITE("interface/ErrorCovariance");

    ts->add(BOOST_TEST_CASE(&testErrorCovarianceZero<MODEL>));
    ts->add(BOOST_TEST_CASE(&testErrorCovarianceInverse<MODEL>));
    ts->add(BOOST_TEST_CASE(&testErrorCovarianceSym<MODEL>));

    boost::unit_test::framework::master_test_suite().add(ts);
  }
};

// =============================================================================

}  // namespace test

#endif  // TEST_INTERFACE_ERRORCOVARIANCE_H_
