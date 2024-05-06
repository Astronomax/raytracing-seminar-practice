#include <parse_input.hpp>

#include <utils.hpp>

#include <algorithm>
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
	TRIANGLE,
	POSITION,
	ROTATION,
	COLOR,
	RAY_DEPTH,
	METALLIC,
	DIELECTRIC,
	IOR,
    	SAMPLES,
    	EMISSION,
	COMMANDS_NUMBER
};

Command
parse_command(std::string &command_name)
{
	if (command_name == "DIMENSIONS")	return Command::DIMENSIONS;
	if (command_name == "BG_COLOR")		return Command::BG_COLOR;
	if (command_name == "CAMERA_POSITION")	return Command::CAMERA_POSITION;
	if (command_name == "CAMERA_RIGHT")	return Command::CAMERA_RIGHT;
	if (command_name == "CAMERA_UP")	return Command::CAMERA_UP;
	if (command_name == "CAMERA_FORWARD")	return Command::CAMERA_FORWARD;
	if (command_name == "CAMERA_FOV_X")	return Command::CAMERA_FOV_X;
	if (command_name == "NEW_PRIMITIVE")	return Command::NEW_PRIMITIVE;
	if (command_name == "PLANE")		return Command::PLANE;
	if (command_name == "ELLIPSOID")	return Command::ELLIPSOID;
	if (command_name == "BOX")		return Command::BOX;
	if (command_name == "TRIANGLE")		return Command::TRIANGLE;
	if (command_name == "POSITION")		return Command::POSITION;
	if (command_name == "ROTATION")		return Command::ROTATION;
	if (command_name == "COLOR") 		return Command::COLOR;
	if (command_name == "RAY_DEPTH")	return Command::RAY_DEPTH;
	if (command_name == "METALLIC")		return Command::METALLIC;
	if (command_name == "DIELECTRIC")	return Command::DIELECTRIC;
	if (command_name == "IOR")		return Command::IOR;
	if (command_name == "SAMPLES")		return Command::SAMPLES;
	if (command_name == "EMISSION")		return Command::EMISSION;
	return Command::UNKNOWN;
}

Scene
parse_input(std::ifstream &s)
{
	Scene scene;
	std::vector<Primitive> primitives;
	std::string command_name;
	while (s >> command_name) {
		auto command = parse_command(command_name);
		switch (command) {
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
			case (Command::CAMERA_FOV_X): {
				float fov_x;
				s >> fov_x;
				scene.camera.tan_fov_x = tanf(fov_x * 0.5f);
				break;
			}
			case (Command::NEW_PRIMITIVE):
				primitives.emplace_back();
				break;
			case (Command::PLANE): {
				primitives.back().type = PrimitiveType::PLANE;
				auto &primitive = primitives.back();
				s >> primitive.primitive_specific[0].x
				  >> primitive.primitive_specific[0].y
				  >> primitive.primitive_specific[0].z;
				break;
			}
			case (Command::ELLIPSOID): {
				primitives.back().type = PrimitiveType::ELLIPSOID;
				auto &primitive = primitives.back();
				s >> primitive.primitive_specific[0].x
				  >> primitive.primitive_specific[0].y
				  >> primitive.primitive_specific[0].z;
				break;
			}
			case (Command::BOX): {
				primitives.back().type = PrimitiveType::BOX;
				auto &primitive = primitives.back();
				s >> primitive.primitive_specific[0].x
				  >> primitive.primitive_specific[0].y
				  >> primitive.primitive_specific[0].z;
				break;
			}
			case (Command::TRIANGLE): {
				primitives.back().type = PrimitiveType::TRIANGLE;
				auto &primitive = primitives.back();
				for (auto & i : primitive.primitive_specific)
					s >> i.x >> i.y >> i.z;
				break;
			}
			case (Command::POSITION): {
				auto &primitive = primitives.back();
				s >> primitive.position.x
				  >> primitive.position.y
				  >> primitive.position.z;
				break;
			}
			case (Command::ROTATION): {
				auto &primitive = primitives.back();
				s >> primitive.rotation.x
				  >> primitive.rotation.y
				  >> primitive.rotation.z
				  >> primitive.rotation.w;
				break;
			}
			case (Command::COLOR): {
				auto &primitive = primitives.back();
				s >> primitive.color.r
				  >> primitive.color.g
				  >> primitive.color.b;
				break;
			}
			case (Command::METALLIC): {
				auto &primitive = primitives.back();
				primitive.material = Material::METALLIC;
				break;
			}
			case (Command::DIELECTRIC): {
				auto &primitive = primitives.back();
				primitive.material = Material::DIELECTRIC;
				break;
			}
			case (Command::IOR): {
				auto &primitive = primitives.back();
				s >> primitive.ior;
				break;
			}
			case (Command::EMISSION): {
				auto &primitive = primitives.back();
				s >> primitive.emission.r
				  >> primitive.emission.g
				  >> primitive.emission.b;
				break;
			}
			case (Command::RAY_DEPTH): {
				s >> scene.ray_depth;
				break;
			}
			case (Command::SAMPLES): {
				s >> scene.samples;
				break;
			}
			default:
				unreachable();
		}
	}
	auto planes_end = std::partition(primitives.begin(), primitives.end(),
		       [](const Primitive &primitive)
	{ return primitive.type == PrimitiveType::PLANE; });
	auto prim_end = std::partition(planes_end, primitives.end(),
					 [](const Primitive &primitive)
					 { return true; /*return primitive.type != PrimitiveType::TRIANGLE || glm::length(primitive.emission) < EPS5;*/ });
	scene.primitives = std::vector<Primitive>(planes_end, prim_end);
	scene.planes = std::vector<Primitive>(primitives.begin(), planes_end);
	scene.camera.tan_fov_y = scene.camera.tan_fov_x * (float)scene.camera.height / (float)scene.camera.width;
	return scene;
}