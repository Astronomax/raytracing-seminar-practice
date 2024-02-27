#include "parse_input.hpp"

#include <fstream>

enum command {
	UNKNOWN = -1,
	DIMENSIONS,
	BG_COLOR,
	CAMERA_POSITION,
	CAMERA_RIGHT,
	CAMERA_UP,
	CAMERA_FORWARD,
	CAMERA_FOV_X,
	NEW_PRIMITIVE,
	PLANE,
	ELLIPSOID,
	BOX,
	POSITION,
	ROTATION,
	COLOR,
	COMMANDS_NUMBER
};

command parse_command(std::string &command_name) {
	if (command_name == "DIMENSIONS")		return DIMENSIONS;
	if (command_name == "BG_COLOR")			return BG_COLOR;
	if (command_name == "CAMERA_POSITION")	return CAMERA_POSITION;
	if (command_name == "CAMERA_RIGHT")		return CAMERA_RIGHT;
	if (command_name == "CAMERA_UP")		return CAMERA_UP;
	if (command_name == "CAMERA_FORWARD")	return CAMERA_FORWARD;
	if (command_name == "CAMERA_FOV_X")		return CAMERA_FOV_X;
	if (command_name == "NEW_PRIMITIVE")	return NEW_PRIMITIVE;
	if (command_name == "PLANE")			return PLANE;
	if (command_name == "ELLIPSOID")		return ELLIPSOID;
	if (command_name == "BOX")				return BOX;
	if (command_name == "POSITION")			return POSITION;
	if (command_name == "ROTATION")			return ROTATION;
	if (command_name == "COLOR") 			return COLOR;
	return UNKNOWN;
}

Scene parse_input(std::ifstream &s) {
	Scene scene;
	std::string command_name;
	while (s >> command_name) {
		auto command = parse_command(command_name);
		switch (command) {
			case (NEW_PRIMITIVE):
			case (UNKNOWN):
				break;
			case (DIMENSIONS):
				s >> scene.camera.width
				  >> scene.camera.height;
				break;
			case (BG_COLOR):
				s >> scene.bg_color.x
				  >> scene.bg_color.y
				  >> scene.bg_color.z;
				break;
			case (CAMERA_POSITION):
				s >> scene.camera.position.x
				  >> scene.camera.position.y
				  >> scene.camera.position.z;
				break;
			case (CAMERA_RIGHT):
				s >> scene.camera.right.x
				  >> scene.camera.right.y
				  >> scene.camera.right.z;
				break;
			case (CAMERA_UP):
				s >> scene.camera.up.x
				  >> scene.camera.up.y
				  >> scene.camera.up.z;
				break;
			case (CAMERA_FORWARD):
				s >> scene.camera.forward.x
				  >> scene.camera.forward.y
				  >> scene.camera.forward.z;
				break;
			case (CAMERA_FOV_X):
				s >> scene.camera.fov_x;
				break;
			case (PLANE): {
				// here we depend strictly on the placement of parents in memory
				scene.primitives.emplace_back(new Plane());
				auto plane = dynamic_cast<Plane*>(scene.primitives.back().get());
				s >> plane->normal.x
				  >> plane->normal.y
				  >> plane->normal.z;
				break;
			}
			case (ELLIPSOID): {
				// here we depend strictly on the placement of parents in memory
				scene.primitives.emplace_back(new Ellipsoid());
				auto ellipsoid = dynamic_cast<Ellipsoid*>(scene.primitives.back().get());
				s >> ellipsoid->radius.x
				  >> ellipsoid->radius.y
				  >> ellipsoid->radius.z;
				break;
			}
			case (BOX): {
				// here we depend strictly on the placement of parents in memory
				scene.primitives.emplace_back(new Box());
				auto box = dynamic_cast<Box*>(scene.primitives.back().get());
				s >> box->diagonal.x
				  >> box->diagonal.y
				  >> box->diagonal.z;
				break;
			}
			case (POSITION): {
				auto primitive = scene.primitives.back().get();
				s >> primitive->position.x
				  >> primitive->position.y
				  >> primitive->position.z;
				break;
			}
			case (ROTATION): {
				auto primitive = scene.primitives.back().get();
				s >> primitive->rotation.x
				  >> primitive->rotation.y
				  >> primitive->rotation.z
				  >> primitive->rotation.w;
				break;
			}
			case (COLOR): {
				auto primitive = scene.primitives.back().get();
				s >> primitive->color.r
				  >> primitive->color.g
				  >> primitive->color.b;
				break;
			}
			default:
				//unreachable;
				break;
		}
	}
	scene.camera.fov_y = atanf(tanf(scene.camera.fov_x * 0.5f) \
			* (float)scene.camera.height / (float)scene.camera.width) * 2.f;
	return scene;
}