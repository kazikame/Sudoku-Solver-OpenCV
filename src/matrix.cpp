#include "stdafx.h"
#include "matrix.h"
#include <iostream>

using std::cout;
using std::endl;

typedef unsigned short us;
typedef unsigned int   ui;
typedef unsigned long  ul;


matrix::matrix() {

    for (ui c = 0; c < CELLS; ++c) {
        for (ui k = 0; k < DIGITS; ++k) {

            us row = c*DIGITS + k;

            m[row][c] = 1;
            m[row][c/DIGITS*DIGITS + CELLS + k] = 1;
            m[row][c%DIGITS*DIGITS + 2*CELLS + k] = 1;
            m[row][(c/N - c/DIGITS*N + c/(N*DIGITS)*N)*DIGITS + 3*CELLS + k] = 1;
        }
    }
}


void matrix::print_m() {
    // print m
    for (ui i = 0; i < ROWS; ++i) {
        for (ui j = 0; j < COLNS; ++j) {
            cout << m[i][j] << " ";
        }
        cout << endl;
    }
}
