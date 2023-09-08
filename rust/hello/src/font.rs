use image::*;
use vulkano::image::Image;
use glam::*;
use std::sync::Arc;

pub struct FixedFont {
    pub char_size: UVec2,
    pub tex_size: UVec2,
    pub texture: Arc<Image>,
}

impl FixedFont {
    pub fn from_file(file: &str) -> Result<FixedFont, ImageError> {
        let img = image::io::Reader::open(&file)?.decode()?;
        //println!("{img:?}");
        let DynamicImage::ImageLuma8(gray_img) = img else {
            return Result::Err(ImageError::Unsupported(error::UnsupportedError::from_format_and_kind(
                error::ImageFormatHint::Unknown, 
                error::UnsupportedErrorKind::Format(error::ImageFormatHint::Unknown))));    
        };

        let img_size: UVec2 = gray_img.dimensions().into();
        let num_chars: u32 = 0x80-0x20;
        // we assume 1:2 width to height ratio
        let char_width = f64::sqrt((img_size.x * img_size.y / (num_chars * 2)) as f64) as u32;
        let char_size = uvec2(char_width, char_width * 2);
        assert_eq!(img_size % char_size, UVec2::ZERO);
        let char_cells = img_size / char_size;
        assert_eq!(char_cells.x * char_cells.y, num_chars);

        

        //let mut font = FixedFont {
          //  char_size: uvec2
        //}

        Result::Err(ImageError::Limits(error::LimitError::from_kind(error::LimitErrorKind::InsufficientMemory)))
    }
}