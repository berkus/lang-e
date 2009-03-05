int main(void) {
        v_uptr stream1, stream2;

	/* Create two streams */
	stream1 = mk_stream();
	stream2 = mk_stream();

	/* Demonstrate that they are independent */
	printf("stream 1 = [%d, %d ..]\n", stream1(), stream1());
	printf("stream 2 = [%d, %d ..]\n", stream2(), stream2());
	return 0;
}
