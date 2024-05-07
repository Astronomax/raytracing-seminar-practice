#include "render.hpp"

#include <fstream>

int main(int argc, const char *argv[]) {
	assert(argc >= 5);

	auto scene = load_scene(argv[1]);
	scene.camera.width = strtol(argv[2], nullptr, 10);
	scene.camera.height = strtol(argv[3], nullptr, 10);
	scene.samples = strtol(argv[4], nullptr, 10);
	scene.camera.tan_fov_y = tanf(scene.camera.fov_y * 0.5f);
	scene.camera.tan_fov_x = scene.camera.tan_fov_y * (float)scene.camera.width / (float)scene.camera.height;
	scene.ray_depth = 6;

	auto image = render(scene);
	std::ofstream out(argv[5]);
	image.save(out);
	out.close();
	return 0;
}