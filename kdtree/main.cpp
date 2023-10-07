#include <cstdlib>
#include <memory>
#include <optional>
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

////////////////////////////////////////////////////////////////////////////////////

bool DebugIsDisabled = true;

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

void TraverseKDTree(PNode root, std::vector<TPoint>& results, TPoint lower, TPoint upper)
{
    if (!root) {
        return;
    }

    if (root->IsLeaf()) {
        auto point = TPoint{*root->X, *root->Y};

        debugStream << "traverse check node: " << point << std::endl;

        if (IsInRange(point.X, lower.X, upper.X) && IsInRange(point.Y, lower.Y, upper.Y))
        {
            results.push_back(point);
        }
        return;
    }

    if (root->X) {
        auto x = *root->X;

        debugStream << "traverse visit x edge: " << x  << std::endl;

        if (x >= lower.X) {
            TraverseKDTree(root->Left, results, lower, upper);
        }
        if (x <= upper.X) {
            TraverseKDTree(root->Right, results, lower, upper);
        }
    } else {
        auto y = *root->Y;
        debugStream << "traverse visit y edge: " << y  << std::endl;

        if (y >= lower.Y) {
            TraverseKDTree(root->Left, results, lower, upper);
        }
        if (y <= upper.Y) {
            TraverseKDTree(root->Right, results, lower, upper);
        }
    }
}

std::vector<TPoint> KDTree(const std::vector<TPoint>& input, const TPoint& lower, const TPoint& upper)
{
    auto kdTree = ConstructKDTree(input);

    std::vector<TPoint> results;
    TraverseKDTree(kdTree, results, lower, upper);

    return results;
}

////////////////////////////////////////////////////////////////////////////////////

std::vector<TPoint> BruteForce(const std::vector<TPoint>& input, const TPoint& lower, const TPoint& upper)
{
    std::vector<TPoint> result;
    for (const auto& p : input)
    {
        if (p.X >= lower.X && p.Y >= lower.Y && p.X <= upper.X && p.Y <= upper.Y)
        {
            result.push_back(p);
        }
    }
    return result;
}

struct TTestCase
{
    std::vector<TPoint> Input;
    TPoint Lower;
    TPoint Upper;
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
    auto results = KDTree(testCase.Input, testCase.Lower, testCase.Upper);
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
        .Lower = {30, 30},
        .Upper = {60, 60},
    };

    testCase.Input.assign(index.begin(), index.end());
    auto output = BruteForce(testCase.Input, testCase.Lower, testCase.Upper);
    testCase.Output.insert(output.begin(), output.end());

    CheckTestCase(testCase);
}

///////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    std::vector<TTestCase> tests {
        {
            .Input = {{-10, -10}, {0, 0}, {10, 10}, {20, 20}},
            .Lower = {0, 0},
            .Upper = {10, 10},
            .Output = {{0, 0}, {10, 10}},
        },
        {
            .Input = { {0 , 6}, {9 , 1}, {6 , 2}, {0 , 9}, {3 , 5}, {2 , 6}, {7 , 5}, {2 , 7}, {3 , 6}, },
            .Lower = {3, 3},
            .Upper = {6, 6},
            .Output = { {3 , 5}, {3 , 6}, },
        }
    };

    for (const auto& testCase : tests) {
        CheckTestCase(testCase);
    }

    for (int i = 0; i < 1000; ++i) {
        StressTest();
    }

    return 0;
}