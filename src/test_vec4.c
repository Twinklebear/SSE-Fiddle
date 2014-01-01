#include <stdio.h>
#include <time.h>
#include "vec4.h"

/* Make sure the math is correctly implemented for SSE */
void basic_test(void);

int main(void){
	basic_test();

	return 0;
}
void basic_test(void){
	vec4_t a = vec4_new(1, 2, 3, 4);
	vec4_t b = vec4_new(4, 3, 2, 1);
	vec4_print(a);
	vec4_print(b);

	printf("a dot b = %.2f\n", vec4_dot(a, b));

	a.f[0] = 1;
	a.f[1] = 0;
	a.f[2] = 0;
	a.f[3] = 0;
	b.f[0] = 0;
	b.f[1] = 1;
	b.f[2] = 0;
	b.f[3] = 0;
	printf("a=");
	vec4_print(a);
	printf("b=");
	vec4_print(b);

	a = vec4_cross(a, b);
	if (!vec4_eq(a, vec4_new(0, 0, 1, 0))){
		printf("Cross product is wrong\n");
	}
	printf("a = a X b=");
	vec4_print(a);

	a = vec4_add(a, b);
	if (!vec4_eq(a, vec4_new(0, 1, 1, 0))){
		printf("Addition is wrong\n");
	}
	printf("a = a + b=");
	vec4_print(a);

	a = vec4_scale(a, 2);
	if (!vec4_eq(a, vec4_new(0, 2, 2, 0))){
		printf("Scalar mult is wrong\n");
	}
	printf("a = a * 2=");
	vec4_print(a);

	a = vec4_mult(a, b);
	if (!vec4_eq(a, vec4_new(0, 2, 0, 0))){
		printf("Vector mult is wrong\n");
	}
	printf("a = a * b=");
	vec4_print(a);

	a = vec4_sub(a, b);
	if (!vec4_eq(a, vec4_new(0, 1, 0, 0))){
		printf("Subtraction is wrong\n");
	}
	printf("a = a - b=");
	vec4_print(a);

	a.f[0] = 10;
	a = vec4_veq(a, b);
	vec4_print(a);

	a = vec4_new(5, 0, 0, 0);
	if (vec4_len(a) != 5.f){
		printf("Lenght is wrong\n");
	}

	a = vec4_normalize(a);
	if (!vec4_eq(a, vec4_new(1, 0, 0, 0))){
		printf("Normalize is wrong\n");
	}
}

