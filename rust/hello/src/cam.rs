use glam::{Vec3, Mat4, Quat};

pub struct Camera {
    pub transform: Mat4,
    pub projection: Mat4,
}

impl Camera {
    pub fn view_proj(&self) -> Mat4 {
        (self.projection * self.transform.inverse()).inverse()
    }
    pub fn modify_transform(&mut self, delta_pos: Vec3, delta_rot: Quat) {
        let (_, rot, pos) = self.transform.to_scale_rotation_translation();
        self.transform = Mat4::from_rotation_translation(rot * delta_rot, pos + delta_pos);
    }
}
