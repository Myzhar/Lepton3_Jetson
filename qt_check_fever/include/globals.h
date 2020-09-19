#ifndef GLOBALS_H
#define GLOBALS_H

#include <QColor>

const double TEMP_MIN_HUMAN = 34.5f;
const double TEMP_WARN = 37.0f;
const double TEMP_FEVER = 37.5f;
const double TEMP_MAX_HUMAN = 42.0f;

const quint8 ALPHA_VAL=175;

const QColor COL_NORM_TEMP = QColor(15,200,15,ALPHA_VAL);
const QColor COL_WARN_TEMP = QColor(200,170,10,ALPHA_VAL);
const QColor COL_FEVER_TEMP = QColor(220,40,40,ALPHA_VAL);

// Hypothesis: sensor is linear in 14bit dynamic
// If the range of the sensor is [0,150] °C in High Gain mode
const double temp_scale_factor = 0.0092; // 150/(2^14-1))

#endif // GLOBALS_H
