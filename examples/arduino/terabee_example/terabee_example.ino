/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2022 Bolder Flight Systems Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the “Software”), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include "terabee.h"

bfs::Terabee range(&Serial4);
bfs::Terabee::RangeData data;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  if (!range.Begin()) {
    Serial.println("Error initializing and configuring sensor");
    while (1) {}
  }
}

void loop() {
  if (range.Read(&data)) {
    for (int8_t i = 0; i < data.MAX_CH; i++) {
      Serial.print("Sensor: ");
      Serial.print(i);
      Serial.print("\tNew Data: ");
      Serial.print(data.sensor[i].updated);
      Serial.print("\tRange (m): ");
      Serial.println(data.sensor[i].range_m);
    }
  }
}
