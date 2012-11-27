#ifndef artdaq_DAQrate_detail_FragCounter_hh
#define artdaq_DAQrate_detail_FragCounter_hh

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace artdaq {
  namespace detail {
    class FragCounter;
  }
}

class artdaq::detail::FragCounter {
public:
  explicit FragCounter(size_t nSlots, size_t offset = 0);
  void incSlot(size_t slot);
  void incSlot(size_t slot, size_t inc);

  size_t nSlots() const;
  size_t count() const;
  size_t slotCount(size_t slot) const;

private:
  size_t computedSlot_(size_t slot) const;

  typedef std::lock_guard<std::mutex> lock_t;
  class SlotLock;

  size_t offset_;
  std::vector<size_t> receipts_;
  mutable std::mutex protectTotal_;
  mutable std::vector<std::mutex> protectSlots_;
};

class artdaq::detail::FragCounter::SlotLock {
public:
  SlotLock(size_t computed_slot,
           std::mutex & protectTotal,
           std::vector<std::mutex> & protectSlots)
    :
    total_lock_(protectTotal),
    slot_lock_(protectSlots[computed_slot])
    {
    }
private:
  lock_t total_lock_;
  lock_t slot_lock_;
};

inline
artdaq::detail::FragCounter::
FragCounter(size_t nSlots, size_t offset)
  :
  offset_(offset),
  receipts_(nSlots, 0),
  protectTotal_(),
  protectSlots_(nSlots)
{
}

inline
void
artdaq::detail::FragCounter::
incSlot(size_t slot)
{
  size_t cs(computedSlot_(slot));
  SlotLock slot_lock(cs, protectTotal_, protectSlots_);
  ++receipts_[cs];
}

inline
void
artdaq::detail::FragCounter::
incSlot(size_t slot, size_t inc)
{
  size_t cs(computedSlot_(slot));
  SlotLock slot_lock(cs, protectTotal_, protectSlots_);
  receipts_[cs] += inc;
}

inline
size_t
artdaq::detail::FragCounter::
nSlots() const
{
  return receipts_.size();
}

inline
size_t
artdaq::detail::FragCounter::
count() const
{
  lock_t total_lock(protectTotal_);
  return
    std::accumulate(receipts_.begin(),
                    receipts_.end(),
                    0);
}

inline
size_t
artdaq::detail::FragCounter::
slotCount(size_t slot) const
{
  size_t cs(computedSlot_(slot));
  SlotLock slot_lock(cs, protectTotal_, protectSlots_);
  return receipts_[cs];
}

#endif /* artdaq_DAQrate_detail_FragCounter_hh */
