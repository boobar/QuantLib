/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2005, 2006 Theo Boafo

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

/*! \file dividendschedule.hpp
    \brief Schedule of dividend dates
*/

#ifndef quantlib_dividend_schedule_hpp
#define quantlib_dividend_schedule_hpp

#include <ql/cashflows/dividend.hpp>
#include <ql/event.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <vector>

namespace QuantLib {
    typedef std::vector<ext::shared_ptr<Dividend> > DividendSchedule;


    // Hans Buehler
    // Volatility and Dividends
    // Volatility Modelling with Cash Dividends and simple Credit Risk
    // SSRN 1141877

    // Credit risk not included

    class GetEscrow : public AcyclicVisitor,
                      public Visitor<Dividend>,
                      public Visitor<FixedDividend>,
                      public Visitor<FractionalDividend> {

    public:

        GetEscrow(const Handle<YieldTermStructure>& rateCurve,const Handle<YieldTermStructure>& dividendYield) : _propAmount(1),_cashAmount(0),_rateCurve(rateCurve),_dividendYield(dividendYield) {}

        Real propFactor() const {
            return _propAmount;
        }

        Real escrowAmount() const {
            return _cashAmount;
        }

        void visit(FixedDividend& cash) {
            _cashAmount += cash.amount() / _propAmount / _dividendYield->discount(cash.date()) * _rateCurve->discount(cash.date());
        }

        void visit(FractionalDividend& prop) {
            _propAmount  *=  (1-prop.rate());
        }

        void visit(Dividend& div) {
        }

    protected:
        Real _propAmount;
        Real _cashAmount;
        Handle<YieldTermStructure> _rateCurve;
        Handle<YieldTermStructure> _dividendYield;

    };

}
#endif
