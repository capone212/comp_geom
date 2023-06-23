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
    double Y = 0;
    double X = 0;

    auto operator<=>(const TPoint&) const = default;
};

struct TSegment {
    TPoint Begin;
    TPoint End;

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
    double K = 0;
    double C = 0;

    auto operator<=>(const TLineParameters&) const = default;
};

TLineParameters GetLineParameters(const TSegment& segment) {
    const auto& [p1, p2] = segment;
    double k = 0;
    if (p2.X - p1.X != 0) {
        k = static_cast<double>(p2.Y - p1.Y) / (p2.X - p1.X);
    }

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

    auto x = (line2.C - line1.C) / (line1.K - line2.K);
    auto y1 = RoundToPrecision(line1.K * x + line1.C);
    auto y2 = RoundToPrecision(line2.K * x + line2.C);

    if (y1 != y2) {
        return {};
    }

    return TPoint{
        .Y = y1,
        .X = RoundToPrecision(x),
    };
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
            std::cout << "Intersections at list does not match! Point: "
                << point << " expected:" << expectedSegments
                << point << " actual:" << it->second
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
    TTest test{
        .Input = {{{1,1}, {4, 4}}, {{1, 8}, {2, 4}}},
        .Expected = { {{RoundToPrecision(2.4), RoundToPrecision(2.4)}, {{{1,1}, {4, 4}}, {{1, 8}, {2, 4}}}}, },
    };

    if (!CheckTestCase(test, BruteForce(test.Input))) {
        return 1;
    }

    std::cout << "OK" << std::endl;
    return 0;
}