// Copyright 2021 Demon1747 Specdok117@mail.ru

#include "convert.h"

#include <cstring>

std::vector<uint8_t> num_to_bytes(size_t number) {
  std::vector<uint8_t> vec(sizeof(number));
  std::memcpy(vec.data(), &number, sizeof(number));
  //  std::copy_n(&number, sizeof(number), vec.begin());
  return vec;
}

size_t bytes_to_num(const std::vector<uint8_t>& bytes) {
  size_t num = 0;
  std::memcpy(&num, bytes.data(), sizeof(size_t));
  return num;
}

std::vector<uint8_t> str_to_bytes(const std::string& input) {
  std::vector<uint8_t> out(input.begin(), input.end());
  return out;
}

std::string bytes_to_str(const std::vector<uint8_t>& input) {
  std::string out(input.begin(), input.end());
  return out;
}

std::vector<uint8_t> operator+(const std::vector<uint8_t>& a,
                               const std::vector<uint8_t>& b) {
  std::vector<uint8_t> output(a);
  output.reserve(a.size() + b.size());
  output.insert(output.end(), b.begin(), b.end());
  return output;
}

