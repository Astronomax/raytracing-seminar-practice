#include "parse_input.hpp"
#include "render.hpp"

#include <fstream>
#include <iostream>

int main(int argc, const char *argv[]) {
	assert(argc >= 2);
	std::ifstream in(argv[1]);
	auto scene = parse_input(in);
	Random rnd;
	scene.init(rnd);
	/*for(int i = 0; i < 10; i++) {
		auto v = scene.distrib->sample(glm::vec3(0.f, 0.f, 0.f), glm::normalize(glm::vec3(0.f, 1.f, 1.f)));
		std::cout << v.x << " " << v.y << " " << v.z << "\n";
	}*/
	//std::cout << scene.distrib->pdf(glm::vec3(0.f, 0.f, 0.f), glm::normalize(glm::vec3(0.f, 1.f, 1.f)), glm::vec3(0.f, 0.f, 1.f)) << "\n";
	auto image = render(rnd, scene);
	std::ofstream out(argv[2]);
	image.save(out);
	in.close();
	out.close();
	return 0;
}