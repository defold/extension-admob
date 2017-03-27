// Copyright 2016 Google Inc. All Rights Reserved.

#ifndef FIREBASE_DATABASE_CLIENT_CPP_SRC_INCLUDE_FIREBASE_DATABASE_MUTABLE_DATA_H_
#define FIREBASE_DATABASE_CLIENT_CPP_SRC_INCLUDE_FIREBASE_DATABASE_MUTABLE_DATA_H_

#include "firebase/internal/common.h"
#include "firebase/variant.h"

namespace firebase {
namespace database {
namespace internal {
class DatabaseReferenceInternal;
class MutableDataInternal;
}  // namespace internal

/// Instances of this class encapsulate the data and priority at a location. It
/// is used in transactions, and it is intended to be inspected and then updated
/// to the desired data at that location.
class MutableData {
 public:
  /// Destructor.
  ~MutableData();

  /// @brief Used to obtain a MutableData instance that encapsulates
  /// the data and priority at the given relative path.
  ///
  /// @param[in] path Path relative to this snapshot's location.
  /// The pointer only needs to be valid during this call.
  ///
  /// @returns MutableData for the Child relative to this location. The memory
  /// will be freed when the Transaction is finished.

  MutableData Child(const char* path);
  /// @brief Used to obtain a MutableData instance that encapsulates
  /// the data and priority at the given relative path.
  ///
  /// @param[in] path Path relative to this snapshot's location.
  ///
  /// @returns MutableData for the Child relative to this location. The memory
  /// will be freed when the Transaction is finished.
  MutableData Child(const std::string& path);

  /// @brief Get all the immediate children of this location.
  ///
  /// @returns The immediate children of this location.
  std::vector<MutableData> children();

  /// @brief Get all the immediate children of this location.
  ///
  /// @returns The immediate children of this location.
  ///
  /// @deprecated Renamed to children().
  FIREBASE_DEPRECATED std::vector<MutableData> GetChildren() {
    return children();
  }

  /// @brief Get the number of children of this location.
  ///
  /// @returns The number of immediate children of this location.
  size_t children_count();

  /// @brief Get the number of children of this location.
  ///
  /// @returns The number of immediate children of this location.
  ///
  /// @deprecated Renamed to children_count().
  FIREBASE_DEPRECATED size_t GetChildrenCount() { return children_count(); }

  /// @brief Get the key name of the source location of this data.
  ///
  /// @note The returned pointer is only guaranteed to be valid during the
  /// transaction.
  ///
  /// @returns Key name of the source location of this data.
  const char* key() const;

  /// @brief Get the key name of the source location of this data.
  ///
  /// @note The returned pointer is only guaranteed to be valid during the
  /// transaction.
  ///
  /// @returns Key name of the source location of this data.
  ///
  /// @deprecated Renamed to key().
  FIREBASE_DEPRECATED const char* GetKey() const { return key(); }

  /// @brief Get the key name of the source location of this data.
  ///
  /// @returns Key name of the source location of this data.
  std::string key_string() const;

  /// @brief Get the key name of the source location of this data.
  ///
  /// @returns Key name of the source location of this data.
  ///
  /// @deprecated Renamed to key_string().
  FIREBASE_DEPRECATED std::string GetKeyString() const { return key_string(); }

  /// @brief Get the value of the data contained at this location.
  ///
  /// @returns The value of the data contained at this location.
  Variant value() const;

  /// @brief Get the value of the data contained at this location.
  ///
  /// @returns The value of the data contained at this location.
  ///
  /// @deprecated Renamed to value().
  FIREBASE_DEPRECATED Variant GetValue() const { return value(); }

  /// @brief Get the priority of the data contained at this snapshot.
  ///
  /// @returns The value of this location's Priority relative to its siblings.
  Variant priority();

  /// @brief Get the priority of the data contained at this snapshot.
  ///
  /// @returns The value of this location's Priority relative to its siblings.
  ///
  /// @deprecated Renamed to priority().
  FIREBASE_DEPRECATED Variant GetPriority() { return priority(); }

  /// @brief Does this MutableData have data at a particular location?
  ///
  /// @param[in] path Path relative to this data's location.
  /// The pointer only needs to be valid during this call.
  ///
  /// @returns True if there is data at the specified location, false if not.
  bool HasChild(const char* path) const;

  /// @brief Does this MutableData have data at a particular location?
  ///
  /// @param[in] path Path relative to this data's location.
  /// @returns True if there is data at the specified location, false if not.
  bool HasChild(const std::string& path);

  /// @brief Sets the data at this location to the given value.
  ///
  /// @param[in] value The value to set this location to. The Variant's type
  /// corresponds to the types accepted by the database JSON:
  /// Null: Deletes this location from the database.
  /// Int64: Inserts an integer value into this location.
  /// Double: Inserts a floating point value into this location.
  /// String: Inserts a string into this location.
  ///         (Accepts both Mutable and Static strings)
  /// Vector: Inserts a JSON array into this location. The elements can be any
  ///         Variant type, including Vector and Map.
  /// Map: Inserts a JSON associative array into this location. The keys must
  ///      be of type String (or Int64/Double which are converted to String).
  ///      The values can be any Variant type, including Vector and Map.
  void set_value(Variant value);

  /// @brief Sets the data at this location to the given value.
  ///
  /// @param[in] value The value to set this location to. The Variant's type
  /// corresponds to the types accepted by the database JSON:
  /// Null: Deletes this location from the database.
  /// Int64: Inserts an integer value into this location.
  /// Double: Inserts a floating point value into this location.
  /// String: Inserts a string into this location.
  ///         (Accepts both Mutable and Static strings)
  /// Vector: Inserts a JSON array into this location. The elements can be any
  ///         Variant type, including Vector and Map.
  /// Map: Inserts a JSON associative array into this location. The keys must
  ///      be of type String (or Int64/Double which are converted to String).
  ///      The values can be any Variant type, including Vector and Map.
  ///
  /// @deprecated Renamed to set_value().
  FIREBASE_DEPRECATED void SetValue(Variant value) { set_value(value); }

  /// @brief Sets the priority of this field, which controls its sort
  /// order relative to its siblings.
  ///
  /// @see firebase::database::DatabaseReference::SetPriority() for information
  /// on how Priority affects the ordering of a node's children.
  ///
  /// @param[in] priority Sort priority for this child relative to its siblings.
  /// The Variant types accepted are Null, Int64, Double, and String. Other
  /// types will return kErrorInvalidVariantType.
  void set_priority(Variant priority);

  /// @brief Sets the priority of this field, which controls its sort
  /// order relative to its siblings.
  ///
  /// @see firebase::database::DatabaseReference::SetPriority() for information
  /// on how Priority affects the ordering of a node's children.
  ///
  /// @param[in] priority Sort priority for this child relative to its siblings.
  /// The Variant types accepted are Null, Int64, Double, and String. Other
  /// types will return kErrorInvalidVariantType.
  ///
  /// @deprecated Renamed to set_priority().
  FIREBASE_DEPRECATED void SetPriority(Variant priority) {
    set_priority(priority);
  }

 private:
  /// @cond FIREBASE_APP_INTERNAL
  friend class internal::DatabaseReferenceInternal;
  friend class internal::MutableDataInternal;
  /// @endcond

  MutableData(internal::MutableDataInternal* internal);

  internal::MutableDataInternal* internal_;
};

}  // namespace database
}  // namespace firebase

#endif  // FIREBASE_DATABASE_CLIENT_CPP_SRC_INCLUDE_FIREBASE_DATABASE_MUTABLE_DATA_H_
