#pragma once

#include "geom.h"

NAMESPACE_BEGIN(util)

template <typename VecType>
struct GeomPrimitive {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	using RotationV = Rotation<Vec>;
	using Rot = typename RotationV::Rot;
	using BoxV = Box<Vec>;
	using OBoxV = OrientedBox<Vec>;
	using SphereV = Sphere<Vec>;

	enum class Kind {
		None,
		Point,
		Box,
		OrientedBox,
		Sphere,
	};

	GeomPrimitive() { Set(); }
	GeomPrimitive(GeomPrimitive const &other) { Set(other); }
	explicit GeomPrimitive(Vec const &p) { Set(p); }
	explicit GeomPrimitive(BoxV const &box) { Set(box); }
	explicit GeomPrimitive(OBoxV const &obox) { Set(obox); }
	explicit GeomPrimitive(SphereV const &sphere) { Set(sphere); }

	GeomPrimitive &operator=(GeomPrimitive const &other) { Set(other); }

	Kind GetKind() const { return _kind; }

	void Set()
	{
		_kind = Kind::None;
	}

	void Set(Vec const &p)
	{
		_kind = Kind::Point;
		_center = p;
	}

	void Set(BoxV const &box)
	{
		_kind = Kind::Box;
		_center = box.GetCenter();
		_halfExtent = box.GetSize() / 2;
	}

	void Set(OBoxV const &obox)
	{
		_kind = Kind::OrientedBox;
		_center = obox._center;
		_halfExtent = obox._halfSize;
		_orientation = obox._orientation;
	}

	void Set(SphereV const &sphere)
	{
		_kind = Kind::Sphere;
		_center = sphere._center;
		_radius = sphere._radius;
	}

	void Set(GeomPrimitive const &other)
	{
		_kind = other._kind;
		_center = other._center;
		_halfExtent = other._halfExtent;
		_orientation = other._orientation;
		_radius = other._radius;
	}

	Vec GetPoint() const
	{
		ASSERT(_kind == Kind::Point);
		return _center;
	}

	BoxV GetBox() const
	{
		ASSERT(_kind == Kind::Box);
		return BoxV::FromCenterHalfSize(_center, _halfExtent);
	}

	OBoxV GetOrientedBox() const
	{
		ASSERT(_kind == Kind::OrientedBox);
		return OBoxV(_center, _halfExtent, _orientation);
	}

	SphereV GetSphere() const
	{
		ASSERT(_kind == Kind::Sphere);
		return SphereV(_center, _radius);
	}

	BoxV GetBoundingBox() const
	{
		switch (_kind) {
			case Kind::Point:
				return BoxV::FromPoint(_center);
			case Kind::Box:
				return GetBox();
			case Kind::OrientedBox:
				return GetOrientedBox().GetBox();
			case Kind::Sphere:
				return GetSphere().GetBox();
			default:
				ASSERT(0);
				return BoxV();
		}
	}

private:
	Vec _center, _halfExtent;
	Rot _orientation;
	Num _radius;
	Kind _kind;
};


NAMESPACE_END(util)