#ifndef SOLVE_H
#define SOLVE_H

#include "matrix.h"

typedef unsigned short us;
typedef unsigned int ui;
typedef unsigned long ul;

typedef struct link {
    link  *left;
    link  *right;
    link  *up;
    link  *down;
    link  *coln;
    link  *out;

    us name;
    us size;
}* node;



class solve_sudoku : public matrix{

public:
    explicit solve_sudoku();

    void init_dlx();
    void choose_coln();
    void cover(node );
    void uncover(node );
    void search(ul );
    void print_solution();

    // to solve_sudoku a particular puzzle
    void cover_colns(char* );
    void restore_colns();
    void solve_puzzle(char*& );

    bool quiet = false;

protected:
    us solutions = 0;
    node    solution[INFTY];
    char    solution_str[81]; // To be printed

private:
    us max_solns = 2;

    // variables to parse the toroidal quadruply linked list
    node    root;
    node    this_head;
    node    tmp_head;
    node    tmp_row;

    node    coln_headers[COLNS];

    // other variables
    ul      min_size;
    node    min_coln;


};

#endif // SOLVE_H
