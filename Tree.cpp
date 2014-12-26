#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Bit

#include "Test.h"

#include <cstdint>
#include <deque>
#include <iostream>
#include <utility>

#include <boost/test/unit_test.hpp>

// #define DEBUG

#ifdef DEBUG
#define debug(x) std::cerr << "DEBUG: " << x << std::endl
#else
#define debug(x)
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

  private:
    Index _a;
    Index _b;
};

Interval::Interval(Index a_, Index b_)
  : _a(a_), _b(b_)
{
    if (_b < _a)
    {
        // throw
    }
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
    // TODO: too slow
    Index a2 = _a;
    Index b2 = _a;
    while (0 < nth_--)
    {
        if (b2 < _b)
        {
            ++b2;
        }
        else
        {
            assert(a2 < _b);
            ++a2;
            b2 = a2;
        }
    }
    return Interval(a2, b2);
}

bool Interval::contains(const Interval& i_) const
{
    return _a <= i_._a && i_._b <= _b;
}

bool Interval::hasStrictlyWithin(const Interval& i_) const
{
    return _a < i_._a && i_._b < _b;
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

    typedef std::deque<Node> NodeList;

    Tree(const Interval& interval_);

    Count countIntervals() const;
    // NOTE: zero-indexed
    Interval findNthInterval(Count nth_) const;
    bool complementOf(const Interval& interval_);

  private:
    Interval findNthIntervalImpl(Count nth_, Index i_) const;
    Index addNode(const Interval& interval_);
    void updateNode(Index i_);
    void balanceNode(Index i_);
    bool complementOfImpl(const Interval& interval_, Index i_);
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
                << "{ interval : " << node_.interval << "\n"
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

Interval Tree::findNthIntervalImpl(Count nth_, Index i_) const
{
    const Node& node(_nodes[i_]);
    if (node.isLeaf)
    {
        return node.interval.findNthInterval(nth_);
    }
    const Count leftCount = left(node).count;
    if (nth_ < leftCount)
    {
        return findNthIntervalImpl(nth_, node.left);
    }
    nth_ -= leftCount;
    assert(nth_ < node.count);
    return findNthIntervalImpl(nth_, node.right);
}

Index Tree::addNode(const Interval& interval_)
{
    Index i = _nodes.size();
    _nodes.push_back({true, interval_, 0, 0, interval_.numIntervals(), 0});
    debug("Created new node " << i << ":" << _nodes[i]);
    return i;
}

void Tree::updateNode(Index i_)
{
    Node& node(_nodes[i_]);
    if (node.isLeaf)
    {
        node.count = node.interval.numIntervals();
        node.depth = 0;
        return;
    }
    node.interval = Interval(left(node).interval.a(), right(node).interval.b());
    node.count = left(node).count + right(node).count;
    node.depth = std::max(1 + left(node). depth, 1 + right(node).depth);
}

void Tree::balanceNode(Index i_)
{
    Node& node(_nodes[i_]);
    if (node.isLeaf)
    {
        return;
    }
    int depthDiff = left(node).depth - right(node).depth;
    if (1 < depthDiff)
    {
        debug("Rotate right");
        //     Q        P
        //    / \      / \ .
        //   P   C    A   Q
        //  / \          / \.
        // A   B        B   C
        //
        Index P = node.left;
        Index Q = i_;
        Index B = _nodes[P].right;
        _nodes[P].right = Q;
        _nodes[Q].left  = B;
        std::swap(_nodes[P], _nodes[Q]);
        assert(_nodes[i_].right == Q);
        updateNode(Q);
        updateNode(P);
    }
    if (depthDiff < -1)
    {
        debug("Rotate left");
        //   P          Q
        //  / \        / \.
        // A   Q      P   C
        //    / \    / \.
        //   B   C  A   B
        //
        Index P = i_;
        Index Q = node.right;
        Index B = _nodes[Q].left;
        _nodes[P].right = B;
        _nodes[Q].left  = P;
        std::swap(_nodes[P], _nodes[Q]);
        assert(_nodes[i_].left == P);
        updateNode(P);
        updateNode(Q);
    }
}

bool Tree::complementOfImpl(const Interval& interval_, Index i_)
{
    Node& node(_nodes[i_]);
    debug("Processing complement of node " << i_ << ":" << node);
    if (!node.interval.contains(interval_))
    {
        debug(node.interval << " does not contain " << interval_);
        return false;
    }
    bool updated = false;
    if (node.isLeaf)
    {
        const Index a  = node.interval.a();
        const Index b  = node.interval.b();
        const Index ap = interval_.a();
        const Index bp = interval_.b();
        if (node.interval.hasStrictlyWithin(interval_))
        {
            node.isLeaf = false;
            node.left   = addNode(Interval(a,      ap - 1));
            node.right  = addNode(Interval(bp + 1, b     ));
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
        }
        updated = true;
    }
    else if (left(node).interval.contains(interval_))
    {
        updated = complementOfImpl(interval_, node.left);
    }
    else if (right(node).interval.contains(interval_))
    {
        updated = complementOfImpl(interval_, node.right);
    }
    if (updated)
    {
        updateNode(i_);
        balanceNode(i_);
        debug("Updated node " << i_ << ":" << node);
    }
    return updated;
}

const Tree::Node& Tree::left(const Node& node_) const
{
    assert(node_.left < _nodes.size());
    return _nodes[node_.left];
}

Tree::Node& Tree::left(const Node& node_)
{
    return const_cast<Node&>(static_cast<const Tree*>(this)->left(node_));
}

const Tree::Node& Tree::right(const Node& node_) const
{
    assert(node_.right < _nodes.size());
    return _nodes[node_.right];
}

Tree::Node& Tree::right(const Node& node_)
{
    return const_cast<Node&>(static_cast<const Tree*>(this)->right(node_));
}

BOOST_AUTO_TEST_SUITE(IntervalTree)

Test::Predicate someCheck()
{
    Test::TestIssues issues;
    issues << "Sorry, there were issues";
    return issues.predicate;
}

BOOST_AUTO_TEST_CASE(Preliminary)
{
    BOOST_CHECK_EQUAL(Interval(0, 0), Interval(0, 0).findNthInterval(0));
    BOOST_CHECK_EQUAL(Interval(0, 0), Interval(0, 1).findNthInterval(0));
    BOOST_CHECK_EQUAL(Interval(0, 0), Interval(0, 5).findNthInterval(0));
    BOOST_CHECK_EQUAL(Interval(4, 7), Interval(3, 7).findNthInterval(8));
    BOOST_CHECK_EQUAL(Interval(2, 2), Interval(1, 2).findNthInterval(2));
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

BOOST_AUTO_TEST_CASE(Large)
{
    Index N = 1e4;
    Tree t(Interval(0, N - 1));

    // Remove all [<even>, <even>]
    for (Index i = 0; i < N; i += 2)
    {
        BOOST_REQUIRE(t.complementOf(Interval(i, i)));
    }
    BOOST_REQUIRE_EQUAL(N / 2, t.countIntervals());

}

BOOST_AUTO_TEST_SUITE_END()
