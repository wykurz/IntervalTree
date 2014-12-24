#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Bit

#include "Test.h"

#include <cstdint>
#include <deque>
#include <iostream>
#include <utility>

#include <boost/test/unit_test.hpp>

typedef std::size_t             Count;
typedef std::size_t             Index;
typedef std::pair<Index, Index> Interval;

// #define DEBUG

#ifdef DEBUG
#define debug(x) std::cerr << "DEBUG: " << x << std::endl
#else
#define debug(x)
#endif

namespace std {

    inline ostream& operator<<(ostream& stream_, const Interval& interval_)
    {
        stream_ << "[" << interval_.first << ", " << interval_.second << "]";
        return stream_;
    }

}

Count numAllIntervals(const Interval& interval_)
{
    if (interval_.second < interval_.first)
    {
        return 0;
    }
    Count n = interval_.second - interval_.first + 1;
    return n * (n + 1) / 2;
}

Interval findNthContInterval(Count nth_, const Interval& interval_)
{
    // TODO: too slow
    Index A = interval_.first;
    Index B = interval_.second;
    Index a = A;
    Index b = A;
    Interval interval(interval_.first, interval_.first);
    while (0 < nth_--)
    {
        if (b < B)
        {
            ++b;
        }
        else
        {
            assert(a < B);
            ++a;
            b = a;
        }
    }
    return Interval(a, b);
}

bool intervalContains(const Interval& a_, const Interval& b_)
{
    return a_.first <= b_.first && b_.second <= a_.second;
}

bool intervalStrictlyWithin(const Interval& a_, const Interval& b_)
{
    return a_.first < b_.first && b_.second < a_.second;
}

class Tree
{
  public:
    struct Node
    {
        Interval interval;
        Index    left;
        Index    right;
        Count    count;
    };

    typedef std::deque<Node> NodeList;

    Tree(const Interval& interval_);

    Count countIntervals() const;
    // NOTE: zero-indexed
    Interval findNthInterval(Count nth_) const;
    bool complementOf(const Interval& interval_);

  private:
    Interval findNthIntervalImpl(Count nth_, Index i_) const;
    Index addNode(const Interval& interval_);
    bool complementOfImpl(const Interval& interval_, Index i_);

    NodeList _nodes;
};


namespace std {

    inline ostream& operator<<(ostream& stream_, const Tree::Node& node_)
    {
        stream_ << "\n"
                << "{ interval : " << node_.interval << "\n"
                << "  left     : " << node_.left << "\n"
                << "  right    : " << node_.right << "\n"
                << "  count    : " << node_.count << " }";
        return stream_;
    }

}

Tree::Tree(const Interval& interval_)
{
    _nodes.push_back({interval_, 0, 0, numAllIntervals(interval_)});
}

Count Tree::countIntervals() const
{
    return _nodes[0].count;
}

Interval Tree::findNthInterval(Count nth_) const
{
    return findNthIntervalImpl(nth_, 0);
}

bool Tree::complementOf(const Interval& interval_)
{
    return complementOfImpl(interval_, 0);
}

Interval Tree::findNthIntervalImpl(Count nth_, Index i_) const
{
    const Node& node(_nodes[i_]);
    if (0 != node.left)
    {
        const Count leftCount = _nodes[node.left].count;
        if (nth_ < leftCount)
        {
            return findNthIntervalImpl(nth_, node.left);
        }
        else
        {
            nth_ -= leftCount;
        }
    }
    if (0 != node.right && 0 != _nodes[node.right].count)
    {
        return findNthIntervalImpl(nth_, node.right);
    }
    assert(nth_ < node.count);
    return findNthContInterval(nth_, node.interval);
}

Index Tree::addNode(const Interval& interval_)
{
    Index i = _nodes.size();
    _nodes.push_back({interval_, 0, 0, numAllIntervals(interval_)});
    debug("Created new node " << i << ":" << _nodes[i]);
    return i;
}

bool Tree::complementOfImpl(const Interval& interval_, Index i_)
{
    Node& node(_nodes[i_]);
    debug("Processing complement of node " << i_ << ":" << node);
    if (0 != node.left && intervalContains(_nodes[node.left].interval, interval_))
    {
        if (complementOfImpl(interval_, node.left))
        {
            node.count = _nodes[node.left].count;
            if (0 != node.right)
            {
                node.count += _nodes[node.right].count;
            }
            return true;
        }
        return false;
    }
    if (0 != node.right && intervalContains(_nodes[node.right].interval, interval_))
    {
        if (complementOfImpl(interval_, node.right))
        {
            node.count = _nodes[node.right].count;
            if (0 != node.left)
            {
                node.count += _nodes[node.left].count;
            }
            return true;
        }
        return false;
    }
    if (!intervalContains(node.interval, interval_))
    {
        debug(node.interval << " does not contain " << interval_);
        return false;
    }
    if (0 == node.left && 0 == node.right)
    {
        const Index a  = node.interval.first;
        const Index b  = node.interval.second;
        const Index ap = interval_.first;
        const Index bp = interval_.second;
        if (intervalStrictlyWithin(node.interval, interval_))
        {
            node.left  = addNode(Interval(a,      ap - 1));
            node.right = addNode(Interval(bp + 1, b     ));
            node.count = _nodes[node.left].count + _nodes[node.right].count;
        }
        else
        {
            if (a < ap)
            {
                assert(b <= bp);
                node.interval = Interval(a, ap - 1);
            }
            else if (bp < b)
            {
                assert(ap <= a);
                node.interval = Interval(bp + 1, b);
            }
            else
            {
                // Remove whole node
                assert(ap == a);
                assert(bp == b);
                node.interval = Interval(1, 0); // invalid
            }
            node.count = numAllIntervals(node.interval);
        }
        debug("Updated node " << i_ << ":" << node);
        return true;
    }
    return false;
}

BOOST_AUTO_TEST_SUITE(IntervalTree)

Test::Predicate someCheck()
{
    Test::TestIssues issues;
    issues << "Sorry, there were issues";
    return issues.predicate;
}

// bool intervalContains(const Interval& a_, const Interval& b_)
// bool intervalStrictlyWithin(const Interval& a_, const Interval& b_)

BOOST_AUTO_TEST_CASE(Preliminary)
{
    BOOST_CHECK_EQUAL(Interval(0, 0), findNthContInterval(0, Interval(0, 0)));
    BOOST_CHECK_EQUAL(Interval(0, 0), findNthContInterval(0, Interval(0, 1)));
    BOOST_CHECK_EQUAL(Interval(0, 0), findNthContInterval(0, Interval(0, 5)));
    BOOST_CHECK_EQUAL(Interval(4, 7), findNthContInterval(8, Interval(3, 7)));
    BOOST_CHECK_EQUAL(Interval(2, 2), findNthContInterval(2, Interval(1, 2)));
    BOOST_CHECK_EQUAL(Interval(1e3, 1e3), findNthContInterval(500499, Interval(1, 1e3)));

    BOOST_CHECK( intervalContains(Interval(0, 4), Interval(0, 0)));
    BOOST_CHECK( intervalContains(Interval(0, 4), Interval(4, 4)));
    BOOST_CHECK( intervalContains(Interval(0, 4), Interval(0, 4)));
    BOOST_CHECK( intervalContains(Interval(0, 4), Interval(1, 3)));
    BOOST_CHECK(!intervalContains(Interval(0, 4), Interval(1, 5)));
    BOOST_CHECK(!intervalContains(Interval(0, 4), Interval(5, 5)));
    BOOST_CHECK(!intervalContains(Interval(2, 4), Interval(0, 1)));

    BOOST_CHECK(!intervalStrictlyWithin(Interval(0, 4), Interval(0, 0)));
    BOOST_CHECK(!intervalStrictlyWithin(Interval(0, 4), Interval(4, 4)));
    BOOST_CHECK(!intervalStrictlyWithin(Interval(0, 4), Interval(0, 4)));
    BOOST_CHECK( intervalStrictlyWithin(Interval(0, 4), Interval(1, 1)));
    BOOST_CHECK( intervalStrictlyWithin(Interval(0, 4), Interval(1, 2)));
    BOOST_CHECK( intervalStrictlyWithin(Interval(0, 4), Interval(1, 3)));
    BOOST_CHECK( intervalStrictlyWithin(Interval(0, 4), Interval(2, 3)));
    BOOST_CHECK( intervalStrictlyWithin(Interval(0, 4), Interval(3, 3)));
    BOOST_CHECK(!intervalStrictlyWithin(Interval(0, 4), Interval(1, 5)));
    BOOST_CHECK(!intervalStrictlyWithin(Interval(0, 4), Interval(5, 5)));
    BOOST_CHECK(!intervalStrictlyWithin(Interval(2, 4), Interval(0, 1)));
}

BOOST_AUTO_TEST_CASE(Basic)
{
    // [0, 0], [0, 1], [0, 2], [0, 3], [0, 4] - 5 : 5
    //         [1, 1], [1, 2], [1, 3], [1, 4] - 4 : 9
    //                 [2, 2], [2, 3], [2, 4] - 3 : 12
    //                         [3, 3], [3, 4] - 2 : 14
    //                                 [4, 4] - 1 : 15
    Tree t(Interval(0, 4));
    {
        const Interval i = t.findNthInterval(6);
        BOOST_REQUIRE_EQUAL(Interval(1, 2), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    // [0, 0], [3, 3], [3, 4], [4, 4]
    BOOST_REQUIRE_EQUAL(4, t.countIntervals());
    {
        const Interval i = t.findNthInterval(1);
        BOOST_REQUIRE_EQUAL(Interval(3, 3), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    // [0, 0], [4, 4]
    BOOST_REQUIRE_EQUAL(2, t.countIntervals());
    {
        const Interval i = t.findNthInterval(1);
        BOOST_REQUIRE_EQUAL(Interval(4, 4), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    // [0, 0]
    BOOST_REQUIRE_EQUAL(1, t.countIntervals());
    {
        const Interval i = t.findNthInterval(1);
        BOOST_REQUIRE_EQUAL(Interval(0, 0), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    // empty
    BOOST_REQUIRE_EQUAL(0, t.countIntervals());
}

BOOST_AUTO_TEST_CASE(RemoveInterval)
{
    Tree t(Interval(0, 4));
    // [0, 4]
    BOOST_CHECK(!t.complementOf(Interval(0, 5)));
    BOOST_CHECK(!t.complementOf(Interval(5, 5)));
    BOOST_CHECK( t.complementOf(Interval(1, 3)));
    // [0, 0], [4, 4]
    BOOST_CHECK(!t.complementOf(Interval(0, 1)));
    BOOST_CHECK(!t.complementOf(Interval(3, 4)));
    BOOST_CHECK( t.complementOf(Interval(0, 0)));
    // [4, 4]
    BOOST_CHECK(!t.complementOf(Interval(5, 4)));
    BOOST_CHECK( t.complementOf(Interval(4, 4)));
}

BOOST_AUTO_TEST_CASE(Large)
{
    Index N = 1e6;
    Tree t(Interval(0, N - 1));

    // Remove all [<even>, <even>]
    for (Index i = 0; i < N; i += 2)
    {
        BOOST_CHECK(t.complementOf(Interval(i, i)));
    }
    BOOST_CHECK_EQUAL(N / 2, t.countIntervals());

}

BOOST_AUTO_TEST_SUITE_END()
