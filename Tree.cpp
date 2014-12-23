#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Bit

#include "Test.h"

#include <cstdint>
#include <vector>

#include <boost/test/unit_test.hpp>

namespace IntervalTree {

    class Tree
    {
      public:
        typedef std::size_t Index;
        typedef std::size_t Count;
    };

    BOOST_AUTO_TEST_SUITE(IntervalTree)

    Test::Predicate someCheck()
    {
        Test::TestIssues issues;
        issues << "Sorry, there were issues";
        return issues.predicate;
    }

    BOOST_AUTO_TEST_CASE(Test01)
    {
        BOOST_CHECK(false);
    }

    BOOST_AUTO_TEST_CASE(Test02)
    {
        BOOST_CHECK(true);
    }

BOOST_AUTO_TEST_SUITE_END()

}
