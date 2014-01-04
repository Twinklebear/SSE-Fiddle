#ifndef SSE_MAT4_H
#define SSE_MAT4_H

#include <xmmintrin.h>
#include <math.h>
#include <stdio.h>
#include "vec4.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

/* 4x4 matrix stored in column major order
 */
struct mat4_t {
	vec4_t col[4];
} ALIGN_16;
typedef struct mat4_t mat4_t;
/* Create a new mat4, the matrix will be the identity matrix
 */
static inline mat4_t mat4_new(void){
	mat4_t m;
	for (int i = 0; i < 4; ++i){
		m.col[i].v = _mm_setzero_ps();
		m.col[i].f[i] = 1;
	}
	return m;
}
static inline mat4_t mat4_transpose(mat4_t m){
	__m128 tmp[4];
	tmp[0] = _mm_unpacklo_ps(m.col[0].v, m.col[1].v);
	tmp[1] = _mm_unpackhi_ps(m.col[0].v, m.col[1].v);
	tmp[2] = _mm_unpacklo_ps(m.col[2].v, m.col[3].v);
	tmp[3] = _mm_unpackhi_ps(m.col[2].v, m.col[3].v);
	m.col[0].v = _mm_movelh_ps(tmp[0], tmp[2]);
	m.col[1].v = _mm_movehl_ps(tmp[2], tmp[0]);
	m.col[2].v = _mm_movelh_ps(tmp[1], tmp[3]);
	m.col[3].v = _mm_movehl_ps(tmp[3], tmp[1]);
	return m;
}
/* Create a mat4 interpreting the values in the array passed as cols
 * r must contain at least 16 floats and should be 16-byte aligned
 */
static inline mat4_t mat4_from_cols(const float *c){
	mat4_t m;
	for (int i = 0; i < 4; ++i){
		m.col[i].v = _mm_load_ps(c + 4 * i);
	}
	return m;
}
/* Create a mat4 interpreting the values in the array passed as rows
 * r must contain at least 16 floats and should be 16-byte aligned
 */
static inline mat4_t mat4_from_rows(const float *r){
	/* We simply load as cols then transpose to */
	return mat4_transpose(mat4_from_cols(r));
}
/* Arithmetic operations */
static inline mat4_t mat4_add(mat4_t a, mat4_t b){
	mat4_t c;
	for (int i = 0; i < 4; ++i){
		c.col[i] = vec4_add(a.col[i], b.col[i]);
	}
	return c;
}
static inline mat4_t mat4_sub(mat4_t a, mat4_t b){
	mat4_t c;
	for (int i = 0; i < 4; ++i){
		c.col[i] = vec4_sub(a.col[i], b.col[i]);
	}
	return c;
}
static inline mat4_t mat4_mult(mat4_t a, mat4_t b){
	mat4_t c;
	/* b/c we store column major transpose a to move the rows into the columns */
	a = mat4_transpose(a);
	/* Can we do better? */
	for (int i = 0; i < 4; ++i){
		for (int j = 0; j < 4; ++j){
			c.col[i].f[j] = vec4_dot(a.col[j], b.col[i]);
		}
	}
	return c;
}
static inline vec4_t mat4_vec_mult(mat4_t a, vec4_t b){
	vec4_t c;
	a = mat4_transpose(a);
	for (int i = 0; i < 4; ++i){
		c.f[i] = vec4_dot(a.col[i], b);
	}
	return c;
}
/* Create a translation matrix to move by the vector */
static inline mat4_t mat4_translate(vec4_t v){
	mat4_t m = mat4_new();
	m.col[3].v = _mm_shuffle_ps(v.v, v.v, SHUFFLE_SELECT(0, 1, 2, 3));
	m.col[3].f[3] = 1;
	return m;
}
/* Create a scaling matrix to scale the x,y,z coords by x,y,z */
static inline mat4_t mat4_scale(float x, float y, float z){
	mat4_t m = mat4_new();
	m.col[0].f[0] = x;
	m.col[1].f[1] = y;
	m.col[2].f[2] = z;
	return m;
}
/* Create the rotation matrix to rotate by d degrees about the vector v
 * (v.w should be 0)
 */
static inline mat4_t mat4_rotate(float d, vec4_t v){
	d = d * M_PI / 180.0;
	float c = cosf(d);
	float s = sinf(d);
	v = vec4_normalize(v);
	float ALIGN_16 cols[16] = { c + powf(v.f[0], 2.0) * (1 - c),
		v.f[1] * v.f[0] * (1 - c) + v.f[2] * s, v.f[2] * v.f[0] * (1 - c) - v.f[1] * s, 0,
		v.f[0] * v.f[1] * (1 - c) - v.f[2] * s, c + powf(v.f[1], 2.0) * (1 - c),
		v.f[2] * v.f[1] * (1 - c) + v.f[0] * s, 0,
		v.f[0] * v.f[2] * (1 - c) + v.f[1] * s, v.f[1] * v.f[2] * (1 - c) - v.f[0] * s,
		c + powf(v.f[2], 2.0) * (1 - c), 0,
		0, 0, 0, 1
	};
	return mat4_from_cols(cols);
}
/* Create the look at matrix with the camera at eye, looking at center and with
 * up as the camera's up vector. The w coord for each vector should be 0
 */
static inline mat4_t mat4_look_at(vec4_t eye, vec4_t center, vec4_t up){
	vec4_t f = vec4_sub(center, eye);
	f = vec4_normalize(f);
	up = vec4_normalize(up);
	vec4_t s = vec4_cross(f, up);
	vec4_t u = vec4_cross(vec4_normalize(s), f);
	mat4_t m = mat4_new();
	m.col[0] = s;
	m.col[1] = u;
	m.col[2] = vec4_scale(f, -1);
	m = mat4_transpose(m);
	return mat4_mult(m, mat4_translate(vec4_scale(eye, -1)));
}
/* Calculate the orthographic projection matrix */
static inline mat4_t mat4_ortho(float l, float r, float b, float t, float n, float f){
	mat4_t m = mat4_scale(2 / (r - l), 2 / (t - b), -2 / (f - n));
	m.col[3] = vec4_new(-(r + l) / (r - l), -(t + b) / (t - b),
		-(f + n) / (f - n), 1);
	return m;
}
/* Calculate the perspective matrix */
static inline mat4_t mat4_perspective(float fovY, float aspect, float n, float f){
	float p = 1.f / tan(fovY * 0.5 * M_PI / 180.0);
	mat4_t m = mat4_scale(p / aspect, p, (f + n) / (n - f));
	m.col[2].f[3] = -1;
	m.col[3].f[2] = 2 * f * n / (n - f);
	m.col[3].f[3] = 0;
	return m;
}
/* See if the two matrices are equal. Mostly for testing really */
static inline int mat4_eq(mat4_t a, mat4_t b){
	return vec4_eq(a.col[0], b.col[0]) && vec4_eq(a.col[1], b.col[1])
		&& vec4_eq(a.col[2], b.col[2]) && vec4_eq(a.col[3], b.col[3]);
}
/* Print out a matrix row by row */
static inline void mat4_print(mat4_t m){
	/* Transpose so it's easier to print row-by-row */
	m = mat4_transpose(m);
	for (int i = 0; i < 4; ++i){
		vec4_print(m.col[i]);
	}
}

#endif

