/* Simple program to specialize power to compute x^3.  */
int main(void) {
	/* NOTE: we must cast the function pointer to specify
	   that the function expects a floating point parameter.
	   Otherwise C will, by default, treat the value as an
	   integer. */
	float (*pow3)(float) = (float (*)(float))specialize_power(3);
	float (*pow10)(float) = (float (*)(float))specialize_power(10);

	printf("9.0 ^ 3 = %f, 2.0 ^ 10 = %f\n", pow3(9.0), pow10(2.0));
	return 0;
}
