#include "online_set_cover.h"

#include <vector>
#include <cstring>
#include <string>
#include <memory>
#include <map>
#include <list>
#include <algorithm> // for std::sort
#include <stdint.h>
#include <utility>
#include <log4cxx/logger.h>

#include "gtest/gtest_prod.h"
#include "set_cover.h"
#include "lazy_set_cover.h"
#include "greedy_set_cover.h"

namespace incremental_atpg {
  using std::vector;
  using std::list;
  using std::string;
  using std::strcmp;
  using std::unique_ptr;
  using std::map;
  using std::make_pair;
  using std::pair;
  using std::bind;
  using std::sort;
  using std::unique;

  using log4cxx::LoggerPtr;
  using log4cxx::Logger;
  using log4cxx::Level;

  void OnlineSetCover::AddRule(const vector<string>& sets) {
    if (adds_ != updates_) {
      LOG4CXX_WARN(online_set_cover_logger, "Update once after add.");
      return;
    }
    ++adds_;
    LazySetCover::AddRule(sets);
  }

  void OnlineSetCover::UpdateCover() {
    if (updates_ + 1 != adds_) {
      LOG4CXX_WARN(online_set_cover_logger, "Update once after add.");
      return;
    }
    ++updates_;
    LazySetCover::UpdateCover();
    if (!GoodEnough(*cover_.get())) {

      gr_.reset(new GreedySetCover(set_infos_.release(),
				   rule_infos_.release()));
      //set_processing_infos_.release(),
      //rule_processing_infos_.release(),
      //cover_.release()));
      gr_->UpdateCover();
      set_infos_.reset(gr_->ReleaseSetInfos());
      rule_infos_.reset(gr_->ReleaseRuleInfos());
      set_processing_infos_.reset(gr_->ReleaseSetProcessingInfos());
      rule_processing_infos_.reset(gr_->ReleaseRuleProcessingInfos());
      cover_.reset(gr_->ReleaseCover());
      cover_order_.reset(new map<string, uint64_t>);
      gr_.reset(nullptr);
    }
  }

  bool OnlineSetCover::GoodEnough(const list<string>& cover) {
    if (rule_infos_.get() == nullptr) {
      LOG4CXX_ERROR(online_set_cover_logger, "In GoodEnough, but rule_infos_ is NULL.");
      return false;
    }
    LOG4CXX_WARN(online_set_cover_logger, "In GoodEnough, rule_infos_ has "
		 << rule_infos_->size() << " rules.");
    return (rule_infos_->size() % 2 == 0);
  }

  bool OnlineSetCover::SanityCheck() {
    if (adds_ != updates_) {
      LOG4CXX_WARN(online_set_cover_logger, "Call after update.");
      return false;
    }

    if (rule_infos_.get() == nullptr) {
      LOG4CXX_ERROR(online_set_cover_logger, "In SanityCheck, but rule_infos_ is NULL.");
      return false;
    }

    uint64_t num_uncovered = rule_infos_->size();
    set<uint64_t> rules_covered;
    for (auto set_name : *cover_.get()) {
      if (set_processing_infos_->find(set_name) 
	  == set_processing_infos_->end()) {
	LOG4CXX_ERROR(online_set_cover_logger, "Set " << set_name
		      << " not in set_processing_infos_.");
	return false;
      }
      SetProcessingInfo sp = set_processing_infos_->at(set_name);
      if (num_uncovered != sp.num_uncovered) {
	LOG4CXX_ERROR(online_set_cover_logger, "Set " << set_name
		      << " says " << num_uncovered << " uncovered rules,"
		      " expected " << num_uncovered);
	return false;
      }
      for (auto rule_id : sp.GetRules()) {
	num_uncovered -= 1;
	if (rule_id >= rule_processing_infos_->size()) {
	  LOG4CXX_ERROR(online_set_cover_logger, "Rule " << rule_id
			<< " in Set " << set_name 
			<< " not in rule_processing_infos_.");
	  return false;
	}
	RuleProcessingInfo rp = rule_processing_infos_->at(rule_id);
	if (rp.first_covered_by != set_name) {
	  LOG4CXX_ERROR(online_set_cover_logger, "Rule " << rule_id
			<< " in Set " << set_name 
			<< " but processing info says it's first "
			" covered by " << rp.first_covered_by);
	  return false;
	}
	if (!rules_covered.insert(rule_id).second) {
	  LOG4CXX_ERROR(online_set_cover_logger, "Rule " << rule_id
			<< " already covered.");
	  return false;
	}
      }
    }

    if (rules_covered.size() != rule_infos_->size()) {
      LOG4CXX_ERROR(online_set_cover_logger, "Not all rules covered.");
      return false;
    }
    return true;
  }
}  // namespace incremental_atpg

