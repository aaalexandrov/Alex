use image::*;
use vulkano::{
    image::{
        Image, ImageCreateInfo, ImageUsage, sampler::{Sampler, SamplerCreateInfo, Filter, SamplerMipmapMode, SamplerAddressMode}, view::{ImageViewCreateInfo, ImageView}
    }, 
    memory::allocator::AllocationCreateInfo, format::Format, command_buffer::{AutoCommandBufferBuilder, PrimaryAutoCommandBuffer, CopyBufferToImageInfo}, 
    buffer::{BufferUsage, Subbuffer}, DeviceSize, pipeline::{GraphicsPipeline, Pipeline, PipelineBindPoint}, descriptor_set::{PersistentDescriptorSet, WriteDescriptorSet}
};
use glam::*;
use std::sync::Arc;

use crate::app::Renderer;

use crate::solid;

#[derive(Clone)]
pub struct FixedFontData {
    pub char_size: UVec2,
    pub texture_view: Arc<ImageView>,
    pub uniform_buffer: Subbuffer<solid::UniformData>,
    pub sampler: Arc<Sampler>,
    pub pipeline: Arc<GraphicsPipeline>,
    pub desc_set: Arc<PersistentDescriptorSet>,
}

impl FixedFontData {
    pub fn from_file(file: &str, renderer: &Renderer, attachment_format: Format, upload_builder: &mut AutoCommandBufferBuilder<PrimaryAutoCommandBuffer>) -> Self {
        let img = image::io::Reader::open(&file).unwrap().decode().unwrap();
        //println!("{img:?}");
        let DynamicImage::ImageLuma8(gray_img) = img else {
            panic!("Unexpected font image format");
        };

        let img_size: UVec2 = gray_img.dimensions().into();
        let num_chars: u32 = 0x80-0x20;
        // we assume 1:2 width to height ratio
        let char_width = f64::sqrt((img_size.x * img_size.y / (num_chars * 2)) as f64) as u32;
        let char_size = uvec2(char_width, char_width * 2);
        assert_eq!(img_size % char_size, UVec2::ZERO);
        let char_cells = img_size / char_size;
        assert_eq!(char_cells.x * char_cells.y, num_chars);

        let img_size = char_cells * (char_size + 1);
        let texture = Image::new(
            renderer.allocator.as_ref(),
            ImageCreateInfo{
                format: Format::R8_UNORM,
                extent: [img_size.x, img_size.y, 1],
                usage: ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED,
                ..Default::default()
            },
            AllocationCreateInfo::default()
        ).unwrap();

        let upload_pixels = renderer.get_buffer_write(BufferUsage::TRANSFER_SRC, (img_size.x * img_size.y) as DeviceSize, |pixels| {
            for py in 0..img_size.y {
                let cy = py / (char_size.y + 1);
                let y = py % (char_size.y + 1);
                for px in 0..img_size.x {
                    let cx = px / (char_size.x + 1);
                    let x = px % (char_size.x + 1);
                    let pix = if y < char_size.y && x < char_size.x {
                        gray_img.get_pixel(cx * char_size.x + x, cy * char_size.y + y).0[0]
                    } else {
                        0 as u8
                    };
                    pixels[(py * img_size.x + px) as usize] = pix;
                }
            }
        });

        upload_builder
            .copy_buffer_to_image(CopyBufferToImageInfo::buffer_image(
                upload_pixels, 
                texture.clone()
            )).unwrap();

        let uniform_buffer = renderer.get_buffer_data(BufferUsage::UNIFORM_BUFFER, solid::UniformData{ view_proj: Mat4::IDENTITY.to_cols_array_2d() });

        let sampler = Sampler::new(
            renderer.device.clone(),
            SamplerCreateInfo {
                mag_filter: Filter::Linear,
                min_filter: Filter::Linear,
                mipmap_mode: SamplerMipmapMode::Linear,
                address_mode: [SamplerAddressMode::Repeat; 3],
                ..SamplerCreateInfo::default()
            }
        ).unwrap();

        let texture_view = ImageView::new(
            texture.clone(), 
            ImageViewCreateInfo {
                usage: ImageUsage::SAMPLED,
                ..ImageViewCreateInfo::from_image(&texture)
            }).unwrap();

        let pipeline = crate::solid::load_pipeline(&renderer, attachment_format);

        let desc_set = PersistentDescriptorSet::new(
            &renderer.descriptor_set_allocator,
            pipeline.layout().set_layouts()[0].clone(),
            [
                WriteDescriptorSet::buffer(0, uniform_buffer.clone()),
                WriteDescriptorSet::sampler(1, sampler.clone()),
                WriteDescriptorSet::image_view(2, texture_view.clone()),
            ],
            []
        ).unwrap();

        Self {
            char_size,
            texture_view,
            uniform_buffer,
            sampler,
            pipeline,
            desc_set,
        }
    }
}

pub struct StrData {
    pub pos: Vec2,
    pub char_size: Vec2,
    pub str: String,
    pub color: [u8; 4],
}

pub struct FixedFont {
    pub data: FixedFontData,
    pub strings: Vec::<StrData>,
}

impl FixedFont {
    pub fn new(data: FixedFontData) -> Self {
        Self { 
            data,
            strings: Vec::new(),
        }
    }

    pub fn clear(&mut self) {
        self.strings.clear();
    }

    pub fn add_str(&mut self, pos: Vec2, char_size: Vec2, str: String, color: [u8; 4]) {
        self.strings.push(StrData{
            pos: pos + char_size * vec2(0.0, -1.0),
            char_size,
            str,
            color,
        });
    }

    pub fn add_str_pix(&mut self, pos: UVec2, img_size: UVec2, str: String, color: [u8; 4]) {
        let img_size_vec2 = img_size.as_vec2();
        self.add_str(pos.as_vec2() / img_size_vec2, self.data.char_size.as_vec2() / img_size_vec2, str, color);
    }

    pub fn render(&self, renderer: &Renderer, cmd_builder: &mut AutoCommandBufferBuilder<PrimaryAutoCommandBuffer>) {
        let chars = self.get_num_characters() as DeviceSize;
        let indices = renderer.get_buffer_write(BufferUsage::INDEX_BUFFER, chars * 6, |inds: &mut [u32]| {
            for i in 0..inds.len()/6 {
                let t = (i * 4) as u32;
                inds[i*6..(i+1)*6].copy_from_slice(&[t + 0, t + 1, t + 2, t + 2, t + 1, t + 3]);
            }
        });
        let vertices = renderer.get_buffer_write(BufferUsage::VERTEX_BUFFER, chars * 4, |verts: &mut [solid::SolidVertex]| {
            let tex_size = UVec2::from_slice(&self.data.texture_view.image().extent());
            let char_cells = tex_size / (self.data.char_size + 1);
            let tc_size = self.data.char_size.as_vec2() / tex_size.as_vec2();
            let tc_cell_size = (self.data.char_size + 1).as_vec2() / tex_size.as_vec2();
            let mut i = 0;
            const V01: Vec2 = vec2(0.0, 1.0);
            const V10: Vec2 = vec2(1.0, 0.0);
            for s in self.strings.iter() {
                let mut pos = s.pos;
                for c in s.str.chars() {
                    if (c as u32) < 0x20 || 0x7f < (c as u32) {
                        continue;
                    }
                    let char_ind = c as u32 - 0x20;
                    let char_cell = uvec2(char_ind % char_cells.x, char_ind / char_cells.x);
                    assert!(char_cell.cmplt(char_cells).all());

                    let tc = char_cell.as_vec2() * tc_cell_size;

                    verts[i + 0] = solid::SolidVertex {
                        pos: (pos * 2.0 - 1.0, 0.0).into(),
                        tc,
                        color: s.color,
                    };

                    verts[i + 1] = solid::SolidVertex {
                        pos: ((pos + s.char_size * V10) * 2.0 - 1.0, 0.0).into(),
                        tc: tc + tc_size * V10,
                        color: s.color,
                    };

                    verts[i + 2] = solid::SolidVertex {
                        pos: ((pos + s.char_size * V01) * 2.0 - 1.0, 0.0).into(),
                        tc: tc + tc_size * V01,
                        color: s.color,
                    };

                    verts[i + 3] = solid::SolidVertex {
                        pos: ((pos + s.char_size) * 2.0 - 1.0, 0.0).into(),
                        tc: tc + tc_size,
                        color: s.color,
                    };

                    pos += s.char_size * V10;
                    i += 4;
                }
            }
        });

        let num_indices = indices.len() as u32;
        cmd_builder
            .bind_pipeline_graphics(self.data.pipeline.clone()).unwrap()
            .bind_descriptor_sets(PipelineBindPoint::Graphics, self.data.pipeline.layout().clone(), 0, self.data.desc_set.clone()).unwrap()
            .bind_vertex_buffers(0, vertices).unwrap()
            .bind_index_buffer(indices).unwrap()
            .draw_indexed(num_indices, 1, 0, 0, 0).unwrap();    
    }

    fn get_num_characters(&self) -> usize {
        self.strings.iter().map(|s| s.str.len()).sum()
    }
}