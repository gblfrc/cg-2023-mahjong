#include "Tile.hpp"
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <glm/gtx/string_cast.hpp>

using json = nlohmann::json;
using namespace std;

class MahjongGame {

public:
	vector<Tile> tiles;
	bool isInitialized;

	MahjongGame(string path) {
		isInitialized = true;
		// open Input File Stream (ifstream) associated to structure json
		ifstream f(path);
		// parse structure as json
		json data = json::parse(f);
		// close stream, not needed anymore after parsing
		f.close();
		// convert json information into list of tiles
		// import array of suit indices
		vector<int> suits;
		for (auto& el : data["suits"].items())
		{
			suits.push_back(el.value());
		}
		// randomize array of indices
		// based on https://stackoverflow.com/a/6926473
		auto rd = random_device{};
		auto rng = default_random_engine{ rd() };
		shuffle(suits.begin(), suits.end(), rng);
		// work with array of tiles
		for (auto& el : data["tiles"].items()) {
			json tile = el.value();
			int tileIdx = tile["tileIdx"];
			vector<int> under;
			vector<int> over;
			vector<int> left;
			vector<int> right;
			// import bottom vector
			for (auto& uel : tile["under"].items()) {
				under.push_back(uel.value());
			}
			// import top vector
			for (auto& oel : tile["over"].items()) {
				over.push_back(oel.value());
			}
			// import left vector
			for (auto& lel : tile["left"].items()) {
				left.push_back(lel.value());
			}
			// import right vector
			for (auto& rel : tile["right"].items()) {
				right.push_back(rel.value());
			}
			// import position vector
			vector<double> coords;
			for (auto& coord : tile["position"].items()) {
				coords.push_back(coord.value());
			}
			glm::vec3 position = glm::vec3(coords[0], coords[1], coords[2]);
			tiles.push_back(Tile(tileIdx, suits[tileIdx], over, under, left, right, position));
		}
	}

	void removeTiles(int idx0, int idx1) {
		// access tiles
		Tile tile0 = tiles[idx0];
		Tile tile1 = tiles[idx1];
		// check on suit idx
		if (((idx0 < 40 && idx1 == idx0) ||
			(idx0 >= 40 && idx0 < 44 && idx1 >= 40 && idx1 < 44) ||
			(idx0 >= 44 && idx0 < 48 && idx1 >= 44 && idx1 < 48)) && tile0.isOpen() && tile1.isOpen()) {
			// for each tile, remove its index from neighbours
			for (Tile tile : {tile0, tile1}) {
				// remove current tile index from neighbors
				for (int leftIdx : tile.left) {
					Tile leftTile = tiles[leftIdx];
					leftTile.right.erase(remove(leftTile.right.begin(), leftTile.right.end(), tile.tileIdx), leftTile.right.end());
				}
				for (int rightIdx : tile.right) {
					Tile rightTile = tiles[rightIdx];
					rightTile.right.erase(remove(rightTile.left.begin(), rightTile.left.end(), tile.tileIdx), rightTile.left.end());
				}
				for (int underIdx : tile.under) {
					Tile underTile = tiles[underIdx];
					underTile.over.erase(remove(underTile.over.begin(), underTile.over.end(), tile.tileIdx), underTile.over.end());
				}
				// remove all neightbors from current tile
				tile.left.clear();
				tile.right.clear();
				tile.under.clear();
			}
		}
	}

};