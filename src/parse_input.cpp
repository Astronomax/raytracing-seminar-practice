#include "parse_input.hpp"

#include <fstream>

enum class Command {
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
	RAY_DEPTH,
	AMBIENT_LIGHT,
	NEW_LIGHT,
	LIGHT_INTENSITY,
	LIGHT_DIRECTION,
	LIGHT_POSITION,
	LIGHT_ATTENUATION,
	METALLIC,
	DIELECTRIC,
	IOR,
	COMMANDS_NUMBER
};

Command parse_command(std::string &command_name) {
	if (command_name == "DIMENSIONS")		return Command::DIMENSIONS;
	if (command_name == "BG_COLOR")			return Command::BG_COLOR;
	if (command_name == "CAMERA_POSITION")	return Command::CAMERA_POSITION;
	if (command_name == "CAMERA_RIGHT")		return Command::CAMERA_RIGHT;
	if (command_name == "CAMERA_UP")		return Command::CAMERA_UP;
	if (command_name == "CAMERA_FORWARD")	return Command::CAMERA_FORWARD;
	if (command_name == "CAMERA_FOV_X")		return Command::CAMERA_FOV_X;
	if (command_name == "NEW_PRIMITIVE")	return Command::NEW_PRIMITIVE;
	if (command_name == "PLANE")			return Command::PLANE;
	if (command_name == "ELLIPSOID")		return Command::ELLIPSOID;
	if (command_name == "BOX")				return Command::BOX;
	if (command_name == "POSITION")			return Command::POSITION;
	if (command_name == "ROTATION")			return Command::ROTATION;
	if (command_name == "COLOR") 			return Command::COLOR;
	if (command_name == "RAY_DEPTH")		return Command::RAY_DEPTH;
	if (command_name == "AMBIENT_LIGHT")	return Command::AMBIENT_LIGHT;
	if (command_name == "NEW_LIGHT")		return Command::NEW_LIGHT;
	if (command_name == "LIGHT_INTENSITY")	return Command::LIGHT_INTENSITY;
	if (command_name == "LIGHT_DIRECTION")	return Command::LIGHT_DIRECTION;
	if (command_name == "LIGHT_POSITION")	return Command::LIGHT_POSITION;
	if (command_name == "LIGHT_ATTENUATION")return Command::LIGHT_ATTENUATION;
	if (command_name == "METALLIC")			return Command::METALLIC;
	if (command_name == "DIELECTRIC")		return Command::DIELECTRIC;
	if (command_name == "IOR")				return Command::IOR;
	return Command::UNKNOWN;
}

Scene parse_input(std::ifstream &s) {
	Scene scene;
	std::string command_name;
	while (s >> command_name) {
		auto command = parse_command(command_name);
		switch (command) {
			case (Command::NEW_PRIMITIVE):
			case (Command::UNKNOWN):
				break;
			case (Command::DIMENSIONS):
				s >> scene.camera.width
				  >> scene.camera.height;
				break;
			case (Command::BG_COLOR):
				s >> scene.bg_color.x
				  >> scene.bg_color.y
				  >> scene.bg_color.z;
				break;
			case (Command::CAMERA_POSITION):
				s >> scene.camera.position.x
				  >> scene.camera.position.y
				  >> scene.camera.position.z;
				break;
			case (Command::CAMERA_RIGHT):
				s >> scene.camera.right.x
				  >> scene.camera.right.y
				  >> scene.camera.right.z;
				break;
			case (Command::CAMERA_UP):
				s >> scene.camera.up.x
				  >> scene.camera.up.y
				  >> scene.camera.up.z;
				break;
			case (Command::CAMERA_FORWARD):
				s >> scene.camera.forward.x
				  >> scene.camera.forward.y
				  >> scene.camera.forward.z;
				break;
			case (Command::CAMERA_FOV_X):
				s >> scene.camera.fov_x;
				break;
			case (Command::PLANE): {
				// here we depend strictly on the placement of parents in memory
				scene.primitives.emplace_back(new Plane());
				auto plane = dynamic_cast<Plane*>(scene.primitives.back().get());
				s >> plane->normal.x
				  >> plane->normal.y
				  >> plane->normal.z;
				break;
			}
			case (Command::ELLIPSOID): {
				// here we depend strictly on the placement of parents in memory
				scene.primitives.emplace_back(new Ellipsoid());
				auto ellipsoid = dynamic_cast<Ellipsoid*>(scene.primitives.back().get());
				s >> ellipsoid->radius.x
				  >> ellipsoid->radius.y
				  >> ellipsoid->radius.z;
				break;
			}
			case (Command::BOX): {
				// here we depend strictly on the placement of parents in memory
				scene.primitives.emplace_back(new Box());
				auto box = dynamic_cast<Box*>(scene.primitives.back().get());
				s >> box->diagonal.x
				  >> box->diagonal.y
				  >> box->diagonal.z;
				break;
			}
			case (Command::POSITION): {
				auto primitive = scene.primitives.back().get();
				s >> primitive->position.x
				  >> primitive->position.y
				  >> primitive->position.z;
				break;
			}
			case (Command::ROTATION): {
				auto primitive = scene.primitives.back().get();
				s >> primitive->rotation.x
				  >> primitive->rotation.y
				  >> primitive->rotation.z
				  >> primitive->rotation.w;
				break;
			}
			case (Command::COLOR): {
				auto primitive = scene.primitives.back().get();
				s >> primitive->color.r
				  >> primitive->color.g
				  >> primitive->color.b;
				break;
			}
			case (Command::METALLIC): {
				auto primitive = scene.primitives.back().get();
				primitive->material = Material::METALLIC;
				break;
			}
			case (Command::DIELECTRIC): {
				auto primitive = scene.primitives.back().get();
				primitive->material = Material::DIELECTRIC;
				break;
			}
			case (Command::IOR): {
				auto primitive = scene.primitives.back().get();
				s >> primitive->ior;
				break;
			}
			case (Command::RAY_DEPTH): {
				s >> scene.ray_depth;
				break;
			}
			case (Command::AMBIENT_LIGHT): {
				s >> scene.ambient.r
				  >> scene.ambient.g
				  >> scene.ambient.b;
				break;
			}
			case (Command::NEW_LIGHT): {
				// here we depend strictly on the placement of parents in memory
				scene.lights.emplace_back(new Light());
				break;
			}
			case (Command::LIGHT_INTENSITY): {
				auto light = scene.lights.back().get();
				s >> light->intensity.r
				  >> light->intensity.g
				  >> light->intensity.b;
				break;
			}
			case (Command::LIGHT_DIRECTION): {
				auto light = scene.lights.back().get();
				glm::vec3 direction;
				s >> direction.x
				  >> direction.y
				  >> direction.z;
				light->direction = direction;
				break;
			}
			case (Command::LIGHT_POSITION): {
				auto light = scene.lights.back().get();
				s >> light->position.x
				  >> light->position.y
				  >> light->position.z;
				break;
			}
			case (Command::LIGHT_ATTENUATION): {
				auto light = scene.lights.back().get();
				s >> light->attenuation.x
				  >> light->attenuation.y
				  >> light->attenuation.z;
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