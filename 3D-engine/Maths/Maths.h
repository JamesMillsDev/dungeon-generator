#pragma once

class Maths
{
public:
	/** @brief Value close to 0 (1.4e^-45) */
	static float const EPSILON;
	static float const E;
	static float const PI;
	static float const TAU;
	static float const POSITIVE_INFINITY;
	static float const NEGATIVE_INFINITY;

public:
	static float Sin(float v);

	static float Cos(float v);

	static float Tan(float v);

	static float ASin(float v);

	static float ACos(float v);

	static float ATan(float v);

	static float ATan2(float y, float x);

	static float Sqrt(float v);

	static float Abs(float v);

	static int Abs(int v);

	static float Min(float a, float b);

	static int Min(int a, int b);

	template<class... ARGS>
	static float Min(int count, ARGS... values);

	static float Max(float a, float b);

	static int Max(int a, int b);

	template<class... ARGS>
	static float Max(int count, ARGS... values);

	static float Pow(float v, float p);

	static float Exp(float v);

	static float Log(float v);

	static float Log10(float v);

	static float Ceil(float v);

	static int CeilToInt(float v);

	static float Floor(float v);

	static int FloorToInt(float v);

	static float Round(float v);

	static int RoundToInt(float v);

	static float Sign(float v);

	static bool Approx(float a, float b, float e = EPSILON);

	static bool IsNearZero(float v, float e = EPSILON);

	static bool IsNaN(float v);

	static bool IsInRange(float v, float min, float max);

	static float FMod(float a, float b);

	static float Clamp(float v, float min, float max);

	static int Clamp(int v, int min, int max);

	static float Clamp01(float v);

	static float Lerp(float a, float b, float t);

	static float LerpAngle(float a, float b, float t);

	static float LerpUnclamped(float a, float b, float t);

	static float Gamma(float v, float absMax, float gamma);

	static float MoveTowards(float current, float target, float maxDelta);

	static float SmoothStep(float from, float to, float t);

	static float Repeat(float v, float length);

	static float PingPong(float v, float length);

	static float DeltaAngle(float current, float target);

};

template <class ... ARGS>
float Maths::Min(const int count, ARGS... values)
{
	float value = POSITIVE_INFINITY;

	for (int i = 0; i < count; ++i)
	{
		value = std::min(value, values[i]);
	}

	return value;
}

template <class ... ARGS>
float Maths::Max(const int count, ARGS... values)
{
	float value = NEGATIVE_INFINITY;

	for (int i = 0; i < count; ++i)
	{
		value = std::max(value, values[i]);
	}

	return value;
}
