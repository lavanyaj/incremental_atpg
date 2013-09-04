#include "online_set_cover.h"
#include "gtest/gtest.h"

#include <ctime>
#include <memory>
#include <stdlib.h> // srand, rand
#include <stdint.h>
#include <set>
#include <sstream>
#include <vector>

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "set_cover.h"

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

  class OnlineSetCoverTest : public testing::Test {
  protected:
    OnlineSetCoverTest() { 
      log4cxx::BasicConfigurator::resetConfiguration();
      log4cxx::BasicConfigurator::configure();
    }

    string GetString(uint64_t num) {
      return static_cast<ostringstream*> (&(ostringstream() << num) )->str();
    }

    uint64_t GetZipf() {
      double r = ((double) rand() / (RAND_MAX));
      vector<double>::const_iterator up = upper_bound (zipf.begin(),
						 zipf.end(),
						 r);
      return up - zipf.begin() + 1;
    }

    virtual void SetUp() {
      sc_.reset(new OnlineSetCover);
    }
    virtual void TearDown() {
    }
    std::unique_ptr<OnlineSetCover> sc_;
    const uint64_t num_rules_ = 700000;
    const uint64_t num_sets_ = 400000;
    const uint64_t rules_per_set_ = 20; // Make it Zipfian, few packets have 200, most have fewer.
    static const vector<double> zipf;
    // N = 300; s=0.4; y = 1:N, proby = dzipf(y, N=N, s=s), cumsum(proby)
  };
    
    const vector<double> OnlineSetCoverTest::zipf({0.02000759,0.0351705,0.04806327,0.05955461,0.07006471,0.0798356,0.08902222,0.09773102,0.106039,0.1140042,0.1216714,0.1290764,0.136248,0.1432102,0.1499828,0.1565828,0.1630248,0.1693211,0.1754827,0.1815191,0.1874389,0.1932496,0.1989578,0.2045697,0.2100908,0.2155258,0.2208795,0.2261558,0.2313586,0.2364913,0.2415571,0.246559,0.2514997,0.2563818,0.2612075,0.2659793,0.2706989,0.2753686,0.2799899,0.2845647,0.2890945,0.2935809,0.2980252,0.3024289,0.3067931,0.3111192,0.3154082,0.3196612,0.3238793,0.3280635,0.3322146,0.3363336,0.3404214,0.3444787,0.3485063,0.352505,0.3564755,0.3604185,0.3643346,0.3682244,0.3720886,0.3759278,0.3797425,0.3835332,0.3873005,0.3910449,0.3947668,0.3984667,0.4021451,0.4058023,0.4094389,0.4130552,0.4166515,0.4202284,0.4237861,0.427325,0.4308455,0.4343478,0.4378323,0.4412994,0.4447492,0.4481822,0.4515985,0.4549986,0.4583825,0.4617507,0.4651034,0.4684407,0.471763,0.4750705,0.4783634,0.4816419,0.4849063,0.4881568,0.4913935,0.4946167,0.4978265,0.5010233,0.504207,0.507378,0.5105364,0.5136824,0.5168161,0.5199377,0.5230474,0.5261454,0.5292317,0.5323065,0.5353701,0.5384225,0.5414638,0.5444943,0.5475139,0.550523,0.5535216,0.5565098,0.5594878,0.5624556,0.5654135,0.5683614,0.5712996,0.5742282,0.5771472,0.5800567,0.5829569,0.5858479,0.5887298,0.5916026,0.5944665,0.5973216,0.6001679,0.6030056,0.6058348,0.6086554,0.6114677,0.6142718,0.6170676,0.6198552,0.6226349,0.6254066,0.6281704,0.6309264,0.6336746,0.6364153,0.6391483,0.6418739,0.644592,0.6473027,0.6500062,0.6527024,0.6553915,0.6580735,0.6607485,0.6634165,0.6660776,0.6687318,0.6713793,0.6740201,0.6766542,0.6792818,0.6819027,0.6845172,0.6871253,0.689727,0.6923224,0.6949115,0.6974944,0.7000712,0.7026418,0.7052064,0.7077649,0.7103175,0.7128642,0.715405,0.7179401,0.7204693,0.7229928,0.7255106,0.7280228,0.7305294,0.7330305,0.735526,0.7380161,0.7405008,0.7429801,0.745454,0.7479227,0.750386,0.7528442,0.7552972,0.757745,0.7601877,0.7626254,0.765058,0.7674856,0.7699083,0.772326,0.7747389,0.7771468,0.77955,0.7819484,0.784342,0.7867309,0.7891151,0.7914946,0.7938695,0.7962398,0.7986056,0.8009668,0.8033235,0.8056758,0.8080236,0.8103669,0.8127059,0.8150406,0.8173709,0.8196969,0.8220186,0.8243361,0.8266494,0.8289584,0.8312634,0.8335641,0.8358608,0.8381533,0.8404418,0.8427263,0.8450068,0.8472832,0.8495557,0.8518243,0.8540889,0.8563497,0.8586066,0.8608596,0.8631088,0.8653542,0.8675958,0.8698337,0.8720678,0.8742983,0.876525,0.8787481,0.8809675,0.8831833,0.8853955,0.8876041,0.8898091,0.8920106,0.8942085,0.896403,0.8985939,0.9007814,0.9029655,0.9051461,0.9073233,0.9094971,0.9116675,0.9138346,0.9159983,0.9181588,0.9203159,0.9224697,0.9246203,0.9267676,0.9289117,0.9310526,0.9331902,0.9353247,0.937456,0.9395842,0.9417093,0.9438312,0.94595,0.9480657,0.9501784,0.952288,0.9543946,0.9564981,0.9585987,0.9606962,0.9627908,0.9648824,0.9669711,0.9690568,0.9711396,0.9732195,0.9752965,0.9773706,0.9794419,0.9815103,0.9835759,0.9856386,0.9876986,0.9897557,0.9918101,0.9938617,0.9959105,0.9979566,1});


  TEST_F(OnlineSetCoverTest, UpdateCoverManyGreedy) {
    vector<vector<string> > sets(num_rules_);
    srand(10);

    for (uint64_t set_id = 0; set_id < num_sets_; set_id++) {
      uint64_t rules_per_set = GetZipf();
      if (set_id % 100 == 0) {
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Making set #" << set_id
		     << " of " << num_sets_
		     << " with " << rules_per_set << " rules.");
      }
      set<uint64_t> added;
      for (uint64_t rule = 0; rule < rules_per_set; rule++) {
	uint64_t rule_uint64 = rand()%num_rules_;
	if (added.insert(rule_uint64).second) {
	  sets[rule_uint64].push_back(GetString(set_id));
	}
      }
      //EXPECT_TRUE(sc_->SanityCheck());
    }


    clock_t begin = clock();
    clock_t last = begin;
    uint64_t num_rules_added = 0;
    uint64_t max_sets_per_rule = 0;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      uint64_t sizez = sets[rule].size();
      if (sizez > 0) {
	if ( sizez > max_sets_per_rule) {
	  max_sets_per_rule = sizez;
	}
	sc_->AddRule(sets[rule]);
	sc_->GreedyUpdateCover();
	num_rules_added++;

	if (num_rules_added % 1000 == 0) {
	  clock_t now = clock();
	  LOG4CXX_WARN(sc_->online_set_cover_logger, "Adding rule #" << num_rules_added
		       << " has " << sizez << " sets, "
		       << " " << double(now - begin) / CLOCKS_PER_SEC
		       << " seconds after start"
		       << " and " << double(now - last) / CLOCKS_PER_SEC
		       << " seconds after last check.");
	  //	  EXPECT_TRUE(sc_->SanityCheck());
	  last = now;
	  sc_->ShowStats();
	}
      }
      //EXPECT_TRUE(sc_->SanityCheck());
    }

    LOG4CXX_WARN(sc_->online_set_cover_logger, "num_rules_added: " << num_rules_added
		 << ", max_sets_per_rule: " << max_sets_per_rule);
  }


  TEST_F(OnlineSetCoverTest, UpdateCoverMany) {
    vector<vector<string> > sets(num_rules_);
    srand(10);
    for (uint64_t set_id = 0; set_id < num_sets_; set_id++) {
      if (set_id % 100 == 0) {
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Making set #" << set_id
		     << " of " << num_sets_);
      }
      set<uint64_t> added;
      for (uint64_t rule = 0; rule < rules_per_set_; rule++) {
	uint64_t rule_uint64 = rand()%num_rules_;
	if (added.insert(rule_uint64).second) {
	  sets[rule_uint64].push_back(GetString(set_id));
	}
      }
      //EXPECT_TRUE(sc_->SanityCheck());
    }

    clock_t begin = clock();
    clock_t last = begin;
    uint64_t num_rules_added = 0;
    uint64_t max_sets_per_rule = 0;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      uint64_t sizez = sets[rule].size();
      if ( sizez > 0) {
	if (sizez > max_sets_per_rule) {
	  max_sets_per_rule = sizez;
	}
	sc_->AddRule(sets[rule]);
	sc_->UpdateCover();
	num_rules_added++;

	if (num_rules_added % 1000 == 0) {
	  clock_t now = clock();
	  LOG4CXX_WARN(sc_->online_set_cover_logger, "Adding rule #" << num_rules_added
		       << " has " << sizez << " sets, "
		       << " " << double(now - begin) / CLOCKS_PER_SEC
		       << " seconds after start"
		       << " and " << double(now - last) / CLOCKS_PER_SEC
		       << " seconds after last check.");
	  //EXPECT_TRUE(sc_->SanityCheck());
	  last = now;
	  sc_->ShowStats();
	}
      }
      //EXPECT_TRUE(sc_->SanityCheck());
    }

    LOG4CXX_WARN(sc_->online_set_cover_logger, "num_rules_added: " << num_rules_added
		 << ", max_sets_per_rule: " << max_sets_per_rule);
  }


  /*
  TEST_F(OnlineSetCoverTest, UpdateCoverMany) {
    srand(10);
    vector< vector<string> > rules;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      if (rule % 100 == 0) {
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Making rule #" << rule
		     << " of " << num_rules_);
      }
      vector<string> sets; set<uint64_t> added;
      for (uint64_t set_num = 0; set_num < sets_per_rule_; set_num++) {
	uint64_t set_uint64 = rand()%num_sets_;
	if (added.insert(set_uint64).second) {
	  sets.push_back(GetString(set_uint64));
	}
      }
      rules.push_back(sets);
      //EXPECT_TRUE(sc_->SanityCheck());
    }


    clock_t begin = clock();
    clock_t last = begin;
    for (uint64_t rule = 0; rule < num_rules_; rule++) {
      if (rule % 100 == 0) {
	clock_t now = clock();
	LOG4CXX_WARN(sc_->online_set_cover_logger, "Adding rule #" << rule
		     << " " << double(now - begin) / CLOCKS_PER_SEC
		     << " seconds after start"
		     << " and " << double(now - last) / CLOCKS_PER_SEC
		     << " seconds after last check.");
	EXPECT_TRUE(sc_->SanityCheck());
	last = now;
	sc_->ShowStats();
      }
      sc_->AddRule(rules[rule]);
      sc_->UpdateCover();
      //EXPECT_TRUE(sc_->SanityCheck());
    }
  }
  */
  /*
  TEST_F(OnlineSetCoverTest, UpdateCover) {
    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"dog\"}");
    sc_->AddRule({"dog"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(1, sc_->cover_->size());

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"dog\", \"cat\"}");
    sc_->AddRule({"dog", "cat"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(1, sc_->cover_->size());

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"rain\"}");
    sc_->AddRule({"rain"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("dog", sc_->rule_processing_infos_->at(0).first_covered_by);
    EXPECT_EQ("dog", sc_->rule_processing_infos_->at(1).first_covered_by);
    EXPECT_EQ("rain", sc_->rule_processing_infos_->at(2).first_covered_by);

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"rain\"}");
    sc_->AddRule({"rain"});
    sc_->UpdateCover();
    EXPECT_TRUE(sc_->SanityCheck());
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("rain", sc_->cover_->front());

    LOG4CXX_WARN(sc_->online_set_cover_logger, "\n\nAdding {\"rain\"}");
    sc_->AddRule({"rain"});
    sc_->UpdateCover();
    EXPECT_EQ(2, sc_->cover_->size());
    EXPECT_EQ("rain", sc_->cover_->front());
    EXPECT_EQ("dog", sc_->cover_->back());
  }
  */
}  // namespace incremental_atpg
