#include "greedy_set_cover.h"

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <stdint.h>
#include <log4cxx/logger.h>

#include "gtest/gtest_prod.h"

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::map;
  using std::pair;
  using std::make_pair;

  using log4cxx::LoggerPtr;
  using log4cxx::Logger;
  using log4cxx::Level;
  //LoggerPtr GreedySetCover::logger(Logger::getLogger("GreedySetCover"));

  GreedySetCover::GreedySetCover() 
    : heap_(new fibonacci_heap<heap_data>),
      handles_(new map<string, pair<handle_t, uint64_t> >) {
    greedy_set_cover_logger = Logger::getLogger("GreedySetCover");
    greedy_set_cover_logger->setLevel(log4cxx::Level::getWarn());
    }

  GreedySetCover::GreedySetCover(map<string, SetInfo>* set_infos,
			      vector<RuleInfo>* rule_infos)
    : SetCover(set_infos, rule_infos),
      heap_(new fibonacci_heap<heap_data>),
      handles_(new map<string, pair<handle_t, uint64_t> >) {
    greedy_set_cover_logger = Logger::getLogger("GreedySetCover");
    greedy_set_cover_logger->setLevel(log4cxx::Level::getWarn());
    }

  GreedySetCover::GreedySetCover(map<string, SetInfo>* set_infos,
				 vector<RuleInfo>* rule_infos,
				 map<string, SetProcessingInfo>* set_processing_infos,
				 vector<RuleProcessingInfo>* rule_processing_infos,
				 list<string>* cover)
    : SetCover(set_infos, rule_infos, set_processing_infos, 
	       rule_processing_infos, cover),
      heap_(new fibonacci_heap<heap_data>),
      handles_(new map<string, pair<handle_t, uint64_t> >) {
    greedy_set_cover_logger = Logger::getLogger("GreedySetCover");
    greedy_set_cover_logger->setLevel(log4cxx::Level::getWarn());
    }

  void GreedySetCover::AddAllSetsToHeap() {
    // add all sets to heap
    uint64_t uncovered;
    handle_t ht;
    pair<handle_t, uint64_t> handle_value;

    handles_.reset(new map<string, pair<handle_t, uint64_t> >);
    heap_.reset(new fibonacci_heap<heap_data>);

    for (auto const& set_info : *set_infos_) {
      uncovered = set_info.second.all_rules.size();
      ht = heap_->push(heap_data(set_info.first, uncovered));
      handle_value = make_pair(ht, uncovered);
      LOG4CXX_INFO(greedy_set_cover_logger, "Updating handles_ with " << set_info.first 
		   << " - ( x, " << handle_value.second << ").");
      handles_->operator[](set_info.first) = handle_value;
    }
    
  }

  void GreedySetCover::UpdateProcessingInfo(uint64_t* num_covered,
					     map<string, uint64_t>* key_changes) {
    if (cover_->size() == 0) {
      LOG4CXX_WARN(greedy_set_cover_logger,
		   "Cover is empty.");
      return;
    }
    const string& set_name = cover_->back();
    if (set_infos_->find(set_name) == set_infos_->end()) {
      LOG4CXX_WARN(greedy_set_cover_logger,
		   "Can't find " << set_name << " in set_infos_.");
      return;
    }
    map<string, SetProcessingInfo>::iterator spIt;
    if ((spIt = set_processing_infos_->find(set_name))
	!= set_processing_infos_->end()) {
      LOG4CXX_WARN(set_cover_logger, "Set " << set_name << " duplicate in cover.");
      return;
    }

      // If there's any new rule, add it to rule_processing_infos_
    int diff = rule_infos_->size() - rule_processing_infos_->size();
    RuleProcessingInfo uncovered_rule;
    if (diff != 1) {
	LOG4CXX_WARN(set_cover_logger, diff << " new rules since last update.");
    }
    while (diff > 0) {
      rule_processing_infos_->push_back(uncovered_rule);
      --diff;
    }
     
    SetProcessingInfo sp;
    // TODO(lav): Okay to say rule_infos_ has all rules.
    if (rule_infos_->size() < *num_covered) {
      LOG4CXX_WARN(set_cover_logger, *num_covered << " rules covered out of " 
		   << rule_infos_->size() << ".");
    }
    sp.num_uncovered = rule_infos_->size() - *num_covered;
    spIt = set_processing_infos_->insert(make_pair(set_name, sp)).first;
    

    const vector<uint64_t>& all_rules = set_infos_->operator[](set_name).all_rules;
    for (auto const& rule_id: all_rules) {
      if (rule_id >= rule_processing_infos_->size()) {
	LOG4CXX_WARN(set_cover_logger, rule_id << " not in rule_processing_infos_");
      }
      RuleProcessingInfo& rp = rule_processing_infos_->at(rule_id);
      if (rp.first_covered_by.empty()) {
	*num_covered += 1;
	spIt->second.AddRule(rule_id);
	rp.first_covered_by = set_name;
	const RuleInfo& rule_info = rule_infos_->at(rule_id);
	  for (auto const& set_id: rule_info.all_sets) {
	    if (!key_changes->insert(make_pair(set_id, 1)).second) {
	      key_changes->operator[](set_id) += 1;
	    }
	  }
	}
    }
  }

  void GreedySetCover::UpdateSetsInHeap(const map<string, uint64_t>& key_changes) {
    LOG4CXX_INFO(greedy_set_cover_logger,
		 key_changes.size() << " keys to update in heap.");

    for (auto const& change: key_changes) {
      if (handles_->find(change.first) == handles_->end()) {
	if (change.first != cover_->back()) {
	    LOG4CXX_WARN(greedy_set_cover_logger, "Key " << change.first
			 << " no longer in handles_. Popped out?");
	  }
	continue;
      }
      pair<handle_t, uint64_t>& handle_value = handles_->at(change.first);
      if (change.second > handle_value.second) {
	LOG4CXX_ERROR(greedy_set_cover_logger, "Set " << change.first 
		      << " does not have " << change.second << " rules.");
	continue;
      }
      uint64_t new_value = handle_value.second - change.second;
      handle_t h = handle_value.first;
      heap_data new_heap_data(change.first, new_value);
      handles_->operator[](change.first) = make_pair(h, new_value);
      heap_->decrease(h, new_heap_data);
      LOG4CXX_INFO(greedy_set_cover_logger, "Updated handle to " << *h << ".");
    }
  }

  void GreedySetCover::UpdateCover() {
    // Clear cover
    cover_->clear();
    // Goes through all rules once only.
    ResetProcessingInfo();
    AddAllSetsToHeap();

    uint64_t num_rules = rule_infos_->size();
    uint64_t num_covered = 0;
    while(num_covered < num_rules) {
      heap_data data = heap_->top();
      heap_->pop();
      handles_->erase(data.key);

      cover_->push_back(data.key);

      LOG4CXX_INFO(greedy_set_cover_logger,
		 "Pushed back " << data.key << " on cover.");

      map<string, uint64_t> key_changes;
      UpdateProcessingInfo(&num_covered, 
			   &key_changes);

      // TODO(lav): Add to test.
      // Aha! When you pop a key and try to change its handle!
      UpdateSetsInHeap(key_changes);
    }	    
  }
 

}  // namespace incremental_atpg
