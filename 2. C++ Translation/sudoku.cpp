#include <iostream>
#include <cassert>
#include <fstream>
#include <chrono>
#include <utility>
#include "sudoku.hpp"

using std::cout;
using std::endl;
using std::move;
using std::ifstream;
using std::chrono::high_resolution_clock;
using std::chrono::duration;

/***************************************************************************************
                                    HELPER FUNCTIONS
***************************************************************************************/

/*
 * def cross(A, B):
 *     return [a+b for a in A for b in B]
 */
vector<string> cross(const string &A, const string &B) {
    vector<string> cross;
    string sq;
    for (const auto &a : A) {
        for (const auto &b : B) {
            sq += a;
            sq += b;
            cross.push_back(move(sq));
        }
    }
    return cross;
}

/*
 * A helper function to check whether a string vector contains a particular string
 */
bool vector_contains(const vector<string> &vec, const string &s) {
    for (const auto &i : vec) {
        if (i == s) {
            return true;
        }
    }
    return false;
}

/*
 * A helper function to check whether a string contains a particular substring
 */
bool string_contains(const string &str, const string &substring) {
    if (str.find(substring) != string::npos) {
        return true;
    } else {
        return false;
    }
}

/*
 * A helper function to erase a substring from a string
 */
string string_eliminate(string str, const string &substring) {
    int position = str.find(substring);
    if (position == string::npos) {
        return str;
    } else {
        return str.replace(position, substring.length(), "");
    }
}

/*
 * A helper function to center a string inside a given width
 */
string center_string(string str, int width) {
    int leading_spaces = (width - str.length()) / 2;
    int trailing_spaces = width - str.length() - leading_spaces;
    str.insert(0, leading_spaces, ' ');
    str.append(trailing_spaces, ' ');
    return str;
}

/*
 * A helper function to find the maximum length of the values in the grid
 */
int find_max_length(const map<string, string> &values) {
    int max_length = 0;
    for (const auto &i : values) {
        if ((i.second).length() > max_length) {
            max_length = (i.second).length();
        }
    }
    return max_length;
}

/*
 * A helper function to find the square with the minimum number of possibilities
 */
string find_min_possibilities(const map<string, string> &values, int size) {
    int min_length = size*size + 1;
    string min_square;
    for (const auto &i : values) {
        if ((i.second).length() < min_length && (i.second).length() > 1) {
            min_length = (i.second).length();
            min_square = i.first;
        }
    }
    return min_square;
}

/*
 * A helper function to check if a grid is solved
 */
bool all_ones(const map<string, string> &values) {
    for (const auto &i : values) {
        if ((i.second).length() != 1) {
            return false;
        }
    }
    return true;
}

/*
 * A helper function to create a horizontal line on a sudoku grid
 * given a single column width
 */
string create_line(int width, int size) {
    string line;
    for (int i = 0; i < size*size; ++i) {
        for (int j = 0; j < width; ++j) {
            line += '-';
        }
        if ((i + 1) % size == 0 && (i + 1) < size*size) {
            line += '+';
        }
    }
    return line;
}

/*
 * This function displays the values in a hash map as a sudoku grid
 *
 * def display(values):
 *     "Display these values as a 2-D grid."
 *     width = 1+max(len(values[s]) for s in squares)
 *     line = '+'.join(['-'*(width*3)]*3)
 *     for r in rows:
 *         print ''.join(values[r+c].center(width)+('|' if c in '36' else '') for c in cols)
 *         if r in 'CF':
 *             print line
 *     print
 */
void sudoku::display(map<string, string> &values) {
    if (values["A1"] == "false") {
        cout << "**NO SOLUTION IS FOUND**\n" << endl;
        return;
    }

    int width = 1 + find_max_length(values);
    string line = create_line(width, size);

    for (int i = 0; i < rows.length(); ++i) {
        for (int j = 0; j < cols.length(); ++j) {

            cout << center_string(values[squares[i * size*size + j]], width);

            if ((j + 1) % size == 0 && (j + 1) < size*size) {
                cout << '|';
            }
        }
        cout << '\n';

        if ((i + 1) % size == 0 && (i + 1) < size*size) {
            cout << line << '\n';
        }
    }
    cout << endl;
}

/*
 * This function converts a string of values into a map of {square: char}
 * with '0' or '.' for empties.
 *
 * def grid_values(grid):
 *     "Convert grid into a dict of {square: char} with '0' or '.' for empties."
 *     chars = [c for c in grid if c in digits or c in '0.']
 *     assert len(chars) == 81
 *     return dict(zip(squares, chars))
 */
void sudoku::grid_values(const string &grid, map<string, string> &grid_val) {
    vector<string> chars;
    for (int i = 0; i < grid.length(); ++i) {
        if (string_contains(digits, grid.substr(i, 1))) {
            chars.push_back(grid.substr(i, 1));
        } else if (string_contains("0.", grid.substr(i, 1))) {
            chars.push_back(".");
        }
    }
    assert(chars.size() == squares.size());
    for (int i = 0; i < squares.size(); ++i) {
        grid_val[squares[i]] = chars[i];
    }
}

/***************************************************************************************
                            INITIALIZATION OF MEMBER VARIABLES
***************************************************************************************/

sudoku::sudoku() {
    size = 3;
    digits = "123456789";
    rows = "ABCDEFGHI";
    cols = digits;
    squares = cross(rows, cols);

    /*
     * string rows_threes[3] = {"ABC", "DEF", "GHI"};
     * string cols_threes[3] = {"123", "456", "789"};
     */
    vector<string> rows_threes;
    vector<string> cols_threes;
    for (int i = 0; i < size*size; i += size) {
        rows_threes.push_back(rows.substr(i, size));
        cols_threes.push_back(cols.substr(i, size));
    }

    /*
     * Creating vector<vector<string> > unitlist:
     *
     * unitlist = ([cross(rows, c) for c in cols] +
     *             [cross(r, cols) for r in rows] +
     *             [cross(rs, cs) for rs in ('ABC','DEF','GHI') for cs in ('123','456','789')])
     */
    for (int i = 0; i < rows.length(); ++i) {
        unitlist.push_back(cross(rows.substr(i, 1), digits));
    }
    for (int i = 0; i < digits.length(); ++i) {
        unitlist.push_back(cross(rows, digits.substr(i, 1)));
    }
    for (const auto &rs : rows_threes) {
        for (const auto &cs : cols_threes) {
            unitlist.push_back(cross(rs, cs));
        }
    }

    /*
     * Creating map<string, vector<vector<string> > > units:
     *
     * units = dict((s, [u for u in unitlist if s in u]) for s in squares)
     *
     *      can also be expressed as:
     *
     * units = {}
     * for s in squares:
     *     units[s] = []
     *     for u in unitlist:
     *         if s in u:
     *             units[s].append(u)
     */
    for (const auto &s : squares) {
        for (const auto &u : unitlist) {
            if (vector_contains(u, s)) {
                units[s].push_back(u);
            }
        }
    }

    /*
     * Creating map<string, set<string> > peers:
     *
     * peers = dict((s, set(sum(units[s],[]))-set([s])) for s in squares)
     *
     *      can also be expressed as:
     *
     * peers = {}
     * for s in squares:
     *     peers[s] = set()
     *     for u in units[s]:
     *         for square in u:
     *             peers[s].add(square)
     *     peers[s].remove(s)
     */
    for (const auto &s : squares) {
        for (const auto &u : units[s]) {
            for (const auto &square : u) {
                peers[s].insert(square);
            }
        }
        peers[s].erase(s);
    }

    // Since there is no input, initialize an empty grid
    // If the grid is empty then each square can be any number from 1 to 9
    for (const auto &s : squares) {
        input[s] = ".";
        solution[s] = digits;
    }

    parsed_grid = solution;
}

/***************************************************************************************
                            PART I: CONSTRAINT PROPAGATION
***************************************************************************************/

/*
 * This function uses constraint propagation to compute all the possible numbers
 * in each square based on the initial input. You may think of 'propagation' as
 * a sort of chain reaction or a ripple effect where assigning or eliminating a number
 * may lead to the elimination of many other numbers because of the constraints.
 *
 * def parse_grid(grid):
 *     """Convert grid to a dict of possible values, {square:digits}, or
 *     return False if a contradiction is detected."""
 *     ## To start, every square can be any digit; then assign values from the grid.
 *     values = dict((s, digits) for s in squares)
 *     for s,d in grid_values(grid).items():
 *         if d in digits and not assign(values, s, d):
 *             return False ## (Fail if we can't assign d to square s.)
 *     return values
 */
bool sudoku::parse_grid(const string &grid) {
    for (const auto &s : squares) {
        solution[s] = digits;
    }
    grid_values(grid, input);
    for (const auto &i : input) {
        if (string_contains(digits, i.second) && !assign(solution, i.first, i.second)) {
            solution["A1"] = "false";
            parsed_grid = solution;
            return false;
        }
    }
    parsed_grid = solution;
    return true;
}

/*
 * A function to assign a number to a square in the sudoku board
 * Assigning a number is equivalent to eliminating all the other possible numbers
 *
 * def assign(values, s, d):
 *     """Eliminate all the other values (except d) from values[s] and propagate.
 *     Return values, except return False if a contradiction is detected."""
 *     other_values = values[s].replace(d, '')
 *     if all(eliminate(values, s, d2) for d2 in other_values):
 *         return values
 *     else:
 *         return False
 */
bool sudoku::assign(map<string, string> &values, const string &s, const string &d) {
    string other_values = string_eliminate(values[s], d);
    for (int i = 0; i < other_values.length(); ++i) {
        if (!eliminate(values, s, other_values.substr(i, 1))) {
            return false;
        }
    }
    return true;
}

/*
 * def eliminate(values, s, d):
 *     """Eliminate d from values[s]; propagate when values or places <= 2.
 *     Return values, except return False if a contradiction is detected."""
 *     if d not in values[s]:
 *         return values ## Already eliminated
 *     values[s] = values[s].replace(d,'')
 *     ## (1) If a square s is reduced to one value d2, then eliminate d2 from the peers.
 *     if len(values[s]) == 0:
 *         return False ## Contradiction: removed last value
 *     elif len(values[s]) == 1:
 *         d2 = values[s]
 *         if not all(eliminate(values, s2, d2) for s2 in peers[s]):
 *             return False
 *     ## (2) If a unit u is reduced to only one place for a value d, then put it there.
 *     for u in units[s]:
 *         dplaces = [s for s in u if d in values[s]]
 *         if len(dplaces) == 0:
 *             return False ## Contradiction: no place for this value
 *         elif len(dplaces) == 1:
 *             ## d can only be in one place in unit; assign it there
 *             if not assign(values, dplaces[0], d):
 *                 return False
 *     return values
 */
bool sudoku::eliminate(map<string, string> &values, const string &s, const string &d) {
    if (!string_contains(values[s], d)) {
        return true;
    }
    values[s] = string_eliminate(values[s], d);
    if (values[s].length() == 0) {
        return false;
    } else if (values[s].length() == 1) {
        for (const auto &s2 : peers[s]) {
            if (!eliminate(values, s2, values[s])) {
                return false;
            }
        }
    }
    vector<string> dplaces;
    for (const auto &u : units[s]) {
        for (const auto &s : u) {
            if (string_contains(values[s], d)) {
                dplaces.push_back(s);
            }
        }
        if (dplaces.size() == 0) {
            return false;
        } else if (dplaces.size() == 1) {
            if (!assign(values, dplaces[0], d)) {
                return false;
            }
        }
        dplaces.clear();
    }
    return true;
}

/***************************************************************************************
                            PART II: RECURSIVE SEARCH
***************************************************************************************/

/*
 * After some values have been eliminated, if there are still unsolved squares,
 * use a trial-and-error algorithm to solve the remaining squares
 *
 * def search(values):
 *     "Using depth-first search and propagation, try all possible values."
 *     if values is False:
 *         return False ## Failed earlier
 *     if all(len(values[s]) == 1 for s in squares):
 *         return values ## Solved!
 *     ## Chose the unfilled square s with the fewest possibilities
 *     n,s = min((len(values[s]), s) for s in squares if len(values[s]) > 1)
 *     for d in values[s]:
 *         result = search(assign(values.copy(), s, d))
 *         if result: return result
 */
bool sudoku::search(map<string, string> &values) {
    if (values["A1"] == "false") {
        return false;
    } else if (all_ones(values)) {
        return true;
    }

    string s = find_min_possibilities(values, size);
    map<string, string> copy = values;

    for (int i = 0; i < values[s].length(); ++i) {
        if (assign(values, s, values[s].substr(i, 1))) {
            if (search(values)) {
                return true;
            }
        }
        values = copy;
    }

    return false;
}

bool sudoku::solve(const string &grid) {
    parse_grid(grid);
    if (!search(solution)) {
        solution["A1"] = "false";
        return false;
    }
    return true;
}

/***************************************************************************************
                                    SYSTEM TEST
***************************************************************************************/

void sudoku::test() {
    assert(squares.size() == 81);
    assert(unitlist.size() == 27);
    for (const auto &u : units) {
        assert(u.second.size() == 3);
        for (const auto &unit : u.second) {
            assert(unit.size() == 9);
        }
    }
    for (const auto &p : peers) {
        assert(p.second.size() == 20);
    }
    cout << "All tests pass." << endl;
}

void sudoku::solve_all(const string &file_name, const string &name, double show_if) {
    string grid;
    vector<bool> results;
    int N, frequency;
    int sum_results = 0;
    double sum_time = 0;
    double max_time = 0;
    double avg_time;
    ifstream myfile(file_name);

    if (myfile.is_open()) {
        while (getline(myfile, grid)) {
            time_solve(grid, results, sum_time, max_time, show_if);
        }
        myfile.close();
    } else {
        cout << "Unable to open file." << endl;
        return;
    }

    for (const auto &r : results) {
        if (r) {
            ++sum_results;
        }
    }

    N = results.size();
    avg_time = sum_time / N;
    frequency = N / sum_time;

    printf("Solved %d of %d %s puzzles (avg %.2f secs (%d Hz), max %.2f secs).\n",
            sum_results, N, name.c_str(), avg_time, frequency, max_time);
    fflush(stdout);
}

void sudoku::time_solve(const string &grid, vector<bool> &results, double &sum_time, double &max_time, double show_if) {
    auto t1 = high_resolution_clock::now();
    solve(grid);
    auto t2 = high_resolution_clock::now();
    duration<double> elapsed = t2 - t1;

    sum_time += elapsed.count();
    if (elapsed.count() > max_time) {
        max_time = elapsed.count();
    }

    results.push_back(solved(solution));

    if (elapsed.count() > show_if && show_if != 0) {
        cout << '\n';
        display(input);
        display(solution);
        printf("(%.2f seconds)\n\n", elapsed.count());
        fflush(stdout);
    }
}

bool sudoku::solved(map<string, string> &values) {
    if (values["A1"] == "false") {
        return false;
    }

    set<string> set_digits;
    for (int i = 0; i < digits.length(); ++i) {
        set_digits.insert(digits.substr(i, 1));
    }

    set<string> set_val;
    for (const auto &unit : unitlist) {
        for (const auto &s : unit) {
            set_val.insert(values[s]);
        }
        if (set_val != set_digits) {
            return false;
        }
        set_val.clear();
    }

    return true;
}

/***************************************************************************************
                                ADDITIONAL FUNCTIONS
***************************************************************************************/

void sudoku::display_input() {
    display(input);
}

void sudoku::display_parsed_grid() {
    display(parsed_grid);
}

void sudoku::display_solution() {
    display(solution);
}

map<string, string> sudoku::get_input() {
    return input;
}

map<string, string> sudoku::get_parsed_grid() {
    return parsed_grid;
}

map<string, string> sudoku::get_solution() {
    return solution;
}