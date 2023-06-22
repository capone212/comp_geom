#include <algorithm>
#include <compare>
#include <iostream>

#include <map>
#include <ostream>
#include <sstream>
#include <vector>
#include <set>
#include <cmath>

double RoundTo(double value, double precision = 0.00001)
{
    return std::round(value / precision) * precision;
}

struct TPoint {
    int Y = 0;
    int X = 0;

    auto operator<=>(const TPoint&) const = default;
};

struct TSegment {
    TPoint Begin;
    TPoint End;

    auto operator<=>(const TSegment&) const = default;
};

///////////////////////////////////////////////////////////////////////

std::ostream& operator <<(std::ostream& stream, const TPoint& point) {
    stream << "(" << point.X << " , " << point.Y << ")" << std::endl;
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const TSegment& segment) {
    stream << "[" << segment.Begin << " , " << segment.End << "]" << std::endl;
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

///////////////////////////////////////////////////////////////////////

using TIntersections = std::map<TPoint, std::set<TSegment>>;

struct TTest {
    std::vector<TSegment> Input;
    TIntersections Expected;
};

bool CheckTestCase(const TTest& testCases, const TIntersections& output) {
    bool failed = false;

    for (const auto& [point, expectedSegments] : testCases.Expected) {
        auto it = output.find(point);

        if (it == output.end()) {
            failed = true;
            std::cout << "Intersections is not found in results! Expected point: "
                << point << " expected segments:" << expectedSegments << std::endl;
        }

        if (expectedSegments != it->second) {
            failed = true;
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

        failed = true;
        std::cout << "Unexpected intersection is found in results! Point: "
            << point << " segments:" << unexpected << std::endl;
    }

    return failed;
}

///////////////////////////////////////////////////////////////////////

// TIntersections BruteForce(const std::vector<TSegment>& input) {
//     for (const auto& x: input) {
//         for (const auto& y: input) {

//         }
//     }
// }

///////////////////////////////////////////////////////////////////////

int main() {
    return 0;
}