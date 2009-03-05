extern void f(int);
main() {
     int i;
     switch (i) {
     case 1: f(1); break;
     case 100: f(100); break;
     default:
	  f(-1);
     }
     f(0);
     switch (i) {
     case 1: f(1); break;
     case 2: f(2); break;
     case 3: f(3); break;
     case 4: f(4); break;
     case 5: f(5); break;
     case 100: f(100); break;
     default:
	  f(-1);
     }
}
