//fastSS : Fast Sudoku Solver
/*MIT License

Copyright (c) 2017 E Bhavani Shankar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "solve.h"
#include "matrix.h"
#include <iostream>

typedef unsigned short  us;
typedef unsigned int ui;
typedef unsigned long ul;

using std::cout;
using std::endl;
using std::ifstream;

solve_sudoku::solve_sudoku() {
  init_dlx();
}


void solve_sudoku::init_dlx() {
    root      = new link();
    this_head = new link();

    root->right = this_head;
    this_head->left = root;
    this_head->name = 0;
    this_head->size = DIGITS;
    coln_headers[0] = this_head;

    // create and link all column headers
    for (us i = 1; i < COLNS; ++i) {
        tmp_head = new link();
        tmp_head->name = i;
        tmp_head->size = DIGITS;
        this_head->right = tmp_head;
        tmp_head->left = this_head;
        this_head = tmp_head;
        coln_headers[i] = this_head;
    }

    coln_headers[COLNS-1]->right = root;
    root->left = coln_headers[COLNS-1];

    // store immediately above and left
    node above[COLNS] = {};
    node before[ROWS] = {};

    for (us i = 0; i < ROWS; ++i) {
        for (us j = 0; j < COLNS; ++j) {
            if (m[i][j]) {
                tmp_row = new link();
                tmp_row->name = i;
                tmp_row->coln = coln_headers[j];

                // vertical links
                if (above[j] == NULL) {
                    coln_headers[j]->down = tmp_row;
                    tmp_row->up = coln_headers[j];
                }
                else {
                    above[j]->down = tmp_row;
                    tmp_row->up = above[j];
                }
                above[j] = tmp_row;

                // horizontal links
                if (before[i] != NULL) {
                    before[i]->right = tmp_row;
                    tmp_row->left = before[i];
                }
                before[i] = tmp_row;
            }
        }
    }

    // make colns circular
    for (us i = 0; i < COLNS; ++i) {
        coln_headers[i]->up = above[i];
        above[i]->down = coln_headers[i];
    }

    // make rows circular
    for (us i = 0; i < ROWS; ++i) {
        node f_row = before[i]->left->left->left;
        before[i]->right = f_row;
        f_row->left = before[i];
    }
}

inline void solve_sudoku::choose_coln() {

    // Minimize branching
    min_coln = root->right;
    min_size = root->right->size;
    for (node cH = root->right; cH != root; cH = cH->right) {

        if (cH->size == 1) {
            min_size = 1;
            min_coln = cH;
            break;
        }

        if (cH->size < min_size) {
            min_coln = cH;
            min_size = cH->size;
        }
    }
}

inline void solve_sudoku::cover(node c) {

    c->left->right = c->right;
    c->right->left = c->left;

    for (node  i = c->down; i != c; i = i->down) {

        node j = i->right;
        node k = j->right;
        node l = k->right;

        j->down->up = j->up;
        k->down->up = k->up;
        l->down->up = l->up;

        j->up->down = j->down;
        k->up->down = k->down;
        l->up->down = l->down;

        --j->coln->size;
        --k->coln->size;
        --l->coln->size;
    }
}


inline void solve_sudoku::uncover(node c) {

    for (node  i = c->up; i != c; i = i->up) {

        node j = i->left;
        node k = j->left;
        node l = k->left;

        ++j->coln->size;
        ++k->coln->size;
        ++l->coln->size;

        j->up->down = j;
        k->up->down = k;
        l->up->down = l;

        j->down->up = j;
        k->down->up = k;
        l->down->up = l;
    }

    c->left->right = c;
    c->right->left = c;
}


void solve_sudoku::search(ul k) {

    if (max_solns <= solutions)
        return;

    if (root->right == root) {
        ++solutions;
        print_solution();
        return;
    }

    choose_coln();
    node c = min_coln;
    cover(c);

    for (node  r = c->down; r != c; r = r->down) {
        //O_k <--- r
        solution[k] = r;

        node j_ = r->right; node k_ = j_->right; node l_ = k_->right;
        cover(j_->coln); cover(k_->coln); cover(l_->coln);

        search(k+1);
        // r <--- O_k
        r = solution[k];
        c = r->coln;

        j_ = r->left; k_ = j_->left; l_ = k_->left;
        uncover(j_->coln); uncover(k_->coln); uncover(l_->coln);
    }
    uncover(c);
    return;
}


void solve_sudoku::print_solution() {

    for (us i = 0; solution[i]; ++i) {
        us r = solution[i]->name;
        us cell = r/DIGITS;
        us digit = r%DIGITS + 1;
        solution_str[cell] = digit + 48;
    }

}


void solve_sudoku::cover_colns(char *puzzle) {

    node tmp = root->out; // immediately linked to root
    for (us cell = 0; cell < CELLS; ++cell) {

        if (puzzle[cell] != '0') {
            us d = puzzle[cell] - '1';

            // indices of covered colns
            us c1 = cell;
            us c2 = cell/DIGITS*DIGITS + CELLS + d;
            us c3 = cell%DIGITS*DIGITS + 2*CELLS + d;
            us c4 = (cell/N - cell/DIGITS*N + cell/(N*DIGITS)*N)*DIGITS + 3*CELLS + d;

            node C1 = coln_headers[c1]; node C2 = coln_headers[c2];
            node C3 = coln_headers[c3]; node C4 = coln_headers[c4];

            cover(C1);  cover(C2);
            cover(C3);  cover(C4);

            solution_str[cell] = puzzle[cell];

            root->out = C4;
            C4->out = C3; C3->out = C2;
            C2->out = C1; C1->out = tmp;
            tmp = C4;
        }
    }
}


void solve_sudoku::restore_colns() {

    for (node c = root->out; c; c = c->out)
        uncover(c);

    solutions = 0;
    root->out = NULL;
}


void solve_sudoku::solve_puzzle(char*& puzzle) {
    cover_colns(puzzle);
    search(0);
	cout << "Searched!" << endl;
	
	for (int i = 0; i < 81; i++)
		puzzle[i] = solution_str[i];
    restore_colns();
	cout << "\n Out of solver!";
}
