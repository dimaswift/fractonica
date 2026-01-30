//
// Created by Dmitry Popov on 29.01.2026.
//

#include "MatrixClock.h"

namespace Fractonica {
    MatrixClock::MatrixClock(IMatrix *matrix, IUnixClock *clock) : matrix_(matrix), clock_(clock) {

    }

    void MatrixClock::loop() {

    }
}
