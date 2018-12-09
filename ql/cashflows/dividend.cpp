/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2005 Joseph Wang
  Copyright (C) 2006 StatPro Italia srl

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

#include <ql/cashflows/dividend.hpp>
#include <ql/patterns/visitor.hpp>
#include <iostream>
#include <ql/instruments/dividendschedule.hpp>

namespace QuantLib {

    void Dividend::accept(AcyclicVisitor& v) {
        Visitor<Dividend>* v1 =
            dynamic_cast<Visitor<Dividend>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

    void FixedDividend::accept(AcyclicVisitor& v) {
        Visitor<FixedDividend>* v1 =
            dynamic_cast<Visitor<FixedDividend>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Dividend::accept(v);
    }

    void FractionalDividend::accept(AcyclicVisitor& v) {
        Visitor<FractionalDividend>* v1 =
            dynamic_cast<Visitor<FractionalDividend>*>(&v);
        if (v1 != 0) 
            v1->visit(*this);
        else
            Dividend::accept(v);
    }


    DividendSchedule
    DividendVector(const std::vector<Date>& dividendDates,
                   const std::vector<Real>& dividends) {

        QL_REQUIRE(dividendDates.size() == dividends.size(),
                   "size mismatch between dividend dates and amounts");

        std::vector<Date>::const_iterator dd;
        std::vector<Real>::const_iterator d;
        std::vector<ext::shared_ptr<Dividend> > items;
        items.reserve(dividendDates.size());
        for (dd = dividendDates.begin(), d = dividends.begin();
             dd != dividendDates.end(); ++dd, ++d) {
            items.push_back(ext::shared_ptr<Dividend>(new
                                                      FixedDividend(*d, *dd)));
        }
        return items;
    }


    DividendSchedule
    DividendVector(const std::vector<Date>& dividendDates,
                   const std::vector<Real>& dividends,
                   const std::vector<Dividend::Type>& types) {

        QL_REQUIRE(dividendDates.size() == dividends.size(),
                   "size mismatch between dividend dates and amounts");

        QL_REQUIRE(types.size() == dividends.size(),
                   "size mismatch between dividend dates and types");


        std::vector<Date>::const_iterator dd;
        std::vector<Real>::const_iterator d;
        std::vector<Dividend::Type>::const_iterator type;
        std::vector<ext::shared_ptr<Dividend> > items;
        items.reserve(dividendDates.size());
        for (dd = dividendDates.begin(), d = dividends.begin(), type = types.begin();
             dd != dividendDates.end(); ++dd, ++d, ++type) {

            if (*type == Dividend::Fixed) {
                items.push_back(ext::shared_ptr<Dividend>(new
                                                          FixedDividend(*d, *dd)));
            } else if (*type == Dividend::Fractional) {
                items.push_back(ext::shared_ptr<Dividend>(new
                                                          FractionalDividend(*d, *dd)));
            } else {
                QL_FAIL("Unknown dividend type");
            }
        }
        return items;
    }



}
