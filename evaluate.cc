#include "evaluate.h"

#include <ctime>
#include <fstream>
#include <memory>
#include <stdlib.h> // srand, rand
#include <stdint.h>
#include <set>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "set_cover.h"
#include "greedy_set_cover.h"
#include "online_set_cover.h"

namespace incremental_atpg {
  using std::to_string;
  using std::string;
  using std::map;
  using std::make_pair;
  using std::set;
  using std::ostringstream;
  using std::clock_t;
  using std::vector;
  using std::upper_bound;
  using std::ofstream;

  void Evaluate::Compare(uint64_t from_num_rules, uint64_t for_time) {
    uint64_t num_rules_seen = 0;
    uint64_t num_rules_added = 0;
    uint64_t total_rules = sets_.size();
    gr_.reset(new GreedySetCover);
    if (from_num_rules > total_rules) {
      LOG4CXX_ERROR(evaluate_logger, "Can't start from " << from_num_rules
		    << ", sets_ has only " << total_rules);
      return;
    }
    LOG4CXX_WARN(evaluate_logger, "sets_ has " << total_rules << " rules.");
    clock_t begin = clock();
    clock_t last = begin;
    while (num_rules_added < from_num_rules && num_rules_seen < total_rules) {
      const vector<string>& rule = sets_[num_rules_seen];
      num_rules_seen++;
      if (rule.size() > 0) {
	gr_->AddRule(rule);
	num_rules_added++;
      }
    }
    uint64_t time_taken = 0;
    clock_t now = clock();
    time_taken = double(now-begin)/CLOCKS_PER_SEC;
    LOG4CXX_WARN(evaluate_logger, "Adding " << num_rules_added
		 << " rules to Greedy took " 
		 << time_taken << " seconds");
    
    begin = now;
    gr_->UpdateCover();
    now = clock();
    time_taken = double(now-begin)/CLOCKS_PER_SEC;
    LOG4CXX_WARN(evaluate_logger, "Finished updating GreedySetCover: " 
		 << "  which took " << time_taken
		 << " seconds.");
    
    begin = now;
    on_.reset(new OnlineSetCover(
				gr_->ReleaseSetInfos(),
				gr_->ReleaseRuleInfos(),
				gr_->ReleaseSetProcessingInfos(),
				gr_->ReleaseRuleProcessingInfos(),
				gr_->ReleaseCover()));
    now = clock();
    time_taken = double(now-begin)/CLOCKS_PER_SEC;
    LOG4CXX_WARN(evaluate_logger, "Setting up OnlineSetCover: " 
		 << "  took " << double(now - begin) / CLOCKS_PER_SEC
		 << " seconds.");

    begin = now;
    now = clock();
    time_taken = double(now-begin)/CLOCKS_PER_SEC;
    gr_.reset(nullptr);
    while(num_rules_seen < total_rules && time_taken < for_time) {
      const vector<string>& rule = sets_[num_rules_seen];
      num_rules_seen++;
      if (rule.size() > 0) {
	on_->AddRule(rule);
	num_rules_added++;
	on_->UpdateCover();

	if (num_rules_added % 2 == 0) {
	  now = clock();
	  time_taken = double(now-begin)/CLOCKS_PER_SEC;
	  LOG4CXX_WARN(evaluate_logger, "Updated for rule #" << num_rules_added
		       << " at time: " << double(now - begin) / CLOCKS_PER_SEC
		       << " i.e., " << double(now - last) / CLOCKS_PER_SEC
		       << " seconds for last 2 rules.");
	  last = now;
	  on_->ShowStats();
	}
      }
    }
      	  on_->ShowStats();
	  LOG4CXX_WARN(evaluate_logger, "Online could process "
		       << num_rules_added - from_num_rules
		       << " new rules in " << time_taken << " seconds."
		       << " starting from " << from_num_rules);
  }

}  // namespace incremental_atpg
