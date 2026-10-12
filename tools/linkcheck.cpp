// Built once per shared library with every object of that library linked in
// (CMakeLists.txt): it links only if the library and the ones below it have
// everything it calls, so no library reaches into a higher one unseen.
int main() { return 0; }
