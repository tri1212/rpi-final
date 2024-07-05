#include "math.h"

double sqrt(double n) 
{
    if (n <= 0) return 0.0;

    double x = n;
    double y = 1;
    
    while (fabs(x - y) > 0.0001) {
        x = (x + y) / 2;
        y = n / x;
    }
    return x;
}

double pow(double base, int exp)
{
    double result = 1.0;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

int round(double num) 
{
    int whole = (int)num;
    
    return (num - whole >= 0.5) ? whole + 1 : whole;
}

int ceil(double n)
{
    int whole = (int)n;

    return n > whole ? whole + 1 : whole;
}

int max(int a, int b) 
{
    return a > b ? a : b;
}

int min(int a, int b) 
{
    return a < b ? a : b;
}

int abs(int n)
{
    return n < 0 ? n * -1 : n;
}

double fabs(double n)
{
    return n < 0 ? n * -1.0 : n;
}

double sin(double x)
{
    if (x == 0)
        return 0.0;

    double result = 0.0;
    int sign = 1;
    double term = x;
    double term_squared = x * x;
    for (int n = 1; n <= 10; n++)
    {
        result += sign * term;
        sign *= -1;
        term *= term_squared / ((2 * n) * (2 * n + 1));
    }
    return result;
}

double cos(double x)
{
    if (x == 0)
        return 1.0;

    double result = 0.0;
    int sign = 1;
    double term = 1.0;
    double term_squared = x * x;
    for (int n = 0; n <= 10; n++)
    {
        result += sign * term;
        sign *= -1;
        term *= term_squared / ((2 * n + 1) * (2 * n + 2));
    }
    return result;
}

double atan(double x) {
    if (x == 0)
        return 0.0;
    else if (x > 1)
        return M_PI / 2 - atan(1 / x);
    else if (x < -1)
        return -(M_PI / 2 - atan(-1 / x));

    const int max_iters = 100;
    double sum = 0;
    double term = x;
    double term_squared = x * x;
    for (int i = 1; i <= max_iters && fabs(term) > 1e-6; i += 2) {
        sum += term;
        term *= -term_squared * (i / (i + 2.0));
    }
    return sum;
}

double atan2(double a, double b)
{
    if (b > 0)
        return atan(a / b);
    else if (b < 0 && a >= 0)
        return atan(a / b) + M_PI;
    else if (b < 0 && a < 0)
        return atan(a / b) - M_PI;
    else if (b == 0 && a > 0)
        return M_PI / 2;
    else if (b == 0 && a < 0)
       return 0 - (M_PI / 2);
    return 0.0;
}
