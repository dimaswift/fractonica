//
// Created by Dmitry Popov on 29.01.2026.
//

#ifndef FRACTONICA_MATRIXCLOCK_H
#define FRACTONICA_MATRIXCLOCK_H
#include "IMatrix.h"
#include "IUnixClock.h"

namespace Fractonica {
    class MatrixClock {
        IMatrix *matrix_;
        IUnixClock *clock_;
    public:
        MatrixClock(IMatrix *matrix, IUnixClock *clock);
        void loop();
    };
}

#endif //FRACTONICA_MATRIXCLOCK_H