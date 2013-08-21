#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>

#include "dispatch.h"
#include "storage_types.h"

using value_id_t = std::int64_t;

class ADictionary : public Typed {
 public:

};

template <typename T>
class BaseDictionary : public ADictionary {
 public:
  virtual std::size_t size() const = 0;
  virtual value_id_t add(const T&) = 0;
  virtual T get(const value_id_t&) const = 0;
};

template <typename T>
class OrderedDictionary final : public BaseDictionary<T> {
public:
  static type_id_t typeId;

  std::size_t size() const { return _values.size(); }

  value_id_t add(const T& value) override {
    if (value <= _values.back()) {
      throw std::runtime_error("may only insert linearly");
    }
    _values.push_back(value);
    return _values.size() - 1;
  }

  T get(const value_id_t& value_id) const override {
    return _values.at(value_id);
  }


private:
  std::vector<T> _values;
};

template <typename T>
type_id_t OrderedDictionary<T>::typeId = typeid(OrderedDictionary<T>).hash_code();

template <typename T>
class UnorderedDictionary final : public BaseDictionary<T> {
public:
  static type_id_t typeId;

  std::size_t size() const { return _values.size(); }

  value_id_t add(const T& value) {
    auto iter = _index.insert(std::make_pair(value, _values.size() + 1));
    if (iter.second) {
      _values.push_back(value);
    }
    return iter.first->second;
  }

  T get(const value_id_t& value_id) const override {
    return _values.at(value_id);
  }
private:
  std::vector<T> _values;
  std::unordered_map<T, value_id_t> _index;
};

template <typename T>
type_id_t UnorderedDictionary<T>::typeId = typeid(UnorderedDictionary<T>).hash_code();

class AStorage : public Typed {
 public:
  ~AStorage();
  virtual void set(std::size_t x, value_id_t vid) = 0;
  virtual value_id_t get(std::size_t x) const = 0;
};

class FixedStorage final : public AStorage {
 public:
  static  type_id_t typeId;
  explicit FixedStorage(std::size_t len);
  void set(std::size_t x, value_id_t vid);
  value_id_t get(std::size_t x) const;
  const value_id_t& getRef(std::size_t x) const;
 private:
  std::vector<value_id_t> _values;
};



class BitStorage final : public AStorage {
 public:
  static  type_id_t typeId;
  explicit BitStorage(std::size_t len);
  void set(std::size_t x, value_id_t vid);
  value_id_t get(std::size_t x) const;
 private:
  std::vector<value_id_t> _values;
};


class ATable : public Typed {
 public:
  ~ATable();
};

class Table final : public ATable {
public:
  static  type_id_t typeId;
};



class RawTable final : public ATable {
  static  type_id_t typeId;
};
