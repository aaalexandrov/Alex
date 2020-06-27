#pragma once

#include "geom.h"

NAMESPACE_BEGIN(util)

template <typename VecType>
struct Polytope {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	using Plane = Plane<Vec>;
	using Line = Line<Vec>;
	using Interval = Box<Num>;
	using Box = Box<Vec>;
	using OBox = OrientedBox<Vec>;
	using Sphere = Sphere<Vec>;

	struct Edge {
		Line _line;
		Interval _interval = Interval::GetMaximum();
		std::array<int, 2> _sideIndices;

		template <typename Shape>
		bool Intersects(Shape const &shape) const
		{
			Interval shapeInt = _line.GetIntersection(shape);
			return shapeInt.Intersects(_interval);
		}
	};

	std::vector<Plane> _sides;
	std::vector<Edge> _edges;
	std::vector<Vec> _points;
	std::vector<Vec> _sideDirections;
	std::vector<Vec> _edgeDirections;

	void AddSide(Plane const &side)
	{
		std::vector<Edge> newEdges;
		for (int s = 0; s < _sides.size(); ++s) {
			Edge edge;
			edge._line = _side[s].GetIntersectionLine(sides).Normalized();
			ASSERT(edge._line.IsValid()); // otherwise we either have a repeating plane, or an empty polytope
			edge._sideIndices = { s, _sides.size() };
			newEdges.push_back(edge);
		}

		IntersectEdges(_edges, side);

		for (int s = 0; s < _sides.size(); ++s) {
			IntersectEdges(newEdges, _sides[s]);
		}

		_sides.insert(_sides.end(), newEdges.begin(), newEdges.end());
	}

	static void IntersectEdges(std::vector<Edge> &edges, Plane const &side) 
	{
		for (int e = edges.size() - 1; e >= 0; --e) {
			Interval intersect = edges[e]._line.GetIntersectionInterval(side);
			edges[e]._interval = edges[e]._interval.GetIntersection(intersect);
			if (edges[e]._interval.IsEmpty())
				RemoveElement(edges, e);
		}
	}

	void UpdateAfterAddingSides()
	{
		_points.clear();
		_sideDirections.clear();
		_edgeDirections.clear();

		for (auto &side : _sides) {
			AddUniqueVec(_sideDirections, glm::normalize(side._normal), true);
		}

		for (auto &edge : _edges) {
			Vec p0 = edge._line.GetPoint(edge._interval._min);
			Vec p1 = edge._line.GetPoint(edge._interval._max);
			AddUniqueVec(_points, p0, false);
			AddUniqueVec(_points, p1, false);
			AddUniqueVec(_edgeDirections, edge._line._direction, true);
		}
	}

	static bool AddUniqueVec(std::vector<Vec> &vecs, Vec const &v, bool oppositeIsEqual, Num eps = NumericTraits<Num>::Eps)
	{
		auto found = std::find_if(vecs.begin(), vecs.end(), [&](Vec const &e) { return IsEqual(e, v, eps) || oppositeIsEqual && IsEqual(-e, v, eps); });
		if (found != vecs.end())
			return false;
		vecs.push_back(v);
		return true;
	}

	constexpr int GetNumPoints() const { return static_cast<int>(_points.size()); }
	constexpr Vec GetPoint(int pointInd) const { return _points[pointInd]; }
	constexpr int GetNumSideDirections() const { return static_cast<int>(_sideDirections.size()); }
	constexpr Vec GetSideDirection(int dirInd) const { return _sideDirections[dirInd]; }
	constexpr int GetNumEdgeDirections() const { return static_cast<int>(_edgeDirections.size()); }
	constexpr Vec GetEdgeDirection(int dirInd) const { return _edgeDirections[dirInd]; }

	template <typename Shape>
	bool Intersects(Shape const &shape) const
	{
		return SatTest<Shape, Polytope>::Intersects(shape, *this);
	}
};

template <typename VecType>
struct PlaneEval<Polytope<VecType>> {
	using Shape = Polytope<VecType>;
	using Vec = typename Shape::Vec;
	using Num = typename VecTraits<Vec>::ElemType;
	using Plane = Plane<Vec>;
	using Interval = Box<Num>;

	static constexpr Interval Eval(Plane const &plane, Shape const &polytope)
	{
		return plane.Eval(polytope._points);
	}
};

NAMESPACE_END(util)