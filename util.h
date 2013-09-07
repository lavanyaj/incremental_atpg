#ifndef INCREMENTAL_ATPG_UTIL_H_
#define INCREMENTAL_ATPG_UTIL_H_
#include <vector>
#include <string>
#include <memory>
#include <stdlib.h> // srand, rand
#include <list>
#include <fstream>
#include <set>
#include <map>
#include <stdint.h>
#include <utility>
#include <ctime>
#include <log4cxx/logger.h>

#include "gtest/gtest_prod.h"

namespace incremental_atpg {
  using std::vector;
  using std::string;
  using std::unique_ptr;
  using std::map;
  using std::make_pair;
  using std::pair;
  using std::set;
  using std::list;
  using std::to_string;
  using std::string;
  using std::map;
  using std::make_pair;
  using std::set;
  using std::ostringstream;
  using std::clock_t;
  using std::vector;
  using std::ofstream;

  class Util {
  public:
    log4cxx::LoggerPtr util_logger;
    Util();

    string GetString(uint64_t num);
    uint64_t GetZipf(const vector<double>& zipf);
    void ReadRulesFromFile(const string& input_file,
			  vector<vector<string> >* sets);
    void MakeRules(uint64_t num_rules, uint64_t num_sets,
	      uint64_t max_rules_per_set,
	      const vector<double>& zipf,
	      vector<vector<string> >* sets);
    void WriteRulesToFile(const vector<vector<string> >& sets,
			  const string& output_file);
    void ShowSetsPerRule(const vector<vector<string> >& sets);
    void ShowRulesPerSet(const vector<vector<string> >& sets);
    static const vector<double> zipf_1;

  private:
    friend class UtilTest;
    FRIEND_TEST(UtilTest, UpdateCover);
  };

  

}  // namespace incremental_atpg
#endif  // INCREMENTAL_ATPG_UTIL_H_
