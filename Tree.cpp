#define BOOST_TEST_MODULE Tree

#include "Test.h"

#include <cmath>
#include <cstdint>
#include <vector>
#include <iostream>
#include <utility>

#include <boost/test/unit_test.hpp>

// #define LOG_DEBUG

#ifdef LOG_DEBUG
#define DEBUG(x) std::cerr << "DEBUG: " << x << std::endl
#else
#define DEBUG(x)
#endif

#define LOG_INFO

#ifdef LOG_INFO
#define INFO(x) std::cerr << "INFO: " << x << std::endl
#else
#define INFO(x)
#endif

// Disabling assertions gives about 40% perf improvement
// #define CHECK_ASSERT

#ifdef CHECK_ASSERT
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

typedef std::size_t Count;
typedef std::size_t Index;

class Interval
{
  public:
    Interval(Index a_, Index b_);

    Index a() const;
    Index b() const;

    bool          operator==(const Interval& i_  ) const;
    Count       numIntervals(                    ) const;
    Interval findNthInterval(Count           nth_) const;
    bool            contains(const Interval& i_  ) const;
    bool   hasStrictlyWithin(const Interval& i_  ) const;
    bool             isValid()                     const;

  private:
    Index _a;
    Index _b;
};

Interval::Interval(Index a_, Index b_)
  : _a(a_), _b(b_)
{
}

Index Interval::a() const
{
    return _a;
}

Index Interval::b() const
{
    return _b;
}

bool Interval::operator==(const Interval& i_  ) const
{
    return _a == i_._a && _b == i_._b;
}

Count Interval::numIntervals() const
{
    Count n = _b - _a + 1;
    return n * (n + 1) / 2;
}

Interval Interval::findNthInterval(Count nth_) const
{
    const Index d = _b - _a + 1;
    // x * d - x * (x - 1) / 2 + y < n
    auto f = [=](Index x_)
        {
            const Index sum = x_ * (x_ - 1) / 2;
            return (x_ <= d) && (sum <= x_ * d) && (x_ * d <= sum + nth_ );
        };
    Index x = 0;
    while (f(x + 1))
    {
        Index k = 1;
        while (f(x + 2 * k))
        {
            k *= 2;
        }
        x += k;
    }
    ASSERT( f(x));
    ASSERT(!f(x + 1));
    nth_ -= d * x - x * (x - 1) / 2;
    return Interval(_a + x, _a + x + nth_);
}

bool Interval::contains(const Interval& i_) const
{
    return _a <= i_._a && i_._b <= _b;
}

bool Interval::hasStrictlyWithin(const Interval& i_) const
{
    return _a < i_._a && i_._b < _b;
}

bool Interval::isValid() const
{
    return _a <= _b;
}

namespace std {

    inline ostream& operator<<(ostream& stream_, const Interval& interval_)
    {
        stream_ << "[" << interval_.a() << ", " << interval_.b() << "]";
        return stream_;
    }

}

class Tree
{
  public:
    struct Node
    {
        bool     isLeaf;
        Interval interval;
        Index    left;
        Index    right;
        Count    count;
        Count    depth;
    };

    typedef std::vector<Node> NodeList;

    Tree(const Interval& interval_);

    Count countIntervals() const;
    // NOTE: zero-indexed
    Interval findNthInterval(Count nth_) const;
    bool complementOf(const Interval& interval_);

    Count maxDepth() const;

  private:
    Interval findNthIntervalImpl(Count nth_, Index i_) const;
    Index addNode(const Interval& interval_);
    void updateNode(Index i_);
    void rotateRight(Index i_);
    void rotateLeft(Index i_);
    void balanceNode(Index i_);
    bool complementOfImpl(const Interval& interval_, Index i_);
    const Node& node(Index i_) const;
          Node& node(Index i_);
    const Node& left( const Node& node_) const;
          Node& left( const Node& node_);
    const Node& right(const Node& node_) const;
          Node& right(const Node& node_);

    NodeList _nodes;
};

namespace std {

    inline ostream& operator<<(ostream& stream_, const Tree::Node& node_)
    {
        stream_ << "\n"
                << "{ isLeaf   : " << node_.isLeaf << "\n"
                << "  interval : " << node_.interval << "\n"
                << "  left     : " << node_.left << "\n"
                << "  right    : " << node_.right << "\n"
                << "  count    : " << node_.count << "\n"
                << "  depth    : " << node_.depth << " }";
        return stream_;
    }

}

Tree::Tree(const Interval& interval_)
{
    addNode(interval_);
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

Count Tree::maxDepth() const
{
    return _nodes[0].depth;
}

Interval Tree::findNthIntervalImpl(Count nth_, Index i_) const
{
    if (node(i_).isLeaf)
    {
        return node(i_).interval.findNthInterval(nth_);
    }
    const Count leftCount = left(node(i_)).count;
    if (nth_ < leftCount)
    {
        return findNthIntervalImpl(nth_, node(i_).left);
    }
    nth_ -= leftCount;
    ASSERT(nth_ < node(i_).count);
    return findNthIntervalImpl(nth_, node(i_).right);
}

Index Tree::addNode(const Interval& interval_)
{
    Index i = _nodes.size();
    _nodes.push_back({true, interval_, 0, 0, interval_.numIntervals(), 0});
    DEBUG("Created new node " << i << ":" << node(i));
    return i;
}

void Tree::updateNode(Index i_)
{
    if (node(i_).isLeaf)
    {
        node(i_).count = node(i_).interval.numIntervals();
        node(i_).depth = 0;
        return;
    }
    if (!left(node(i_)).interval.isValid())
    {
        std::swap(right(node(i_)), node(i_));
        return;
    }
    if (!right(node(i_)).interval.isValid())
    {
        std::swap(left(node(i_)), node(i_));
        return;
    }
    node(i_).interval = Interval(left(node(i_)).interval.a(), right(node(i_)).interval.b());
    node(i_).count = left(node(i_)).count + right(node(i_)).count;
    node(i_).depth = std::max(1 + left(node(i_)). depth, 1 + right(node(i_)).depth);
}

void Tree::rotateRight(Index i_)
{
    DEBUG("Rotate right");
    ASSERT(!node(i_).isLeaf);
    //     Q        P
    //    / \      / \ .
    //   P   C    A   Q
    //  / \          / \.
    // A   B        B   C
    //
    Index P = node(i_).left;
    Index Q = i_;
    Index B = node(P).right;
    std::swap(node(P), node(Q));
    node(Q).right = P;
    node(P).left  = B;
    ASSERT(node(i_).left  != i_);
    ASSERT(node(i_).right != i_);
    ASSERT(node(i_).right == P);
    updateNode(P);
    updateNode(Q);
}

void Tree::rotateLeft(Index i_)
{
    DEBUG("Rotate left");
    ASSERT(!node(i_).isLeaf);
    //   P          Q
    //  / \        / \.
    // A   Q      P   C
    //    / \    / \.
    //   B   C  A   B
    //
    Index P = i_;
    Index Q = node(i_).right;
    Index B = node(Q).left;
    std::swap(node(P), node(Q));
    node(Q).right = B;
    node(P).left  = Q;
    ASSERT(node(i_).left  != i_);
    ASSERT(node(i_).right != i_);
    ASSERT(node(i_).left  == Q);
    updateNode(Q);
    updateNode(P);
}

void Tree::balanceNode(Index i_)
{
    // TODO: consider left-right & right-left cases
    if (node(i_).isLeaf)
    {
        return;
    }
    bool rotateL = ( left(node(i_)).depth + 1) < right(node(i_)).depth;
    bool rotateR = (right(node(i_)).depth + 1) <  left(node(i_)).depth;
    if (rotateL)
    {
        if (!(left(right(node(i_))).depth < right(right(node(i_))).depth))
        {
            rotateRight(node(i_).right);
        }
        ASSERT(left(right(node(i_))).depth < right(right(node(i_))).depth);
        rotateLeft(i_);
    }
    else if (rotateR)
    {
        if (!(right(left(node(i_))).depth < left(left(node(i_))).depth))
        {
            rotateLeft(node(i_).left);
        }
        ASSERT(right(left(node(i_))).depth < left(left(node(i_))).depth);
        rotateRight(i_);
    }
    ASSERT(std::abs(static_cast<int>(left(node(i_)).depth) - static_cast<int>(right(node(i_)).depth) < 2));
}

bool Tree::complementOfImpl(const Interval& interval_, Index i_)
{
    DEBUG("Processing complement of node " << i_ << ":" << node(i_));
    if (!node(i_).interval.contains(interval_))
    {
        DEBUG(node(i_).interval << " does not contain " << interval_);
        return false;
    }
    bool updated = false;
    if (node(i_).isLeaf)
    {
        const Index a  = node(i_).interval.a();
        const Index b  = node(i_).interval.b();
        const Index ap = interval_.a();
        const Index bp = interval_.b();
        if (node(i_).interval.hasStrictlyWithin(interval_))
        {
            node(i_).isLeaf = false;
            {
                Index left  = addNode(Interval(a,      ap - 1));
                Index right = addNode(Interval(bp + 1, b     ));
                node(i_).left  = left;
                node(i_).right = right;
            }
            ASSERT(node(i_).left  != node(i_).right);
            ASSERT(node(i_).left  != i_);
            ASSERT(node(i_).right != i_);
            DEBUG("Created leaf nodes " << node(i_).left << " and " << node(i_).right);
        }
        else
        {
            if (a < ap)
            {
                ASSERT(b <= bp);
                node(i_).interval = Interval(a, ap - 1);
            }
            else if (bp < b)
            {
                ASSERT(ap <= a);
                node(i_).interval = Interval(bp + 1, b);
            }
            else
            {
                DEBUG("Remove whole node " << i_ << ":" << node(i_));
                ASSERT(ap == a);
                ASSERT(bp == b);
                node(i_).interval = Interval(1, 0); // invalid
            }
        }
        updated = true;
    }
    else if (left(node(i_)).interval.contains(interval_))
    {
        updated = complementOfImpl(interval_, node(i_).left);
    }
    else if (right(node(i_)).interval.contains(interval_))
    {
        updated = complementOfImpl(interval_, node(i_).right);
    }
    if (updated)
    {
        updateNode(i_);
        balanceNode(i_);
        DEBUG("Updated node " << i_ << ":" << node(i_));
    }
    return updated;
}

const Tree::Node& Tree::node(Index i_) const
{
    ASSERT(i_ < _nodes.size());
    return _nodes[i_];
}

Tree::Node& Tree::node(Index i_)
{
    return const_cast<Node&>(static_cast<const Tree*>(this)->node(i_));
}

const Tree::Node& Tree::left(const Node& node_) const
{
    ASSERT(node_.left < _nodes.size());
    return _nodes[node_.left];
}

Tree::Node& Tree::left(const Node& node_)
{
    return const_cast<Node&>(static_cast<const Tree*>(this)->left(node_));
}

const Tree::Node& Tree::right(const Node& node_) const
{
    ASSERT(node_.right < _nodes.size());
    return _nodes[node_.right];
}

Tree::Node& Tree::right(const Node& node_)
{
    return const_cast<Node&>(static_cast<const Tree*>(this)->right(node_));
}

BOOST_AUTO_TEST_SUITE(IntervalTree)

BOOST_AUTO_TEST_CASE(Preliminary)
{
    BOOST_CHECK_EQUAL(Interval(0, 0), Interval(0, 0).findNthInterval(0));
    BOOST_CHECK_EQUAL(Interval(0, 0), Interval(0, 1).findNthInterval(0));
    BOOST_CHECK_EQUAL(Interval(0, 0), Interval(0, 5).findNthInterval(0));
    BOOST_CHECK_EQUAL(Interval(4, 7), Interval(3, 7).findNthInterval(8));
    BOOST_CHECK_EQUAL(Interval(2, 2), Interval(1, 2).findNthInterval(2));
    BOOST_CHECK_EQUAL(Interval(1e1, 1e1), Interval(1, 1e1).findNthInterval(54));
    BOOST_CHECK_EQUAL(Interval(1e2, 1e2), Interval(1, 1e2).findNthInterval(5049));
    BOOST_CHECK_EQUAL(Interval(1e3, 1e3), Interval(1, 1e3).findNthInterval(500499));

    BOOST_CHECK( Interval(0, 4).contains(Interval(0, 0)));
    BOOST_CHECK( Interval(0, 4).contains(Interval(4, 4)));
    BOOST_CHECK( Interval(0, 4).contains(Interval(0, 4)));
    BOOST_CHECK( Interval(0, 4).contains(Interval(1, 3)));
    BOOST_CHECK(!Interval(0, 4).contains(Interval(1, 5)));
    BOOST_CHECK(!Interval(0, 4).contains(Interval(5, 5)));
    BOOST_CHECK(!Interval(2, 4).contains(Interval(0, 1)));

    BOOST_CHECK(!Interval(0, 4).hasStrictlyWithin(Interval(0, 0)));
    BOOST_CHECK(!Interval(0, 4).hasStrictlyWithin(Interval(4, 4)));
    BOOST_CHECK(!Interval(0, 4).hasStrictlyWithin(Interval(0, 4)));
    BOOST_CHECK( Interval(0, 4).hasStrictlyWithin(Interval(1, 1)));
    BOOST_CHECK( Interval(0, 4).hasStrictlyWithin(Interval(1, 2)));
    BOOST_CHECK( Interval(0, 4).hasStrictlyWithin(Interval(1, 3)));
    BOOST_CHECK( Interval(0, 4).hasStrictlyWithin(Interval(2, 3)));
    BOOST_CHECK( Interval(0, 4).hasStrictlyWithin(Interval(3, 3)));
    BOOST_CHECK(!Interval(0, 4).hasStrictlyWithin(Interval(1, 5)));
    BOOST_CHECK(!Interval(0, 4).hasStrictlyWithin(Interval(5, 5)));
    BOOST_CHECK(!Interval(2, 4).hasStrictlyWithin(Interval(0, 1)));
}

BOOST_AUTO_TEST_CASE(IntervalPerf)
{
    for (std::size_t i = 1; i <= 1e6; ++i)
    {
        BOOST_CHECK_EQUAL(Interval(1e9, 1e9), Interval(1, 1e9).findNthInterval(500000000499999999));
    }
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
        const Interval i = t.findNthInterval(0);
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
    BOOST_CHECK(!t.complementOf(Interval(5, 5)));
    BOOST_CHECK( t.complementOf(Interval(4, 4)));
}

Test::Predicate perfCheck(const Index n_)
{
    Test::TestIssues issues;
    Tree t(Interval(0, n_ - 1));
    // Remove all [<even>, <even>]
    for (Index i = 0; i < n_; i += 2)
    {
        if (!t.complementOf(Interval(i, i)))
        {
            issues << "t.complementOf(Interval(" << i << ", " << i << "))) failed!";
        }
    }
    const Count result = t.countIntervals();
    if (n_ / 2 != result)
    {
        issues << "Result is wrong: " << (n_ / 2) << " != " << result;
    }
    INFO("Max tree depth: " << t.maxDepth());
    return issues.predicate;
}

BOOST_AUTO_TEST_CASE(Medium)
{
    perfCheck(1e4);
}

BOOST_AUTO_TEST_CASE(Large)
{
    perfCheck(1e6);
}

BOOST_AUTO_TEST_CASE(Huge)
{
    Index N = 1e9;
    Tree t(Interval(0, N - 1));
    // Remove all [<even>, <even>]
    for (Index i = 0; i < N; i += 1e3)
    {
        BOOST_REQUIRE(t.complementOf(Interval(i, i)));
    }
    BOOST_REQUIRE_EQUAL(499500000000, t.countIntervals());
}

BOOST_AUTO_TEST_CASE(Wika0)
{
    Tree t(Interval(1, 5));
    {
        const Interval i = t.findNthInterval(6);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(2, 3), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    {
        const Interval i = t.findNthInterval(2);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(4, 5), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    {
        const Interval i = t.findNthInterval(0);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(1, 1), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    BOOST_REQUIRE_EQUAL(0, t.countIntervals());
}

BOOST_AUTO_TEST_CASE(Wika1)
{
    Tree t(Interval(1, 6));
    {
        const Interval i = t.findNthInterval(6);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(2, 2), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    {
        const Interval i = t.findNthInterval(6);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(4, 5), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    {
        const Interval i = t.findNthInterval(2);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(6, 6), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    {
        const Interval i = t.findNthInterval(1);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(3, 3), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    {
        const Interval i = t.findNthInterval(0);
        std::cerr << i << std::endl;
        BOOST_REQUIRE_EQUAL(Interval(1, 1), i);
        BOOST_REQUIRE(t.complementOf(i));
    }
    BOOST_REQUIRE_EQUAL(0, t.countIntervals());
}

BOOST_AUTO_TEST_SUITE_END()
