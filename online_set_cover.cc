#include "online_set_cover.h"

#include <vector>
#include <cstring>
#include <string>
#include <memory>
#include <map>
#include <math.h>
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
  using std::log;
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
    if (!GoodEnough()) {

      gr_.reset(new GreedySetCover(set_infos_.release(),
				   rule_infos_.release()));
      //set_processing_infos_.release(),
      //rule_processing_infos_.release(),
      //cover_.release()));
      ++greedy_updates_;
      gr_->UpdateCover();

      set_infos_.reset(gr_->ReleaseSetInfos());
      rule_infos_.reset(gr_->ReleaseRuleInfos());
      set_processing_infos_.reset(gr_->ReleaseSetProcessingInfos());
      rule_processing_infos_.reset(gr_->ReleaseRuleProcessingInfos());
      cover_.reset(gr_->ReleaseCover());
      double min = 1.0;
      if (GetMin(&min)) {
	LOG4CXX_INFO(online_set_cover_logger, "Reset best_greedy_fraction_ to " << min
		     << ". Reciprocal is " << (1.0/best_greedy_fraction_) << ".");
	best_greedy_fraction_ = min;
      } else {
	LOG4CXX_INFO(online_set_cover_logger, "Couldn't reset best_greedy_fraction_ to " << min);
      }

      cover_order_.reset(new map<string, uint64_t>);
      gr_.reset(nullptr);
    }
  }

  void OnlineSetCover::ShowStats() {
    if (NoNullPtrs()  && rule_infos_->size() > 0) {
    LOG4CXX_WARN(online_set_cover_logger, "Size of cover is " << cover_->size() << ", "
		 << "Lower bound on best set cover is " << (1.0/best_greedy_fraction_) << ", "
		 << "Size of cover within " << (2.0 * log(rule_infos_->size()))/best_greedy_fraction_ << ", "
		 << "Number of rules is " << rule_infos_->size() << ", "
		 << "Number of sets is " << set_infos_->size() << ", "
		 << greedy_updates_ << " greedy updates of " << updates_ << ".");
    } else {
      LOG4CXX_ERROR(online_set_cover_logger, "ShowStats has nullptrs.");
      return;
    }
  }

  bool OnlineSetCover::NoNullPtrs() {
    if (rule_infos_.get() == nullptr) {
      LOG4CXX_ERROR(online_set_cover_logger, "rule_infos_ is NULL.");
      return false;
    }
    if (cover_.get() == nullptr) {
      LOG4CXX_ERROR(online_set_cover_logger, "cover_ is NULL.");
      return false;
    }
    if (set_processing_infos_.get() == nullptr) {
      LOG4CXX_ERROR(online_set_cover_logger, "set_processing_infos_ is NULL.");
      return false;
    }
    return true;
  } 

  bool OnlineSetCover::GetSum(double* sum) {
    if (!NoNullPtrs()) {
      LOG4CXX_ERROR(online_set_cover_logger, "In GetSum, with nullptrs.");
      return false;
    }
        
    *sum = 0.0;
    uint64_t num_sets = 0;
    for (auto set_name : *cover_.get()) {
      if (set_processing_infos_->find(set_name) 
	  == set_processing_infos_->end()) {
	LOG4CXX_ERROR(online_set_cover_logger, "Set " << set_name
		      << " not in set_processing_infos_.");
	return false;
      }
      SetProcessingInfo sp = set_processing_infos_->at(set_name);
      if (sp.GetNumRules() == 0) {
	LOG4CXX_ERROR(online_set_cover_logger, "Set " << set_name << " covers no new rules.");
	return false;
      }
      if (sp.num_uncovered != 0) {
	*sum += ((double)sp.GetNumRules())/sp.num_uncovered;
      } else {
	LOG4CXX_ERROR(online_set_cover_logger, "Zero uncovered rules before " << set_name) ;
	return false;
      }
      ++num_sets;
      //if (*sum > 1.0) {
      //	LOG4CXX_WARN(online_set_cover_logger, "Sum exceeds 1.0 after " << num_sets
      //		     << " of " << cover_->size()) ;
      //return false;
      //}
    }
    return true;
  }

  bool OnlineSetCover::GetMin(double* min) {
    if (!NoNullPtrs()) {
      LOG4CXX_ERROR(online_set_cover_logger, "In GetMin, with nullptrs.");
      return false;
    }
        
    *min = 1.0;
    uint64_t num_sets = 0;
    for (auto set_name : *cover_.get()) {
      if (set_processing_infos_->find(set_name) 
	  == set_processing_infos_->end()) {
	LOG4CXX_ERROR(online_set_cover_logger, "Set " << set_name
		      << " not in set_processing_infos_.");
	return false;
      }
      SetProcessingInfo sp = set_processing_infos_->at(set_name);
      if (sp.GetNumRules() == 0) {
	LOG4CXX_ERROR(online_set_cover_logger, "Set " << set_name << " covers no new rules.");
	return false;
      }
      if (sp.num_uncovered != 0) {
	double fraction =  ((double)sp.GetNumRules())/sp.num_uncovered;
	if (fraction < *min) {
	  *min = fraction;
	}
      } else {
	LOG4CXX_ERROR(online_set_cover_logger, "Zero uncovered rules before " << set_name) ;
	return false;
      }
      ++num_sets;
    }
    if (*min >= 1.0) {
      return false;
    }
    return true;
  }


  bool OnlineSetCover::GoodEnough() {
    if (!NoNullPtrs()) {
      LOG4CXX_ERROR(online_set_cover_logger, "In GoodEnough, with nullptrs.");
      return false;
    }
        
    double sum = 0.0;
    double lower_bound = (cover_->size() * best_greedy_fraction_)/2.0;
    if (best_greedy_fraction_ < 1.0 && GetSum(&sum)) {
      if (sum > lower_bound) {
	return true;
      }
    }
    return false;
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

