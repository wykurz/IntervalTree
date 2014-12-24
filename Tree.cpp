#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Bit

#include "Test.h"

#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include <boost/test/unit_test.hpp>

typedef std::size_t             Count;
typedef std::size_t             Index;
typedef std::pair<Index, Index> Interval;

namespace std {

    inline std::ostream& operator<<(std::ostream& stream_, const Interval& i_)
    {
        stream_ << "[" << i_.first << ", " << i_.second << "]";
        return stream_;
    }

}

class Tree
{
  public:
    Tree(const Interval& i_);

    Count countIntervals() const;
    Interval findNthInterval(Count nth_) const;
    bool removeInterval(const Interval& i_);
};

Tree::Tree(const Interval& i_)
{
}

Count Tree::countIntervals() const
{
    return 0;
}

Interval Tree::findNthInterval(Count nth_) const
{
    return Interval(0, 0);
}

bool Tree::removeInterval(const Interval& i_)
{
    return false;
}

BOOST_AUTO_TEST_SUITE(IntervalTree)

Test::Predicate someCheck()
{
    Test::TestIssues issues;
    issues << "Sorry, there were issues";
    return issues.predicate;
}

BOOST_AUTO_TEST_CASE(Test01)
{
    // [0, 0], [0, 1], [0, 2], [0, 3], [0, 4] - 5 : 5
    //         [1, 1], [1, 2], [1, 3], [1, 4] - 4 : 9
    //                 [2, 2], [2, 3], [2, 4] - 3 : 12
    //                         [3, 3], [3, 4] - 2 : 14
    //                                 [4, 4] - 1 : 15
    Tree t(Interval(1, 5));
    {
        const Interval i = t.findNthInterval(6);
        BOOST_CHECK_EQUAL(Interval(1, 2), i);
        t.removeInterval(i);
    }
    // [0, 0], [3, 3], [3, 4], [4, 4]
    BOOST_CHECK_EQUAL(4, t.countIntervals());
    {
        const Interval i = t.findNthInterval(1);
        BOOST_CHECK_EQUAL(Interval(3, 3), i);
        t.removeInterval(i);
    }
    // [0, 0], [4, 4]
    BOOST_CHECK_EQUAL(2, t.countIntervals());
    {
        const Interval i = t.findNthInterval(1);
        BOOST_CHECK_EQUAL(Interval(4, 4), i);
        t.removeInterval(i);
    }
    // [0, 0]
    BOOST_CHECK_EQUAL(1, t.countIntervals());
    {
        const Interval i = t.findNthInterval(1);
        BOOST_CHECK_EQUAL(Interval(0, 0), i);
        t.removeInterval(i);
    }
    // empty
    BOOST_CHECK_EQUAL(0, t.countIntervals());
}

BOOST_AUTO_TEST_CASE(Test02)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()
