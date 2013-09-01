#ifndef INCREMENTAL_ATPG_GREEDY_SET_COVER_H_
#define INCREMENTAL_ATPG_GREEDY_SET_COVER_H_
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <stdint.h>
#include <utility>
#include <log4cxx/logger.h>
#include <boost/heap/fibonacci_heap.hpp>

#include "gtest/gtest_prod.h"
#include "set_cover.h"

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::map;
  using boost::heap::fibonacci_heap;
  using std::make_pair;
  using std::pair;

  struct heap_data {
    string key;
    uint64_t value;
  heap_data(string key, uint64_t value) :
    key(key),
      value(value) {}

    bool operator==(heap_data const & rhs) const {
      return (value == rhs.value && key == rhs.key);
    }
    bool operator<(heap_data const & rhs) const {
      if (value == rhs.value) {
	return key < rhs.key;
      } else {
	return value < rhs.value;
      }
    }
    void operator+(uint64_t change) {
      value += change;
    }
  };

  template<typename charT, typename traits>
    std::basic_ostream<charT, traits> &
    operator<< (std::basic_ostream<charT, traits> &lhs, heap_data const &rhs) {
    return lhs << "heap_data(\"" << rhs.key << "\", " << rhs.value << ")";
  }

  typedef typename fibonacci_heap<heap_data>::handle_type handle_t;

  class GreedySetCover : public SetCover {
  public:
    log4cxx::LoggerPtr greedy_set_cover_logger;
    GreedySetCover();
    GreedySetCover(map<string, SetInfo>* set_infos,
		     vector<RuleInfo>* rule_infos);
    // AddRule inherited from SetCover.
    // Get..ProcessingInfo also.
    // Finds set cover from scratch for rules in latest @rule_infos_.
  void UpdateCover();
 
  protected:
    // Adds all sets in @set_infos_ to @heap_ and populates @handles_. 
    void AddAllSetsToHeap();

    // Given that a new set was just added to cover,
    // updates processing_info for affected rules and sets
    // and gets net @key_changes for affected sets.
    // set_processing_infos_ should have info. for all sets in cover up to new set.
    // rule_processing_infos_ should have info for all rules added up to when
    // new set was added.
    void UpdateProcessingInfo(uint64_t* num_covered,
			      map<string, uint64_t>* key_changes);
    // Given set name and change in number of uncovered rules it has, updates
    // set in @heap_ using @handles_.
    void UpdateSetsInHeap(const map<string, uint64_t>& key_changes);

    unique_ptr<fibonacci_heap<heap_data> > heap_;
    unique_ptr<map<string, pair<handle_t, uint64_t> > > handles_;
    
  private:
    friend class GreedySetCoverTest;
    FRIEND_TEST(GreedySetCoverTest, AddAllSetsToHeap);
    FRIEND_TEST(GreedySetCoverTest, UpdateProcessingInfo);
    FRIEND_TEST(GreedySetCoverTest, UpdateSetsInHeap);
    FRIEND_TEST(GreedySetCoverTest, UpdateCover);
    FRIEND_TEST(GreedySetCoverTest, AddRule);
  };
}  // namespace incremental_atpg
#endif  // INCREMENTAL_ATPG_GREEDY_SET_COVER_H_
