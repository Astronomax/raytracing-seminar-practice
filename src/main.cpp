#include "parse_input.hpp"
#include "render.hpp"

#include <fstream>

int main(int argc, const char *argv[]) {
	assert(argc >= 2);
	std::ifstream in(argv[1]);
	auto scene = parse_input(in);
	auto rnd = std::make_shared<Random>();
	scene.init(rnd);
	auto image = render(rnd, scene);
	std::ofstream out(argv[2]);
	image.save(out);
	in.close();
	out.close();
	return 0;
}