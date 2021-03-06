#include "flat_noise.hpp"
#include <stdexcept>
#include <cmath>

bool FlatNoise::__random_inited = init_random();
float FlatNoise::__epsilon = 1e-8f;

bool FlatNoise::init_random()
{
	srand(time(0));
	return true;
}

inline float FlatNoise::get_random()
{
	return static_cast<float>(rand()) / RAND_MAX;
}

inline float FlatNoise::mid_disp(float v1, float v2, float offset)
{
	return 0.5f * (v1 + v2) + (2.0f * get_random() - 1.0f) * offset;
}

inline float FlatNoise::mid_disp(float v1, float v2, float v3, float offset)
{
	static float factor = 1.0f / 3.0f;
	return factor * (v1 + v2 + v3) + (2.0f * get_random() - 1.0f) * offset;
}

inline float FlatNoise::mid_disp(float v1, float v2, float v3, float v4, float offset)
{
	return 0.25f * (v1 + v2 + v3 + v4) + (2.0f * get_random() - 1.0f) * offset;
}

inline float FlatNoise::mid(float v1, float v2)
{
	return 0.5 * (v1 + v2);
}

FlatNoise::FlatNoise(size_t row_count, size_t column_count, float low, float high, float offset, float compress)
	:_row_count(row_count)
	,_column_count(column_count)
	,_v(row_count * column_count)
{
	generate(low, high, offset, compress);
}

inline void FlatNoise::generate()
{
	generate(0.0f, 0.1f, 0.5f, 0.5f);
}

inline void FlatNoise::generate(float low, float high)
{
	generate(low, high, 0.5f * fabs(high - low), 0.5f);
}

void FlatNoise::generate(float low, float high, float offset, float compress)
{
	_compress = compress;

	get(0, 0)                              = get_random();
	get(_row_count - 1, 0)                 = get_random();
	get(0, _column_count - 1)              = get_random();
	get(_row_count - 1, _column_count - 1) = get_random();

	make_square(0, 0, _column_count - 1, _row_count - 1, offset / (high - low));
	transform_to(low, high);
}

void FlatNoise::transform_to(float low, float high)
{
	float current_high, current_low, current_value;
	current_high = current_low = _v[0];

	std::vector<float>::iterator p = _v.begin(), end = _v.end();
	for (; p != end; ++p)
	{
		current_value = *p;
		if      (current_value > current_high) current_high = current_value;
		else if (current_value < current_low)  current_low  = current_value;
	}

	float scale, offset;

	if (current_high - current_low <= __epsilon)
	{
		scale = 0.0f;
		offset = 0.5f * (low + high);
	}
	else
	{
		scale = (high - low) / (current_high - current_low);
		offset = high - scale * current_high;
	}

	for(p = _v.begin(); p != end; ++p)
	{
		(*p) *= scale;
		(*p) += offset;
	}
}

inline size_t FlatNoise::row_count() const
{
	return _row_count;
}

inline size_t FlatNoise::column_count() const
{
	return _column_count;
}

inline float& FlatNoise::get(size_t row, size_t column)
{
	if (row > _row_count || column > _column_count) throw std::out_of_range("vector::_M_range_check");
	return _v.at(row * _column_count + column);
}

inline const float& FlatNoise::get(size_t row, size_t column) const
{
	if (row > _row_count || column > _column_count) throw std::out_of_range("vector::_M_range_check");
	return _v.at(row * _column_count + column);
}

void FlatNoise::make_square(size_t left, size_t top, size_t right, size_t bottom, float offset)
{
	float top_left     = get(top, left);
	float top_right    = get(top, right);
	float bottom_left  = get(bottom, left);
	float bottom_right = get(bottom, right);

	size_t h_mid = left + right >> 1;
	size_t v_mid = top + bottom >> 1;

	if (right <= 1 + left && bottom <= 1 + top)
	{
		return;
	}
	if (right <= 1 + left)
	{
		get(v_mid, left)  = mid(top_left,  bottom_left);
		get(v_mid, right) = mid(top_right, bottom_right);

		make_square(left, top,   right,  v_mid, offset);
		make_square(left, v_mid, right, bottom, offset);
		return;
	}
	if (bottom <= 1 + top)
	{
		get(top,    h_mid) = mid(top_left,    top_right);
		get(bottom, h_mid) = mid(bottom_left, bottom_right);

		make_square(left,  top, h_mid, bottom, offset);
		make_square(h_mid, top, right, bottom, offset);
		return;
	}

	get(v_mid,  h_mid) = mid_disp(top_left, top_right, bottom_left, bottom_right, offset);
	get(v_mid,  left)  = mid(top_left,    bottom_left);
	get(v_mid,  right) = mid(top_right,   bottom_right);
	get(top,    h_mid) = mid(top_left,    top_right);
	get(bottom, h_mid) = mid(bottom_left, bottom_right);

	offset *= _compress;

	make_square(left,  top,   h_mid, v_mid,  offset);
	make_square(h_mid, top,   right, v_mid,  offset);
	make_square(left,  v_mid, h_mid, bottom, offset);
	make_square(h_mid, v_mid, right, bottom, offset);
}
