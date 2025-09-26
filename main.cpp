#include <chrono>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unordered_map>
#include <unordered_set>

#include "lib/tinyxml2.h"
#include "src/image.h"
#include "src/imagemosaic_wfc.h"
#include "src/multi_array.h"
#include "src/overlapping_wfc.h"
#include "src/simpletiled_wfc.h"
#include "src/symmetry.h"

#define _DEBUG 1

using namespace std;
using namespace tinyxml2;
using namespace chrono;
namespace fs = filesystem;
using hrc = high_resolution_clock;

const char* StringAttribute(XMLElement* elem, const char* name, const char* default_value) {
	const char* value = elem->Attribute(name);
	if (value == nullptr)
		return default_value;
	return value;
}

unordered_set<string> ReadSubsetNames(XMLElement* root_elem, const string& subset) {
	unordered_set<string> subset_names;
	XMLElement* subsets_elem = root_elem->FirstChildElement("subsets");
	if (subsets_elem == nullptr)
		return subset_names;
	XMLElement* subset_elem = subsets_elem->FirstChildElement("subset");
	while (subset_elem != nullptr && subset_elem->Attribute("name") != subset)
		subset_elem = subset_elem->NextSiblingElement("subset");
	if (subset_elem == nullptr)
		return subset_names;
	XMLElement* tile_elem = subset_elem->FirstChildElement("tile");
	while (tile_elem != nullptr) {
		subset_names.insert(tile_elem->Attribute("name"));
		tile_elem = tile_elem->NextSiblingElement("tile");
	}
	return subset_names;
}

unordered_map<string, Tile> ReadTiles(XMLElement* root_elem, const string& directory, const string& subset) {
	unordered_map<string, Tile> tiles;
	unordered_set<string> subset_names = ReadSubsetNames(root_elem, subset);

	bool unique = root_elem->BoolAttribute("unique", false);
	XMLElement* tiles_elem = root_elem->FirstChildElement("tiles");
	XMLElement* tile_elem = tiles_elem->FirstChildElement("tile");
	while (tile_elem != nullptr) {
		string name = tile_elem->Attribute("name");
		if (subset_names.size() > 0 && subset_names.find(name) == subset_names.end()) {
			tile_elem = tile_elem->NextSiblingElement("tile");
			continue;
		}

		double weight = tile_elem->DoubleAttribute("weight", 1.0);
		Symmetry symmetry(StringAttribute(tile_elem, "symmetry", "X")[0]);

		if (unique) {
			vector<Image> images;
			for (uint32_t i = 0; i < symmetry.orientations(); i++) {
				string image_path = directory + "/" + name + " " + to_string(i) + ".png";
				Image image = LoadImage(image_path);
				images.push_back(image);
			}
			Tile tile(images, symmetry, weight);
			tiles.insert({name, tile});
		} else {
			string image_path = directory + "/" + name + ".png";
			Image image = LoadImage(image_path);
			Tile tile(image, symmetry, weight);
			tiles.insert({name, tile});
		}
		tile_elem = tile_elem->NextSiblingElement("tile");
	}
	return tiles;
}

unordered_map<string, ImageWeight> ReadImageWeight(XMLElement* root_elem, const string& directory, const string& subset) {
	unordered_map<string, ImageWeight> tiles;
	unordered_set<string> subset_names = ReadSubsetNames(root_elem, subset);

	XMLElement* tiles_elem = root_elem->FirstChildElement("tiles");
	XMLElement* tile_elem = tiles_elem->FirstChildElement("tile");
	while (tile_elem != nullptr) {
		string name = tile_elem->Attribute("name");
		if (subset_names.size() > 0 && subset_names.find(name) == subset_names.end()) {
			tile_elem = tile_elem->NextSiblingElement("tile");
			continue;
		}

		double weight = tile_elem->DoubleAttribute("weight", 1.0);
		string image_path = directory + "/" + name + ".png";
		Image image = LoadImage(image_path);
		ImageWeight tile(image, weight);
		tiles.insert({name, tile});
		tile_elem = tile_elem->NextSiblingElement("tile");
	}
	return tiles;
}

struct Neighbor {
	string left_tile;
	uint32_t left_orientation;
	string right_tile;
	uint32_t right_orientation;
};

vector<Neighbor> ReadNeighbors(XMLElement* root_elem) {
	vector<Neighbor> neighbors;
	XMLElement* neighbors_elem = root_elem->FirstChildElement("neighbors");
	XMLElement* neighbor_elem = neighbors_elem->FirstChildElement("neighbor");
	while (neighbor_elem != nullptr) {
		Neighbor neighbor = {"", 0, "", 0};
		string left = neighbor_elem->Attribute("left");
		size_t delim = left.find(' ');
		neighbor.left_tile = left.substr(0, delim);
		if (delim != string::npos)
			neighbor.left_orientation = stoi(left.substr(delim + 1));

		string right = neighbor_elem->Attribute("right");
		delim = right.find(' ');
		neighbor.right_tile = right.substr(0, delim);
		if (delim != string::npos)
			neighbor.right_orientation = stoi(right.substr(delim + 1));

		neighbors.push_back(neighbor);
		neighbor_elem = neighbor_elem->NextSiblingElement("neighbor");
	}
	return neighbors;
}

void ReadSimpletiled(XMLElement* elem) {
	string name = elem->Attribute("name");
	string subset = StringAttribute(elem, "subset", "tiles");
	uint32_t size = elem->UnsignedAttribute("size", 24);
	uint32_t width = elem->UnsignedAttribute("width", size);
	uint32_t height = elem->UnsignedAttribute("height", size);
	bool periodic_output = elem->BoolAttribute("periodic", false);
	uint32_t screenshots = elem->UnsignedAttribute("screenshots", 2);

	printf("< %s\n", name.c_str());
	string config_file = "tilesets/" + name + ".xml";
	XMLDocument rules_document;
	if (rules_document.LoadFile(config_file.c_str()) != XML_SUCCESS)
		throw runtime_error(config_file + " not found");

	uint32_t index = 0;
	vector<Tile> tiles;
	unordered_map<string, uint32_t> tile_indices;
	XMLElement* root_elem = rules_document.FirstChildElement("set");
	unordered_map<string, Tile> tiles_map = ReadTiles(root_elem, "tilesets/" + name, subset);
	for (unordered_map<string, Tile>::const_iterator it = tiles_map.begin(); it != tiles_map.end(); it++) {
		tile_indices.insert({it->first, index});
		tiles.push_back(it->second);
		index++;
	}

	vector<NeighborIndex> neighbors_indices;
	vector<Neighbor> neighbors = ReadNeighbors(root_elem);
	for (vector<Neighbor>::const_iterator it = neighbors.begin(); it != neighbors.end(); it++) {
		if (tile_indices.find(it->left_tile) == tile_indices.end())
			continue;
		if (tile_indices.find(it->right_tile) == tile_indices.end())
			continue;
		NeighborIndex neighbor_index;
		neighbor_index.left_index = tile_indices[it->left_tile];
		neighbor_index.left_orientation = it->left_orientation;
		neighbor_index.right_index = tile_indices[it->right_tile];
		neighbor_index.right_orientation = it->right_orientation;
		neighbors_indices.push_back(neighbor_index);
	}

	WFCOptions options;
	options.periodic_output = periodic_output;
	SimpletiledWFC wfc(vec2(height, width), tiles, neighbors_indices, options);
	for (uint32_t i = 0; i < screenshots; i++) {
#ifdef _DEBUG
		srand(i);
#endif
		bool failed = true;
		for (uint32_t k = 0; k < 10; k++) {
			int seed = rand();
			optional<Image> success = wfc.execute(seed);
			if (success.has_value()) {
				SaveImagePNG("output/" + name + "_" + to_string(seed) + ".png", success.value());
				printf("> %d DONE\n", i);
				failed = false;
				break;
			}
			printf("> %d CONTRADICTION %d\n", i, k);
		}
		if (failed)
			printf("> %d FAILED\n", i);
	}
}

void ReadOverlapping(XMLElement* elem) {
	string name = elem->Attribute("name");
	uint32_t size = elem->UnsignedAttribute("size", 48);
	uint32_t height = elem->UnsignedAttribute("height", size);
	uint32_t width = elem->UnsignedAttribute("width", size);
	uint32_t screenshots = elem->UnsignedAttribute("screenshots", 2);

	OverlappingWFCOptions options;
	options.ground = elem->BoolAttribute("ground", false);
	options.periodic_input = elem->BoolAttribute("periodicInput", true);
	options.periodic_output = elem->BoolAttribute("periodic", false);
	options.out_size = vec2(height, width);
	options.symmetry = elem->UnsignedAttribute("symmetry", 8);
	options.pattern_size = elem->UnsignedAttribute("N", 3);

	printf("< %s\n", name.c_str());
	string image_path = "samples/" + name + ".png";
	Image image = LoadImage(image_path);
	OverlappingWFC wfc(image, options);
	for (uint32_t i = 0; i < screenshots; i++) {
#ifdef _DEBUG
		srand(i);
#endif
		bool failed = true;
		for (uint32_t k = 0; k < 10; k++) {
			int seed = rand();
			optional<Image> success = wfc.execute(seed);
			if (success.has_value()) {
				SaveImagePNG("output/" + name + "_" + to_string(seed) + ".png", success.value());
				printf("> %d DONE\n", i);
				failed = false;
				break;
			}
			printf("> %d CONTRADICTION %d\n", i, k);
		}
		if (failed)
			printf("> %d FAILED\n", i);
	}
}

void ReadImagemosaic(XMLElement* elem) {
	string name = elem->Attribute("name");
	string subset = StringAttribute(elem, "subset", "tiles");
	uint32_t size = elem->UnsignedAttribute("size", 24);
	uint32_t width = elem->UnsignedAttribute("width", size);
	uint32_t height = elem->UnsignedAttribute("height", size);
	bool periodic_output = elem->BoolAttribute("periodic", false);
	uint32_t screenshots = elem->UnsignedAttribute("screenshots", 2);

	printf("< %s\n", name.c_str());
	string config_file = "resources/" + name + ".xml";
	XMLDocument rules_document;
	if (rules_document.LoadFile(config_file.c_str()) != XML_SUCCESS)
		throw runtime_error(config_file + " not found");

	uint32_t index = 0;
	vector<ImageWeight> tiles;
	unordered_map<string, uint32_t> tile_indices;
	XMLElement* root_elem = rules_document.FirstChildElement("set");
	unordered_map<string, ImageWeight> tiles_map = ReadImageWeight(root_elem, "resources/" + name, subset);
	for (unordered_map<string, ImageWeight>::const_iterator it = tiles_map.begin(); it != tiles_map.end(); it++) {
		tile_indices.insert({it->first, index});
		printf("[INFO] %d: %s\n", index, it->first.c_str());
		tiles.push_back(it->second);
		index++;
	}

	Array3D<uint8_t> neighbors(Propagator::DIRECTIONS, tiles_map.size(), tiles_map.size());
	neighbors.fill(false);
	XMLElement* neighbors_elem = root_elem->FirstChildElement("neighbors");
	XMLElement* tile_elem = neighbors_elem->FirstChildElement("tile");
	while (tile_elem != nullptr) {
		string first_tile_name = tile_elem->Attribute("name");
		if (tile_indices.find(first_tile_name) == tile_indices.end())
			throw runtime_error("Unknown tile name: " + first_tile_name);
		uint32_t first_tile_index = tile_indices[first_tile_name];
		XMLElement* neighbor_elem = tile_elem->FirstChildElement("neighbor");
		while (neighbor_elem != nullptr) {
			string second_tile_name = neighbor_elem->Attribute("name");
			if (tile_indices.find(second_tile_name) == tile_indices.end())
				throw runtime_error("Unknown neighbor name: " + second_tile_name);
			uint32_t second_tile_index = tile_indices[second_tile_name];
			neighbors(0, first_tile_index, second_tile_index) = neighbor_elem->BoolAttribute("up");
			neighbors(1, first_tile_index, second_tile_index) = neighbor_elem->BoolAttribute("left");
			neighbors(2, first_tile_index, second_tile_index) = neighbor_elem->BoolAttribute("right");
			neighbors(3, first_tile_index, second_tile_index) = neighbor_elem->BoolAttribute("down");
			neighbor_elem = neighbor_elem->NextSiblingElement("neighbor");
		}
		tile_elem = tile_elem->NextSiblingElement("tile");
	}

	WFCOptions options;
	options.periodic_output = periodic_output;
	ImagemosaicWFC wfc(vec2(height, width), tiles, neighbors, options);
	for (uint32_t i = 0; i < screenshots; i++) {
		bool failed = true;
		for (uint32_t k = 0; k < 10; k++) {
			int seed = rand();
			optional<Image> success = wfc.execute(seed);
			if (success.has_value()) {
				SaveImagePNG("output/" + name + "_" + to_string(seed) + ".png", success.value());
				printf("> %d DONE\n", i);
				failed = false;
				break;
			}
			printf("> %d CONTRADICTION %d\n", i, k);
		}
		if (failed)
			printf("> %d FAILED\n", i);
	}
}

void ReadConfigFile(const string& config_path) {
	XMLDocument document;
	if (document.LoadFile(config_path.c_str()) != XML_SUCCESS)
		throw runtime_error(config_path + " not found");
	XMLElement* root_elem = document.FirstChildElement("samples");
	XMLElement* elem = root_elem->FirstChildElement("simpletiled");
	while (elem != nullptr) {
		ReadSimpletiled(elem);
		elem = elem->NextSiblingElement("simpletiled");
	}
	elem = root_elem->FirstChildElement("overlapping");
	while (elem != nullptr) {
		ReadOverlapping(elem);
		elem = elem->NextSiblingElement("overlapping");
	}
	elem = root_elem->FirstChildElement("imagemosaic");
	while (elem != nullptr) {
		ReadImagemosaic(elem);
		elem = elem->NextSiblingElement("imagemosaic");
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s <sampling file>\n", argv[0]);
		return 1;
	}
	srand(time(NULL));
	fs::create_directories("output");
	hrc::time_point start = hrc::now();
	ReadConfigFile(argv[1]);
	hrc::time_point end = hrc::now();
	double elapsed = duration_cast<milliseconds>(end - start).count();
	printf("time = %d.%03ds\n", (uint32_t)elapsed / 1000, (uint32_t)elapsed % 1000);
	return 0;
}
