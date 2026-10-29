// Replaces the global allocation functions so tests can check that audio
// processing is allocation-free.

#include <cstdlib>
#include <new>

#include "test_util.h"

namespace {

thread_local bool counting = false;
thread_local int allocations = 0;

void* Allocate(std::size_t size) {
  if (counting) ++allocations;
  if (void* memory = std::malloc(size == 0 ? 1 : size)) return memory;
  throw std::bad_alloc();
}

}  // namespace

void* operator new(std::size_t size) { return Allocate(size); }
void* operator new[](std::size_t size) { return Allocate(size); }
void operator delete(void* memory) noexcept { std::free(memory); }
void operator delete[](void* memory) noexcept { std::free(memory); }
void operator delete(void* memory, std::size_t) noexcept { std::free(memory); }
void operator delete[](void* memory, std::size_t) noexcept {
  std::free(memory);
}

namespace voice_changer::testing {

AllocationCounter::AllocationCounter() {
  allocations = 0;
  counting = true;
}

AllocationCounter::~AllocationCounter() { counting = false; }

int AllocationCounter::count() const { return allocations; }

}  // namespace voice_changer::testing
