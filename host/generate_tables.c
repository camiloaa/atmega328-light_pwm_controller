#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/** GET_ROOT_PTR (pointer2member, memberName, typeName)
 * Given a pointer to an element within an structure,
 * returns a pointer to the structure itself.
 *
 * i.e. Given this (pseudo)code:
 *         typedef struct dummyS {
 *            uint32_t field32bits;
 *            char     *string;
 *            uint32_t other_field;
 *         } dummyT;
 *
 *         dummyT a;
 *         uint32_t *b = &a.other_field;
 *         dummyT *c = GET_ROOT_PTR(b, other_field, dummyT);
 *
 * Variable 'c' contains '&a'
 *
 * */
#define GET_ROOT_PTR(ptr2Member, memberName, typeName)  \
    ((typeName*) ((uint8_t*)ptr2Member - (uint32_t)&((typeName*)0)->memberName))

#define GET_MEMBER_OFFSET(memberName, typeName) \
	((uintptr_t)(&((typeName*)0)->memberName))

#define BETWEEN(x,a,b) ( ((x)>=(a)) && ((x)<(b)) )
#define ROUNDUP(x,i) ((0==((x)%(i)))? (x):((x) + (i) - (x)%(i)))

/* Generate 4 tables for different visual curves
 *      linear = Just one-to-one
 *      linear_log = Linear until log catches
 *      average = Average between linear_log and log until log catches
 *      log = Logarithmic scale
 * Input
 *      top: Maximum value in X-axis
 *      max: Maxumum value in Y-axis
 *      nom,den: Nominator and denominator such 'max = e^(top*nom/den) - 1
 *               Has to be calculated by a calculator because i don't know how to do it myself.
 *      step: Step for the linear part in the linear+log curve
 * */
void genarray(int *array, int top, int max, double nom, double den, double step) {
    int j;
    double x;
    double w = 0;
    for (j = 0; j < top; j++) {
        x = j;
        double e = x * nom / den;
        double y = exp(e) - 1;
        array[j] = j * max / top; // Full linear
        array[j + 3 * top] = round(y); // Full log
        if (y < w) { // Log line hasn't catched 'average' line
            array[j + top] = round(w); // Log after slow linear
            array[j + 2 * top] = round((w + y) / 2); // Log after weighed linear/log
        } else {
            array[j + top] = array[j + 2 * top] = round(y);
        }
        w += step;
    }

}

void printarray(int *array, int top, int base, int split, int k) {
	int j;
	printf("{");
	for (j = 0; j < top; j++) {
		if (0 == j % split)
			printf("\n");
		printf("0x%02x", array[j + top * k]);
		if (j != top - 1)
			printf(",");
	}
	printf("}\n");
	printf("{");
	for (j = 0; j < top; j++) {
		if (0 == j % split)
			printf("\n");
		printf("0x%02x", base - array[j + top * k]);
		if (j != top - 1)
			printf(",");
	}
	printf("}\n");
}

/* Experimented with different values trying to match the curve showed in
 * http://what-when-how.com/display-interfaces/the-human-visual-system-display-interfaces-part-2/
 * A 0.4 step gave best results
 * */
int main() {
    int j, k;
    double x = 0;
    double w = 0;
    int yi = 0;
    double r100_101 = 207818 / 4502981; // 100
    double r255_101 = 130729 / 2357526; // 255
    int base100[101 * 4];
    int base127[128 * 4];
    int base255[256 * 4];

    genarray(base100, 101, 100, 207818, 4502981, 0.5);
    // genarray(base127, 128, 127, 53273, 1394400, 0.5); // Translate 127->127
    genarray(base127, 128, 256, 53273, 1220100, 0.4); // Translate 127->255
    genarray(base255, 256, 256, 44553, 2048810, 0.5); // Translate 255->255
    printarray(base255, 256, 255, 16, 2);
    return 0;
}

