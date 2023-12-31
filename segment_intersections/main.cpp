#include <algorithm>
#include <compare>
#include <iostream>

#include <iterator>
#include <map>
#include <math.h>
#include <optional>
#include <ostream>
#include <sstream>

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include <utility>
#include <vector>
#include <set>
#include <cmath>
#include <assert.h>

bool DebugIsDisabled = true;

#define debugStream \
    if (DebugIsDisabled) {} \
    else std::cerr

static constexpr double Precision = 0.000001;

double RoundToPrecision(double value, double precision = Precision) {
    return std::round(value / precision) * precision;
}

struct TPoint
{
    double X = 0;
    double Y = 0;

    auto operator<=>(const TPoint&) const = default;

    void Normalize()
    {
        X = RoundToPrecision(X);
        Y = RoundToPrecision(Y);
    }
};

struct TSegment
{
    TPoint Begin;
    TPoint End;

    void Normalize()
    {
        Begin.Normalize();
        End.Normalize();

        if (End < Begin) {
            std::swap(Begin, End);
        }
    }

    bool Contains(TPoint point) const
    {
        if (point.X > std::max(Begin.X, End.X) || point.X < std::min(Begin.X, End.X)) {
            return false;
        }

        if (point.Y < std::min(Begin.Y, End.Y) || point.Y > std::max(Begin.Y, End.Y)) {
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

std::ostream& operator <<(std::ostream& stream, const std::vector<TSegment>& segments) {
    stream << "{ ";

    for (const auto& segment : segments) {
        stream << segment << ", ";
    }

    stream << " }";
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const std::unordered_set<const TSegment*>& segments) {
    stream << "{ ";

    for (const auto* segment : segments) {
        stream << *segment << ", ";
    }

    stream << " }";
    return stream;
}

///////////////////////////////////////////////////////////////////////

struct TLineParameters
{
    std::optional<double> K;
    double C = 0;

    auto operator<=>(const TLineParameters&) const = default;

    std::optional<double> GetAtX(double x) const
    {
        if (!K) {
            return {};
        }

        return RoundToPrecision(*K * x + C);
    }
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
        .K = k,
        .C = c,
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
        y = *line2.K * x + line2.C;
    } else {
        x = (line2.C - line1.C) / (*line1.K - *line2.K);
        y = *line1.K * x + line1.C;
        auto y2 = *line2.K * x + line2.C;
        if (std::abs(y - y2) > 2 * Precision) {
            debugStream << "intersections does not match :" << x << " y1:" << y << " y2:" << y2 << std::endl;
            return {};
        }
    }

    auto result = TPoint{
        .X = RoundToPrecision(x),
        .Y = RoundToPrecision(y),
    };

    if (s1.Contains(result) && s2.Contains(result)) {
        return result;
    }

    debugStream << "intersection is not contained point:"
        << result
        <<" s1" << s1 << " contains:" << s1.Contains(result)
        <<" s2" << s2 << " contains:" << s2.Contains(result)
        << std::endl;

    return {};
}

///////////////////////////////////////////////////////////////////////

using TIntersections = std::map<TPoint, std::set<TSegment>>;

struct TTest
{
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
    for (int i = 0; i < std::ssize(input); ++i) {
        for (int j = i+1; j < std::ssize(input); ++j) {
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

struct TEvent
{
    std::unordered_set<const TSegment*> Starting;
    std::unordered_set<const TSegment*> Intersecting;
    std::unordered_set<const TSegment*> Ending;
};

struct TTrackingSegment
{
    const TSegment* Segment = nullptr;
    TLineParameters Parameters;
    double StartX = 0;

    TTrackingSegment(const TSegment* segment, double startX)
        : Segment(segment)
        , Parameters(GetLineParameters(*segment))
        , StartX(startX)
    { }

    bool operator<(const TTrackingSegment& other) const {
        auto startX = std::max(StartX, other.StartX);
        auto endX = std::max(Segment->End.X, other.Segment->End.X);
        
        auto leftStart = Parameters.GetAtX(startX);
        auto leftEnd = Parameters.GetAtX(endX);

        auto rightStart = other.Parameters.GetAtX(startX);
        auto rightEnd = other.Parameters.GetAtX(endX);

        return std::make_pair(*leftStart, *leftEnd) < std::make_pair(*rightStart, *rightEnd);
    }
};

void Verify(bool predicate)
{
    if (!predicate) {
        throw std::runtime_error("Critical condition is failed");
    }
}

struct TSweepingLine
{
    using TSegmentsTree = std::multiset<TTrackingSegment>;

    TSegmentsTree Segments;
    std::unordered_map<const TSegment*, TSegmentsTree::iterator> Index;

    void Add(const TSegment* segment, double startX)
    {
        Verify(Index.count(segment) == 0);
        auto it = Segments.insert(TTrackingSegment(segment, startX));
        Index.insert(std::make_pair(segment, it));
    }

    void Remove(const TSegment* segment)
    {
        auto it = Index.find(segment);
        Verify(it != Index.end());

        Segments.erase(it->second);
        Index.erase(it);
    }

    std::array<const TSegment*, 2> GetNeighbors(const TSegment* segment)
    {
        auto indexIt = Index.find(segment);
        Verify(indexIt != Index.end());

        auto it = indexIt->second;

        std::array<const TSegment*, 2> result = {};

        if (it != Segments.begin()) {
            auto before = std::prev(it);
            result[0] = before->Segment;
        }

        auto after = std::next(it);
        if (after != Segments.end()) {
            result[1] = after->Segment;
        }

        return result;
    }
};

std::map<TPoint, TEvent> MakeEventQueue(const std::vector<TSegment>& input)
{
    std::map<TPoint, TEvent> result;
    for (const auto& segment : input) {
        result[segment.Begin].Starting.insert(&segment);
        result[segment.End].Ending.insert(&segment);
    }
    return result;
}

TIntersections SweepLine(const std::vector<TSegment>& input)
{
    auto queue = MakeEventQueue(input);
    TSweepingLine sweepLine;
    TIntersections result;

    auto handleIntersections = [&] (const TPoint& eventPoint, const TSegment* left, const TSegment* right) {
        if (left == nullptr || right == nullptr) {
            return;
        }
        auto intersection = GetIntersection(*left, *right);
        if (!intersection || *intersection < eventPoint) {
            return;
        }

        debugStream << "found intersection point:" << *intersection << std::endl;

        auto& item = queue[*intersection];

        item.Intersecting.insert(left);
        item.Intersecting.insert(right);
    };

    auto addSegment = [&] (const TSegment* segment, const TPoint& eventPoint) {
        debugStream << "adding segment:" << segment  << " value:" << *segment << std::endl;
        sweepLine.Add(segment, eventPoint.X);

        for (const auto* neighbor : sweepLine.GetNeighbors(segment)) {
            debugStream << "check neighbor:" << neighbor << std::endl;
            handleIntersections(eventPoint, segment, neighbor);
        }
    };

    while(!queue.empty()) {
        auto current = queue.begin();
        const auto& [eventPoint, event] = *current;

        for (const auto* segment : event.Starting) {
            addSegment(segment, eventPoint);
        }

        for (const auto* segment : event.Intersecting) {
            debugStream << "handling intersection:" << eventPoint << " segment:" << *segment << std::endl;
            result[eventPoint].insert(*segment);
            sweepLine.Remove(segment);
        }

        for (const auto* segment : event.Intersecting) {
            auto moved = eventPoint;
            moved.X += Precision;
            addSegment(segment, moved);
        }

        for (const auto* segment : event.Ending) {
            debugStream << "removing segment:" << eventPoint << " segment:" << *segment << std::endl;
            auto [before, after] = sweepLine.GetNeighbors(segment);
            sweepLine.Remove(segment);
            handleIntersections(eventPoint, before, after);
        }

        Verify(queue.begin() == current);
        queue.erase(current);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////

void Normalize(TTest& testCase)
{
    for (auto& segment : testCase.Input) {
        segment.Normalize();
    }

    for (auto& [point, segments] : testCase.Expected) {
        std::set<TSegment> normalized;
        for (auto segment : segments) {
            segment.Normalize();
            normalized.insert(segment);
        }
        normalized.swap(segments);
    }
}

void Normalize(std::vector<TTest>& testCases)
{
    for (auto& testCase : testCases) {
        Normalize(testCase);
    }
}

double RandomInRange(int min, int max)
{
    int total = std::abs(min) + max;
    return rand() % total - std::abs(min);
}

TPoint RandomPoint(int min, int max)
{
    return {.X = RandomInRange(min, max), .Y = RandomInRange(min, max)};
}

int StressTest()
{
    static const int SegmentsCount = 3;
    static const int TestsCount = 1000000;

    int minValue = -10;
    int maxValue = 10;

    for (int i = 0; i < TestsCount; ++i) {
        std::unordered_set<double> uniqueness;
        TTest testCase;

        for (int segmentIndex = 0; segmentIndex < SegmentsCount; ++segmentIndex) {
            TSegment segment {
                .Begin = RandomPoint(minValue, maxValue),
                .End = RandomPoint(minValue, maxValue),
            };

            segment.Normalize();
            auto parameters = GetLineParameters(segment);
            if (!parameters.K) {
                continue;
            }

            if (!uniqueness.insert(*parameters.K).second) {
                continue;
            }

            testCase.Input.push_back(std::move(segment));
            testCase.Expected = BruteForce(testCase.Input);
            if (!CheckTestCase(testCase, SweepLine(testCase.Input))) {
                std::cout << "Input: " << testCase.Input << std::endl;

                for (const auto& [point, segments] : testCase.Expected) {
                    std::cout << "Intersection : " << point  << " segments: " << segments << std::endl;
                }
                return 1;
            }
        }
    }

    std::cout << "OK" << std::endl;
    return 0;
}

int test()
{
    std::vector<TTest> tests = {
        // {
        //     .Input = {{{1,1}, {4, 4}}, {{8, 1}, {4, 2}}},
        //     .Expected = {},
        // },
        // {
        //     .Input = { {{1,1}, {4, 4}}, {{8, 1}, {0, 3}} },
        //     .Expected = { {{RoundToPrecision(2.4), RoundToPrecision(2.4)}, {{{1,1}, {4, 4}}, {{8, 1}, {0, 3}}, }}},
        // },
        // {
        //     .Input = {{{3,1}, {7, 5}}, {{5, 5}, {5, 0}}},
        //     .Expected = { {{RoundToPrecision(5), RoundToPrecision(3)}, {{{{3,1}, {7, 5}}, {{5, 5}, {5, 0}}} }}},
        // },
        // {
        //     .Input = {{{4,0}, {4, 4}}, {{0, 2}, {8, 2}}, {{0, -6}, {5, 4}}},
        //     .Expected = { {{RoundToPrecision(4), RoundToPrecision(2)}, {{{{0, -6}, {5, 4}}, {{4,0}, {4, 4}}, {{0, 2}, {8, 2}}} }}},
        // },
        // {
        //     .Input = {{{3, -4}, {5, 0}}, {{1, -10}, {6, 3}}, {{-8, 0}, {6, -9}}},
        //     .Expected = {
        //         {{RoundToPrecision(2.29956), RoundToPrecision(-6.62115)}, {{{{-8, 0}, {6, -9}}, {{1, -10}, {6, 3}}, } }},
        //         {{RoundToPrecision(4.33333), RoundToPrecision(-1.33333)}, {{{{1, -10}, {6, 3}}, {{3, -4}, {5, 0}}, } }}
        //     },
        // },
        // {
        //     .Input = {{{3, -4}, {5, 0}}, {{1, -10}, {6, 3}}, {{-8, 0}, {6, -9}}},
        //     .Expected = {
        //         {{RoundToPrecision(2.29956), RoundToPrecision(-6.62115)}, {{{{-8, 0}, {6, -9}}, {{1, -10}, {6, 3}}, } }},
        //         {{RoundToPrecision(4.33333), RoundToPrecision(-1.33333)}, {{{{1, -10}, {6, 3}}, {{3, -4}, {5, 0}}, } }}
        //     },
        // },
        // {
        //     .Input = {{{3, 4}, {7, 0}}, {{-7, 4}, {4, 3}}, },
        //     .Expected = {
        //         {{RoundToPrecision(4), RoundToPrecision(3)}, {{{{3, 4}, {7, 0}}, {{-7, 4}, {4, 3}}, } }},
        //     },
        // },
        // {
        //     .Input = {{{-4, 0}, {-1, 9}}, {{-1, 9}, {6, -6}},},
        //     .Expected = {
        //         {{RoundToPrecision(-1), RoundToPrecision(9)}, {{ {{-4, 0}, {-1, 9}}, {{-1, 9}, {6, -6}}, } }},
        //     },
        // },
        // {
        //     .Input = {{{-9 , -7}, {-1 , 7}}, {{-10 , -4}, {1 , 2}}, {{-6 , -1}, {8 , -3}}},
        //     .Expected = {
        //         {{RoundToPrecision(-6.0566), RoundToPrecision(-1.84906)}, {{ {{-10 , -4}, {1 , 2}}, {{-9 , -7}, {-1 , 7}}, } }},
        //         {{RoundToPrecision(-5.60377), RoundToPrecision(-1.0566)}, {{ {{-9 , -7}, {-1 , 7}}, {{-6 , -1}, {8 , -3}}, } }},
        //         {{RoundToPrecision(-4.81132), RoundToPrecision(-1.16981)}, {{ {{-10 , -4}, {1 , 2}}, {{-6 , -1}, {8 , -3}}, } }},
        //     },
        // },
        // {
        //     .Input = {{{-8 , 5}, {3 , -2}}, {{-5 , -4}, {5 , 0}}, {{-7 , -6}, {-4 , -3}}},
        //     .Expected = {
        //         {{RoundToPrecision(-5), RoundToPrecision(-4)}, {{ {{-5 , -4}, {5 , 0}}, {{-7 , -6}, {-4 , -3}}, } }},
        //         {{RoundToPrecision(1.84211), RoundToPrecision(-1.26316)}, {{ {{-8 , 5}, {3 , -2}}, {{-5 , -4}, {5 , 0}}, } }}
        //     },
        // },
        // {
        //     .Input = {{{-10 , -3}, {5 , -10}}, {{-3 , -9}, {0 , 6}}, {{-10 , -3}, {0 , 5}}},
        //     .Expected = {
        //         {{RoundToPrecision(-10), RoundToPrecision(-3)}, {{ {{-10 , -3}, {5 , -10}}, {{-10 , -3}, {0 , 5}}, } }},
        //         {{RoundToPrecision(-2.5), RoundToPrecision(-6.5)}, {{ {{-10 , -3}, {5 , -10}}, {{-3 , -9}, {0 , 6}}, } }},
        //         {{RoundToPrecision(-0.2381), RoundToPrecision(4.80952)}, {{ {{-10 , -3}, {0 , 5}}, {{-3 , -9}, {0 , 6}}, } }}
        //     },
        // },
        // {
        //     .Input = {{{3, -10}, {4, 3}}, {{-5 , -8}, {8 , 0}}, {{-2 , -10}, {8 , -8}}},
        //     .Expected = {
        //         {{RoundToPrecision(3.55901), RoundToPrecision(-2.73292)}, {{ {{-5 , -8}, {8 , 0}}, {{3 , -10}, {4 , 3}}, } }},
        //         {{RoundToPrecision(3.07813), RoundToPrecision(-8.98438)}, {{ {{-2 , -10}, {8 , -8}}, {{3 , -10}, {4 , 3}}, } }},
        //     },
        // },
        {
            .Input = {{{-1, 0}, {1, 2}}, {{-10 , -2}, {-6 , -8}}, {{-7 , -9}, {3 , 8}}},
            .Expected = {
                {{RoundToPrecision(-6.21875), RoundToPrecision(-7.67188)}, {{ {{-10 , -2}, {-6 , -8}}, {{-7 , -9}, {3 , 8}} } }},
            },
        },
    };

    Normalize(tests);

    // const auto& test = tests.front();

    // auto s1 = test.Input[0];
    // auto s2 = test.Input[2];

    // auto pointX = RoundToPrecision(-10);
    // TTrackingSegment ts1(&s1, pointX);
    // TTrackingSegment ts2(&s2, pointX);

    // bool result = ts1 < ts2;
    // std::cout << result << std::endl;

    // std::cout << "ts1: " << *ts1.Parameters.GetAtX(pointX) << std::endl; 
    // std::cout << "ts2: " << *ts2.Parameters.GetAtX(pointX) << std::endl;

    for (const auto& test : tests) {
        if (!CheckTestCase(test, BruteForce(test.Input))) {
            return 1;
        }
    }

    std::cout << "OK" << std::endl;
    return 0;
}

int main() {
    return test();
}