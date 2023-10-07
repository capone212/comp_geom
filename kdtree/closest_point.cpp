#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>
#include <queue>
#include <sstream>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>
#include <assert.h>
#include <memory>
#include <unordered_set>
#include <cmath>
#include <limits.h>
#include <queue>

////////////////////////////////////////////////////////////////////////////////////

bool DebugIsDisabled = false;

#define debugStream \
    if (DebugIsDisabled) {} \
    else std::cerr

////////////////////////////////////////////////////////////////////////////////////

struct TPoint
{
    int X = 0;
    int Y = 0;

    bool operator==(const TPoint& other) const
    {
        return X == other.X && Y == other.Y;
    }
};

////////////////////////////////////////////////////////////////////////////////////

double Distance(TPoint p1, TPoint p2)
{
    auto xdiff = p1.X - p2.X;
    auto ydiff = p1.Y - p2.Y;

    return std::sqrt(xdiff * xdiff + ydiff * ydiff);
}

////////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline void hash_combine(std::size_t &seed, const T &val) {
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// auxiliary generic functions to create a hash value using a seed
template <typename T> inline void hash_val(std::size_t &seed, const T &val) {
    hash_combine(seed, val);
}

template <typename T, typename... Types>
inline void hash_val(std::size_t &seed, const T &val, const Types &... args) {
    hash_combine(seed, val);
    hash_val(seed, args...);
}

template <typename... Types>
inline std::size_t hash_val(const Types &... args) {
    std::size_t seed = 0;
    hash_val(seed, args...);
    return seed;
}

struct TPointHash {
    std::size_t operator()(const TPoint& p) const {
        return hash_val(p.X, p.Y);
    }
};

////////////////////////////////////////////////////////////////////////////////////

struct TOrderByX
{
    bool operator()(const TPoint& left, const TPoint& right) const
    {
        return std::tie(left.X, left.Y) < std::tie(right.X, right.Y);
    }
};

struct TOrderByY
{
    bool operator()(const TPoint& left, const TPoint& right) const
    {
        return std::tie(left.Y, left.X) < std::tie(right.Y, right.X);
    }
};

////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator <<(std::ostream& stream, const TPoint& point) {
    stream << "{" << point.X << " , " << point.Y << "}";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const std::vector<TPoint>& points) {
    stream << "{ ";

    for (const auto& point : points) {
        stream << point << ", ";
    }

    stream << " }";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const std::set<TPoint, TOrderByX>& points) {
    stream << "{ ";

    for (const auto& point : points) {
        stream << point << ", ";
    }

    stream << " }";
    return stream;
}

////////////////////////////////////////////////////////////////////////////////////

struct TDistancePair
{
    double Distance = 0;
    TPoint Point;

    bool operator<(const TDistancePair& other) const
    {
        return Distance < other.Distance;
    }
};

////////////////////////////////////////////////////////////////////////////////////

struct TNode;
using PNode = std::shared_ptr<TNode>;

struct TNode
{
    std::optional<int> X;
    std::optional<int> Y;
    PNode Left;
    PNode Right;

    explicit TNode(std::optional<int> x, std::optional<int> y)
        : X(x)
        , Y(y)
    { }

    bool IsLeaf() const
    {
        return X.has_value() && Y.has_value();
    }
};

void splitByPredicate(
    const std::vector<TPoint>& input, 
    std::vector<TPoint>& lower,
    std::vector<TPoint>& upper,
    auto predicate)
{
    for (const auto& point : input) {
        if (predicate(point)) {
            lower.push_back(point);
        } else {
            upper.push_back(point);
        }
    }
}

PNode ConstructKdTreeRecursive(const std::vector<TPoint>& orderedByX, const std::vector<TPoint>& orderedByY, int depth = 0)
{
    assert(orderedByX.size() == orderedByY.size());

    if (orderedByX.empty())
    {
        return {};
    }

    if (orderedByX.size() == 1) {
        debugStream << "leaf node: " << orderedByX.front() << std::endl;
        return std::make_shared<TNode>(orderedByX.front().X, orderedByX.front().Y);
    }

    auto root = std::make_shared<TNode>(std::nullopt, std::nullopt);

    std::vector<TPoint> lowerByX;
    std::vector<TPoint> lowerByY;

    std::vector<TPoint> upperByX;
    std::vector<TPoint> upperByY;

    std::function<bool (TPoint)> lowerThanMedian;

    if (depth % 2 == 0) {
        // Split by x
        auto median = orderedByX[(orderedByX.size()) / 2];

        debugStream << "x median " << median << std::endl;

        root->X = median.X;
        lowerThanMedian = [median] (TPoint point) {
            return TOrderByX{}(point, median);
        };
    } else {
        // Split by Y
        auto median = orderedByY[orderedByY.size() / 2];

        debugStream << "y median " << median << std::endl;

        root->Y = median.Y;

        lowerThanMedian = [median] (TPoint point) {
            return TOrderByY{}(point, median);
        };
    }

    splitByPredicate(orderedByX, lowerByX, upperByX, lowerThanMedian);
    splitByPredicate(orderedByY, lowerByY, upperByY, lowerThanMedian);

    root->Left = ConstructKdTreeRecursive(lowerByX, lowerByY, depth + 1);
    root->Right = ConstructKdTreeRecursive(upperByX, upperByY, depth + 1);

    return root;
}

PNode ConstructKDTree(const std::vector<TPoint>& input)
{
    auto orderedByX = input;
    std::sort(orderedByX.begin(), orderedByX.end(), TOrderByX{});

    auto orderedByY = input;
    std::sort(orderedByY.begin(), orderedByY.end(), TOrderByY{});

    return ConstructKdTreeRecursive(orderedByX, orderedByY);
}

bool IsInRange(int value, int lower, int upper)
{
    return value >= lower && value <= upper;
}

int GetClosestFromRange(int value, int lower, int upper)
{
    if (IsInRange(value, lower, upper)) {
        return value;
    }

    return std::abs(value - lower) < std::abs(value - upper) ? lower : upper;
}

void TraverseKDTree(
    PNode kdTree,
    TPoint thePoint,
    std::optional<TDistancePair>& best)
{
    if (!kdTree) {
        return;
    }

    struct TNodePriority
    {
        int Distance = 0;
        PNode Node;

        int LowerX = INT_MIN / 2;
        int UpperX = INT_MAX / 2;
        int LowerY = INT_MIN / 2;
        int UpperY = INT_MAX / 2;

        bool operator<(const TNodePriority& other) const
        {
            return other.Distance < Distance;
        }

        TPoint Closest(const TPoint& thePoint)
        {
            return TPoint {
                .X = GetClosestFromRange(thePoint.X, LowerX, UpperX),
                .Y = GetClosestFromRange(thePoint.Y, LowerY, UpperY),
            };
        }

        void SetDistance(const TPoint& thePoint)
        {
            auto closest = Closest(thePoint);
            Distance = ::Distance(closest, thePoint);
        }
    };

    std::priority_queue<TNodePriority> pq;
    pq.push({
        .Node = kdTree,
    });

    while (!pq.empty()) {
        auto next = pq.top();
        pq.pop();

        const auto& node = next.Node;

        if (node->IsLeaf()) {
            auto point = TPoint{*node->X, *node->Y};

            TDistancePair current {
                .Distance = Distance(point, thePoint),
                .Point = point,
            };

            debugStream << "traverse check node: " << point << std::endl;

            if (!best || current < *best)
            {
                best = current;
            }
            continue;;
        }

        // Check feasibility
        if (best)
        {
            if (Distance(next.Closest(thePoint), thePoint) > best->Distance) {
                // There is no point to visit this square.
                continue;;
            }
        }

        auto lower = next;
        lower.Node = node->Left;

        auto upper = next;
        upper.Node = node->Right;

        if (node->X) {
            auto x = *node->X;
            debugStream << "traverse visit x edge: " << x  << std::endl;
            lower.UpperX = x; 
            upper.LowerX = x; 
        } else {
            auto y = *node->Y;
            debugStream << "traverse visit y edge: " << y  << std::endl;

            lower.UpperY = y; 
            upper.LowerY = y; 
        }

        for (auto child : {lower, upper}) {
            if (!child.Node) {
                continue;
            }

            child.SetDistance(thePoint);
            pq.push(child);
        }
    }
}

std::vector<TPoint> KDTree(const std::vector<TPoint>& input, const TPoint& thePoint)
{
    auto kdTree = ConstructKDTree(input);

    // Separate construction and search.
    debugStream << std::endl;

    std::optional<TDistancePair> best;
    TraverseKDTree(kdTree, thePoint, best);

    if (!best) {
        return {};
    }

    return { best->Point };
}

////////////////////////////////////////////////////////////////////////////////////

std::vector<TPoint> BruteForce(const std::vector<TPoint>& input, const TPoint& thePoint)
{
    TPoint result = input.front();
    int minDistance = Distance(thePoint, result);

    for (const auto& p : input) {
        auto distance = Distance(thePoint, p);
        if (distance < minDistance) {
            minDistance = distance;
            result = p;
        }

    }

    return { result };
}

struct TTestCase
{
    std::vector<TPoint> Input;
    TPoint ThePoint;
    std::set<TPoint, TOrderByX> Output;
};


///////////////////////////////////////////////////////////////////////////////////////////////

int RandomInRange(int min, int max)
{
    int total = std::abs(min) + max;
    return rand() % total - std::abs(min);
}

TPoint RandomPoint(int min, int max)
{
    return {.X = RandomInRange(min, max), .Y = RandomInRange(min, max)};
}

void CheckTestCase(const TTestCase& testCase)
{
    auto results = KDTree(testCase.Input, testCase.ThePoint);

    std::set<TPoint, TOrderByX> indexed(results.begin(), results.end());
    bool failed = false;

    if (std::ssize(results) != std::ssize(testCase.Output)) {
        std::cout << "Result size does not match. Expected: " << std::ssize(testCase.Output) << " got: " << std::ssize(results) << std::endl;;
        failed = true;
    }

    for (const auto& p : testCase.Output) {
        if (indexed.count(p) == 0) {
            std::cout << "Did not report expecting point: x: " << p.X << ", y:" << p.Y << std::endl;
            failed = true;
        }
    }

    for (const auto& p : results) {
        if (testCase.Output.count(p) == 0) {
            std::cout << "Unexpected point is reported: x: " << p.X << ", y:" << p.Y << std::endl;
            failed = true;
        }
    }

    if (failed) {
        std::cout << "Input: " << testCase.Input << std::endl;
        std::cout << "Expected output: " << testCase.Output << std::endl;

        exit(-1);
    }
}

void StressTest()
{
    static const int PointsCount = 1000;
    std::unordered_set<TPoint, TPointHash> index;

    for (int i = 0; i < PointsCount; ++i) {
        index.insert(RandomPoint(0, 100));
    }

    TTestCase testCase = {
        .ThePoint = {30, 30},
    };

    testCase.Input.assign(index.begin(), index.end());
    auto output = BruteForce(testCase.Input, testCase.ThePoint);
    testCase.Output.insert(output.begin(), output.end());

    CheckTestCase(testCase);
}

///////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    std::vector<TTestCase> tests {
        // {
        //     .Input = {{-10, -10}, {0, 0}, {10, 10}, {20, 20}},
        //     .ThePoint = {19, 19},
        //     .Output = {{20, 20}},
        // },
        {
            .Input = { {0 , 6}, {9 , 1}, {6 , 2}, {0 , 9}, {3 , 5}, {2 , 6}, {7 , 5}, {2 , 7}, {3 , 6}, },
            .ThePoint = {6, 6},
            .Output = { {7 , 5} },
        }
    };

    for (const auto& testCase : tests) {
        CheckTestCase(testCase);
    }

    // for (int i = 0; i < 1000; ++i) {
    //     StressTest();
    // }

    return 0;
}