#include "flat_noise.hpp"
#include <stdexcept>

bool FlatNoise::__random_inited = init_random();
float FlatNoise::__epsilon = 1e-8f;

bool FlatNoise::init_random()
{
	srand(time(0));
	return true;
}

FlatNoise::FlatNoise(size_t row_count, size_t column_count, float low, float high, float offset, float compress)
	:_row_count(row_count)
	,_column_count(column_count)
	,_v(row_count * column_count)
{
}

void FlatNoise::generate()
{
}

void FlatNoise::generate(float low, float high)
{
}

void FlatNoise::generate(float low, float high, float offset, float compress)
{
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

size_t FlatNoise::row_count() const
{
	return _row_count;
}

size_t FlatNoise::column_count() const
{
	return _column_count;
}

float& FlatNoise::get(size_t row, size_t column)
{
	if (row > _row_count || column > _column_count) throw std::out_of_range("vector::_M_range_check");
	return _v.at(row * _column_count + column);
}

const float& FlatNoise::get(size_t row, size_t column) const
{
	if (row > _row_count || column > _column_count) throw std::out_of_range("vector::_M_range_check");
	return _v.at(row * _column_count + column);
}
