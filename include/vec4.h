#ifndef SSE_VEC4_H
#define SSE_VEC4_H

#include <xmmintrin.h>
#include <stdio.h>
#include <math.h>

#define ALIGN_16 __attribute__((aligned(16)))
/*
 * Macro to select which components will be in the result of a _mm_shuffle_ps(a, b, SHUFFLE_SELECT(A, B, C, D))
 * the resulting vector will contain: (a[A], a[B], b[C], b[D])
 * Valid values for A, B, C, D are [0, 3]
 */
#define SHUFFLE_SELECT(A, B, C, D) ((A) | ((B) << 2) | ((C) << 4) | ((D) << 6))
/*
 * Retrieve a float at some index from a __m128 vector, valid indices are [0, 3]
 * This is also the only valid way to access elements in C++ since accessing
 * inactive union members is undefined, but in C it's fine
 */
#define VEC_AT(V, I) \
	(_mm_cvtss_f32(_mm_shuffle_ps((V), (V), SHUFFLE_SELECT((I), (I), (I), (I)))))
/*
 * Basic 4 component vector. Individual components should be accessed
 * via VEC_AT(vector.v, index) in C++ and the setter functions should be used
 * in C++. However in C accessing inactive union members is defined behavior
 * so it's ok
 */
union vec4_t {
	__m128 v;
	float f[4];
} ALIGN_16;
typedef union vec4_t vec4_t;
/*
 * Create a new 4 component vector
 */
static inline vec4_t vec4_new(float x, float y, float z, float w){
	vec4_t v;
	/* We need things to be 16-byte aligned, so make sure it is */
	float ALIGN_16 f[4] = { x, y, z, w };
	//Why does this piece of shit crash on return?
	v.v = _mm_load_ps(f);
	return v;
}
/*
 * Set the components in the vector individually
 * This would only be necessary in C++ where accessing inactive
 * union members is UB
 */
static inline void vec4_set_x(vec4_t *v, float x){
	/* b = [x, v.x, x, v.y] */
	__m128 b = _mm_unpacklo_ps(_mm_load_ps1(&x), v->v);
	v->v = _mm_shuffle_ps(b, v->v, SHUFFLE_SELECT(0, 3, 2, 3));
}
static inline void vec4_set_y(vec4_t *v, float y){
	/* b = [y, v.x, y, v.y] */
	__m128 b = _mm_unpacklo_ps(_mm_load_ps1(&y), v->v);
	v->v = _mm_shuffle_ps(b, v->v, SHUFFLE_SELECT(1, 0, 2, 3));
}
static inline void vec4_set_z(vec4_t *v, float z){
	/* b = [z, v.z, z, v.w] */
	__m128 b = _mm_unpackhi_ps(_mm_load_ps1(&z), v->v);
	v->v = _mm_shuffle_ps(v->v, b, SHUFFLE_SELECT(0, 1, 0, 3));
}
static inline void vec4_set_w(vec4_t *v, float w){
	/* b = [w, v.z, w, v.w] */
	__m128 b = _mm_unpackhi_ps(_mm_load_ps1(&w), v->v);
	v->v = _mm_shuffle_ps(v->v, b, SHUFFLE_SELECT(0, 1, 1, 0));
}
/* Arithmetic operations */
static inline vec4_t vec4_add(vec4_t a, vec4_t b){
	vec4_t c;
	c.v = _mm_add_ps(a.v, b.v);
	return c;
}
static inline vec4_t vec4_sub(vec4_t a, vec4_t b){
	a.v = _mm_sub_ps(a.v, b.v);
	return a;
}
static inline vec4_t vec4_mult(vec4_t a, vec4_t b){
	a.v = _mm_mul_ps(a.v, b.v);
	return a;
}
static inline vec4_t vec4_scale(vec4_t a, float s){
	a.v = _mm_mul_ps(a.v, _mm_load_ps1(&s));
	return a;
}
/* Geometric operations */
static inline float vec4_dot(vec4_t a, vec4_t b){
	a.v = _mm_mul_ps(a.v, b.v);
	return a.f[0] + a.f[1] + a.f[2] + a.f[3];
}
static inline float vec4_len(vec4_t a){
	return sqrtf(vec4_dot(a, a));
}
static inline vec4_t vec4_normalize(vec4_t a){
	float l = vec4_len(a);
	return vec4_scale(a, 1.f / l);
}
/*
 * For the cross product the vector4's are treated as regular 3-vectors, and the
 * w component is set to 0
 */
static inline vec4_t vec4_cross(vec4_t a, vec4_t b){
	__m128 lhs = _mm_mul_ps(_mm_shuffle_ps(a.v, a.v, SHUFFLE_SELECT(1, 2, 0, 3)),
		_mm_shuffle_ps(b.v, b.v, SHUFFLE_SELECT(2, 0, 1, 3)));
	__m128 rhs = _mm_mul_ps(_mm_shuffle_ps(a.v, a.v, SHUFFLE_SELECT(2, 0, 1, 3)),
		_mm_shuffle_ps(b.v, b.v, SHUFFLE_SELECT(1, 2, 0, 3)));
	a.v = _mm_sub_ps(lhs, rhs);
	return a;
}
/* Comparisons */
/* Returns 1 if all elements are equal, 0 if not */
static inline int vec4_eq(vec4_t a, vec4_t b){
	a.v = _mm_cmpeq_ps(a.v, b.v);
	return a.f[0] && a.f[1] && a.f[2] && a.f[3];
}
/* Get back a vector for each element, the element is 0 if not equal */
static inline vec4_t vec4_veq(vec4_t a, vec4_t b){
	a.v = _mm_cmpeq_ps(a.v, b.v);
	/* Because Intel uses 0xffffffff (NaN) to indicate equality, let's make those 1's */
	a.v = _mm_and_ps(a.v, _mm_set_ps1(1));
	return a;
}
/* Handy utility for printing vectors */
static inline void vec4_print(vec4_t v){
	printf("[%.2f, %.2f, %.2f, %.2f]\n", v.f[0], v.f[1], v.f[2], v.f[3]);
}

#endif

