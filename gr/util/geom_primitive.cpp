#include "geom_primitive.h"

NAMESPACE_BEGIN(util)

template <typename VecType>
struct GeomPrimitive {
	using Vec = VecType;
	using Num = typename VecTraits<Vec>::ElemType;
	static constexpr int Dim = VecTraits<Vec>::Length;
	using Rotation = Rotation<Vec>;
	using Rot = typename Rotation::Rot;
	using Box = Box<Vec>;
	using OBox = OrientedBox<Vec>;
	using Sphere = Sphere<Vec>;

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
	explicit GeomPrimitive(Box const &box) { Set(box); }
	explicit GeomPrimitive(OBox const &obox) { Set(obox); }
	explicit GeomPrimitive(Sphere const &sphere) { Set(sphere); }

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

	void Set(Box const &box)
	{
		_kind = Kind::Box;
		_center = box.GetCenter();
		_halfExtent = box.GetSize() / 2;
	}

	void Set(OBox const &obox)
	{
		_kind = Kind::OrientedBox;
		_center = obox._center;
		_halfExtent = obox._halfSize;
		_orientation = obox._orientation;
	}

	void Set(Sphere const &sphere)
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

	Box GetBox() const
	{
		ASSERT(_kind == Kind::Box);
		return Box::FromCenterHalfSize(_center, _halfExtent);
	}

	OBox GetOrientedBox() const
	{
		ASSERT(_kind == Kind::OrientedBox);
		return OBox(_center, _halfExtent, _orientation);
	}

	Sphere GetSphere() const
	{
		ASSERT(_kind == Kind::Sphere);
		return Sphere(_center, _radius);
	}

	Box GetBoundingBox() const
	{
		switch (_kind) {
			case Kind::Point:
				return Box::FromPoint(_center);
			case Kind::Box:
				return GetBox();
			case Kind::OrientedBox:
				return GetOrientedBox().GetBox();
			case Kind::Sphere:
				return GetSphere().GetBox();
			default:
				ASSERT(0);
				return Box();
		}
	}

private:
	Vec _center, _halfExtent;
	Rot _orientation;
	Num _radius;
	Kind _kind;
};

NAMESPACE_END(util)