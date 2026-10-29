#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "test_util.h"

namespace voice_changer {
namespace {

using testing::AllocationCounter;

// Guards the allocation-free tests against passing because nothing is counted.
TEST(AllocationCounterTest, CountsHeapAllocations) {
  AllocationCounter allocations;
  auto value = std::make_unique<int>(1);
  std::vector<float> buffer(16);

  // At least two: debug standard libraries may add bookkeeping allocations.
  EXPECT_GE(allocations.count(), 2);
}

}  // namespace
}  // namespace voice_changer
