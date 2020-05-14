#define _XOPEN_SOURCE 1234

#include <math.h>
#include <stdlib.h>

#include "shared/math/geom.h"
#include "shared/math/perlin.h"

static int permutation[] = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96,
			     53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21,
			     10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219,
			     203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125,
			     136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146,
			     158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55,
			     46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209,
			     76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86,
			     164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5,
			     202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16,
			     58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44,
			     154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253,
			     19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
			     228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51,
			     145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184,
			     84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
			     222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };

void
perlin_noise_shuf(void)
{
	int i, a, b;
	int tmp;

	for (i = 0; i < 256; i++) {
		a = random() % 256;
		b = random() % 256;
		tmp = permutation[a];
		permutation[a] = permutation[b];
		permutation[b] = tmp;
	}
}

static float
noise(int x, int y)
{
	int n;

	n = x + y * 57;
	n = (n << 13) ^ n;
	//n = 1.0 - ( (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff) / 1073741824.0;
	return 1.0 - ((float)permutation[n % 256] / 128.0);
	//return 1.0 - ;
}

static float
interpolate(float a, float b, float x)
{
	float pi_mod;
	float f_unk;

	pi_mod = x * PI;
	f_unk = (1 - cos(pi_mod)) * 0.5;
	return a * (1 - f_unk) + b * x;
}

static float
smooth_noise(int x, int y)
{
	float corners;
	float center;
	float sides;

	corners = (noise(x - 1, y - 1) + noise(x + 1, y - 1) +
		   noise(x - 1, x + 1) + noise(x + 1, y + 1)) / 16;
	sides = (noise(x - 1, y) + noise(x + 1, y) + noise(x, y - 1) +
		 noise(x, y + 1)) / 8;
	center = noise(x, y) / 4;
	return corners + sides + center;
}

static float
noise_handler(float x, float y)
{
	int int_val[2];
	float frac_val[2];
	float value[4];
	float res[2];

	int_val[0] = (int)floor(x);
	int_val[1] = (int)floor(y);
	frac_val[0] = x - int_val[0];
	frac_val[1] = y - int_val[1];
	value[0] = smooth_noise(int_val[0], int_val[1]);
	value[1] = smooth_noise(int_val[0] + 1, int_val[1]);
	value[2] = smooth_noise(int_val[0], int_val[1] + 1);
	value[3] = smooth_noise(int_val[0] + 1, int_val[1] + 1);
	res[0] = interpolate(value[0], value[1], frac_val[0]);
	res[1] = interpolate(value[2], value[3], frac_val[0]);
	return interpolate(res[0], res[1], frac_val[1]);
}

float
perlin_two(float x, float y, float amp, int octs, float freq, float lacu)
{
	int i;
	float total = 0.0f;
	float oamp = amp;

	for (i = 0; i < octs; ++i) {
		total += noise_handler(x * freq, y * freq) * amp;

		freq *= lacu;
		amp *= oamp;
	}

	return total;
}
