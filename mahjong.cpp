// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include <glm/ext/vector_common.hpp>
#include <glm/ext/scalar_common.hpp>
#include "MahjongGame.hpp"
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <string>

//link windows multimedia library to our program
#pragma comment(lib, "winmm.lib")

// correct alignas(...) values for uniform buffer objects data structures:
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)

struct CommonUniformBlock {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
	alignas(4) float transparency;
};

struct TileUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
	alignas(4) int tileIdx;
	alignas(4) int suitIdx;
	alignas(4) float transparency;
	alignas(4) int hoverIdx;
	alignas(4) int selectedIdx;
	alignas(4) int textureIdx;
};

struct RoughSurfaceUniformBlock {
	alignas(4) float amb;
	alignas(4) float sigma;
};

struct SmoothSurfaceUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 sColor;
};

struct GlobalUniformBlock {
	alignas(4) float beta;					// decay factor of the point light
	alignas(4) float g;						// distance parameter for point light
	alignas(16) glm::vec3 PlightPos;		// point light position
	alignas(16) glm::vec3 PlightColor;		// original color of the point light
	alignas(16) glm::vec3 AmbLightColor;	// ambient color
	alignas(16) glm::vec3 eyePos;			// viewer position
};

struct UIUniformBlock {
	alignas(4) float visible;
	alignas(4) float transparency;
	alignas(4) int objectIdx;
};

// The vertices data structures
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexUI {
	glm::vec3 pos;
	glm::vec2 UV;
};

// MAIN ! 
class Mahjong : public BaseProject {
protected:

	// Current aspect ratio (used by the callback that resized the window)
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo;		// DSL for GlobalUniformBufferObject
	DescriptorSetLayout DSLTile;		// DSL for Tile objects
	DescriptorSetLayout DSLPlain;		// DSL with 1 UNIFORM and 1 TEXTURE
	DescriptorSetLayout DSLGeneric;		// DSL with 2 UNIFORM and 1 TEXTURE
	DescriptorSetLayout DSLTextureOnly;	// DSL with only 1 TEXTURE

	// Vertex formats
	VertexDescriptor VMesh;
	VertexDescriptor VUI;

	// Pipelines [Shader couples]
	Pipeline PPlain;
	Pipeline PTile;
	Pipeline PRoughSurfaces;
	Pipeline PUI;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	Model<VertexMesh> MBackground;
	Model<VertexMesh> MTile;
	Model<VertexMesh> MWall;
	Model<VertexMesh> MFloor;
	Model<VertexMesh> MCeiling;
	Model<VertexMesh> MTable;
	Model<VertexMesh> MHome;
	Model<VertexMesh> MWindow;
	Model<VertexMesh> MGameTitle;
	Model<VertexMesh> MLandscape;
	Model<VertexUI> MGameOver;
	Model<VertexUI> MYouWin;
	Model<VertexMesh> MPlainRectangle;
	Model<VertexMesh> MArrowButton;

	DescriptorSet DSGubo;
	DescriptorSet DSBackground;
	DescriptorSet DSTile[144];
	DescriptorSet DSTileTexture;
	DescriptorSet DSWall;
	DescriptorSet DSFloor;
	DescriptorSet DSCeiling;
	DescriptorSet DSTable;
	DescriptorSet DSWindow1, DSWindow2, DSWindow3;
	DescriptorSet DSHTile;
	DescriptorSet DSHome;
	DescriptorSet DSGameTitle;
	DescriptorSet DSLandscape;
	// Descriptor sets for UI elements
	DescriptorSet DSGameOver;
	DescriptorSet DSYouWin;
	DescriptorSet DSButton1, DSButton2, DSButton3;
	DescriptorSet DSArrowButton1_left, DSArrowButton2_left, DSArrowButton3_left;
	DescriptorSet DSArrowButton1_right, DSArrowButton2_right, DSArrowButton3_right;
	DescriptorSet DSPlayButton;
	DescriptorSet DSSelection1, DSSelection2, DSSelection3;
	DescriptorSet DSTileText;

	Texture TPoolCloth;
	Texture TTile;
	Texture TWallDragon;
	Texture TFloor;
	Texture TCeiling;
	Texture TTable;
	Texture TWindow;
	Texture TGameTitle;
	Texture TLandscape;
	Texture TGameOver;
	Texture TYouWin;
	//Buttons
	Texture TButton;
	Texture TArrowButtonLeft, TArrowButtonRight;
	Texture TPlayButton;
	Texture TSelection1;
	Texture TSelection2;
	Texture TSelection3;
	Texture TTileSelText;

	// C++ storage for uniform variables
	TileUniformBlock tileubo[144];	//not necessary as an array, works also with only one TileUniformBlock??
	RoughSurfaceUniformBlock bgubo;
	GlobalUniformBlock gubo;
	RoughSurfaceUniformBlock wallubo;
	RoughSurfaceUniformBlock floorubo;
	RoughSurfaceUniformBlock ceilingubo;
	RoughSurfaceUniformBlock tableubo;
	RoughSurfaceUniformBlock window1ubo, window2ubo, window3ubo;
	UIUniformBlock gameoverubo;
	UIUniformBlock youwinubo;
	TileUniformBlock tileHomeubo; //rotating tile in home menu screen
	CommonUniformBlock tileSelTextubo;

	// Geometry blocks for objects different from tiles
	// [0] - Background
	// [1] - Room walls
	// [2] - Room ceiling
	// [3] - Room floor
	// [4] - Table
	// [5-7] - Windows
	// [8] - Landscape
	// [9] - Home screen background
	// [10] - Game title
	// [11] - Button1
	// [12] - Button2
	// [13] - Button3
	// [14] - Arrow Left 1
	// [15] - Arrow Left 2
	// [16] - Arrow Left 3
	// [17] - Arrow Right 1
	// [18] - Arrow Right 2
	// [19] - Arrow Right 3
	// [20] - Play button
	// [21] - Game setting
	// [22] - Tile type selection title
	// [23] - Board design selection title
	CommonUniformBlock commonubo[24];

	// Other application parameters
	int tileTextureIdx = 0;
	// Camera parameters
	const float FOVy = glm::radians(90.0f);
	const float nearPlane = 0.01f;
	const float farPlane = 10.0f;
	const float rotSpeed = glm::radians(90.0f);
	const float movSpeed = 1.0f;
	float CamH, CamRadius, CamPitch, CamYaw;
	const float initialCamRadius = 0.3f;
	const float initialPitch = glm::radians(60.0f);
	//const float initialPitch = glm::radians(90.0f);
	const float initialYaw = glm::radians(0.0f);
	
	//other parameters
	int gameState = 0;
	float DisappearingTileTransparency = 1.0f;
	const float homeTileRotSpeed = glm::radians(100.0f);
	int firstTileIndex = -1;
	int secondTileIndex = -1;
	const glm::mat4 removedTileWorld = glm::translate(glm::mat4(1.0), glm::vec3(10.0f, -20.0f, 0.0f)) * 
								glm::scale(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
	bool disappearedTiles[144] = {0};
	const glm::vec3 homeMenuPosition = glm::vec3(30.0f, 0.0f, 0.0f);
	const glm::mat4 homeMenuWorld = glm::translate(glm::mat4(1.0f), homeMenuPosition);


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, title and initial background
		windowWidth = 1200;
		windowHeight = 900;
		windowTitle = "Mahjong";
		windowResizable = GLFW_FALSE;
		initialBackgroundColor = { 0.0f, 0.005f, 0.01f, 1.0f };

		// Descriptor pool sizes
		uniformBlocksInPool = 300; //177;
		texturesInPool = 40;//19;
		setsInPool = 300;// 170;

		// Initialize aspect ratio
		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
		windowWidth = w;
		windowHeight = h;
	}

	// Here you load and setup all your Vulkan Models and Textures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Descriptor Set Layouts
		DSLTile.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}			// tile block
			});
		DSLPlain.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},			// common block
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}	// texture
			});
		DSLGeneric.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},			// common block
					{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},			// shading block
					{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}	// texture
			});
		DSLTextureOnly.init(this, {
					{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	// texture
			});
		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}			// gubo block
			});
		// Vertex descriptors
		VMesh.init(this, {
			{0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV),
					   sizeof(glm::vec2), UV}
			});
		VUI.init(this, {
			{0, sizeof(VertexUI), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexUI, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexUI, UV),
					   sizeof(glm::vec2), UV}
			});


		// Pipelines 
		// PPlain --> Pipeline for elements that have to be 'copied' from textures
		PPlain.init(this, &VMesh, "shaders/PlainVert.spv", "shaders/PlainFrag.spv", { &DSLPlain });
		PPlain.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, true); 
		// PUI --> Pipeline for UI elements
		PUI.init(this, &VUI, "shaders/UIVert.spv", "shaders/UIFrag.spv", { &DSLPlain });
		PUI.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, true);
		// PTile --> Pipeline for objects representing Mahjong tiles
		PTile.init(this, &VMesh, "shaders/TileVert.spv", "shaders/TileFrag.spv", { &DSLGubo, &DSLTile, &DSLTextureOnly });
		PTile.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, true);
		// Other pipelines
		PRoughSurfaces.init(this, &VMesh, "shaders/PhongVert.spv", "shaders/OrenNayarFrag.spv", { &DSLGeneric, &DSLGubo });
		PRoughSurfaces.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, true);

		//----------------------------
		// Models, textures and Descriptors (values assigned to the uniforms)
		

		//----------------------------
		// MODELS CREATION
		
		//home menu base coordinates
		float a = 2.0f;
		float b = 3.0f;
		MHome.vertices = {
			{{-a, b, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
			{ {a, b, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f} },
			{ {a, 0.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f} },
			{ {-a, 0.0f, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 1.0f} },
		};
		MHome.indices = { 0, 2, 1,    0, 3, 2 };
		MHome.initMesh(this, &VMesh);
		//Game Title base coordinates
		float half_width_for_vert = 1.33f; 
		float hUp = 2.5f;
		float hDown = 1.5f;
		MGameTitle.vertices = { 
			{{-half_width_for_vert, hUp, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }}, 
			{ {half_width_for_vert, hUp, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f} }, 
			{ {half_width_for_vert, hDown, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f} }, 
			{ {-half_width_for_vert, hDown, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 1.0f} }, 
		};
		MGameTitle.indices = { 0, 2, 1,    0, 3, 2 };
		MGameTitle.initMesh(this, &VMesh);

		// Button base coordinates
		half_width_for_vert = 1.0f; 
		hUp = 1.0f;
		hDown = 0.0f;
		MPlainRectangle.vertices = { 
			{{-half_width_for_vert, hUp, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }}, 
			{ {half_width_for_vert, hUp, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f} }, 
			{ {half_width_for_vert, hDown, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f} },
			{ {-half_width_for_vert, hDown, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 1.0f} },
		};
		MPlainRectangle.indices = { 0, 2, 1,    0, 3, 2 };
		MPlainRectangle.initMesh(this, &VMesh);

		//Arrow base coordinates
		float squareSide = 1.0f;
		MArrowButton.vertices = {
			{{-squareSide/2, squareSide, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
			{ {squareSide/2, squareSide, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f} },
			{ {squareSide/2, 0.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f} },
			{ {-squareSide/2, 0.0f, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 1.0f} },
		};
		MArrowButton.indices = { 0, 2, 1,    0, 3, 2 };
		MArrowButton.initMesh(this, &VMesh);

		// Background
		float side = 0.25f;
		float baseHeight = 0.6f;
		MBackground.vertices = {
			{{-side, baseHeight, -side},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f}},
			{{side, baseHeight, -side},{0.0f, 1.0f, 0.0f},{1.0f, 0.0f}},
			{{side, baseHeight, side},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f}},
			{{-side, baseHeight, side},{0.0f, 1.0f, 0.0f},{0.0f, 1.0f}},
		};
		MBackground.indices = { 0, 2, 1,   0,3,2};
		MBackground.initMesh(this, &VMesh);
		// Landscape (requires scale and translation in place)
		MLandscape.vertices = {
			{{-1.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 0.0f}},
			{{1.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f}},
			{{1.0f, -1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f}},
			{{-1.0f, -1.0f, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 1.0f}},
		};
		MLandscape.indices = { 0, 2, 1,   0,3,2 };
		MLandscape.initMesh(this, &VMesh);
		// Create walls
		float roomHeight = 3.0f;
		float roomHalfWidth = 2.0f;
		vector<VertexMesh> wallVertices;
		vector<unsigned int> wallIndices;
		int vertexIndex = 0;
		// walls are defined separately because uvs have to change order for each wall
		// left wall
		float x = -roomHalfWidth;
		for (float z : {roomHalfWidth, -roomHalfWidth}) {
			for (float y : {roomHeight, 0.0f}) {
				wallVertices.push_back({ {x,y,z}, {1.0f,0.0f,0.0f}, {vertexIndex / 2, vertexIndex % 2} });
				vertexIndex++;
			}
		}
		// right wall
		x = roomHalfWidth;
		vertexIndex = 0;
		for (float z : {-roomHalfWidth, roomHalfWidth}) {
			for (float y : {roomHeight, 0.0f}) {
				wallVertices.push_back({ {x,y,z}, {-1.0f,0.0f,0.0f}, {vertexIndex / 2, vertexIndex % 2} });
				vertexIndex++;
			}
		}
		// front wall
		float z = -roomHalfWidth;
		vertexIndex = 0;
		for (float x : {-roomHalfWidth, roomHalfWidth}) {
			for (float y : {roomHeight, 0.0f}) {
				wallVertices.push_back({ {x,y,z}, {0.0f,0.0f,1.0f}, {vertexIndex / 2, vertexIndex % 2} });
				vertexIndex++;
			}
		}
		// back wall
		z = roomHalfWidth;
		vertexIndex = 0;
		for (float x : {roomHalfWidth, -roomHalfWidth}) {
			for (float y : {roomHeight, 0.0f}) {
				wallVertices.push_back({ {x,y,z}, {0.0f,0.0f,-1.0f}, {vertexIndex / 2, vertexIndex % 2} });
				vertexIndex++;
			}
		}
		MWall.vertices = wallVertices;
		for (int i = 0; i < wallVertices.size(); i+=4) {
			wallIndices.push_back(i + 0); wallIndices.push_back(i + 1); wallIndices.push_back(i + 2);
			wallIndices.push_back(i + 1); wallIndices.push_back(i + 3); wallIndices.push_back(i + 2);
		}
		MWall.indices = wallIndices;
		MWall.initMesh(this, &VMesh);
		// Create floor
		vector<VertexMesh> floorVertices;
		vertexIndex = 0;
		for (float z : {-roomHalfWidth, roomHalfWidth}) {
			for (float x : {-roomHalfWidth, roomHalfWidth}) {
				floorVertices.push_back({ {x, 0.0f, z}, {0.0f,1.0f,0.0f}, {vertexIndex / 2, vertexIndex % 2} });
				vertexIndex++;
			}
		};
		MFloor.vertices = floorVertices;
		MFloor.indices = { 0, 2, 1,    1, 2, 3 };
		MFloor.initMesh(this, &VMesh);
		// Create ceiling
		vector<VertexMesh> ceilingVertices;
		vertexIndex = 0;
		for (float z : {-roomHalfWidth, roomHalfWidth}) {
			for (float x : {-roomHalfWidth, roomHalfWidth}) {
				ceilingVertices.push_back({ {x, roomHeight, z}, {0.0f,-1.0f,0.0f}, {vertexIndex / 2, vertexIndex % 2} });
				vertexIndex++;
			}
		};
		MCeiling.vertices = ceilingVertices;
		MCeiling.indices = { 0, 1, 2,    2, 1, 3 };
		MCeiling.initMesh(this, &VMesh);
		// UI object models
		// Game over message
		float gameOverHalfX = 0.5f;
		float gameOverHalfY = 0.45f;
		MGameOver.vertices = {
			{{-gameOverHalfX, -gameOverHalfY, 0.1f},{0.0f, 0.0f}},
			{{gameOverHalfX, -gameOverHalfY, 0.1f},{1.0f, 0.0f}},
			{{-gameOverHalfX, gameOverHalfY, 0.1f},{0.0f, 1.0f}},
			{{gameOverHalfX, gameOverHalfY, 0.1f},{1.0f, 1.0f}},
		};
		MGameOver.indices = { 0, 1, 2,    1, 2, 3 };
		MGameOver.initMesh(this, &VUI);
		// You win message
		MYouWin.vertices = MGameOver.vertices;
		MYouWin.indices = MGameOver.indices;
		MYouWin.initMesh(this, &VUI);
		// Import Tile model
		MTile.init(this, &VMesh, "models/Tile.obj", OBJ);
		MTable.init(this, &VMesh, "models/Table.obj", OBJ);
		MWindow.init(this, &VMesh, "models/Window.obj", OBJ);

		//----------------------------
		// Create the TEXTURES
		
		// The second parameter is the file name
		TPoolCloth.init(this, "textures/background/poolcloth.png");
		const char* tileTextureFiles[4] = {
			"textures/tiles/tiles_white_resized.png",
			"textures/tiles/tiles_dark_resized.png",
			"textures/tiles/tiles_lucky_resized.png",
			"textures/tiles/tiles_botanical_resized.png",
		};
		TTile.initFour(this, tileTextureFiles);

		//Tiles style selcetion names
		const char* tileNamesTextureFiles[4] = {
			"textures/buttons/white_tiles_text.png",
			"textures/buttons/black_tiles_text.png",
			"textures/buttons/lucky_tiles_text.png",
			"textures/buttons/botanical_tiles_text.png",
		};
		TTileSelText.initFour(this, tileNamesTextureFiles);

		// Initialize other textures
		TWallDragon.init(this, "textures/room/dragon_texture0.jpg");
		TFloor.init(this, "textures/room/floor.png");
		TCeiling.init(this, "textures/room/ceiling.jpg");
		TTable.init(this, "textures/room/table.jpg");
		TWindow.init(this, "textures/room/window.png");
		TGameTitle.init(this, "textures/title_brush.png");
		TLandscape.init(this, "textures/room/landscape.jpg");
		TGameOver.init(this, "textures/ui/gameover.png");
		TYouWin.init(this, "textures/ui/youwin.png");
		TButton.init(this, "textures/buttons/button_rounded_edges.png");
		TArrowButtonLeft.init(this, "textures/buttons/arrow_button_left.png");
		TArrowButtonRight.init(this, "textures/buttons/arrow_button_right.png");
		TPlayButton.init(this, "textures/buttons/button_with_plant.png");
		TSelection1.init(this, "textures/buttons/settings.png");
		TSelection2.init(this, "textures/buttons/tileDesign.png");
		TSelection3.init(this, "textures/buttons/boardStyle.png");
		
		//-------------------------------
		// Init local variables
		CamH = 1.0f;
		CamRadius = initialCamRadius;
		CamPitch = initialPitch;
		CamYaw = initialYaw;
		gameState = -1;				//INITIAL GAME STATE <-----
		firstTileIndex = -1;
		secondTileIndex = -1;
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PRoughSurfaces.create();
		PTile.create();
		PPlain.create();
		PUI.create();

		// Descriptor Sets

		// UI
		DSGameOver.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(UIUniformBlock), nullptr},
				{1, TEXTURE, 0, &TGameOver}
			});
		DSYouWin.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(UIUniformBlock), nullptr},
				{1, TEXTURE, 0, &TYouWin}
			});
		
		// Plain
		DSLandscape.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TLandscape}
			});
		DSHome.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TPoolCloth}
			});
		DSGameTitle.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TGameTitle}
			});
		DSButton1.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TButton}
			});
		DSButton2.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TButton}
			});
		DSButton3.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TButton}
			});

		DSArrowButton1_left.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TArrowButtonLeft}
			});
		DSArrowButton2_left.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TArrowButtonLeft}
			});
		DSArrowButton3_left.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TArrowButtonLeft}
			});
		DSArrowButton1_right.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TArrowButtonRight}
			});
		DSArrowButton2_right.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TArrowButtonRight}
			});
		DSArrowButton3_right.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TArrowButtonRight}
			});
		DSPlayButton.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TPlayButton}
			});
		DSSelection1.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TSelection1}
			});
		DSSelection2.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TSelection2}
			});
		DSSelection3.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TSelection3}
			});
		DSTileText.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TTileSelText}						//ARRAY OF TEXTURES?
			});
		// Tile
		for (int i = 0; i < 144; i++) {
			DSTile[i].init(this, &DSLTile, {
						{0, UNIFORM, sizeof(TileUniformBlock), nullptr}
				});
		}
		DSHTile.init(this, &DSLTile, {
				{0, UNIFORM, sizeof(TileUniformBlock), nullptr},
			});

		// Texture-only
		DSTileTexture.init(this, &DSLTextureOnly, {
					{0, TEXTURE, 0, &TTile}
			});

		// Gubo
		DSGubo.init(this, &DSLGubo, {
			{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
			});

		// Generic
		DSBackground.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TPoolCloth}
			});

		DSWall.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TWallDragon}
			});

		DSFloor.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TFloor}
			});

		DSCeiling.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TCeiling}
			});

		DSTable.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TTable}
			});

		DSWindow1.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TWindow}
			});
		DSWindow2.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TWindow}
			});
		DSWindow3.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TWindow}
			});

	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PRoughSurfaces.cleanup();
		PTile.cleanup();
		PPlain.cleanup();
		PUI.cleanup();

		// Cleanup descriptor sets
		DSGubo.cleanup();
		DSBackground.cleanup();
		for (int i = 0; i < 144; i++) {
			DSTile[i].cleanup();
		}
		DSWall.cleanup();
		DSFloor.cleanup();
		DSCeiling.cleanup();
		DSTable.cleanup();
		DSWindow1.cleanup();
		DSWindow2.cleanup();
		DSWindow3.cleanup();
		DSTileTexture.cleanup();
		DSLandscape.cleanup();

		//menu
		DSHTile.cleanup();
		DSHome.cleanup();
		DSGameTitle.cleanup();
		DSButton1.cleanup();
		DSButton2.cleanup();
		DSButton3.cleanup();
		DSArrowButton1_left.cleanup();
		DSArrowButton2_left.cleanup();
		DSArrowButton3_left.cleanup();
		DSArrowButton1_right.cleanup();
		DSArrowButton2_right.cleanup();
		DSArrowButton3_right.cleanup();
		DSPlayButton.cleanup();
		DSSelection1.cleanup();
		DSSelection2.cleanup();
		DSSelection3.cleanup();

		DSGameOver.cleanup();
		DSYouWin.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		TPoolCloth.cleanup();
		TTile.cleanup();
		TWallDragon.cleanup();
		TFloor.cleanup();
		TCeiling.cleanup();
		TTable.cleanup();
		TWindow.cleanup();
		TGameTitle.cleanup();
		TLandscape.cleanup();
		TGameOver.cleanup();
		TYouWin.cleanup();
		TButton.cleanup();
		TArrowButtonLeft.cleanup();
		TArrowButtonRight.cleanup();
		TPlayButton.cleanup();
		TSelection1.cleanup();
		TSelection2.cleanup();
		TSelection3.cleanup();

		// Cleanup models
		MBackground.cleanup();
		MTile.cleanup();
		MWall.cleanup();
		MFloor.cleanup();
		MCeiling.cleanup();
		MTable.cleanup();
		MHome.cleanup();
		MWindow.cleanup();
		MGameTitle.cleanup();
		MLandscape.cleanup();
		MGameOver.cleanup();
		MYouWin.cleanup();
		MPlainRectangle.cleanup();
		MArrowButton.cleanup();

		// Cleanup descriptor set layouts
		DSLTile.cleanup();
		DSLGeneric.cleanup();
		DSLGubo.cleanup();
		DSLTextureOnly.cleanup();
		DSLPlain.cleanup();

		// Destroys the pipelines
		PTile.destroy();
		PRoughSurfaces.destroy();
		PPlain.destroy();
		PUI.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

		// PPlain
		//
		PPlain.bind(commandBuffer);
		// Landscape (out of windows)
		MLandscape.bind(commandBuffer);
		DSLandscape.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MLandscape.indices.size()), 1, 0, 0, 0);
		// Home screen background
		MHome.bind(commandBuffer);
		DSHome.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MHome.indices.size()), 1, 0, 0, 0);
		// Game title
		MGameTitle.bind(commandBuffer);
		DSGameTitle.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MGameTitle.indices.size()), 1, 0, 0, 0);
		//Buttons
		MPlainRectangle.bind(commandBuffer);
		DSButton1.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSButton2.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSButton3.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSSelection1.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSSelection2.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSSelection3.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSPlayButton.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		//Arrow buttons
		MArrowButton.bind(commandBuffer);
		DSArrowButton1_left.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MArrowButton.indices.size()), 1, 0, 0, 0);
		DSArrowButton2_left.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MArrowButton.indices.size()), 1, 0, 0, 0);
		DSArrowButton3_left.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MArrowButton.indices.size()), 1, 0, 0, 0);
		DSArrowButton1_right.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MArrowButton.indices.size()), 1, 0, 0, 0);
		DSArrowButton2_right.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MArrowButton.indices.size()), 1, 0, 0, 0);
		DSArrowButton3_right.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MArrowButton.indices.size()), 1, 0, 0, 0);

		// PTile
		//
		// Tiles in main structure
		PTile.bind(commandBuffer);
		MTile.bind(commandBuffer);
		DSGubo.bind(commandBuffer, PTile, 0, currentImage);
		DSTileTexture.bind(commandBuffer, PTile, 2, currentImage);
		for (int i = 0; i < 144; i++) {
			DSTile[i].bind(commandBuffer, PTile, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MTile.indices.size()), 1, 0, 0, 0);
		}
		// Tile in home screen
		DSHTile.bind(commandBuffer, PTile, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MTile.indices.size()), 1, 0, 0, 0);


		// PRoughSurfaces
		//
		PRoughSurfaces.bind(commandBuffer);
		DSGubo.bind(commandBuffer, PRoughSurfaces, 1, currentImage);
		// Background for game
		MBackground.bind(commandBuffer);
		DSBackground.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MBackground.indices.size()), 1, 0, 0, 0);
		// Room walls
		MWall.bind(commandBuffer);
		DSWall.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MWall.indices.size()), 1, 0, 0, 0);
		// Room floor
		MFloor.bind(commandBuffer);
		DSFloor.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MFloor.indices.size()), 1, 0, 0, 0);
		// Room ceiling
		MCeiling.bind(commandBuffer);
		DSCeiling.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCeiling.indices.size()), 1, 0, 0, 0);
		// Table
		MTable.bind(commandBuffer);
		DSTable.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MTable.indices.size()), 1, 0, 0, 0);
		// Windows
		MWindow.bind(commandBuffer);
		DSWindow1.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MWindow.indices.size()), 1, 0, 0, 0);
		DSWindow2.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MWindow.indices.size()), 1, 0, 0, 0);
		DSWindow3.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MWindow.indices.size()), 1, 0, 0, 0);

		// PUI
		PUI.bind(commandBuffer);
		// Game over
		MGameOver.bind(commandBuffer);
		DSGameOver.bind(commandBuffer, PUI, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MGameOver.indices.size()), 1, 0, 0, 0);
		// You win!
		MYouWin.bind(commandBuffer);
		DSYouWin.bind(commandBuffer, PUI, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MYouWin.indices.size()), 1, 0, 0, 0);
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {

		//---------------------
		//GETTING COMMANDS IMPUTS
		//---------------------

		// Standard procedure to quit when the ESC key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		// Integration with the timers and the controllers
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		bool click = false;
		bool enter = false;
		getSixAxis(deltaT, m, r, fire, click, enter); 
		//std::cout << "\nclicked is :" << clicked<<"\n";
		//std::cout << "\nEnter: " << enter << " \n";
		
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		// To debounce the pressing of the fire button, and start the event when the key is released
		static bool wasFire = false;
		bool handleFire = (wasFire && (!fire));
		wasFire = fire;

		// To debounce the pressing of the mouse left click, and start the event when the key is released
		static bool wasClick = false; 
		bool handleClick = (wasClick && (!click)); 
		wasClick = click; 

		double mousex, mousey;
		glfwGetCursorPos(window, &mousex, &mousey);
		int x = int(mousex);
		int y = int(mousey);

		void* data;
		vkMapMemory(device, entityImageMemory, 0, VK_WHOLE_SIZE, 0, &data);
		int* pixels = reinterpret_cast<int*>(data);
		int index = y * windowWidth + x;
		int hoverIndex = 0;
		if (y < windowHeight && x < windowWidth) {
			//cout << x << ", " << y << " ---> " << pixels[index] << "\n";
			hoverIndex = pixels[index];
			//tileTextureIdx = hoverIndex % 4;
		}
		vkUnmapMemory(device, entityImageMemory);

		if(handleClick) std::cout << "\nClick on tile: " << pixels[index]<<"\n";



		string structurePath = "./structure.json";
		static MahjongGame game = MahjongGame(structurePath);


		//--------------------------
		// STATE MACHINE FOR THE GAME
		// -------------------------
		bool enterPressedFirstTime = false;
		//std:cout << "\nGameState: " << gameState<<"\n";	//DEBUG PRINT
		switch (gameState) {		// main state machine implementation
			
			case -1: //menu	
				if (enter) {
					gameState = 0;
					enterPressedFirstTime = true;
				}
				if (handleClick /*&& //button index*/) {
					tileTextureIdx++;
					if (tileTextureIdx == 4) tileTextureIdx = 0;
				}
				/*
				if(handleClick /*&& //button index) {
					tileTextureIdx--;
					if (tileTextureIdx == -1) tileTextureIdx = 3;
				}
				*/
				std::cout << "\nTileTexIdx: " << tileTextureIdx << "\n";
				//get clicks to change textures and shaders
				break;
			case 0:
				//no piece selected
				firstTileIndex = -1;
				secondTileIndex = -1;
				DisappearingTileTransparency = 1.0f; //not transparent
				if (handleClick && hoverIndex > -1) {
					firstTileIndex = hoverIndex;
					gameState = 1;
				}
				break;
			case 1:
				//1 piece selected and highlighted
				if (handleClick && hoverIndex>-1) {
					secondTileIndex = hoverIndex;
					gameState = 2;
				}
				break;
			case 2:
				//2 pieces selected and highlighted
				if (game.canRemoveTiles(firstTileIndex,secondTileIndex)) {
					//correct selection
					gameState = 4;
				}
				else {
					//selected 2 uncompatible tiles
					gameState = 3;
				}
				DisappearingTileTransparency = 1.0f;
				break;
			case 3:
				//wrong choice of second piece
				//notify error, how?
				std::cout << "\n ERROR: tiles cannot be removed together \n";
				PlaySound(TEXT("sounds/game_error_tone_1.wav"), NULL, SND_FILENAME | SND_ASYNC);
				//deselect tiles
				firstTileIndex = -1;
				secondTileIndex = -1;
				gameState = 0;
				break;
			case 4:
				//two pieces start to disappear
				//std::cout <<"\nTile indexes: " << firstTileIndex << ", " << secondTileIndex << "are disappearing with T= "<< DisappearingTileTransparency<<"\n";
				DisappearingTileTransparency = DisappearingTileTransparency - 2.5f * deltaT; //check coefficient 0.1f
				if (DisappearingTileTransparency <= 0) {
					DisappearingTileTransparency = 0;
					gameState = 5;
				}
				break;
			case 5:
				//remove the tile
				game.removeTiles(firstTileIndex, secondTileIndex);
				cout << "Game over: " << game.isGameOver() << endl;
				if (game.isWon() || game.isGameOver()) {
					gameState = 6;
				}
				disappearedTiles[firstTileIndex] = true;
				disappearedTiles[secondTileIndex] = true;
				//game.removeTiles(firstTileIndex, secondTileIndex);
				//go back to initial state
				gameState = 0;
				break;
			case 6:
				if (game.isWon()) {
					youwinubo.visible = 1.0f;
					gameoverubo.visible = 0.0f;
					PlaySound(TEXT("sounds/clapping_people.wav"), NULL, SND_FILENAME | SND_ASYNC);
					std::cout << "\n------\nVictory!\n------\n";
				}
				else if (game.isGameOver()) {
					gameoverubo.visible = 1.0f;
					youwinubo.visible = 0.0f;
					PlaySound(TEXT("sounds/retro_error_long_tone.wav"), NULL, SND_FILENAME | SND_ASYNC);
					std::cout << "\n------\nYou Lost!\n------\n";
				}
				//PRESS ENTER TO GO BACK TO MENU
				if (enter) {
					//go back to menu
					gameState = -1;
					//reinitialise game
					for (int j = 0; j < 144; j++) {
						disappearedTiles[j] = false;
					}
				}
				break;

		}

		//---------------------------
		//CAMERA SETTINGS
		//---------------------------

		//Change position accoring to received commands
		CamH += m.z * movSpeed * deltaT;
		CamRadius -= m.x * movSpeed * deltaT;
		CamRadius = glm::clamp(CamRadius, 0.20f, 10.0f); //minumum and maximum zoom of the cam //REDUCE MAXIMUM ZOOOM <------------

		CamPitch -= r.x * rotSpeed * deltaT;
		CamPitch = glm::clamp(CamPitch, glm::radians(0.0f), glm::radians(89.0f)); //constraints on degrees on elevation of the cam 

		CamYaw += r.y * rotSpeed * deltaT;

		//Game logic: overwrites coordinates if fire (space) is pressed and released
		//Bring to initial position
		if (handleFire || enterPressedFirstTime) { //replace hanfleFire with "wasFire" to have event happen upon pressing and not release of fire key
			//glm::vec3 
			CamRadius = initialCamRadius;
			CamPitch = initialPitch;
			CamYaw = initialYaw;
		}
		//if in menu, fix the camera at a certain point
		if (gameState == -1) {
			CamRadius = 4.0f; //lower to be closer to green board in the menu
			CamPitch = 0.0f;
			CamYaw = 0.0f;
		}


		/* da usare insieme a ubo perch� servono le matrici
		glm::mat4 initalModel = glm::rotate(glm::mat4(1.0f),
								glm::radians(0.0f),
								glm::vec3(0.0f, 0.0f, 1.0f));


		ubo.view = glm::lookAt(game.getCamPos(deltaT),game.getAimPos(),glm::vec3(0.0f, 1.0f, 0.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f),
						swapChainExtent.width / (float) swapChainExtent.height,
						0.1f, 10.0f); ==  glm::perspective(FOVy, Ar, nearPlane, farPlane);
		ubo.proj[1][1] *= -1;*/


		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;

		//a
		//glm::vec3 camTarget = glm::vec3(0,CamH,0);
		glm::vec3 camTarget = glm::vec3(0, 0.6f, 0);
		if (gameState == -1) {
			camTarget = homeMenuPosition + glm::vec3(0, 1.2f, 0);
		}

		//c
		glm::vec3 camPos = camTarget + CamRadius * glm::vec3(cos(CamPitch) * sin(CamYaw), sin(CamPitch), cos(CamPitch) * cos(CamYaw));


		glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));


		//--------------------------
		//BUFFERS FILLING
		//--------------------------

		gubo.PlightPos = glm::vec3(0.0f, 3.0f, 0.0f);	
		gubo.PlightColor = glm::vec3(10.0f);
		gubo.beta = 1.0f;
		gubo.g = 0.5f;
		gubo.AmbLightColor = glm::vec3(0.1f);
		gubo.eyePos = camPos;

		// Writes value to the GPU
		DSGubo.map(currentImage, &gubo, sizeof(gubo), 0);

		// UI elements
		// Game over message
		gameoverubo.transparency = 1.0f;
		gameoverubo.objectIdx = -1;		
		DSGameOver.map(currentImage, &gameoverubo, sizeof(gameoverubo), 0);
		// Victory message
		youwinubo.transparency = 1.0f;
		youwinubo.objectIdx = -1;
		DSYouWin.map(currentImage, &youwinubo, sizeof(youwinubo), 0);

		// Indices for commonubo
		// [0] - Background
		// [1] - Room walls
		// [2] - Room ceiling
		// [3] - Room floor
		// [4] - Table
		// [5-7] - Windows
		// [8] - Landscape
		// [9] - Home screen background
		// [10] - Game title
		// [11] - Button1
		// [12] - Button2
		// [13] - Button3
		// [14] - Arrow Left 1
		// [15] - Arrow Left 2
		// [16] - Arrow Left 3
		// [17] - Arrow Right 1
		// [18] - Arrow Right 2
		// [19] - Arrow Right 3
		// [20] - Play button
		// [21] - Game setting
		// [22] - Tile type selection title
		// [23] - Board design selection title

		glm::mat4 translateUp = glm::translate(glm::mat4(2.0f), glm::vec3(0.0f, 1.5f, 0.0f));

		// Home screen background
		glm::mat4 WorldH = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.5f, 0.0f)) * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 4.0f);
		commonubo[9].mvpMat = Prj * View * WorldH;
		commonubo[9].mMat = WorldH;
		commonubo[9].nMat = glm::inverse(glm::transpose(WorldH));
		commonubo[9].transparency = 0.0f;
		DSHome.map(currentImage, &commonubo[9], sizeof(commonubo[9]), 0);

		//Button1
		glm::mat4 WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.15f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.3f);
		commonubo[11].mvpMat = Prj * View * WorldB;
		commonubo[11].mMat = WorldB;
		commonubo[11].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[11].transparency = 1.0f;
		DSButton1.map(currentImage, &commonubo[11], sizeof(commonubo[11]), 0);

		//Button2
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.8f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.3f);
		commonubo[12].mvpMat = Prj * View * WorldB;
		commonubo[12].mMat = WorldB;
		commonubo[12].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[12].transparency = 1.0f;
		DSButton2.map(currentImage, &commonubo[12], sizeof(commonubo[12]), 0);

		/*//Button3
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.0f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[13].mvpMat = Prj * View * WorldB;
		commonubo[13].mMat = WorldB;
		commonubo[13].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[13].transparency = 1.0f;
		DSButton3.map(currentImage, &commonubo[13], sizeof(commonubo[13]), 0);*/

		//Arrow button 1 Left
		glm::mat4 WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(0.35f, 0.0f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[14].mvpMat = Prj * View * WorldA_B;
		commonubo[14].mMat = WorldA_B;
		commonubo[14].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[14].transparency = 1.0f;
		DSArrowButton1_left.map(currentImage, &commonubo[14], sizeof(commonubo[14]), 0);

		//Arrow button 2 Left
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(0.35f, -1.65f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[15].mvpMat = Prj * View * WorldA_B;
		commonubo[15].mMat = WorldA_B;
		commonubo[15].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[15].transparency = 1.0f;
		DSArrowButton2_left.map(currentImage, &commonubo[15], sizeof(commonubo[15]), 0);

		/*//Arrow button 3 Left
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.0f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[16].mvpMat = Prj * View * WorldA_B;
		commonubo[16].mMat = WorldA_B;
		commonubo[16].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[16].transparency = 1.0f;
		DSArrowButton3_left.map(currentImage, &commonubo[16], sizeof(commonubo[16]), 0);*/

		//Arrow button 1 Right
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(3.7f, 0.0f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[17].mvpMat = Prj * View * WorldA_B;
		commonubo[17].mMat = WorldA_B;
		commonubo[17].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[17].transparency = 1.0f;
		DSArrowButton1_right.map(currentImage, &commonubo[17], sizeof(commonubo[17]), 0);

		//Arrow button 2 Right
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(3.7f, -1.65f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[18].mvpMat = Prj * View * WorldA_B;
		commonubo[18].mMat = WorldA_B;
		commonubo[18].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[18].transparency = 1.0f;
		DSArrowButton2_right.map(currentImage, &commonubo[18], sizeof(commonubo[18]), 0);

		/*//Arrow button 3 Right
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(3.5f, -1.0f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[19].mvpMat = Prj * View * WorldA_B;
		commonubo[19].mMat = WorldA_B;
		commonubo[19].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[19].transparency = 1.0f;
		DSArrowButton3_right.map(currentImage, &commonubo[19], sizeof(commonubo[19]), 0);*/

		//Play button
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.6f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.5f);
		commonubo[20].mvpMat = Prj * View * WorldB;
		commonubo[20].mMat = WorldB;
		commonubo[20].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[20].transparency = 1.0f;
		DSPlayButton.map(currentImage, &commonubo[20], sizeof(commonubo[20]), 0); 
		
		//Game settings title
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.7f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1.0f, 0.5f, 1.0f) * 1.4f);
		commonubo[21].mvpMat = Prj * View * WorldB;
		commonubo[21].mMat = WorldB;
		commonubo[21].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[21].transparency = 1.0f;
		DSSelection1.map(currentImage, &commonubo[21], sizeof(commonubo[21]), 0);

		//tile selection title 
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.05f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1.0f, 0.5f, 1.0f) * 0.8f);
		commonubo[22].mvpMat = Prj * View * WorldB;
		commonubo[22].mMat = WorldB;
		commonubo[22].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[22].transparency = 1.0f;
		DSSelection2.map(currentImage, &commonubo[22], sizeof(commonubo[22]), 0);

		//board selection title
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.6f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1.0f, 0.45f, 1.0f) * 0.8f);
		commonubo[23].mvpMat = Prj * View * WorldB;
		commonubo[23].mMat = WorldB;
		commonubo[23].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[23].transparency = 1.0f;
		DSSelection3.map(currentImage, &commonubo[23], sizeof(commonubo[23]), 0);
		 
		//Matrix setup for rotating tile
		static float ang = 0.0f;
		ang += homeTileRotSpeed * deltaT;
		tileHomeubo.transparency = 1.0f;
		glm::mat4 rotTile = translateUp * homeMenuWorld *
			glm::translate(glm::mat4(1), glm::vec3(-2.4f, -0.4f, 0.2f)) *
			glm::rotate(glm::mat4(1), glm::radians(-80.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1), glm::sin(ang)+0.5f, glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(35.0f)) *
			glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::translate(glm::mat4(1), glm::vec3(0.0f, -0.00675f, 0.0f));
		tileHomeubo.amb = 10.0f;
		tileHomeubo.gamma = 300.0f;
		tileHomeubo.sColor = glm::vec3(0.4f);
		tileHomeubo.mvpMat = Prj * View * rotTile;
		tileHomeubo.mMat = rotTile;
		tileHomeubo.nMat = glm::inverse(glm::transpose(rotTile));
		tileHomeubo.tileIdx = -2;
		tileHomeubo.suitIdx = 10;
		tileHomeubo.textureIdx = tileTextureIdx;
		DSHTile.map(currentImage, &tileHomeubo, sizeof(tileHomeubo), 0);

		//Matrix setup for Game Title
		glm::mat4 WorldTitle = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, -1.0f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.6f);
			//* glm::scale(glm::mat4(1), glm::vec3(-1.0f, 1.0f, -1.0f));
		commonubo[10].mvpMat = Prj * View * WorldTitle;
		commonubo[10].mMat = WorldTitle;
		commonubo[10].nMat = glm::inverse(glm::transpose(WorldTitle));
		commonubo[10].transparency = 1.0f;
		DSGameTitle.map(currentImage, &commonubo[10], sizeof(commonubo[10]), 0);
		
		// Matrix setup for background
		glm::mat4 World = glm::mat4(1);
		World = glm::translate(glm::mat4(1), glm::vec3(0.04f, 0.0f, 0.0f)) * 
				glm::scale(glm::mat4(1), glm::vec3(3.55f, 1.0f, 1.4f));
		bgubo.amb = 1.0f; bgubo.sigma = 1.1f;
		commonubo[0].mvpMat = Prj * View * World;
		commonubo[0].mMat = World;
		commonubo[0].nMat = glm::inverse(glm::transpose(World));
		commonubo[0].transparency = 0.0f;
		DSBackground.map(currentImage, &commonubo[0], sizeof(commonubo[0]), 0);
		DSBackground.map(currentImage, &bgubo, sizeof(bgubo), 1);

		// Matrix setup for walls
		World = glm::mat4(1);
		wallubo.amb = 1.0f; wallubo.sigma = 1.1f;
		commonubo[1].mvpMat = Prj * View * World;
		commonubo[1].mMat = World;
		commonubo[1].nMat = glm::inverse(glm::transpose(World));
		commonubo[1].transparency = 0.0f;
		DSWall.map(currentImage, &commonubo[1], sizeof(commonubo[1]), 0);
		DSWall.map(currentImage, &wallubo, sizeof(wallubo), 1);

		// Matrix setup for floor
		World = glm::mat4(1);
		floorubo.amb = 1.0f; floorubo.sigma = 1.1f;
		commonubo[3].mvpMat = Prj * View * World;
		commonubo[3].mMat = World;
		commonubo[3].nMat = glm::inverse(glm::transpose(World));
		commonubo[3].transparency = 0.0f;
		DSFloor.map(currentImage, &commonubo[3], sizeof(commonubo[3]), 0);
		DSFloor.map(currentImage, &floorubo, sizeof(floorubo), 1);

		// Matrix setup for ceiling
		World = glm::mat4(1);
		ceilingubo.amb = 1.0f; ceilingubo.sigma = 1.1f;
		commonubo[2].mvpMat = Prj * View * World;
		commonubo[2].mMat = World;
		commonubo[2].nMat = glm::inverse(glm::transpose(World));
		commonubo[2].transparency = 0.0f;
		DSCeiling.map(currentImage, &commonubo[2], sizeof(commonubo[2]), 0);
		DSCeiling.map(currentImage, &ceilingubo, sizeof(ceilingubo), 1);

		// Matrix setup for table
		World = glm::mat4(1);
		tableubo.amb = 20.0f; tableubo.sigma = 1.1f;
		commonubo[4].mvpMat = Prj * View * World;
		commonubo[4].mMat = World;
		commonubo[4].nMat = glm::inverse(glm::transpose(World));
		commonubo[4].transparency = 0.0f;
		DSTable.map(currentImage, &commonubo[4], sizeof(commonubo[4]), 0);
		DSTable.map(currentImage, &tableubo, sizeof(tableubo), 1);

		// Matrix setup for windows
		// Window 1
		World = glm::mat4(1);
		glm::mat4 TWindowMat = glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.5f, -2.0f));
		World = TWindowMat;
		window1ubo.amb = 1.0f; window1ubo.sigma = 1.1f;
		commonubo[5].mvpMat = Prj * View * World;
		commonubo[5].mMat = World;
		commonubo[5].nMat = glm::inverse(glm::transpose(World));
		commonubo[5].transparency = 1.0f;
		DSWindow1.map(currentImage, &commonubo[5], sizeof(commonubo[5]), 0);
		DSWindow1.map(currentImage, &window1ubo, sizeof(window1ubo), 1);
		// Window 2
		TWindowMat = glm::translate(glm::mat4(1), glm::vec3(-1.0f, 1.5f, -2.0f));
		World = TWindowMat;
		window2ubo.amb = 1.0f; window2ubo.sigma = 1.1f;
		commonubo[6].mvpMat = Prj * View * World;
		commonubo[6].mMat = World;
		commonubo[6].nMat = glm::inverse(glm::transpose(World));
		commonubo[6].transparency = 1.0f;
		DSWindow2.map(currentImage, &commonubo[6], sizeof(commonubo[6]), 0);
		DSWindow2.map(currentImage, &window2ubo, sizeof(window2ubo), 1);
		// Window 3
		TWindowMat = glm::translate(glm::mat4(1), glm::vec3(1.0f, 1.5f, -2.0f));
		World = TWindowMat;
		window3ubo.amb = 1.0f; window3ubo.sigma = 1.1f;
		commonubo[7].mvpMat = Prj * View * World;
		commonubo[7].mMat = World;
		commonubo[7].nMat = glm::inverse(glm::transpose(World));
		commonubo[7].transparency = 1.0f;
		DSWindow3.map(currentImage, &commonubo[7], sizeof(commonubo[7]), 0);
		DSWindow3.map(currentImage, &window3ubo, sizeof(window3ubo), 1);

		// Matrix setup for landscape
		World = glm::mat4(1);
		glm::mat4 TransLandscape = glm::translate(glm::mat4(1), glm::vec3(0.0f,1.57f,-1.99f));
		glm::mat4 ScaleLandscape = glm::scale(glm::mat4(1), glm::vec3(1.49f,0.75f,1.0f));
		World = TransLandscape * ScaleLandscape;
		commonubo[8].mvpMat = Prj * View * World;
		commonubo[8].mMat = World;
		commonubo[8].nMat = glm::inverse(glm::transpose(World));
		commonubo[8].transparency = 0.0f;
		DSLandscape.map(currentImage, &commonubo[8], sizeof(commonubo[8]), 0);

		// Matrix setup for tiles
		for (int i = 0; i < 144; i++) {
			float scaleFactor = game.tiles[i].isRemoved ? 0.0f : 1.0f;
			//float scaleFactor = 0.0f;
			glm::mat4 Tbase = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.6f, 0.0f));
			glm::mat4 Tmat = glm::translate(glm::mat4(1), game.tiles[i].position * scaleFactor); // matrix for translation
			glm::mat4 Smat = glm::scale(glm::mat4(1), glm::vec3(scaleFactor));

			World = Tbase * Tmat * Smat; // translate tile in position
			//World = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-9.2 + i%10*2, 0, 9.2-i/10*2)), glm::vec3(50.0f));
			tileubo[i].amb = 1.0f; 
			tileubo[i].gamma = 300.0f; //CHANGE GAMMA HIGHER FOR POINT LIGHT
			tileubo[i].sColor = glm::vec3(0.4f);
			tileubo[i].tileIdx = game.tiles[i].tileIdx;
			tileubo[i].suitIdx = game.tiles[i].suitIdx;
			tileubo[i].transparency = 1.0f;
			tileubo[i].textureIdx = tileTextureIdx;

			//highlight the piece on which the mouse is hoovering
			tileubo[i].hoverIdx = hoverIndex;

			//highlight the first selected piece
			if (i==firstTileIndex) {
				tileubo[i].selectedIdx = firstTileIndex;
			}
			//highlight the first selected piece
			else if (i == secondTileIndex) {
				tileubo[i].selectedIdx = secondTileIndex;
			}
			else {
				tileubo[i].selectedIdx = -1;
			}
			
			if (disappearedTiles[i]) {
				tileubo[i].mvpMat = Prj * View * removedTileWorld;
				tileubo[i].mMat = removedTileWorld; 
				tileubo[i].nMat = glm::inverse(glm::transpose(removedTileWorld)); 
				DSTile[i].map(currentImage, &tileubo[i], sizeof(tileubo[i]), 0);
				
			}
			else {
				tileubo[i].mvpMat = Prj * View * World; 
				tileubo[i].mMat = World; 
				tileubo[i].nMat = glm::inverse(glm::transpose(World)); 
				if (gameState==4 & (i == firstTileIndex || i == secondTileIndex)) {
					//set transparency to = DisappearingTileTransparency;
					tileubo[i].transparency = DisappearingTileTransparency;
					//std::cout <<"\ntransparency of tile\n" <<i<<": "<< tileubo[i].transparency<<"\n--------\n";
					//set highlight of selected tile
					
				}
				DSTile[i].map(currentImage, &tileubo[i], sizeof(tileubo[i]), 0); 
			}
		}
	}	
};


// This is the main: probably you do not need to touch this!
int main() {
	Mahjong app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}