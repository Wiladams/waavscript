#pragma once

// Dependency-free polygon intersection for line-only, closed PathPrograms.
// Inputs must already have passed through PathFlattener and PathFillPreparer.
// The algorithm is quadratic in segment count for splitting and uses
// winding tests at split-edge sides. It is intended as an initial implementation;
// near-coincident / highly ill-conditioned geometry needs additional hardening.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <utility>
#include <vector>

#include "pathprogram.h"
#include "pathprogram_builder.h"

namespace waavs
{
    enum class PathFillRule : uint8_t 
    { 
        NonZero, 
        EvenOdd 
    };


    namespace path_boolean_detail
    {
        struct Point { double x = 0.0, y = 0.0; };
        struct Edge { Point a, b; };
        struct DirectedEdge { int a = 0, b = 0; bool used = false; };
        struct Key { int64_t x, y; bool operator<(const Key& v) const noexcept { return x != v.x ? x < v.x : y < v.y; } };

        inline Point add(Point a, Point b) noexcept { return {a.x + b.x, a.y + b.y}; }
        inline Point sub(Point a, Point b) noexcept { return {a.x - b.x, a.y - b.y}; }
        inline Point mul(Point a, double t) noexcept { return {a.x * t, a.y * t}; }
        inline double cross(Point a, Point b) noexcept { return a.x * b.y - a.y * b.x; }
        inline double dot(Point a, Point b) noexcept { return a.x * b.x + a.y * b.y; }
        inline double len(Point a) noexcept { return std::hypot(a.x, a.y); }
        inline Point lerp(Point a, Point b, double t) noexcept { return add(a, mul(sub(b, a), t)); }

        // Read the existing PathProgram directly. No intermediate polygon API.
        inline bool collect(const PathProgram& p, std::vector<Edge>& edges)
        {
            size_t ai = 0;
            Point start{}, prev{};
            bool open = false;
            for (uint8_t raw : p.ops) {
                PathOp op = static_cast<PathOp>(raw);
                uint8_t arity = pathOpArity(op);
                if (arity == kPathOpInvalidArity || ai + arity > p.args.size()) return false;
                if (op == OP_MOVETO) {
                    if (open) return false; // FillPreparer should have closed it.
                    prev = start = {p.args[ai], p.args[ai + 1]};
                    open = true;
                } else if (op == OP_LINETO) {
                    if (!open) return false;
                    Point next{p.args[ai], p.args[ai + 1]};
                    if (next.x != prev.x || next.y != prev.y) edges.push_back({prev, next});
                    prev = next;
                } else if (op == OP_CLOSE) {
                    if (!open) return false;
                    if (prev.x != start.x || prev.y != start.y) edges.push_back({prev, start});
                    open = false;
                } else if (op == OP_END) {
                    if (open) return false;
                } else return false; // Only prepared linear paths are accepted.
                ai += arity;
            }
            return !open && ai == p.args.size();
        }

        inline bool inside(Point p, const std::vector<Edge>& edges, PathFillRule rule) noexcept
        {
            int winding = 0;
            for (const Edge& e : edges) {
                if (e.a.y <= p.y) {
                    if (e.b.y > p.y && cross(sub(e.b, e.a), sub(p, e.a)) > 0) ++winding;
                } else {
                    if (e.b.y <= p.y && cross(sub(e.b, e.a), sub(p, e.a)) < 0) --winding;
                }
            }
            return rule == PathFillRule::NonZero ? winding != 0 : (winding % 2) != 0;
        }

        inline void splitPair(const Edge& a, const Edge& b, std::vector<double>& ta, std::vector<double>& tb, double eps)
        {
            Point r = sub(a.b, a.a), s = sub(b.b, b.a), q = sub(b.a, a.a);
            double den = cross(r, s), scale = len(r) * len(s);
            if (scale == 0) return;
            if (std::abs(den) > 1e-13 * scale) {
                double t = cross(q, s) / den, u = cross(q, r) / den;
                if (t >= -eps && t <= 1 + eps && u >= -eps && u <= 1 + eps) {
                    ta.push_back(std::clamp(t, 0.0, 1.0));
                    tb.push_back(std::clamp(u, 0.0, 1.0));
                }
            } else if (std::abs(cross(q, r)) <= eps * len(r)) {
                // Collinear overlap: insert all endpoints lying on the other segment.
                double rr = dot(r, r), ss = dot(s, s);
                double t0 = dot(sub(b.a, a.a), r) / rr, t1 = dot(sub(b.b, a.a), r) / rr;
                double u0 = dot(sub(a.a, b.a), s) / ss, u1 = dot(sub(a.b, b.a), s) / ss;
                if (t0 >= -eps && t0 <= 1 + eps) ta.push_back(std::clamp(t0, 0.0, 1.0));
                if (t1 >= -eps && t1 <= 1 + eps) ta.push_back(std::clamp(t1, 0.0, 1.0));
                if (u0 >= -eps && u0 <= 1 + eps) tb.push_back(std::clamp(u0, 0.0, 1.0));
                if (u1 >= -eps && u1 <= 1 + eps) tb.push_back(std::clamp(u1, 0.0, 1.0));
            }
        }
    }

    // Both inputs must be flattened and fill-prepared.
    // Output is a line-only, closed PathProgram with nonzero-oriented contours.
    inline bool intersectPreparedPathPrograms(const PathProgram& a, PathFillRule ruleA, const PathProgram& b, PathFillRule ruleB, PathProgram& result)
    {
        using namespace path_boolean_detail;
        std::vector<Edge> ea, eb, all;
        if (!collect(a, ea) || !collect(b, eb)) return false;
        PathProgramBuilder output;
        if (ea.empty() || eb.empty()) {
            if (!output.finalize()) return false;
            result = std::move(output.prog);
            return true;
        }
        all = ea;
        all.insert(all.end(), eb.begin(), eb.end());
        const size_t n = all.size();
        if (n > 20000) return false; // Prevent pathological quadratic workloads.

        double loX = all[0].a.x, hiX = loX, loY = all[0].a.y, hiY = loY;
        for (const Edge& e : all) for (Point p : {e.a, e.b}) {
            loX = std::min(loX, p.x); hiX = std::max(hiX, p.x);
            loY = std::min(loY, p.y); hiY = std::max(hiY, p.y);
        }
        const double span = std::max({hiX - loX, hiY - loY, 1.0});
        const double snap = span * 1e-9;
        const double side = span * 1e-7;
        if (!std::isfinite(span) || span > 1e12) return false;

        std::vector<std::vector<double>> splits(n);
        for (auto& t : splits) t = {0.0, 1.0};
        for (size_t i = 0; i < n; ++i)
            for (size_t j = i + 1; j < n; ++j)
                splitPair(all[i], all[j], splits[i], splits[j], snap / std::max(len(sub(all[i].b, all[i].a)), 1.0));

        std::map<Key, int> vertexMap;
        std::vector<Point> vertices;
        auto vertex = [&](Point p) -> int {
            Key k{static_cast<int64_t>(std::llround((p.x - loX) / snap)), static_cast<int64_t>(std::llround((p.y - loY) / snap))};
            auto it = vertexMap.find(k);
            if (it != vertexMap.end()) return it->second;
            int id = static_cast<int>(vertices.size());
            vertices.push_back(p);
            vertexMap.emplace(k, id);
            return id;
        };

        // Classify both sides of every atomic edge. Reverse edges so that
        // the interior of A intersect B is always on the left.
        std::map<std::pair<int, int>, int> counts;
        for (size_t i = 0; i < n; ++i) {
            auto& t = splits[i];
            std::sort(t.begin(), t.end());
            t.erase(std::unique(t.begin(), t.end(), [](double x, double y) { return std::abs(x - y) <= 1e-12; }), t.end());
            for (size_t j = 1; j < t.size(); ++j) {
                Point p = lerp(all[i].a, all[i].b, t[j-1]);
                Point q = lerp(all[i].a, all[i].b, t[j]);
                Point d = sub(q, p);
                double length = len(d);
                if (length <= snap) continue;
                Point mid = mul(add(p, q), 0.5);
                double offset = std::min(side, length * 0.1);
                Point normal{-d.y / length * offset, d.x / length * offset};
                Point left = add(mid, normal), right = sub(mid, normal);
                bool l = inside(left, ea, ruleA) && inside(left, eb, ruleB);
                bool r = inside(right, ea, ruleA) && inside(right, eb, ruleB);
                if (l == r) continue;
                int u = vertex(p), v = vertex(q);
                if (u == v) continue;
                if (!l) std::swap(u, v);
                // Deduplicate coincident boundary fragments, including overlaps.
                counts[{u, v}] = 1;
            }
        }

        std::vector<DirectedEdge> edges;
        for (auto& pair : counts) edges.push_back({pair.first.first, pair.first.second, false});
        std::vector<std::vector<int>> outgoing(vertices.size());
        for (int i = 0; i < static_cast<int>(edges.size()); ++i) outgoing[edges[i].a].push_back(i);

        // Trace left-filled loops by choosing the most clockwise
        // continuation relative to the reversed incoming edge.
        for (int start = 0; start < static_cast<int>(edges.size()); ++start) {
            if (edges[start].used) continue;
            std::vector<int> contour;
            int cur = start;
            for (size_t guard = 0; guard <= edges.size(); ++guard) {
                if (edges[cur].used) return false;
                edges[cur].used = true;
                contour.push_back(edges[cur].a);
                int at = edges[cur].b;
                if (at == edges[start].a) break;
                Point incoming = sub(vertices[edges[cur].a], vertices[at]);
                int next = -1;
                double bestAngle = -1.0;
                for (int cand : outgoing[at]) {
                    if (edges[cand].used) continue;
                    Point v = sub(vertices[edges[cand].b], vertices[at]);
                    double angle = std::atan2(cross(incoming, v), dot(incoming, v));
                    if (angle < 0) angle += 6.28318530717958647692;
                    // Largest CCW separation equals the smallest clockwise turn.
                    if (next < 0 || angle > bestAngle) { next = cand; bestAngle = angle; }
                }
                if (next < 0) return false;
                cur = next;
            }
            if (contour.size() < 3) continue;
            if (!output.moveTo(static_cast<float>(vertices[contour[0]].x), static_cast<float>(vertices[contour[0]].y))) return false;
            for (size_t j = 1; j < contour.size(); ++j)
                if (!output.lineTo(static_cast<float>(vertices[contour[j]].x), static_cast<float>(vertices[contour[j]].y))) return false;
            if (!output.close()) return false;
        }
        if (!output.finalize()) return false;
        result = std::move(output.prog);
        return true;
    }
}
