#include "flat_noise.hpp"
#include <stdexcept>

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
