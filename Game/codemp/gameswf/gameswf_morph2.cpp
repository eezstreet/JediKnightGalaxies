// gameswf_morph2.cpp -- Alexeev Vitaly 2004

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Loading and rendering of morphing shapes using gameswf_shape.


#include "gameswf_morph2.h"
#include "gameswf_stream.h"


namespace gameswf
{
	morph2_character_def::morph2_character_def()
		:
		m_last_ratio(-1.0f),
		m_mesh(0)
	{
		shape1 = new shape_character_def();
		shape2 = new shape_character_def();
	}


	morph2_character_def::~morph2_character_def()
	{
		// shape1 & shape2 drop their references on destructor.
		//delete shape2;
		//delete shape1;

		delete m_mesh;
	}

	void morph2_character_def::lerp_matrix(matrix* t, const matrix& m1, const matrix& m2, const float ratio)
	{
		t->m_[0][0] = flerp(m1.m_[0][0], m2.m_[0][0], ratio);
		t->m_[1][0] = flerp(m1.m_[1][0], m2.m_[1][0], ratio);
		t->m_[0][1] = flerp(m1.m_[0][1], m2.m_[0][1], ratio);
		t->m_[1][1] = flerp(m1.m_[1][1], m2.m_[1][1], ratio);
		t->m_[0][2] = flerp(m1.m_[0][2], m2.m_[0][2], ratio);
		t->m_[1][2] = flerp(m1.m_[1][2], m2.m_[1][2], ratio);
	}

	void	morph2_character_def::display(character* inst)
	{
		float ratio = inst->m_ratio;

		// bounds
		m_bound.m_x_min = flerp(shape1->m_bound.m_x_min, shape2->m_bound.m_x_min, ratio);
		m_bound.m_y_min = flerp(shape1->m_bound.m_y_min, shape2->m_bound.m_y_min, ratio);
		m_bound.m_x_max = flerp(shape1->m_bound.m_x_max, shape2->m_bound.m_x_max, ratio);
		m_bound.m_y_max = flerp(shape1->m_bound.m_y_max, shape2->m_bound.m_y_max, ratio);

		// fill styles
		for (int i=0; i < m_fill_styles.size(); i++)
		{
			fill_style& fs = m_fill_styles[i];
			fill_style& fs1 = shape1->m_fill_styles[i];
			fill_style& fs2 = shape2->m_fill_styles[i];

			// fill style type
			fs.m_type = fs1.m_type;

			// fill style color
			fs.m_color.set_lerp(fs1.m_color, fs2.m_color, ratio);

			// fill style gradient matrix
			lerp_matrix(&fs.m_gradient_matrix,
				    fs1.m_gradient_matrix,
				    fs2.m_gradient_matrix,
				    ratio);

			// fill style gradients
			for (int i=0; i < fs.m_gradients.size(); i++)
			{
				fs.m_gradients[i].m_ratio = (Uint8)frnd(flerp(fs1.m_gradients[i].m_ratio, fs2.m_gradients[i].m_ratio, ratio));
				fs.m_gradients[i].m_color.set_lerp(fs1.m_gradients[i].m_color, fs2.m_gradients[i].m_color, ratio);
			}

			// fill style bitmap ID
			fs.m_bitmap_character = fs1.m_bitmap_character;

			// fill style bitmap matrix
			lerp_matrix(&fs.m_bitmap_matrix, fs1.m_bitmap_matrix, fs2.m_bitmap_matrix, ratio);
		}

		// line styles
		for (int i=0; i < m_line_styles.size(); i++)
		{
			line_style& ls = m_line_styles[i];
			line_style& ls1 = shape1->m_line_styles[i];
			line_style& ls2 = shape2->m_line_styles[i];
			ls.m_width = (Uint16)frnd(flerp(ls1.m_width, ls2.m_width, ratio));
			ls.m_color.set_lerp(ls1.m_color, ls2.m_color, ratio);
		}

		// shape
		int k=0, n=0;
		for (int i=0; i < m_paths.size(); i++)
		{
			path& p = m_paths[i];
			path& p1 = shape1->m_paths[i];

			p.m_fill0 = p1.m_fill0;
			p.m_fill1 = p1.m_fill1;

 			// @@ hack.
			if (p.m_fill0==0 && p.m_fill1==0)
			{
				if (shape1->m_fill_styles.size() > 0) p.m_fill0 = 1;
			}
      
			p.m_line = p1.m_line;

			p.m_ax = flerp(p1.m_ax, shape2->m_paths[n].m_ax, ratio);
			p.m_ay = flerp(p1.m_ay, shape2->m_paths[n].m_ay, ratio);
      
			//  edges;
			int len = p1.m_edges.size();
			p.m_edges.resize(len);

			for (int i=0; i < p.m_edges.size(); i++)
			{
				p.m_edges[i].m_cx = flerp(p1.m_edges[i].m_cx, shape2->m_paths[n].m_edges[k].m_cx, ratio);
				p.m_edges[i].m_cy = flerp(p1.m_edges[i].m_cy, shape2->m_paths[n].m_edges[k].m_cy, ratio);
				p.m_edges[i].m_ax = flerp(p1.m_edges[i].m_ax, shape2->m_paths[n].m_edges[k].m_ax, ratio);
				p.m_edges[i].m_ay = flerp(p1.m_edges[i].m_ay, shape2->m_paths[n].m_edges[k].m_ay, ratio);
				k++;
				if (shape2->m_paths[n].m_edges.size() <= k)
				{
					k=0; n++;
				}
			}
		}
    
		//  display

		matrix mat = inst->get_world_matrix();
		cxform cx = inst->get_world_cxform();
		float max_error = 20.0f / mat.get_max_scale() /	inst->get_parent()->get_pixel_scale();
		if (ratio != m_last_ratio) {
			delete m_mesh;
			m_last_ratio = ratio;
			m_mesh = new mesh_set(this, max_error * 0.75f);
		}
		m_mesh->display(mat, cx, m_fill_styles, m_line_styles);
	}

  
	void	morph2_character_def::read(stream* in, int tag_type, bool with_style, movie_definition_sub* md)
	{
		UNUSED(tag_type);
		UNUSED(with_style);
		shape1->m_bound.read(in);
		shape2->m_bound.read(in);
		offset = in->read_u32();

		fill_style_count = in->read_variable_count();
		for (int i = 0; i < fill_style_count; i++) {
			fill_style fs1, fs2;

			fs1.m_type = in->read_u8();
			fs2.m_type = fs1.m_type;

			IF_VERBOSE_PARSE(log_msg("morph fill style type = 0x%X\n", fs1.m_type));

			if (fs1.m_type == 0x00)
			{
				fs1.m_color.read_rgba(in);
				fs2.m_color.read_rgba(in);

				IF_VERBOSE_PARSE(log_msg("morph fill style begin color: "); fs1.m_color.print());
				IF_VERBOSE_PARSE(log_msg("morph fill style end color: "); fs2.m_color.print());
			}
			else if (fs1.m_type == 0x10 || fs1.m_type == 0x12)
			{
				matrix	input_matrix1, input_matrix2;

				input_matrix1.read(in);
				input_matrix2.read(in);

				fs1.m_gradient_matrix.set_identity();
				fs2.m_gradient_matrix.set_identity();
				if (fs1.m_type == 0x10)
				{
					fs1.m_gradient_matrix.concatenate_translation(128.f, 0.f);
					fs1.m_gradient_matrix.concatenate_scale(1.0f / 128.0f);
					fs2.m_gradient_matrix.concatenate_translation(128.f, 0.f);
					fs2.m_gradient_matrix.concatenate_scale(1.0f / 128.0f);
				}
				else
				{
					fs1.m_gradient_matrix.concatenate_translation(32.f, 32.f);
					fs1.m_gradient_matrix.concatenate_scale(1.0f / 512.0f);
					fs2.m_gradient_matrix.concatenate_translation(32.f, 32.f);
					fs2.m_gradient_matrix.concatenate_scale(1.0f / 512.0f);
				}

				matrix	m1, m2;
				m1.set_inverse(input_matrix1);
				fs1.m_gradient_matrix.concatenate(m1);
				m2.set_inverse(input_matrix2);
				fs2.m_gradient_matrix.concatenate(m2);

				// GRADIENT
				int	num_gradients = in->read_u8();
				assert(num_gradients >= 1 && num_gradients <= 8);

				fs1.m_gradients.resize(num_gradients);
				fs2.m_gradients.resize(num_gradients);

				for (int i = 0; i < num_gradients; i++)
				{
					fs1.m_gradients[i].read(in, tag_type);
					fs2.m_gradients[i].read(in, tag_type);
				}

				IF_VERBOSE_PARSE(log_msg("morph fsr: num_gradients = %d\n", num_gradients));

				// @@ hack.
				if (num_gradients > 0)
				{
					fs1.m_color = fs1.m_gradients[0].m_color;
					fs2.m_color = fs2.m_gradients[0].m_color;
				}
			}
			else if (fs1.m_type == 0x40 || fs1.m_type == 0x41)
			{

				int	bitmap_char_id = in->read_u16();
				IF_VERBOSE_PARSE(log_msg("morph fsr bitmap_char = %d\n", bitmap_char_id));

				// Look up the bitmap character.
				fs1.m_bitmap_character = md->get_bitmap_character(bitmap_char_id);
				fs2.m_bitmap_character = fs1.m_bitmap_character;

				matrix	m1, m2;
				m1.read(in);
				m2.read(in);

				// For some reason, it looks like they store the inverse of the
				// TWIPS-to-texcoords matrix.
				fs1.m_bitmap_matrix.set_inverse(m1);
				fs2.m_bitmap_matrix.set_inverse(m2);
			}
			shape1->m_fill_styles.push_back(fs1);
			shape2->m_fill_styles.push_back(fs2);
		}

		line_style_count = in->read_variable_count();
		for (int i = 0; i < line_style_count; i++) {
			line_style ls1, ls2;
			ls1.m_width = in->read_u16();
			ls2.m_width = in->read_u16();
			ls1.m_color.read(in, tag_type);
			ls2.m_color.read(in, tag_type);
			shape1->m_line_styles.push_back(ls1);
			shape2->m_line_styles.push_back(ls2);
		}

		shape1->read(in, tag_type, false, md);
		in->align();
		shape2->read(in, tag_type, false, md);

		assert(shape1->m_fill_styles.size() == shape2->m_fill_styles.size());
		assert(shape1->m_line_styles.size() == shape2->m_line_styles.size());

		// setup array size
		m_fill_styles.resize(shape1->m_fill_styles.size());
		for (int i=0; i < m_fill_styles.size(); i++)
		{
			fill_style& fs = m_fill_styles[i];
			fill_style& fs1 = shape1->m_fill_styles[i];
			fs.m_gradients.resize(fs1.m_gradients.size());
		}
		m_line_styles.resize(shape1->m_line_styles.size());
		m_paths.resize(shape1->m_paths.size());

		int edges_count1 = 0;
		for (int i=0; i < m_paths.size(); i++)
		{
			path& p = m_paths[i];
			path& p1 = shape1->m_paths[i];
			int len = p1.m_edges.size();
			edges_count1 += len;
			p.m_edges.resize(len);
		}

		int edges_count2 = 0;
		for (int i=0; i < shape2->m_paths.size(); i++)
		{
			path& p2 = shape2->m_paths[i];
			int len = p2.m_edges.size();
			edges_count2 += len;
		}
		assert(edges_count1 == edges_count2);
	}
}
