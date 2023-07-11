#include <algorithm>
#include <compare>
#include <iostream>

#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <vector>
#include <set>
#include <cmath>

double RoundToPrecision(double value, double precision = 0.00001) {
    return std::round(value / precision) * precision;
}

struct TPoint {
    double X = 0;
    double Y = 0;

    auto operator<=>(const TPoint&) const = default;
};

struct TSegment {
    TPoint Begin;
    TPoint End;

    bool Contains(TPoint point) const
    {
        if (point.X > std::max(Begin.X, End.X)|| point.X < std::min(Begin.X, End.X)) {
            return false;
        }

        if (point.Y < std::min(Begin.Y, End.Y) || point.Y > std::max(End.Y, Begin.X)) {
            return false;
        }

        return true;
    }

    auto operator<=>(const TSegment&) const = default;
};

///////////////////////////////////////////////////////////////////////

std::ostream& operator <<(std::ostream& stream, const TPoint& point) {
    stream << "(" << point.X << " , " << point.Y << ")";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const TSegment& segment) {
    stream << "[" << segment.Begin << " , " << segment.End << "]";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const std::set<TSegment>& segments) {
    stream << "{ ";

    for (const auto& segment : segments) {
        stream << segment << ", ";
    }

    stream << " }";
    return stream;
}

///////////////////////////////////////////////////////////////////////

struct TLineParameters {
    std::optional<double> K;
    double C = 0;

    auto operator<=>(const TLineParameters&) const = default;
};

TLineParameters GetLineParameters(const TSegment& segment) {
    const auto& [p1, p2] = segment;
    double k = 0;
    if (p2.X == p1.X) {
        return TLineParameters {
            .C = p1.X,
        };
    }

    k = static_cast<double>(p2.Y - p1.Y) / (p2.X - p1.X);

    double c = p1.Y - p1.X * k;

    return TLineParameters {
        .K = RoundToPrecision(k),
        .C = RoundToPrecision(c),
    };
}

std::optional<TPoint> GetIntersection(const TSegment& s1, const TSegment& s2) {
    auto line1 = GetLineParameters(s1);
    auto line2 = GetLineParameters(s2);

    if (line1.K == line2.K) { 
        return {};
    }

    double x = 0;
    double y = 0;

    if (!line2.K) {
        std::swap(line1, line2);
    }

    if (!line1.K) {
        x = line1.C;
        y = RoundToPrecision(*line2.K * x + line2.C);
    } else {
        x = (line2.C - line1.C) / (*line1.K - *line2.K);
        y = RoundToPrecision(*line1.K * x + line1.C);
        auto y2 = RoundToPrecision(*line2.K * x + line2.C);

        if (y != y2) {
            return {};
        }
    }

    auto result = TPoint{
        .X = RoundToPrecision(x),
        .Y = y,
    };

    if (s1.Contains(result) && s2.Contains(result)) {
        return result;
    }

    return {};
}

///////////////////////////////////////////////////////////////////////

using TIntersections = std::map<TPoint, std::set<TSegment>>;

struct TTest {
    std::vector<TSegment> Input;
    TIntersections Expected;
};

bool CheckTestCase(const TTest& testCases, const TIntersections& output) {
    bool passed = true;

    for (const auto& [point, expectedSegments] : testCases.Expected) {
        auto it = output.find(point);

        if (it == output.end()) {
            passed = false;
            std::cout << "Intersections is not found in results! Expected point: "
                << point << " expected segments:" << expectedSegments << std::endl;
            continue;
        }

        if (expectedSegments != it->second) {
            passed = false;
            std::cout << "Intersections list at point does not match! Point: "
                << point << " expected:" << expectedSegments
                << " actual:" << it->second
                << std::endl;
        }
    }

    for (const auto& [point, unexpected] : output) {
        if (testCases.Expected.count(point) > 0) {
            continue;
        }

        passed = false;
        std::cout << "Unexpected intersection is found in results! Point: "
            << point << " segments:" << unexpected << std::endl;
    }

    return passed;
}

///////////////////////////////////////////////////////////////////////

TIntersections BruteForce(const std::vector<TSegment>& input) {
    TIntersections result;
    for (int i = 0; i < input.size(); ++i) {
        for (int j = 1; j < input.size(); ++j) {
            auto point = GetIntersection(input[i], input[j]);
            if (!point) {
                continue;
            }
            result[*point].insert(input[i]);
            result[*point].insert(input[j]);
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////
//  pdd A = make_pair(1, 1);
//     pdd B = make_pair(4, 4);
//     pdd C = make_pair(1, 8);
//     pdd D = make_pair(2, 4);
int main() {

    std::vector<TTest> tests = {
        {
            .Input = {{{1,1}, {4, 4}}, {{8, 1}, {4, 2}}},
            .Expected = {},
        },
        {
            .Input = { {{1,1}, {4, 4}}, {{8, 1}, {0, 3}} },
            .Expected = { {{RoundToPrecision(2.4), RoundToPrecision(2.4)}, {{{1,1}, {4, 4}}, {{8, 1}, {0, 3}}, }}},
        },
        {
            .Input = {{{3,1}, {7, 5}}, {{5, 5}, {5, 0}}},
            .Expected = { {{RoundToPrecision(5), RoundToPrecision(3)}, {{{{3,1}, {7, 5}}, {{5, 5}, {5, 0}}} }}},
        },
        {
            .Input = {{{4,0}, {4, 4}}, {{0, 2}, {8, 2}}, {{0, -6}, {5, 4}}},
            .Expected = { {{RoundToPrecision(4), RoundToPrecision(2)}, {{{{0, -6}, {5, 4}}, {{4,0}, {4, 4}}, {{0, 2}, {8, 2}}} }}},
        },
    };

    for (const auto& test : tests) {
        if (!CheckTestCase(test, BruteForce(test.Input))) {
            return 1;
        }
    }

    std::cout << "OK" << std::endl;
    return 0;
}