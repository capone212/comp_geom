#include <cstdlib>
#include <tuple>
#include <iostream>
#include <vector>
#include <set>

struct TPoint
{
    int X = 0;
    int Y = 0;

    bool operator=(const TPoint& other)
    {
        return X == other.X && Y == other.Y;
    }
};

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

int main()
{
    std::vector<TTestCase> tests {
        {
            .Input = {{-10, -10}, {0, 0}, {10, 10}, {20, 20}},
            .Lower = {0, 0},
            .Upper = {10, 10},
            .Output = {{0, 0}, {10, 11}},
        }
    };

    for (const auto& testCase : tests) {
        auto results = BruteForce(testCase.Input, testCase.Lower, testCase.Upper);
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
            exit(-1);
        }
    }

    return 0;
}