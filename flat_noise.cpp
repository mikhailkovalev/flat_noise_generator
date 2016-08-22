#include "flat_noise.hpp"
#include <stdexcept>
//#include <cstdio>

bool FlatNoise::__random_inited = init_random();
float FlatNoise::__epsilon = 1e-8f;

bool FlatNoise::init_random()
{
	srand(time(0));
	return true;
}

float FlatNoise::get_random()
{
	return static_cast<float>(rand()) / RAND_MAX;
}

float FlatNoise::mid_disp(float v1, float v2, float offset)
{
	return 0.5f * (v1 + v2) + (2.0f * get_random() - 1.0f) * offset;
}

float FlatNoise::mid_disp(float v1, float v2, float v3, float offset)
{
	return (v1 + v2 + v3) / 3.0f + (2.0f * get_random() - 1.0f) * offset;
}

float FlatNoise::mid_disp(float v1, float v2, float v3, float v4, float offset)
{
	return 0.25f * (v1 + v2 + v3 + v4) + (2.0f * get_random() - 1.0f) * offset;
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
	generate(0.0, 0.1, 0.5, 0.5);
}

inline void FlatNoise::generate(float low, float high)
{
	generate(low, high, 0.5 * (low + high), 0.5);
}

void FlatNoise::generate(float low, float high, float offset, float compress)
{
	_compress = compress;

	get(0, 0)                              = get_random();
	get(_row_count - 1, 0)                 = get_random();
	get(0, _column_count - 1)              = get_random();
	get(_row_count - 1, _column_count - 1) = get_random();

	/*printf("%f -> [0, 0]\n", get(0, 0));
	printf("%f -> [%u, 0]\n", get(_row_count - 1, 0), _row_count - 1);
	printf("%f -> [0, %u]\n", get(0, _column_count - 1), _column_count - 1);
	printf("%f -> [%u, %u]\n", get(_row_count - 1, _column_count - 1), _row_count - 1, _column_count - 1);
	printf("\n\n");*/

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

	//printf("square {(%u, %u), (%u, %u)}\n", left, top, right, bottom);

	float mid = mid_disp(top_left, top_right, bottom_left, bottom_right, offset);

	size_t h_mid = left + right >> 1;
	size_t v_mid = top + bottom >> 1;

	size_t dh = right - left >> 1;
	size_t dv = bottom - top >> 1;

	if (dh == 0) dh = 1;
	if (dv == 0) dv = 1;

	if (right <= 1 + left && bottom <= 1 + top)
	{
		//printf("it is too small\n\n\n");
		return;
	}
	if (right <= 1 + left)
	{
		get(v_mid, left)  = mid_disp(top_left,  bottom_left,  0);
		get(v_mid, right) = mid_disp(top_right, bottom_right, 0);

		/*printf("it is vertical bar\n");
		printf("%f -> (%u, %u)\n", get(v_mid, left), v_mid, left);
		printf("%f -> (%u, %u)\n", get(v_mid, right), v_mid, right);
		printf("\n\n");*/

		offset *= _compress;
		make_square(left, top,   right,  v_mid, offset);
		make_square(left, v_mid, right, bottom, offset);
		return;
	}
	if (bottom <= 1 + top)
	{
		get(top,    h_mid) = mid_disp(top_left,    top_right,    0);
		get(bottom, h_mid) = mid_disp(bottom_left, bottom_right, 0);

		/*printf("it is horizontal bar\n");
		printf("%f -> (%u, %u)\n", get(top, h_mid), top, h_mid);
		printf("%f -> (%u, %u)\n", get(bottom, h_mid), bottom, h_mid);
		printf("\n\n");*/

		offset *= _compress;
		make_square(left,  top, h_mid, bottom, offset);
		make_square(h_mid, top, right, bottom, offset);
		return;
	}
	get(v_mid,  h_mid) = mid;

	/*if (left >= dh)
	{
		get(v_mid, left) = mid_disp(top_left, bottom_left, mid, get(v_mid, left - dh), 0);
	}
	else
	{
		get(v_mid, left) = mid_disp(top_left,    bottom_left,  0);
	}*/
	get(v_mid,  left)  = mid_disp(top_left,    bottom_left,  0);

	get(v_mid,  right) = mid_disp(top_right,   bottom_right, 0);

	/*if (top >= dv)
	{
		get(top, h_mid) = mid_disp(top_left, top_right, mid, get(top - dv, h_mid), offset);
	}
	else
	{
		get(top,    h_mid) = mid_disp(top_left,    top_right,    offset);
	}*/
	get(top,    h_mid) = mid_disp(top_left,    top_right, 0);

	get(bottom, h_mid) = mid_disp(bottom_left, bottom_right, 0);

	/*printf("%f -> (%u, %u)\n", get(v_mid, h_mid), v_mid, h_mid);
	printf("%f -> (%u, %u)\n", get(v_mid, left), v_mid, left);
	printf("%f -> (%u, %u)\n", get(v_mid, right), v_mid, right);
	printf("%f -> (%u, %u)\n", get(top, h_mid), top, h_mid);
	printf("%f -> (%u, %u)\n", get(bottom, h_mid), bottom, h_mid);
	printf("\n\n");*/

	offset *= _compress;
	make_square(left,  top,   h_mid, v_mid,  offset);
	make_square(h_mid, top,   right, v_mid,  offset);
	make_square(left,  v_mid, h_mid, bottom, offset);
	make_square(h_mid, v_mid, right, bottom, offset);
}
