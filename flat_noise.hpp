#pragma once

#include <vector>
#include <ctime>
#include <cstdlib>

class FlatNoise
{
public:
	FlatNoise(size_t row_count, size_t column_count, float low = 0.0f, float high = 1.0f, float offset = 0.5f, float compress = 0.5f);

	void generate();                      //low = 0, high = 1, offset = 0.5, compress = 0.5
	void generate(float low, float high); //offset = 0.5 * abs(high - low), compress = 0.5
	void generate(float low, float high, float offset, float compress);
	void transform_to(float low, float high);

	size_t row_count() const;
	size_t column_count() const;

	float& get(size_t row, size_t column);
	const float& get(size_t row, size_t column) const;
protected:
	void make_square(size_t left, size_t top, size_t right, size_t bottom, float offset);

	std::vector<float> _v;

	size_t _row_count;
	size_t _column_count;

	float _compress;
private:
	static bool init_random();
	static float get_random();

	static float mid_disp(float v1, float v2, float offset);
	static float mid_disp(float v1, float v2, float v3, float offset);
	static float mid_disp(float v1, float v2, float v3, float v4, float offset);

	static float mid(float v1, float v2);

	static bool __random_inited;
	static float __epsilon;
};
