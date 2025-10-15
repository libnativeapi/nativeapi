#pragma once

#include <atomic>
#include <cstdint>
#include <utility>

namespace nativeapi {

/**
 * Thread-safe ID allocator with type information.
 *
 * Each ID is a 32-bit value: [Type:8 bits][Sequence:24 bits]
 * Provides unique IDs for different object types with thread-safe allocation.
 *
 * ID Structure (32 bits):
 * +------------+--------------------------+
 * |  Type (8)  |    Sequence (24)         |
 * +------------+--------------------------+
 * Bits: 31-24     23-0
 *
 * Field Details:
 * - Type: 8-bit type identifier (1-10, 0 reserved for invalid)
 * - Sequence: 24-bit sequence number (1-16777215, 0 reserved for invalid)
 * - Invalid ID: 0x00000000 (both type and sequence are 0)
 *
 * Example:
 * - Type 1, Sequence 1: 0x01000001
 * - Type 2, Sequence 100: 0x02000064
 * - Type 5, Sequence 1000: 0x050003E8
 *
 * Thread Safety:
 * - All allocation operations are thread-safe using atomic operations
 * - Each type has its own independent sequence counter
 * - Type assignment is thread-safe and happens only once per type
 */
class IdAllocator {
 public:
  using IdType = uint32_t;
  static_assert(sizeof(IdType) == 4, "IdAllocator::IdType must be 32-bit");

  /// Invalid ID value returned on allocation failure
  static constexpr IdType kInvalidId = 0u;

  /// Bit layout specification: [ type:8 | sequence:24 ]
  /// High 8 bits store the type identifier, low 24 bits store the sequence
  /// number
  static constexpr uint32_t kTypeBits = 8;       ///< Number of bits allocated for type information
  static constexpr uint32_t kSequenceBits = 24;  ///< Number of bits allocated for sequence numbers
  static constexpr uint32_t kTypeShift = 24;     ///< Bit shift amount to extract type from ID
  static constexpr uint32_t kTypeMask =
      0xFF000000u;  ///< Bit mask to extract type bits (high 8 bits)
  static constexpr uint32_t kSequenceMask =
      0x00FFFFFFu;  ///< Bit mask to extract sequence bits (low 24 bits)

  /// Valid type value range [1, 10] - supports up to 10 different object types
  /// Type value 0 is reserved for invalid IDs (kInvalidId)
  static constexpr uint32_t kMinTypeValue = 1u;   ///< Minimum valid type value
  static constexpr uint32_t kMaxTypeValue = 10u;  ///< Maximum valid type value

  /// Maximum number of unique IDs per type (2^24 - 1 = 16,777,215)
  /// Sequence 0 is reserved for invalid IDs, so maximum is kSequenceMask
  static constexpr uint32_t kMaxIdsPerType = kSequenceMask;

 private:
  /**
   * Global counter for assigning unique type values to template types.
   */
  static std::atomic<uint32_t>& GetNextTypeCounter() {
    static std::atomic<uint32_t> next_type{kMinTypeValue};
    return next_type;
  }

  /**
   * Gets the sequence counter for template type T.
   */
  template <typename T>
  static std::atomic<uint32_t>& GetCounter() {
    static std::atomic<uint32_t> counter{0};
    return counter;
  }

  /**
   * Gets the unique type value for template type T.
   */
  template <typename T>
  static uint32_t GetTypeValue() {
    // Use a static local to ensure each type gets a unique value assigned only
    // once
    static uint32_t type_value = []() {
      // Atomically get the next available type value
      uint32_t value = GetNextTypeCounter().fetch_add(1, std::memory_order_relaxed);
      if (value > kMaxTypeValue) {
        // Handle overflow - this shouldn't happen in practice with only 10
        // types but provides safety in case of programming errors
        return kInvalidId;
      }
      return value;
    }();
    return type_value;
  }

 public:
  /**
   * Allocates a new unique ID for type T.
   * @return A unique ID, or kInvalidId if allocation failed.
   */
  template <typename T>
  static IdType Allocate() {
    // Get the unique type value assigned to this template type T
    const uint32_t type_value = GetTypeValue<T>();
    if (type_value == kInvalidId) {
      return kInvalidId;  // Type allocation failed (too many types registered)
    }

    // Atomically increment the sequence counter for this type and skip 0.
    // Using relaxed memory ordering is safe here because we only need
    // atomicity, not ordering guarantees between different operations. This
    // provides optimal performance while maintaining thread safety.
    uint32_t sequence = GetCounter<T>().fetch_add(1, std::memory_order_relaxed) + 1u;

    // Check for overflow: if sequence wraps around to 0 in the low 24 bits,
    // treat as allocation failure to avoid returning kInvalidId
    if ((sequence & kSequenceMask) == 0u) {
      // Sequence counter overflowed - this happens after 2^24 allocations
      // Return kInvalidId to indicate allocation failure
      return kInvalidId;
    }

    // Encode the ID: high 8 bits = type, low 24 bits = sequence
    // This creates a unique ID that encodes both type and sequence information
    return (type_value << kTypeShift) | (sequence & kSequenceMask);
  }

  /**
   * Attempts to allocate an ID, returning kInvalidId on failure.
   */
  template <typename T>
  static IdType TryAllocate() {
    return Allocate<T>();
  }

  // ID Query Methods

  /**
   * Extracts the type from an ID.
   */
  static uint32_t GetType(IdType id);

  /**
   * Extracts the sequence number from an ID.
   */
  static uint32_t GetSequence(IdType id);

  /**
   * Checks if an ID is valid.
   */
  static bool IsValid(IdType id);

  /**
   * Extracts both type and sequence from an ID.
   */
  static std::pair<uint32_t, uint32_t> Decompose(IdType id);

  // Counter Management

  /**
   * Gets the current sequence counter for type T.
   */
  template <typename T>
  static uint32_t GetCurrentCount() {
    return GetCounter<T>().load(std::memory_order_relaxed);
  }

  /**
   * Resets the sequence counter for type T.
   */
  template <typename T>
  static void Reset() {
    GetCounter<T>().store(0, std::memory_order_relaxed);
  }

  /**
   * Validates if a type value is within the valid range.
   */
  static constexpr bool IsValidType(uint32_t type_value) {
    return type_value >= kMinTypeValue && type_value <= kMaxTypeValue;
  }
};

}  // namespace nativeapi
