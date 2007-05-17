/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/optimization/lmdif.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <boost/bind.hpp>

namespace QuantLib {

    LevenbergMarquardt::LevenbergMarquardt(Real epsfcn,
                                           Real xtol,
                                           Real gtol)
    : info_(0), epsfcn_(epsfcn), xtol_(xtol), gtol_(gtol) {}

    Integer LevenbergMarquardt::getInfo() const {
        return info_;
    }

    EndCriteria::Type LevenbergMarquardt::minimize(Problem& P,
                                                   const EndCriteria& endCriteria) {
        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();
        Array x_ = P.currentValue();
        currentProblem_ = &P;
        initCostValues_ = P.costFunction().values(x_);
        int m = initCostValues_.size();
        int n = x_.size();
        boost::scoped_array<double> xx(new double[n]);
        std::copy(x_.begin(), x_.end(), xx.get());
        boost::scoped_array<double> fvec(new double[m]);
        boost::scoped_array<double> diag(new double[n]);
        int mode = 1;
        double factor = 1;
        int nprint = 0;
        int info = 0;
        int nfev =0;
        boost::scoped_array<double> fjac(new double[m*n]);
        int ldfjac = m;
        boost::scoped_array<int> ipvt(new int[n]);
        boost::scoped_array<double> qtf(new double[n]);
        boost::scoped_array<double> wa1(new double[n]);
        boost::scoped_array<double> wa2(new double[n]);
        boost::scoped_array<double> wa3(new double[n]);
        boost::scoped_array<double> wa4(new double[m]);
        // call lmdif to minimize the sum of the squares of m functions
        // in n variables by the Levenberg-Marquardt algorithm.
        QuantLib::MINPACK::LmdifCostFunction lmdifCostFunction = 
            boost::bind(&LevenbergMarquardt::fcn, this, _1, _2, _3, _4, _5);
        QuantLib::MINPACK::lmdif(m, n, xx.get(), fvec.get(),
                                 static_cast<double>(endCriteria.functionEpsilon()),
                                 static_cast<double>(xtol_),
                                 static_cast<double>(gtol_),
                                 static_cast<int>(endCriteria.maxIterations()),
                                 static_cast<double>(epsfcn_),
                                 diag.get(), mode, factor,
                                 nprint, &info, &nfev, fjac.get(),
                                 ldfjac, ipvt.get(), qtf.get(),
                                 wa1.get(), wa2.get(), wa3.get(), wa4.get(),
                                 lmdifCostFunction);
        info_ = info;
        // check requirements & endCriteria evaluation
        QL_REQUIRE(info != 0, "MINPACK: improper input parameters");
        //QL_REQUIRE(info != 6, "MINPACK: ftol is too small. no further "
        //                               "reduction in the sum of squares "
        //                               "is possible.");
        if (info != 6) ecType = QuantLib::EndCriteria::StationaryFunctionValue;
        //QL_REQUIRE(info != 5, "MINPACK: number of calls to fcn has "
        //                               "reached or exceeded maxfev.");
        endCriteria.checkMaxIterations(nfev, ecType);
        QL_REQUIRE(info != 7, "MINPACK: xtol is too small. no further "
                                       "improvement in the approximate "
                                       "solution x is possible.");
        QL_REQUIRE(info != 8, "MINPACK: gtol is too small. fvec is "
                                       "orthogonal to the columns of the "
                                       "jacobian to machine precision.");
        // set problem
        std::copy(xx.get(), xx.get()+n, x_.begin());
        P.setCurrentValue(x_);

        return ecType;
    }

    void LevenbergMarquardt::fcn(int, int n, double* x, double* fvec, int*) {
        Array xt(n);
        std::copy(x, x+n, xt.begin());
        // constraint handling needs some improvement in the future:
        // starting point should not be close to a constraint violation
        if (currentProblem_->constraint().test(xt)) {
            const Array& tmp = currentProblem_->values(xt);
            std::copy(tmp.begin(), tmp.end(), fvec);
        } else {
            std::copy(initCostValues_.begin(), initCostValues_.end(), fvec);
        }
    }

}

