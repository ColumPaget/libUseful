/*
Copyright (c) 2015 Colum Paget <colums.projects@googlemail.com>
* SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef LIBUSEFUL_MEASURE_H
#define LIBUSEFUL_MEASURE_H

#include "defines.h"
#include "includes.h"

/*
Functions relating to convertion between SI Units/ Metric and and IEC units. So, is kilo 1024, or 1000?
*/

#ifdef __cplusplus
extern "C" {
#endif


//a simple power function included to allow libUseful to build without needing libmath/libm
double ToPower(double val, double power);

//Base will be 1000 for metric and 1024 for IEC
//Precision will be 0, 1, 2, 3, 4 depending on how many decimal places you want
//Negative precisions mean 'prefer the higher power if it has less than this many decimal places'
//so -1 would render 750k as 0.7M but 75k as 75.0k, -2 would render 750k as 0.75M and 75k as 0.07M but 7k as 7k
const char *ToSIUnit(double Value, int Base, int Precision);
#define ToIEC(Value, Precision) (ToSIUnit((Value), 1024, Precision))
#define ToMetric(Value, Precision) (ToSIUnit((Value), 1000, Precision))

//Convert to and from metric
double FromSIUnit(const char *Data, int Base);
#define FromIEC(Value, Precision) (FromSIUnit((Value), 1024))
#define FromMetric(Value, Precision) (FromSIUnit((Value), 1000))


#ifdef __cplusplus
}
#endif


#endif
