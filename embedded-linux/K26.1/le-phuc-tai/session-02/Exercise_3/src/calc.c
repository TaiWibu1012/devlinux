#include "calc.h"

double add(double a, double b) { return a + b; }
double subtract(double a, double b) { return a - b; }
double multiply(double a, double b) { return a * b; }
double divide(double a, double b, int *err) {
    if (b == 0.0) {
        if (err) *err = 1;
        return 0.0;
    }
    if (err) *err = 0;
    return a / b;
}