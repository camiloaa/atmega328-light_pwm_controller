#include <stdio.h>
#include <math.h>

/* Generate 4 tables for different visual curves
 *      linear = Just one-to-one
 *      linear_log = Linear until log catches
 *      average = Average between linear_log and log until log catches
 *      log = Logarithmic scale
 * */
void genarray(int *array, int top, int max, double nom, double den, double step) {
	int j;
	double x;
	double w = 0;
	for (j=0; j<top; j++) {
		x = j;
		double e = x*nom/den;
		double y = exp(e) - 1;
		array[j] = j*max/top; // Full linear
		array[j+3*top] = round(y); // Full log
		if (y < w) { // Log line hasn't catched 'average' line
			array[j+top] = round(w); // Log after slow linear
			array[j+2*top] = round((w+y)/2); // Log after weighed linear/log
		} else {
			array[j+top] = array[j+2*top] = round(y);
		}
		w += step;
	}

}

void printarray(int *array, int top, int split) {
	int j, k;
	printf("{");
	for (k=0; k<4; k++) {
		printf("{");
		for (j=0; j<top; j++) {
			if (0 == j%split) printf("\n");
			printf("%3d", array[j+top*k]);
			if (j != top-1) printf(",");
		}
		printf("}\n");
	}
	printf("}\n");
}

int main() {
	int j,k;
	double x=0;
	double w = 0;
	int    yi = 0;
	double r100_101 = 207818/4502981; // 100
	double r255_101 = 130729/2357526; // 255
	int base100[101*4];
	int base127[128*4];

	genarray(base100, 101, 100, 207818, 4502981, 0.5);
	genarray(base127, 128, 127, 53273, 1394400, 0.5); // Translate 127->127
	genarray(base127, 128, 256, 53273, 1220100, 0.4); // Translate 127->255
	printarray(base127, 128, 16);
	return 0;
}

