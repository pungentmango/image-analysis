/*
    file linreg.cpp
*/
#include <math.h>
#include <float.h>
#include "linreg.h"
#include <stdio.h>
#include <sys/stat.h>

LinearRegression::LinearRegression(Point2D *p, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addPoint(p[i]);
}

LinearRegression::LinearRegression(float *x, float *y, long size)
{
    long i;
    a = b = sumX = sumY = sumXsquared = sumYsquared = sumXY = 0.0;
    n = 0L;

    if (size > 0L) // if size greater than zero there are data arrays
        for (n = 0, i = 0L; i < size; i++)
            addXY(x[i], y[i]);
}

void LinearRegression::addXY(const float& x, const float& y)
{
    n++;
    sumX += x;
    sumY += y;
    sumXsquared += x * x;
    sumYsquared += y * y;
    sumXY += x * y;
    Calculate();
}

void LinearRegression::Calculate()
{
    if (haveData())
    {
        if (fabs( float(n) * sumXsquared - sumX * sumX) > DBL_EPSILON)
        {
            b = ( float(n) * sumXY - sumY * sumX) /
                ( float(n) * sumXsquared - sumX * sumX);
            a = (sumY - b * sumX) / float(n);

            float sx = b * ( sumXY - sumX * sumY / float(n) );
            float sy2 = sumYsquared - sumY * sumY / float(n);
            float sy = sy2 - sx;

            coefD = sx / sy2;
            coefC = sqrt(coefD);
            stdError = sqrt(sy / float(n - 2));
        }
        else
        {
            a = b = coefD = coefC = stdError = 0.0;
        }
    }
}

float* LinearRegression::linRegVals(LinearRegression& lr)
{
    float* linRegArray = new float[4];
    linRegArray[0] = lr.getA();
    linRegArray[1] = lr.getB();
    linRegArray[2] = lr.getStdErrorEst();
    linRegArray[3] = lr.getCoefCorrel();
    return linRegArray;
}
