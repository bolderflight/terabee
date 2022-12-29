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

#include "terabee.h"  // NOLINT
#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstddef>
#include <cstdint>
#include "core/core.h"
#endif

namespace bfs {

constexpr uint8_t Terabee::HEADER_[];
constexpr uint8_t Terabee::DISABLE_STREAMING_[];
constexpr uint8_t Terabee::ENABLE_STREAMING_[];
constexpr uint8_t Terabee::ENABLE_STREAMING_RESP_[];
constexpr uint8_t Terabee::TOWER_MODE_[];
constexpr uint8_t Terabee::SEQ_MODE_[];
constexpr uint8_t Terabee::SIM_MODE_[];
constexpr uint8_t Terabee::MODE_RESP_[];
constexpr uint8_t Terabee::RATE_ASAP_[];
constexpr uint8_t Terabee::RATE_50HZ_[];
constexpr uint8_t Terabee::RATE_100HZ_[];
constexpr uint8_t Terabee::RATE_250HZ_[];
constexpr uint8_t Terabee::RATE_500HZ_[];
constexpr uint8_t Terabee::RATE_600HZ_[];
constexpr uint8_t Terabee::RATE_RESP_[];
constexpr uint8_t Terabee::CRC8_TABLE_[];

bool Terabee::Begin() {
  uart_->begin(BAUD_);
  while (tries_ < MAX_RETRIES_) {
    if (Init()) {
      return true;
    }
    delay(1000);
    tries_++;
  }
  return false;
}

bool Terabee::Init() {
  /* Enable communication */
  uart_->write(ENABLE_STREAMING_, sizeof(ENABLE_STREAMING_));
  if (!ParseCmdResp(ENABLE_STREAMING_RESP_, sizeof(ENABLE_STREAMING_RESP_),
      CMD_TIMEOUT_MS_)) {
    return false;
  }
  /* Operating mode */
  switch (op_mode_) {
    case OP_MODE_TOWER: {
      uart_->write(TOWER_MODE_, sizeof(TOWER_MODE_));
      break;
    }
    case OP_MODE_SEQ: {
      uart_->write(SEQ_MODE_, sizeof(SEQ_MODE_));
      break;
    }
    case OP_MODE_SIMULTANEOUS: {
      uart_->write(SIM_MODE_, sizeof(SIM_MODE_));
      break;
    }
  }
  if (!ParseCmdResp(MODE_RESP_, sizeof(MODE_RESP_), CMD_TIMEOUT_MS_)) {
    return false;
  }
  /* Output rate */
  switch (rate_) {
    case OUTPUT_RATE_ASAP: {
      uart_->write(RATE_ASAP_, sizeof(RATE_ASAP_));
      break;
    }
    case OUTPUT_RATE_50HZ: {
      uart_->write(RATE_50HZ_, sizeof(RATE_50HZ_));
      break;
    }
    case OUTPUT_RATE_100HZ: {
      uart_->write(RATE_100HZ_, sizeof(RATE_100HZ_));
      break;
    }
    case OUTPUT_RATE_250HZ: {
      uart_->write(RATE_250HZ_, sizeof(RATE_250HZ_));
      break;
    }
    case OUTPUT_RATE_500HZ: {
      uart_->write(RATE_500HZ_, sizeof(RATE_500HZ_));
      break;
    }
    case OUTPUT_RATE_600HZ: {
      uart_->write(RATE_600HZ_, sizeof(RATE_600HZ_));
      break;
    }
  }
  if (!ParseCmdResp(RATE_RESP_, sizeof(RATE_RESP_), CMD_TIMEOUT_MS_)) {
    return false;
  }
  return true;
}

bool Terabee::Read(RangeData * const data) {
  if (!data) {return false;}
  new_data_ = false;
  do {
    if (Parse()) {
      for (int8_t i = 0; i < RangeData::MAX_CH; i++) {
        data->sensor[i].updated = mask_ & (0x01 << i);
        if (data->sensor[i].updated) {
          data->sensor[i].range_m = static_cast<float>(dist_mm_[i]) / 1000.0f;
        }
      }
      new_data_ = true;
    }
  } while (uart_->available());
  return new_data_;
}

bool Terabee::Parse() {
  while (uart_->available()) {
    cur_byte_ = uart_->read();
    if (state_ < sizeof(HEADER_)) {
      if (cur_byte_ == HEADER_[state_]) {
        buf_[state_++] = cur_byte_;
      }
    } else if (state_ < MSG_LEN_) {
      buf_[state_++] = cur_byte_;
      if (state_ == MSG_LEN_) {
        if (cur_byte_ == Crc8(0, buf_, MSG_LEN_ - 1)) {
          dist_mm_[0] = (static_cast<uint16_t>(buf_[2]) << 8) | buf_[3];
          dist_mm_[1] = (static_cast<uint16_t>(buf_[4]) << 8) | buf_[5];
          dist_mm_[2] = (static_cast<uint16_t>(buf_[6]) << 8) | buf_[7];
          dist_mm_[3] = (static_cast<uint16_t>(buf_[8]) << 8) | buf_[9];
          dist_mm_[4] = (static_cast<uint16_t>(buf_[10]) << 8) | buf_[11];
          dist_mm_[5] = (static_cast<uint16_t>(buf_[12]) << 8) | buf_[13];
          dist_mm_[6] = (static_cast<uint16_t>(buf_[14]) << 8) | buf_[15];
          dist_mm_[7] = (static_cast<uint16_t>(buf_[16]) << 8) | buf_[17];
          mask_ = buf_[18];
          state_ = 0;
          return true;
        } else {
          state_ = 0;
          return false;
        }
      }
    } else {
      state_ = 0;
    }
  }
  return false;
}

bool Terabee::ParseCmdResp(const uint8_t * resp, const size_t len,
                           const int32_t timeout_ms) {
  resp_state_ = 0;
  resp_time_ = millis();
  while ((millis() - resp_time_) < timeout_ms) {
    while (uart_->available()) {
      if (resp_state_ < len) {
        if (uart_->read() == resp[resp_state_]) {
          resp_state_++;
          if (resp_state_ == len) {return true;}
        }
      } else {
        return false;
      }
    }
  }
  return false;
}

uint8_t Terabee::Crc8(uint8_t crc, uint8_t * data, size_t len) {
  if (!data) {
    return 0;
  }
  crc &= 0xFF;
  while (len--) {
    crc = CRC8_TABLE_[crc ^ *data++];
  }
  return crc;
}

}  // namespace bfs
