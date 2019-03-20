#ifndef DATA_CONVERT_H
#define DATA_CONVERT_H

void float2byte(byte &integer, byte &decimal, float value){
  integer = value;
  decimal = byte((value-integer)*(value<100?100:1000));
}

void byte2float(byte integer, byte decimal, float &value){
  value = integer + decimal/(integer<100? 100.0: 1000.0);
}

float byte2float(byte integer, byte decimal){
  return integer + decimal/(integer<100? 100.0: 1000.0);
}

#endif