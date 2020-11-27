#pragma once

#include "mathutl.h"

NAMESPACE_BEGIN(util)

template <typename VecType> struct Box;
template <typename VecType> struct OrientedBox;
template <typename VecType> struct Sphere;
template <typename VecType> struct Plane;
template <typename VecType> struct Line;
template <typename PointArray> struct Polygon;

template <typename VecType>
struct RigidTransform {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	using RotationV = Rotation<Vec>;
	using Rot = typename RotationV::Rot;
	using BoxV = Box<Vec>;
	using OBoxV = OrientedBox<Vec>;
	using SphereV = Sphere<Vec>;
	using PlaneV = Plane<Vec>;
	using LineV = Line<Vec>;

	Vec _position;
	Rot _orientation;
	Num _scale;

	constexpr RigidTransform(Vec const &pos = Vec(0), Rot const &rot = RotationV::Identity(), Num scale = Num(1)) : _position(pos), _orientation(RotationV::Normalize(rot)), _scale(scale) {}

	constexpr bool IsIdentity(Num eps = NumericTraits<Num>::Eps) const 
	{
		return IsZero(_position, eps) && IsEqual(_orientation, Rot::Identity(), eps) && IsEqual(_scale, Num(1), eps);
	}

	constexpr RigidTransform Inverse() const 
	{
		Rot invRot = RotationV::Inverse(_orientation);
		Num invScale = 1 / _scale;
		return RigidTransform(-RotationV::Rotate(invRot, _position * invScale), invRot, invScale);
	}

	constexpr Vec TransformVector(Vec const &v) const { return RotationV::Rotate(_orientation, v * _scale); }
	constexpr Vec TransformPoint(Vec const &v) const { return TransformVector(v) + _position; }

	constexpr Vec InverseTransformVector(Vec const &v) const { return RotationV::Rotate(Rot::Inverse(_orientation), v / _scale); }
	constexpr Vec InverseTransformPoint(Vec const &v) const
	{
		Rot invRot = RotationV::Inverse(_orientation);
		Num invScale = 1 / _scale;
		return RotationV::Rotate(invRot, v * invScale) - Rot::Rotate(invRot, _position * invScale);
	}

	constexpr OBoxV TransformToOrientedBox(BoxV const &box) const { return OBoxV(TransformPoint(box.GetCenter()), box.GetSize() * _scale / 2, _orientation); }
	constexpr BoxV Transform(BoxV const &box) const { return TransformToOrientedBox(box).GetBox(); }
	constexpr OBoxV Transform(OBoxV const &ob) const { return OBoxV(TransformPoint(ob._center), ob._halfSize * _scale, RotationV::Compose(_orientation, ob._orientation)); }
	constexpr SphereV Transform(SphereV const &sphere) const { return SphereV(TransformPoint(sphere._center), sphere._radius * _scale); }
	constexpr PlaneV Transform(PlaneV const &plane) const { return PlaneV(TransformVector(plane._normal), TransformPoint(plane.GetAnyPoint())); }
	constexpr LineV Transform(LineV const &line) const { return LineV(TransformPoint(line._origin), TransformVector(line._direction)); }

	template <typename PointArray>
	constexpr void TransformPoints(PointArray const &src, PointArray &dst) const
	{
		ASSERT(src.size() == dst.size());
		for (int i = 0; i < src.size(); ++i) {
			dst[i] = TransformPoint(src[i]);
		}
	}

	template <typename PointArray>
	constexpr void TransformVectors(PointArray const &src, PointArray &dst) const
	{
		ASSERT(src.size() == dst.size());
		for (int i = 0; i < src.size(); ++i) {
			dst[i] = TransformVector(src[i]);
		}
	}

	template <typename PointArray>
	constexpr Polygon<PointArray> Transform(Polygon<PointArray> const &poly) const
	{
		Polygon<PointArray> transformed = poly;
		TransformPoints(poly._points, transformed._points);
		return transformed;
	}
};

template <typename ShapeType>
struct PlaneEval {
	using Shape = ShapeType;
	using Vec = typename Shape::Vec;
	using Num = typename VecTraits<Vec>::ElemType;
	using PlaneV = Plane<Vec>;
	using Interval = Box<Num>;

	static constexpr Interval Eval(PlaneV const &plane, Shape const &shape)
	{
		return plane.Eval(shape);
	}
};

template <typename ShapeT0, typename ShapeT1, typename Enable = std::enable_if_t<std::is_same_v<typename ShapeT0::Vec, typename ShapeT1::Vec>>>
struct SatTest {
	using Shape0 = ShapeT0;
	using Shape1 = ShapeT1;
	using Vec = typename Shape0::Vec;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	using Interval = Box<Num>;
	using PlaneV = Plane<Vec>;
	using LineV = Line<Vec>;

	static constexpr bool CheckShapeSides(Shape0 const &shape0, Shape1 const &shape1)
	{
		Vec p = shape0.GetPoint(0);
		int dirs = shape0.GetNumSideDirections();
		for (int i = 0; i < dirs; ++i) {
			PlaneV axis(shape0.GetSideDirection(i), p);
			Interval int0 = PlaneEval<Shape0>::Eval(axis, shape0);
			Interval int1 = PlaneEval<Shape1>::Eval(axis, shape1);
			if (!int0.Intersects(int1))
				return false;
		}
		return true;
	}

	static constexpr bool CheckEdgeDirectionCombinations(Shape0 const &shape0, Shape1 const &shape1)
	{
		Vec p = shape0.GetPoint(0);
		int edgeDirs0 = shape0.GetNumEdgeDirections();
		int edgeDirs1 = shape1.GetNumEdgeDirections();
		for (int e0 = 0; e0 < edgeDirs0; ++e0) {
			Vec edgeDir0 = shape0.GetEdgeDirection(e0);
			for (int e1 = 0; e1 < edgeDirs1; ++e1) {
				Vec edgeDir1 = shape1.GetEdgeDirection(e1);
				Vec dir = glm::cross(edgeDir0, edgeDir1);
				if (IsZero(dir))
					continue;
				PlaneV axis(dir, p);
				Interval int0 = PlaneEval<Shape0>::Eval(axis, shape0);
				Interval int1 = PlaneEval<Shape1>::Eval(axis, shape1);
				if (!int0.Intersects(int1))
					return false;
			}
		}
		return true;
	}

	static constexpr bool CheckPointEdgeCombinations(Shape0 const &shape0, Shape1 const &shape1)
	{
		Vec p = shape0.GetPoint(0);
		int numPoints0 = shape0.GetNumPoints();
		int numEdges1 = shape1.GetNumEdges();
		for (int p0 = 0; p0 < numPoints0; ++p0) {
			Vec point0 = shape0.GetPoint(p0);
			for (int e1 = 0; e1 < numEdges1; ++e1) {
				LineV edge1 = shape1.GetEdge(e1);
				Vec point1 = edge1.GetProjection(point0);
				Vec dir = point1 - point0;
				if (IsZero(dir))
					continue;
				PlaneV axis(dir, p);
				Interval int0 = PlaneEval<Shape0>::Eval(axis, shape0);
				Interval int1 = PlaneEval<Shape1>::Eval(axis, shape1);
				if (!int0.Intersects(int1))
					return false;
			}
		}
		return true;
	}

	static constexpr bool CheckPointCombinations(Shape0 const &shape0, Shape1 const &shape1)
	{
		Vec p = shape0.GetPoint(0);
		int numPoints0 = shape0.GetNumPoints();
		int numPoints1 = shape1.GetNumPoints();
		for (int p0 = 0; p0 < numPoints0; ++p0) {
			Vec point0 = shape0.GetPoint(p0);
			for (int p1 = 0; p1 < numPoints1; ++p1) {
				Vec point1 = shape1.GetPoint(p1);
				Vec dir = p1 - p0;
				if (IsZero(dir))
					continue;
				PlaneV axis(dir, p);
				Interval int0 = PlaneEval<Shape0>::Eval(axis, shape0);
				Interval int1 = PlaneEval<Shape1>::Eval(axis, shape1);
				if (!int0.Intersects(int1))
					return false;
			}
		}
		return true;
	}

	static constexpr bool Intersects(Shape0 const &shape0, Shape1 const &shape1)
	{
		if (!CheckShapeSides(shape0, shape1))
			return false;

		if (!CheckShapeSides(shape1, shape0))
			return false;

		if (!CheckEdgeDirectionCombinations(shape0, shape1))
			return false;

		if (Shape0::IsRound && !CheckPointEdgeCombinations(shape0, shape1))
			return false;

		if (Shape1::IsRound && !CheckPointEdgeCombinations(shape1, shape0))
			return false;

		if ((Shape0::IsRound || Shape1::IsRound) && !CheckPointCombinations(shape0, shape1))
			return false;

		return true;
	}
};


template <int Dimensions> struct BoxTraits;

template <typename VecType>
struct Box {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	static constexpr bool IsRound = false;
	using BVec = typename VecTraits<Vec>::template WithElemType<bool>;
	using LineV = Line<Vec>;

	Vec _min = Vec(NumericTraits<Num>::Max);
	Vec _max = Vec(NumericTraits<Num>::Min);

	constexpr Box() = default;
	constexpr Box(Vec const &min, Vec const &max) : _min(min), _max(max) {}

	static constexpr Box GetMaximum() { return Box(Vec(NumericTraits<Num>::Min), Vec(NumericTraits<Num>::Max)); }
	static constexpr Box GetUnit() { return Box(Vec(0), Vec(1)); }
	static constexpr Box GetUnitSymmetric() { return Box(Vec(-1), Vec(1)); }
	static constexpr Box FromPoint(Vec const &v) { return Box(v, v); }
	static constexpr Box FromMinSize(Vec const &min, Vec const &size) { return Box(min, min + size - Num(std::numeric_limits<Num>::is_integer)); }
	static constexpr Box FromCenterHalfSize(Vec const &center, Vec const &halfSize) { return Box(center - halfSize, center + halfSize - Num(std::numeric_limits<Num>::is_integer)); }

	constexpr Vec GetSize() const {	return _max - _min + Num(std::numeric_limits<Num>::is_integer); }
	Box &SetSize(Vec const &size) { _max = _min + size - Num(std::numeric_limits<Num>::is_integer); return *this; }

	constexpr Vec GetCenter() const { return (_min + _max) / 2; }

	constexpr bool IsEmpty() const { return !IsLessEqual(_min, _max); }
	constexpr bool IsFinite() const { return IsFinite(GetSize()); }

	constexpr bool GetClosestPoint(Vec const &v) const { return glm::clamp(v, _min, _max); }

	constexpr bool Contains(Vec const &v) const { return glm::all(glm::lessThanEqual(_min, v) && glm::lessThanEqual(v, _max)); }
	constexpr bool Contains(Box const &other) const { return glm::all(glm::lessThanEqual(_min, other._min) && glm::lessThanEqual(other._max, _max)) && !IsEmpty() && !other.IsEmpty(); }

	constexpr Box GetIntersection(Box const &other) const { return Box(glm::max(_min, other._min), glm::min(_max, other._max)); }
	constexpr bool Intersects(Box const &other) const { return !GetIntersection().IsEmpty(); }

	constexpr Box GetUnion(Vec const &v) const { return Box(glm::min(_min, v), glm::max(_max, v)); }
	constexpr Box GetUnion(Box const &other) const 
	{  
		if (IsEmpty())
			return other;
		if (other.IsEmpty())
			return *this;
		return Box(glm::min(_min, other._min), glm::max(_max, other._max));
	}

	constexpr bool operator ==(Box const &other) const { return _min == other._min && _max == other._max; }
	constexpr bool operator !=(Box const &other) const { return !(*this == other); }

	constexpr int GetNumPoints() const { return 1 << Dim; }
	constexpr Vec GetPoint(int pointInd) const
	{
		ASSERT(0 <= pointInd && pointInd < GetNumPoints());
		return glm::mix(_min, _max, VecFromMask<BVec>(pointInd));
	}

	constexpr int GetNumEdges() const { return BoxTraits<Dim>::NumEdges; }
	constexpr LineV GetEdge(int edgeInd) const
	{
		glm::ivec2 pointInds = BoxTraits<Dim>::GetEdgePointIndices(edgeInd);
		return LineV(GetPoint(pointInds[0]), GetPoint(pointInds[1]));
	}

	constexpr int GetNumEdgeDirections() const { return Dim; }
	constexpr int GetNumEdgeDirections(int dirInd) const { return VecCardinal<Vec>(dirInd); }

	constexpr int GetNumSideDirections() const { return Dim; }
	constexpr int GetSideDirection(int dirInd) const { return VecCardinal<Vec>(dirInd); }
};

template <typename VecType>
struct OrientedBox {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	static constexpr bool IsRound = false;
	using RotationV = Rotation<Vec>;
	using Rot = typename RotationV::Rot;
	using BoxV = Box<Vec>;
	using LineV = Line<Vec>;
	using RigidTransformV = RigidTransform<Vec>;

	Rot _orientation;
	Vec _center, _halfSize;

	constexpr OrientedBox() = default;
	constexpr OrientedBox(Vec const &center, Vec const &halfSize, Rot const &orientation = RotationV::Identity()) : _orientation(orientation), _center(center), _halfSize(halfSize) { ASSERT(IsValid()); }
	constexpr OrientedBox(BoxV const &box, Rot const &orientation = RotationV::Identity()) : _orientation(orientation), _center(box.GetCenter()), _halfSize(box.GetSize() / 2) { ASSERT(IsValid()); }

	constexpr bool IsValid() const { return IsFinite(_center) && IsFinite(_orientation) && IsFinite(_halfSize) && !IsEmpty(); }

	constexpr bool IsEmpty() const { return !glm::all(glm::lessThanEqual(Vec(0), _halfSize)); }

	constexpr Vec GetBoxPointOffset(int pointInd) const
	{
		ASSERT(0 <= pointInd && pointInd < GetNumPoints());
		Vec mul;
		for (int d = 0; d < Dim; ++d) {
			mul[d] = (Num)(pointInd & (1 << d) ? -1 : 1);
		}
		return mul * _halfSize;
	}

	constexpr Vec FromBoxPointOffset(Vec const &v) const { return RotationV::Rotate(_orientation, v) + _center; }

	constexpr Num GetNumPoints() const { return 1 << Dim; }
	constexpr Vec GetPoint(int pointInd) const { FromBoxPointOffset(GetBoxPointOffset(pointInd)); }

	constexpr int GetNumEdges() const { return BoxTraits<Dim>::NumEdges; }
	constexpr LineV GetEdge(int edgeInd) const
	{
		glm::ivec2 pointInds = BoxTraits<Dim>::GetEdgePointIndices(edgeInd);
		return LineV(GetPoint(pointInds[0]), GetPoint(pointInds[1]));
	}

	constexpr int GetNumEdgeDirections() const { return Dim; }
	constexpr int GetNumEdgeDirections(int dirInd) const { return RotationV::Rotate(_orientation, VecCardinal<Vec>(dirInd)); }

	constexpr int GetNumSideDirections() const { return Dim; }
	constexpr int GetSideDirection(int dirInd) const { return RotationV::Rotate(_orientation, VecCardinal<Vec>(dirInd)); }

	constexpr BoxV GetUntransformedBox() const
	{
		return BoxV::FromCenterHalfSize(_center, _halfSize);
	}

	constexpr RigidTransformV GetTransformToBox() const 
	{ 
		ASSERT(IsValid());
		return RigidTransformV(_center, _orientation).Inverse();
	}
	constexpr Vec ToBoxPointOffset(Vec const &v) const { GetTransformToBox().TransformPoint(v); }

	constexpr BoxV GetBox() const
	{
		Vec size(0);
		// We only need to evaluate half the points because of the box symmetry
		for (int i = 0; i < (1 << (Dim - 1)); ++i) {
			Vec boxPointOffs = GetBoxPointOffset(i);
			Vec rotPointOffs = RotationV::Rotate(_orientation, boxPointOffs);
			size = glm::max(size, glm::abs(rotPointOffs));
		}

		return BoxV::FromCenterHalfSize(_center, size);
	}

	constexpr bool Contains(Vec const &v) const
	{
		ASSERT(!IsEmpty());
		Vec boxOffs = ToBoxPointOffset(v);
		return glm::all(glm::lessThanEqual(-_halfSize, boxOffs) && glm::lessThanEqual(boxOffs, _halfSize));
	}

	constexpr bool Intersects(BoxV const &box) const
	{
		return SatTest<BoxV, OrientedBox>::Intersect(box, *this);
	}

	constexpr bool Intersects(OrientedBox const &other) const
	{
		return SatTest<OrientedBox, OrientedBox>::Intersect(*this, other);
	}
};

template <typename VecType>
struct Sphere {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	static constexpr bool IsRound = true;
	using BoxV = Box<Vec>;
	using OBoxV = OrientedBox<Vec>;
	using RigidTransformV = RigidTransform<Vec>;
	using LineV = Line<Vec>;

	Vec _center = Vec(0);
	Num _radius = Num(-1);

	constexpr Sphere() = default;
	constexpr Sphere(Vec const &center, Num radius) : _center(center), _radius(radius) {}

	static constexpr Sphere FromDiameter(Vec const &v0, Vec const &v1) { return Sphere((v0 + v1) / 2, glm::distance(v0, v1) / 2); }

	constexpr bool IsEmpty() const { return _radius < 0; }
	constexpr bool IsFinite() const { return IsFinite(_center) && IsFinite(_radius); }

	constexpr int GetNumPoints() const { return 1; }
	constexpr Vec GetPoint(int pointInd) const { return _center; }
	constexpr int GetNumSideDirections() const { return 0; }
	constexpr Vec GetSideDirection(int dirInd) const { return Vec(0); }
	constexpr int GetNumEdges() const { return 0; }
	constexpr LineV GetEdge(int edgeInd) const { return LineV(Vec(0), Vec(0)); }
	constexpr int GetNumEdgeDirections() const { return 0; }
	constexpr Vec GetEdgeDirection(int dirInd) const { return Vec(0); }

	constexpr BoxV GetBox() const { return BoxV::FromCenterHalfSize(_center, Vec(_radius)); }

	constexpr bool Contains(Vec const &v) const { return glm::distance2(v, _center) <= Sqr(_radius) && !IsEmpty(); }
	constexpr bool Contains(Sphere const &other) const { return _radius >= other._radius && glm::distance2(_center, other._center) <= Sqr(_radius - other._radius) && !other.IsEmpty(); }

	constexpr bool Intersects(BoxV const &box) const { return Contains(box.GetClosestPoint(_center)); }
	constexpr bool Intersects(OBoxV const &ob) const 
	{
		RigidTransformV toBox = ob.GetTransformToBox();
		Sphere boxSphere = toBox.Transform(*this);
		return boxSphere.Intersects(ob.GetUntransformedBox());
	}
	constexpr bool Intersects(Sphere const &other) const { return glm::distance2(_center, other._center) <= Sqr(_radius + other._radius) && !IsEmpty() && !other.IsEmpty(); }

	constexpr Sphere GetUnion(Vec const &v) const
	{
		if (IsEmpty())
			return Sphere(v, Num(0));
		Vec centerV = v - _center;
		Num dist2 = glm::length2(centerV);
		if (dist2 <= Sqr(_radius))
			return *this;
		Num dist = glm::sqrt(dist2);
		Vec opposite = _center - centerV * _radius / dist;
		Sphere result((opposite + v) / 2, (dist + _radius) / 2);
		ASSERT(IsEqual(glm::distance(result._center, v), result._radius));
		ASSERT(IsEqual(glm::distance(result._center, opposite), result._radius));
		return result;
	}

	constexpr Sphere GetUnion(Sphere const &other) const
	{
		if (IsEmpty())
			return other;
		if (other.IsEmpty())
			return *this;
		Vec centerOther = other._center - _center;
		Num dist = glm::length(centerOther);
		if (dist + _radius <= other._radius)
			return other;
		if (dist + other._radius <= _radius)
			return *this;
		centerOther /= dist;
		Vec opposite = _center - centerOther * _radius;
		Vec otherOpposite = other._center + centerOther * other._radius;
		Sphere result((opposite + otherOpposite) / 2, dist + _radius + other._radius);
		ASSERT(IsEqual(glm::distance(result._center, opposite), result._radius));
		ASSERT(IsEqual(glm::distance(result._center, otherOpposite), result._radius));
		return result;
	}

	constexpr bool operator ==(Sphere const &other) const { return _center == other._center && _radius == other._radius; }
	constexpr bool operator !=(Sphere const &other) const { return !(*this == other); }
};

template <typename VecType>
struct Plane {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	static constexpr bool IsRound = false;
	using RotationV = Rotation<Vec>;
	using BVec = glm::vec<Dim, bool>;
	using Interval = Box<Num>;
	using BoxV = Box<Vec>;
	using OBoxV = OrientedBox<Vec>;
	using SphereV = Sphere<Vec>;
	using LineV = Line<Vec>;

	Vec _normal;
	Num _d;

	constexpr Plane(Vec const &normal = Vec(0), Num d = 0) : _normal(normal), _d(d) { ASSERT(IsValid()); }
	constexpr Plane(Vec const &normal, Vec const &point) : _normal(normal), _d(-glm::dot(normal, point)) { ASSERT(IsValid()); }

	static constexpr Plane FromPoints(Vec const &p0, Vec const &p1, Vec const &p2) 
	{
		std::array<Vec, 3> points = { p0, p1, p2 };
		return GetPlane(points);
	}

	constexpr bool IsValid() const { return !IsZero(_normal) && IsFinite(_d); }

	constexpr Plane Normalized() const
	{
		ASSERT(IsValid());
		Num normLen = glm::length(_normal);
		return Plane(_normal / normLen, _d / normLen);
	}

	constexpr Num Eval(Vec const &p) const { return glm::dot(p, _normal) + _d; }
	constexpr Num Distance(Vec const &p) const { return Eval(p) / glm::length(_normal); }

	constexpr Num GetAnyPoint() const
	{
		Vec p = -_d / glm::dot(_normal, _normal) * _normal;
		ASSERT(IsZero(Eval(p)));
		return p;
	}

	template <typename PointsArray>
	static constexpr Vec GetNormal(PointsArray const &points) { return glm::cross(points[0] - points[1], points[2] - points[0]); }

	template <typename PointsArray>
	static constexpr Plane GetPlane(PointsArray const &points) { return Plane(GetNormal(points), points[0]); }

	constexpr Vec GetProjection(Vec const &v) const
	{
		Num t = -(_d + glm::dot(v, _normal)) / glm::dot(_normal, _normal);
		Vec proj = v + t * _normal;
		ASSERT(IsZero(Eval(proj)));
		return proj;
	}

	constexpr LineV GetIntersectionLine(Plane const &other) const
	{
		Vec dir = glm::cross(_normal, other._normal);
		if (IsZero(dir))
			return LineV(Vec(0), Vec(0));
		Vec origin;
		Vec b(-_d, -other._d, 0);
		if (IsZero(b)) {
			// both planes contain 0, that's the common point
			origin = Vec(0);
		} else {
			// find the intersection point of the two planes and a third one perpendicular to the line direction and passing through 0
			glm::mat<3, 3, Num> m(_normal, other._normal, dir);
			origin = b * glm::inverse(m);
		}
		ASSERT(IsZero(Eval(origin)));
		ASSERT(IsZero(other.Eval(origin)));
		return LineV(origin, dir);
	}

	constexpr Plane GetPerpendicularPlaneThroughLine(LineV const &line) const
	{
		Vec norm = glm::cross(line._direction, _normal);
		return Plane(norm, line._origin);
	}

	constexpr Interval Eval(BoxV const &box) const
	{
		ASSERT(box.IsFinite() && !box.IsEmpty());
		glm::bvec3 negative = glm::lessThan(_normal, Vec(0));
		Vec min = glm::mix(box._min, box._max, negative);
		Vec max = glm::mix(box._max, box._min, negative);
		Interval result(Eval(min), Eval(max));
		ASSERT(!result.IsEmpty());
		return result;
	}

	constexpr Interval Eval(OBoxV const &ob) const
	{
		ASSERT(ob.IsValid());
		Num centerEval = Eval(ob._center);
		Num extremaEval = 0;
		for (int i = 0; i < (1 << (Dim - 1)); ++i) {
			Vec extentDir = ob.GetBoxPoint(i);
			extentDir = RotationV::Rotate(ob._orientation, extentDir);
			extremaEval = glm::max(extremaEval, glm::abs(glm::dot(extentDir, _normal)));
		}
		Interval result(centerEval - extremaEval, centerEval + extremaEval);
		return result;
	}

	constexpr Interval Eval(SphereV const &sphere) const
	{
		ASSERT(!sphere.IsEmpty());
		Num centerEval = Eval(sphere._center);
		Num radiusEval = sphere._radius * glm::length(_normal);
		Interval result(centerEval - radiusEval, centerEval + radiusEval);
		return result;
	}

	template <typename PointsArray>
	constexpr Interval Eval(PointsArray const &points) const
	{
		Interval pointsEval;
		for (auto &p : points) {
			pointsEval = pointsEval.GetUnion(Eval(p));
		}
		return pointsEval;
	}
};

template <typename VecType>
struct Line {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	static constexpr bool IsRound = false;
	using Interval = Box<Num>;
	using PlaneV = Plane<Vec>;
	using BoxV = Box<Vec>;
	using OBoxV = OrientedBox<Vec>;
	using SphereV = Sphere<Vec>;
	using Transform = RigidTransform<Vec>;

	Vec _origin, _direction;

	constexpr Line() = default;
	constexpr Line(Vec const &origin, Vec const &dir) : _origin(origin), _direction(dir) {}

	static constexpr Line FromPoints(Vec const &v0, Vec const &v1) { return Line(v0, v1 - v0); }

	constexpr bool IsFinite() const { return IsFinite(_origin) && IsFinite(_direction); }
	constexpr bool IsValid() const { return IsFinite() && !IsZero(_direction); }

	constexpr Vec GetPoint(Num t) const { return _origin + t * _direction; }

	constexpr Line Normalized() const { return Line(_origin, VecNormalize(_direction)); }

	constexpr Num GetProjectionValue(Vec const &v) const
	{
		ASSERT(IsValid());
		return glm::dot(v - _origin, _direction) / glm::dot(_direction, _direction);
	}

	constexpr Vec GetProjection(Vec const &v) const
	{
		return GetPoint(GetProjectionValue(v));
	}

	constexpr Num GetIntersectionValue(PlaneV const &plane) const
	{
		Num dotDirNorm = glm::dot(_direction, plane._normal);
		if (IsZero(dotDirNorm)) {
			if (IsZero(plane.Eval(_origin))) {
				// the whole line lies in the plane
				return std::numeric_limits<Num>::infinity();
			} else {
				// no intersection
				return std::numeric_limits<Num>::quiet_nan();
			}
		}

		Num t = (-plane._d - glm::dot(_origin, plane._normal)) / dotDirNorm;
		ASSERT(IsZero(plane.Eval(GetPoint(t))));
		return t;
	}

	// the negative side of the plane is considered "inside"
	constexpr Interval GetIntersection(PlaneV const &plane) const
	{
		Num t = GetIntersectionValue(plane);
		if (isnan(t))
			return Interval();
		if (isinf(t))
			return Interval::GetMaximum();
		if (glm::dot(_direction, plane._normal) < 0) {
			return Interval(t, std::numeric_limits<Num>::infinity());
		} else {
			return Interval(-std::numeric_limits<Num>::infinity(), t);
		}
	}

	constexpr Interval GetIntersection(BoxV const &box) const
	{
		Interval intersection = Interval::GetMaximum();
		Vec mins = (box._min - _origin) / _direction;
		Vec maxs = (box._max - _origin) / _direction;
		for (int d = 0; d < Dim; ++d) {
			if (IsZero(_direction[d])) {
				if (box._min[d] <= _origin[d] && _origin[d] <= box._max[d])
					continue;
				Interval dimInt = _direction[d] >= 0 ? Interval(mins[d], maxs[d]) : Interval(maxs[d], mins[d]);
				intersection = intersection.GetIntersection(dimInt);
				if (intersection.IsEmpty())
					break;
			}
		}
		return intersection;
	}

	constexpr Interval GetIntersection(OBoxV const &ob) const 
	{
		Line boxLine = ob.GetTransformToBox().Transform(*this);
		return boxLine.GetIntersection(ob.GetUntransformedBox());
	}

	constexpr Interval GetIntersection(SphereV const &sphere) const
	{
		Vec oc = _origin - sphere._center;
		Num a = glm::dot(_direction, _direction);
		Num b = 2 * glm::dot(oc, _direction);
		Num c = dot(oc, oc) - Sqr(sphere._radius);
		Interval intersect;
		SolveQuadratic(a, b, c, intersect._min, intersect._max);
		return intersect;
	}
};

template <typename PointsArray>
struct Polygon {
	using Vec = typename PointsArray::value_type;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	static constexpr bool IsRound = false;
	using VecArray = PointsArray;
	using Interval = Box<Vec>;
	using LineV = Line<Vec>;
	using PlaneV = Plane<Vec>;
	using BoxV = Box<Vec>;
	using OBoxV = OrientedBox<Vec>;
	using SphereV = Sphere<Vec>;

	VecArray _points;

	constexpr PlaneV GetPlane() const 
	{
		if (_points.size() < 3)
			return PlaneV();
		return PlaneV::GetPlane(_points);
	}

	constexpr bool IsConvex() const
	{
		PlaneV plane = GetPlane();
		int prev = _points.size() - 1;
		for (int i = 0; i < _points.size(); prev = i++) {
			LineV edge = LineV::FromPoints(_points[prev], _points[i]);
			PlaneV edgePlane = plane.GetPerpendicularPlaneThroughLine(edge);
			for (int j = 0; j < _points.size(); ++j) {
				if (i == j)
					continue;
				if (edgePlane.Eval(_points[j]) > NumericTraits<Num>::Eps)
					return false;
			}
		}
		return true;
	}

	constexpr bool IsFinite() const
	{
		for (auto &p : _points) {
			if (!IsFinite(p))
				return false;
		}
		return true;
	}

	constexpr Vec GetClosestPoint(Vec v) const
	{
		ASSERT(IsConvex());
		ASSERT(IsFinite(v));

		PlaneV plane = GetPlane();
		ASSERT(plane.IsValid());
		if (!IsZero(plane.Eval(v))) {
			v = plane.GetProjection(v);
		}

		LineV edge;
		auto projValueIfOutside = [&](int edgeFirstInd) {
			edge = LineV::FromPoints(_points[edgeFirstInd % _points.size()], _points[(edgeFirstInd + 1) % _points.size()]);
			PlaneV edgePlane = plane.GetPerpendicularPlaneThroughLine(edge);
			if (edgePlane.Eval(v) <= 0)
				return std::numeric_limits<Num>::quiet_nan();
			return edge.GetProjectionValue(v);
		};

		Num prevProj = projValueIfOutside(_points.size() - 1);
		for (int i = 0; i <= _points.size(); ++i) {
			Num proj = projValueIfOutside(i);
			if (Interval::GetUnit().Contains(proj))
				return edge.GetPoint(proj);
			if (proj < 0 && prevProj > 1)
				return _points[i];
		}

		return v;
	}

	constexpr Num GetDistance(Vec const &v) const
	{
		Vec closest = GetClosestPoint(v);
		return glm::distance(v, closest);
	}

	constexpr int GetNumPoints() const { return (int)_points.size(); }
	constexpr Vec GetPoint(int ptInd) const { return _points[ptInd]; }

	constexpr int GetNumEdgeDirections() const { return (int)_points.size(); }
	constexpr Vec GetEdgeDirection(int dirInd) const 
	{
		int prev = dirInd > 0 ? dirInd - 1 : (int)_points.size() - 1;
		return _points[dirInd] - _points[prev]; 
	}

	constexpr int GetNumEdges() const { return (int)_points.size(); }
	constexpr LineV GetEdge(int edgeInd) const 
	{
		int prev = edgeInd > 0 ? edgeInd - 1 : (int)_points.size() - 1;
		return LineV::FromPoints(_points[prev], _points[edgeInd]);
	}

	constexpr int GetNumSideDirections() const { return 1; }
	constexpr int GetSideDirection(int dirInd) const { return GetPlane()._normal; }


	constexpr bool Intersects(SphereV const &sphere) const
	{
		Vec closest = GetClosestPoint(sphere._center);
		return sphere.Contains(closest);
	}

	constexpr bool Intersects(BoxV const &box) const
	{
		ASSERT(IsConvex());
		ASSERT(IsFinite());
		ASSERT(GetPlane().IsValid());
		return SatTest<Polygon, BoxV>::Intersects(*this, box);
	}

	constexpr bool Intersects(OBoxV const &ob) const
	{
		ASSERT(IsConvex());
		ASSERT(IsFinite());
		ASSERT(GetPlane().IsValid());
		return SatTest<Polygon, OBoxV>::Intersects(*this, ob);
	}

};

template <typename PointsArray>
struct PlaneEval<Polygon<PointsArray>> {
	using Shape = Polygon<PointsArray>;
	using Vec = typename Shape::Vec;
	using Num = typename VecTraits<Vec>::ElemType;
	using PlaneV = Plane<Vec>;
	using Interval = Box<Num>;

	static constexpr Interval Eval(PlaneV const &plane, Shape const &polygon)
	{
		return plane.Eval(polygon._points);
	}
};

template <int Dimensions>
struct BoxTraits {
	static constexpr int Dim = Dimensions;

	static constexpr int NumPoints = 1 << Dim;
	static constexpr int NumEdges = (1 << (Dim - 1)) * Dim;

	static constexpr glm::ivec2 GetEdgePointIndices(int edgeInd)
	{
		ASSERT(0 <= edgeInd && edgeInd < NumEdges);
		const int sidePoints = 1 << (Dim - 1);
		int dim = edgeInd / sidePoints;
		int pointInd = edgeInd % sidePoints;
		pointInd = (int)RotateBitsRight(pointInd, dim, Dim);
		const int dimMask = 1 << (Dim - 1 - dim);
		ASSERT(!(pointInd & dimMask));
		return glm::ivec2(pointInd, pointInd ^ dimMask);
	}
};

using IntervalI = Box<int32_t>;
using IntervalF = Box<float>;
using RectF = Box<glm::vec2>;
using RectI = Box<glm::ivec2>;
using BoxF = Box<glm::vec3>;
using BoxI = Box<glm::ivec3>;
using CircleF = Sphere<glm::vec2>;
using SphereF = Sphere<glm::vec3>;
using OBoxF = OrientedBox<glm::vec3>;
using PlaneF = Plane<glm::vec3>;
using Line3F = Line<glm::vec3>;
using Triangle3F = Polygon<std::array<glm::vec3, 3>>;
using Polygon3F = Polygon<std::vector<glm::vec3>>;
using Transform3F = RigidTransform<glm::vec3>;

NAMESPACE_END(util)

NAMESPACE_BEGIN(std)

template <typename VecType>
struct hash<util::Box<VecType>> {
	using BoxType = util::Box<VecType>;
	size_t operator()(BoxType const &b) const
	{
		hash<typename BoxType::Vec> hasher;
		size_t hash = hasher(b._min);
		hash = 31 * hash + hasher(b._max);
		return hash;
	}
};

template <typename VecType>
struct hash<util::Sphere<VecType>> {
	using SphereType = util::Sphere<VecType>;
	size_t operator()(SphereType const &b) const
	{
		hash<typename SphereType::Vec> hasherVec;
		hash<typename SphereType::Num> hasherNum;
		size_t hash = hasherVec(b._center);
		hash = 31 * hash + hasherNum(b._radius);
		return hash;
	}
};


NAMESPACE_END(std)