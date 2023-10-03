use glam::{Vec3, Mat4};

#[allow(dead_code)]
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Box3 {
    pub min: Vec3,
    pub max: Vec3,
}

#[allow(dead_code)]
impl Box3 {
    pub const EMPTY: Self  = Box3 { min: Vec3::INFINITY, max: Vec3::NEG_INFINITY, };
    pub const MAXIMUM: Self = Box3 { min: Vec3::NEG_INFINITY, max: Vec3::INFINITY };

    #[inline]
    pub fn new(min: Vec3, max: Vec3) -> Box3 {
        Box3 {min, max}
    }

    #[inline]
    pub fn from_min_and_size(min: Vec3, size: Vec3) -> Box3 {
        Box3 { min: min, max: min + size }
    }

    #[inline]
    pub fn from_center_half_size(center: Vec3, half_size: Vec3) -> Box3 {
        Box3 { min: center - half_size, max: center + half_size }
    }

    #[inline]
    pub fn is_empty(&self) -> bool {
        self.min.cmpgt(self.max).any()
    }

    #[inline]
    pub fn size(&self) -> Vec3 {
        self.max - self.min
    }

    #[inline]
    pub fn center(&self) -> Vec3 {
        0.5 * (self.min + self.max)
    }

    #[inline]
    pub fn volume(&self) -> f32 {
        let size = self.size().max(Vec3::ZERO);
        size.x * size.y * size.z
    }

    #[inline]
    pub fn intersection(&self, rhs: &Self) -> Box3 {
        Box3 { min: self.min.max(rhs.min), max: self.max.min(rhs.max) }
    }

    #[inline]
    pub fn intersects(&self, rhs: &Self) -> bool {
        !self.intersection(rhs).is_empty()
    }

    #[inline]
    pub fn union(&self, rhs: &Self) -> Box3 {
        Box3 { min: self.min.min(rhs.min), max: self.max.max(rhs.max) }
    }

    #[inline]
    pub fn union_point(&self, p: Vec3) -> Box3 {
        Box3 { min: self.min.min(p), max: self.max.max(p) }
    }

    #[inline]
    pub fn contains(&self, rhs: &Self) -> bool {
        self.min.cmple(rhs.min).all() && rhs.max.cmple(self.max).all() && !rhs.is_empty()
    }

    #[inline]
    pub fn contains_point(&self, p: Vec3) -> bool {
        self.min.cmple(p).all() && p.cmple(self.max).all()
    }

    pub fn get_point(&self, ind: u8) -> Vec3 {
        let mut v = Vec3::ZERO;
        for i in 0..3 {
            v[i] = if (ind & (1 << i)) == 0 {
                self.min[i]
            } else {
                self.max[i]
            }
        }
        return v;
    }

    pub fn get_transformed(&self, affine: Mat4) -> Box3 {
        let mut b = *self;
        for i in 0..8 {
            b = b.union_point((affine * self.get_point(i).extend(1.0)).truncate());
        }
        b
    }
}

#[allow(dead_code)]
pub fn min_elem_index(v: Vec3) -> u32 {
    let mut ind = 2 as u32;
    if v.x <= v.y {
        if v.x <= v.z {
            ind = 0
        } 
    } else {
        if v.y <= v.z {
            ind = 1
        } 
    }
    assert_eq!(v[ind as usize], v.min_element());
    ind
}

#[allow(dead_code)]
pub fn max_elem_index(v: Vec3) -> u32 {
    let mut ind = 2 as u32;
    if v.x >= v.y {
        if v.x >= v.z {
            ind = 0
        } 
    } else {
        if v.y >= v.z {
            ind = 1
        } 
    }
    assert_eq!(v[ind as usize], v.max_element());
    ind
}