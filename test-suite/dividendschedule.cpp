/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2018 Daniel Barnes

  This file is part of QuantLib, a free-software/open-source library
  for financial quantitative analysts and developers - http://quantlib.org/

  QuantLib is free software: you can redistribute it and/or modify it
  under the terms of the QuantLib license.  You should have received a
  copy of the license along with this program; if not, please email
  <quantlib-dev@lists.sf.net>. The license is also available online at
  <http://quantlib.org/license.shtml>.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#include "dividendschedule.hpp"
#include <ql/instruments/dividendschedule.hpp>
#include <ql/cashflows/dividend.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include "utilities.hpp"
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <iostream>


using namespace QuantLib;
using namespace boost::unit_test_framework;

void DividendScheduleTest::testEscrow() {
  BOOST_TEST_MESSAGE("Testing dividend schedule");

  SavedSettings backup;

  Real tolerance = 1.0e-4;

  DayCounter dc = Actual360();
  Date today = Date::todaysDate();
  Settings::instance().evaluationDate() = today;

  ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.03));
  Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
  ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.04));
  Handle<YieldTermStructure> rTS(flatRate(rRate, dc));


  std::vector<Date> exDate;
  std::vector<Real> amount;
  std::vector<Dividend::Type> types;

  exDate.push_back(today + 1*Months);
  exDate.push_back(today + 2*Months);
  exDate.push_back(today + 3*Months);
  exDate.push_back(today + 4*Months);

  amount.push_back(1.0);
  amount.push_back(1.0);
  amount.push_back(0.01);
  amount.push_back(2.0);

  types.push_back(Dividend::Fixed);
  types.push_back(Dividend::Fixed);
  types.push_back(Dividend::Fractional);
  types.push_back(Dividend::Fixed);

  DividendSchedule divSch = DividendVector(exDate,amount,types);

  Date expiry = today + 2 * Months;
  GetEscrow es(rTS,qTS);

  std::vector<ext::shared_ptr<Dividend> >::const_iterator div;

  for (div = divSch.begin(); div != divSch.end() && (*div)->date() <= expiry; ++div) {
      (*div)->accept(es);
  }

  // Test two cash dividends
  Real expEscrow = 1.0 * rTS->discount(today + 1 * Months) / qTS->discount(today + 1 * Months) + 1.0 * rTS->discount(today + 2 * Months) / qTS->discount(today + 2 * Months);
  if (std::fabs(es.escrowAmount() - expEscrow) > tolerance) {
      BOOST_FAIL("Incorrect escrow dividend amount for two cash dividends\n"
                 << "value = " << es.escrowAmount() << "\n"
                 << "prop = " << es.propFactor());
  }

  // Test two cash followed by proportional dividend
  Date expiry2 = today + 3 * Months;
  GetEscrow es2(rTS,qTS);
  std::cout<<"Oi\n";
  for (div = divSch.begin(); ((*div)->date() <= expiry2) && (div != divSch.end()); ++div) {
      (*div)->accept(es2);
  }
  std::cout<<"Oi\n";
  if (std::fabs(es2.escrowAmount() - expEscrow) > tolerance || std::fabs(es2.propFactor () - 0.99) > tolerance) {
      BOOST_FAIL("Incorrect escrow dividend amount two cash then prop\n"
                 << "cash value = " << es2.escrowAmount() << "\n"
                 << "prop value = " << es2.propFactor());
  }
  std::cout<<"Oi\n";
  // Test two cash followed by proportional dividend then another cash
  Date expiry3 = today + 4 * Months;
  GetEscrow es3(rTS,qTS);
  std::cout<<"Oi\n";
  for (div = divSch.begin(); div != divSch.end(); ++div) {
      std::cout<<"hmm"<<std::endl;
      (*div)->accept(es3);
  }
  std::cout<<"Oi\n";

  expEscrow += 2.0/0.99 * rTS->discount(today + 4 * Months) / qTS->discount(today + 4 * Months);

  if (std::fabs(es3.escrowAmount() - expEscrow) > tolerance || std::fabs(es3.propFactor() - 0.99) > tolerance) {
      BOOST_FAIL("Incorrect escrow dividend amount two cash then prop and then cash\n"
                 << "cash value = " << es3.escrowAmount() << " vs "
                 << expEscrow << "\n"
                 << "prop value = " << es3.propFactor());
  }

}

test_suite* DividendScheduleTest::suite() {
  test_suite* suite = BOOST_TEST_SUITE("Dividend schedule test");
  suite->add(QUANTLIB_TEST_CASE(&DividendScheduleTest::testEscrow));
  return suite;
}

