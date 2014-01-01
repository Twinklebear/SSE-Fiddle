#include <stdio.h>
#include "vec4.h"
#include "mat4.h"

void basic_test(void);
void proj_tests(void);

int main(void){
	basic_test();
	proj_tests();

	return 0;
}
void basic_test(void){
	float ALIGN_16 a_rows[16] = {
		1, 5, 0, 0,
		2, 1, 3, 5,
		6, 9, 0, 2,
		5, 3, 8, 9
	};
	float ALIGN_16 b_rows[16] = {
		4, 0, 2, 0,
		1, 2, 7, 1,
		0, 0, 2, 0,
		1, 2, 0, 1
	};
	mat4_t a = mat4_from_rows(a_rows);
	mat4_t b = mat4_from_rows(b_rows);
	printf("Multiplying a:\n");
	mat4_print(a);
	printf("With b:\n");
	mat4_print(b);

	printf("Multiplication result:\n");
	a = mat4_mult(a, b);
	mat4_print(a);

	vec4_t v = vec4_new(1, 2, 3, 1);
	a = mat4_translate(v);
	printf("translation matrix for [1, 2, 3]:\n");
	mat4_print(a);
	printf("Translated vector:\n");
	v = mat4_vec_mult(a, v);
	vec4_print(v);

	a = mat4_rotate(90, vec4_new(1, 0, 0, 0));
	printf("Rotation matrix:\n");
	mat4_print(a);
	v = mat4_vec_mult(a, vec4_new(0, 1, 0, 0));
	printf("+Y vec rotated 90 deg about +X:\n");
	vec4_print(v);
}
void proj_tests(void){
	mat4_t m = mat4_look_at(vec4_new(0, 0, 5, 0),
		vec4_new(0, 0, 0, 0), vec4_new(0, 1, 0, 0));
	printf("Look at matrix:\n");
	mat4_print(m);

	m = mat4_ortho(-1, 1, -1, 1, 1, 100);
	printf("ortho matrix:\n");
	mat4_print(m);

	m = mat4_perspective(90, 1, 1, 100);
	printf("perspective:\n");
	mat4_print(m);
}

