//COMPUTER GRAPHICS PROJECT 2022/23

// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include "MahjongGame.hpp"

#include <glm/ext/vector_common.hpp>
#include <glm/ext/scalar_common.hpp>
#include <glm/gtx/transform2.hpp>
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <random>


//link windows multimedia library to our program
#pragma comment(lib, "winmm.lib")


//----------------------
// UNIFORM BLOCKS
//----------------------

struct CommonUniformBlock {
	alignas(16) glm::mat4 mvpMat;			// ViewProjection
	alignas(16) glm::mat4 mMat;				// World matrix
	alignas(16) glm::mat4 nMat;				// Normal transformation matrix
	alignas(4) float transparency;			// Either 1.0f or 0.0f
	alignas(4) int textureIdx;				// Id of the texture to take from the sampler2DArray of textures
	alignas(4) int objectIdx;				// Id used to identify object for selection
};

struct TileUniformBlock {
	alignas(4) float amb;					// Coefficient to regulate the effect of ambient light on the tile
	alignas(4) float gamma;					// Gamma coefficient for Blinn shader
	alignas(16) glm::vec3 sColor;			// Specular color
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
	alignas(4) int tileIdx;					// Index of the tile on the board
	alignas(4) int suitIdx;					// Index of the tile w.r.t. the drawing on it
	alignas(4) float transparency;			// Transparency of the tile, used in disappearing animation
	alignas(4) int hoverIdx;				// Index of the tile on which the mouse cursor is hovering
	alignas(4) int selectedIdx;				// Index of the tile that is already selected in game
	alignas(4) int textureIdx;			
	alignas(4) int isInMenu;				// Either 0 or 1, used to define if the tile is in the menu or in game and change lightr accordingly
};

struct RoughSurfaceUniformBlock {
	alignas(4) float amb;
	alignas(4) float sigma;					// Roughness for Oren Nayar shader
};

struct SmoothSurfaceUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;					// Gamma for Blinn shader
	alignas(16) glm::vec3 sColor;
};

struct PlainWithEmissionUniformBlock {
	alignas(16) glm::vec3 emission;			// Emission color
};

struct GlobalUniformBlock {
	alignas(4) float beta;					// Decay factor of the point light
	alignas(4) float g;						// Distance parameter for point light
	alignas(16) glm::vec3 PlightPos;		// Point light position
	alignas(16) glm::vec3 PlightColor;		// Original color of the point light
	alignas(16) glm::vec3 AmbLightColor;	// Ambient color
	alignas(16) glm::vec3 eyePos;			// Viewer position
};

struct UIUniformBlock {
	alignas(4) float visible;				// Either 1.0f or 0.0f
	alignas(4) float transparency;
	alignas(4) int objectIdx;
};

//----------------------
// VERTEX DATA STRUCTURES
//----------------------

struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexUI {
	glm::vec3 pos;
	glm::vec2 UV;
};

//----------------------
// MAIN PROJECT
//----------------------

class Mahjong : public BaseProject {
protected:

	//----------------------
	// VARIABLES DECLARATION
	//----------------------
	
	// Current aspect ratio (used by the callback that resized the window)
	float Ar;

	// Descriptor Set Layouts
	DescriptorSetLayout DSLGubo;		// DSL for GlobalUniformBufferObject
	DescriptorSetLayout DSLTile;		// DSL for Tile objects
	DescriptorSetLayout DSLPlain;		// DSL with 1 UNIFORM and 1 TEXTURE
	DescriptorSetLayout DSLGeneric;		// DSL with 2 UNIFORM and 1 TEXTURE
	DescriptorSetLayout DSLTextureOnly;	// DSL with only 1 TEXTURE

	// Vertex formats
	VertexDescriptor VMesh;
	VertexDescriptor VUI;

	// Pipelines
	Pipeline PPlain;
	Pipeline PTile;
	Pipeline PRoughSurfaces;
	Pipeline PSmoothSurfaces;
	Pipeline PPlainWithEmission;
	Pipeline PUI;

	// Models, Textures and Descriptors (values assigned to the uniforms)
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
	Model<VertexMesh> MPlainRectangle;
	Model<VertexMesh> MArrowButton;
	Model<VertexMesh> MCircleButton;
	Model<VertexMesh> MLion;
	Model<VertexMesh> MPictureFrame;
	Model<VertexMesh> MVase;
	Model<VertexMesh> MChair;
	Model<VertexMesh> MFlame;
	Model<VertexMesh> MCandle;
	Model<VertexMesh> MLamp;
	Model<VertexMesh> MKettle;
	Model<VertexMesh> MDoor;
	Model<VertexMesh> MBlackboardFrame;
	Model<VertexMesh> MBlackboardBoard;
	Model<VertexUI> MGameOver;
	Model<VertexUI> MYouWin;
	Model<VertexUI> MYesButton;
	Model<VertexUI> MNoButton;
	Model<VertexUI> MBackToMenu;

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
	DescriptorSet DSLion;
	DescriptorSet DSPictureFrame1, DSPictureFrame2;
	DescriptorSet DSPictureFrameImage1, DSPictureFrameImage2;
	DescriptorSet DSVase;
	DescriptorSet DSChair;
	DescriptorSet DSFlame;
	DescriptorSet DSCandle;
	DescriptorSet DSLamp;
	DescriptorSet DSKettle;
	DescriptorSet DSDoor;
	DescriptorSet DSBlackboardFrame;
	DescriptorSet DSBlackboardBoard;
	DescriptorSet DSBlackboardText;
	// Descriptor sets for UI elements
	DescriptorSet DSGameOver;
	DescriptorSet DSYouWin;
	DescriptorSet DSButton1, DSButton2, DSButton3;
	DescriptorSet DSArrowButton1_left, DSArrowButton2_left, DSArrowButton3_left;
	DescriptorSet DSArrowButton1_right, DSArrowButton2_right, DSArrowButton3_right;
	DescriptorSet DSCircleButton;
	DescriptorSet DSPlayButton;
	DescriptorSet DSSelection1, DSSelection2, DSSelection3, DSSelection4;
	DescriptorSet DSTileSelText;
	DescriptorSet DSBoardSelText;
	DescriptorSet DSYesButton, DSNoButton;
	DescriptorSet DSBackToMenu;
	
	// Scene
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
	Texture TLion;
	Texture TPictureFrame; 
	Texture TPictureFrameImage1, TPictureFrameImage2;
	Texture TFlame;
	Texture TVase;
	Texture TChair;
	Texture TCandle;
	Texture TLamp;
	Texture TKettle;
	Texture TDoor;
	Texture TBlackboardFrame;
	Texture TBlackboardBoard;
	Texture TBlackboardText; 
	// UI
	Texture TButton;
	Texture TArrowButtonLeft, TArrowButtonRight;
	Texture TCircleButton;
	Texture TPlayButton;
	Texture TSelection1;
	Texture TSelection2;
	Texture TSelection3;
	Texture TSelection4;
	Texture TTileSelText;
	Texture TBoardSelText;
	Texture TYesButton, TNoButton;
	Texture TBackToMenu;
	

	// C++ storage for uniform variables
	GlobalUniformBlock gubo; 
	TileUniformBlock tileubo[144];
	TileUniformBlock tileHomeubo; //rotating tile in home menu screen 
	RoughSurfaceUniformBlock bgubo;
	RoughSurfaceUniformBlock wallubo;
	RoughSurfaceUniformBlock floorubo;
	RoughSurfaceUniformBlock ceilingubo;
	RoughSurfaceUniformBlock tableubo;
	RoughSurfaceUniformBlock window1ubo, window2ubo, window3ubo;
	RoughSurfaceUniformBlock chairubo;
	RoughSurfaceUniformBlock pictureFrameImageubo1, pictureFrameImageubo2;
	RoughSurfaceUniformBlock doorubo;
	RoughSurfaceUniformBlock blackboardTextubo;
	RoughSurfaceUniformBlock lampubo;
	SmoothSurfaceUniformBlock lionubo;
	SmoothSurfaceUniformBlock pictureFrameubo1, pictureFrameubo2;
	SmoothSurfaceUniformBlock vaseubo;
	SmoothSurfaceUniformBlock candleubo;
	SmoothSurfaceUniformBlock kettleubo;
	SmoothSurfaceUniformBlock blackboardFrameubo;
	SmoothSurfaceUniformBlock blackboardBoardubo;	
	CommonUniformBlock tileSelTextubo, boardSelTextubo;
	PlainWithEmissionUniformBlock flameEmissionubo;
	UIUniformBlock gameoverubo; 
	UIUniformBlock youwinubo; 
	UIUniformBlock yesbuttonubo;
	UIUniformBlock nobuttonubo;
	UIUniformBlock backtomenuubo;


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
	// [13] - Button3 (Not used anymore)
	// [14] - Arrow Left 1
	// [15] - Arrow Left 2
	// [16] - Arrow Left 3 (Not used anymore)
	// [17] - Arrow Right 1
	// [18] - Arrow Right 2
	// [19] - Arrow Right 3 (Not used anymore)
	// [20] - Play button
	// [21] - Game setting
	// [22] - Tile type selection title
	// [23] - Board design selection title
	// [24] - Lion statue
	// [25] - Picture frame 1
	// [26] - Picture frame image 1
	// [27] - Vase
	// [28] - Chair
	// [29] - Picture frame 2
	// [30] - Picture frame image 2
	// [31] - Flame
	// [32] - Candle
	// [33] - day/night time selection title
	// [34] - circular day/night button
	// [35] - Lamp
	// [36] - Kettle
	// [37] - Door
	// [38] - Blackboard Frame
	// [39] - Blackboard Board
	// [40] - Blackboard Text
	CommonUniformBlock commonubo[41];

	// Other application parameters
	int tileTextureIdx = 0;					//Id of the current tile texture 
	int boardTextureIdx = 0;				//Id of the current board texture
	int circleTextureIdx = 0;				//Id of the current day/night button texture
	int pictureFrameImageIdx1 = 0;			//Id of the current picture frame image 1 texture
	int pictureFrameImageIdx2 = 0;			//Id of the current picture frame image 2 texture
	int lampTextureIdx = 1;					//Id of the current lamp texture (Alight or not)
	int landscapeTextureIdx = 0;			//Id of the current window landscape texture
	// Camera parameters
	const float FOVy = glm::radians(90.0f);
	const float nearPlane = 0.01f;
	const float farPlane = 10.0f;
	const float rotSpeed = glm::radians(90.0f);
	const float movSpeed = 1.0f;
	float CamH, CamRadius, CamPitch, CamYaw;
	const float initialCamRadius = 0.3f;
	const float initialPitch = glm::radians(60.0f);
	const float initialYaw = glm::radians(0.0f);
	
	//other parameters
	int gameState = -1;
	int isCandleAlight = 0;
	glm::vec3 generalSColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float DisappearingTileTransparency = 1.0f;
	const float homeTileRotSpeed = glm::radians(80.0f);
	int firstTileIndex = -1;
	int secondTileIndex = -1;
	const glm::mat4 removedTileWorld = glm::translate(glm::mat4(1.0), glm::vec3(10.0f, -20.0f, 0.0f)) * 
								glm::scale(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
	const glm::vec3 homeMenuPosition = glm::vec3(-10.0f, 0.0f, -20.0f);
	const glm::mat4 homeMenuWorld = glm::translate(glm::mat4(1.0f), homeMenuPosition);


	// Main application parameters
	void setWindowParameters() {
		// window size, title and initial background
		windowWidth = 1200;
		windowHeight = 900;
		windowTitle = "Mahjong";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.0f, 0.005f, 0.01f, 1.0f };

		// Descriptor pool sizes
		uniformBlocksInPool = 217;
		texturesInPool = 49;
		setsInPool = 195;

		// Initialize aspect ratio
		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
		windowWidth = w;
		windowHeight = h;
	}

	// Load and setup Vulkan Models and Textures.
	// Create the Descriptor Set Layouts and load the shaders for the pipelines
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
		// PRoughSurfaces --> Pipeline for rough objects
		PRoughSurfaces.init(this, &VMesh, "shaders/PhongVert.spv", "shaders/OrenNayarFrag.spv", { &DSLGeneric, &DSLGubo });
		PRoughSurfaces.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, true);
		// PSmoothSurfaces --> Pipeline for smooth objects
		PSmoothSurfaces.init(this, &VMesh, "shaders/PhongVert.spv", "shaders/BlinnFrag.spv", { &DSLGeneric, &DSLGubo });
		PSmoothSurfaces.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
		// PPlainWithEmission --> Pipeline for elements that have to be 'copied' from textures but with an additional emission term
		PPlainWithEmission.init(this, &VMesh, "shaders/PhongVert.spv", "shaders/PlainWithEmissionFrag.spv", { &DSLGeneric});
		PPlainWithEmission.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, true);
		

		//----------------------
		// MODELS
		//----------------------
		
		//----------------------
		// Manual models
		//----------------------
		
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

		//day/night button
		MCircleButton.vertices = MArrowButton.vertices;
		MCircleButton.indices = MArrowButton.indices;
		MCircleButton.initMesh(this, &VMesh);

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
		float gameOverHalfX = 0.44f;
		float gameOverHalfY = 0.5f;
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

		// Back to menu message
		float backToMenuHalfX = 0.44f;
		MBackToMenu.vertices = {
			{{-backToMenuHalfX, -0.5, 0.1f},{0.0f, 0.0f}},
			{{backToMenuHalfX, -0.5, 0.1f},{1.0f, 0.0f}},
			{{-backToMenuHalfX, 0.1, 0.1f},{0.0f, 1.0f}},
			{{backToMenuHalfX, 0.1, 0.1f},{1.0f, 1.0f}},
		};
		MBackToMenu.indices = { 0, 1, 2,    1, 2, 3 };
		MBackToMenu.initMesh(this, &VUI);

		// Yes/No buttons
		float yesButtonTop = 0.1f;
		float yesButtonBottom = 0.3f;
		float yesButtonLeft = -0.40;
		float yesButtonRight = -0.1;
		MYesButton.vertices = {
			{{yesButtonLeft, yesButtonTop, 0.0f},{0.0f, 0.0f}},
			{{yesButtonRight, yesButtonTop, 0.0f},{1.0f, 0.0f}},
			{{yesButtonLeft, yesButtonBottom, 0.0f},{0.0f, 1.0f}},
			{{yesButtonRight, yesButtonBottom, 0.0f},{1.0f, 1.0f}},
		};
		MYesButton.indices = { 0, 1, 2,    1, 2, 3 };
		MYesButton.initMesh(this, &VUI);
		MNoButton.vertices = {
			{{-yesButtonRight, yesButtonTop, 0.1f},{0.0f, 0.0f}},
			{{-yesButtonLeft, yesButtonTop, 0.1f},{1.0f, 0.0f}},
			{{-yesButtonRight, yesButtonBottom, 0.1f},{0.0f, 1.0f}},
			{{-yesButtonLeft, yesButtonBottom, 0.1f},{1.0f, 1.0f}},
		};
		MNoButton.indices = { 0, 1, 2,    1, 2, 3 };
		MNoButton.initMesh(this, &VUI);

		//----------------------
		// Imported models
		//----------------------

		MTile.init(this, &VMesh, "models/Tile.obj", OBJ);
		MTable.init(this, &VMesh, "models/Table.obj", OBJ);
		MWindow.init(this, &VMesh, "models/Window.obj", OBJ);
		MLion.init(this, &VMesh, "models/Lion.obj", OBJ);
		MPictureFrame.init(this, &VMesh, "models/frame.obj", OBJ);
		MVase.init(this, &VMesh, "models/vase.obj", OBJ);
		MChair.init(this, &VMesh, "models/armchair.obj", OBJ);
		MFlame.init(this, &VMesh, "models/Fire.obj", OBJ);
		MCandle.init(this, &VMesh, "models/Candle.obj", OBJ);
		MLamp.init(this, &VMesh, "models/Lamp.obj", OBJ);
		MKettle.init(this, &VMesh, "models/kettle.obj", OBJ);
		MDoor.init(this, &VMesh, "models/door2.obj", OBJ);
		MBlackboardFrame.init(this, &VMesh, "models/Blackboard_1.obj", OBJ);
		MBlackboardBoard.init(this, &VMesh, "models/Blackboard_0.obj", OBJ);

		//----------------------
		// TEXTURES 
		//----------------------

		// Tiles textures
		const char* tileTextureFiles[4] = {
			"textures/tiles/tiles_white_resized.png",
			"textures/tiles/tiles_dark_resized.png",
			"textures/tiles/tiles_lucky_resized.png",
			"textures/tiles/tiles_botanical_resized.png",
		};
		TTile.initFour(this, tileTextureFiles);

		// Background cloth textures
		const char* clothTextureFiles[4] = {
			"textures/background/poolcloth.png",
			"textures/background/redCloth.png",
			"textures/background/wood.png",
			"textures/background/dark_wood_resized.png",
		};
		TPoolCloth.initFour(this, clothTextureFiles);

		// Tiles style name selection
		const char* tileNamesTextureFiles[4] = {
			"textures/buttons/white_tiles_text.png",
			"textures/buttons/black_tiles_text.png",
			"textures/buttons/lucky_tiles_text.png",
			"textures/buttons/botanical_tiles_text.png",
		};
		TTileSelText.initFour(this, tileNamesTextureFiles);

		// Board style name selection
		const char* boardNamesTextureFiles[4] = {
			"textures/buttons/poolTable_board_text.png",
			"textures/buttons/imperialRed_board_text.png",
			"textures/buttons/wood_board_text.png",
			"textures/buttons/darkwood_board_text.png",
		}; 
		TBoardSelText.initFour(this, boardNamesTextureFiles); 

		// Light button styles
		const char* circleNamesTextureFiles[2] = {
			"textures/buttons/dayTime.png",
			"textures/buttons/nightTime.png",
		};
		TCircleButton.initTwo(this, circleNamesTextureFiles);

		// Images that can appear in the picture frame 1
		const char* frameImagesTextureFiles1[4] = {
			"textures/room/picture1.jpg",
			"textures/room/picture2.jpg",
			"textures/room/picture3.jpg",
			"textures/room/picture4.jpg",
		};
		TPictureFrameImage1.initFour(this, frameImagesTextureFiles1);

		// Images that can appear in the picture frame 2
		const char* frameImagesTextureFiles2[5] = { 
			"textures/foto_cina/shanghai.jpg",
			"textures/foto_cina/suzhou.jpg",
			"textures/foto_cina/yunnan.jpg",
			"textures/foto_cina/jiayuguan.jpg",
			"textures/foto_cina/zhangye.jpg",
		};
		TPictureFrameImage2.initFive(this, frameImagesTextureFiles2); 

		// Textures for the hanging lamp (off/on)
		const char* lampTextureFiles[2] = {
			"textures/room/lamp.png",
			"textures/room/lampAlight.jpg",
		};
		TLamp.initTwo(this, lampTextureFiles);

		// Textures of the landscape visible outside the window
		const char* landscapeTextureFiles[2] = {
			"textures/room/landscape.jpg",
			"textures/room/landscape_night.jpg",
		};
		TLandscape.initTwo(this, landscapeTextureFiles);

		// Other textures
		TWallDragon.init(this, "textures/room/dragon_texture0.jpg");
		TFloor.init(this, "textures/room/floor.png");
		TCeiling.init(this, "textures/room/ceiling.jpg");
		TTable.init(this, "textures/room/table.jpg");
		TWindow.init(this, "textures/room/window.png");
		TGameTitle.init(this, "textures/title_brush.png");
		TGameOver.init(this, "textures/ui/gameover.png");
		TYouWin.init(this, "textures/ui/youwin.png");
		TButton.init(this, "textures/buttons/button_rounded_edges.png");
		TArrowButtonLeft.init(this, "textures/buttons/arrow_button_left.png");
		TArrowButtonRight.init(this, "textures/buttons/arrow_button_right.png");
		TPlayButton.init(this, "textures/buttons/button_with_plant.png");
		TSelection1.init(this, "textures/buttons/settings.png");
		TSelection2.init(this, "textures/buttons/tileDesign.png");
		TSelection3.init(this, "textures/buttons/boardDesign.png");
		TSelection4.init(this, "textures/buttons/lightText.png");
		TLion.init(this, "textures/room/lion.png");
		TPictureFrame.init(this, "textures/room/PictureFrame.jpg");
		TVase.init(this, "textures/room/vase_1k.png");
		TChair.init(this, "textures/room/armchair.jpg");
		TFlame.init(this, "textures/room/fire.jpg");
		TCandle.init(this, "textures/room/candle.jpg");
		TKettle.init(this, "textures/room/kettle.jpg");
		TDoor.init(this, "textures/room/wood_door_2.png");
		TBlackboardFrame.init(this, "textures/room/blackboard_frame.png");
		TBlackboardBoard.init(this, "textures/room/blackboard_board.png");
		TBlackboardText.init(this, "textures/room/commands.png");
		TBackToMenu.init(this, "textures/buttons/backtomenu.png");
		TYesButton.init(this, "textures/buttons/yes.png");
		TNoButton.init(this, "textures/buttons/no.png");

		//----------------------
		// INIT LOCAL VARIABLES
		//----------------------
		CamH = 1.0f;
		CamRadius = initialCamRadius;
		CamPitch = initialPitch;
		CamYaw = initialYaw;
	}

	//----------------------------------------
	// PIPELINES AND DESCRIPTOR SETS CREATION
	//----------------------------------------
	void pipelinesAndDescriptorSetsInit() {
		// Create a new pipeline (with the current surface), using its shaders
		PRoughSurfaces.create();
		PSmoothSurfaces.create();
		PTile.create();
		PPlain.create();
		PPlainWithEmission.create();
		PUI.create();

		//----------------------
		// DESCRIPTOR SETS 
		//----------------------

		// UI
		DSGameOver.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(UIUniformBlock), nullptr},
				{1, TEXTURE, 0, &TGameOver}
			});
		DSYouWin.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(UIUniformBlock), nullptr},
				{1, TEXTURE, 0, &TYouWin}
			});
		DSBackToMenu.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(UIUniformBlock), nullptr},
				{1, TEXTURE, 0, &TBackToMenu}
			});
		DSYesButton.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(UIUniformBlock), nullptr},
				{1, TEXTURE, 0, &TYesButton}
			});
		DSNoButton.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(UIUniformBlock), nullptr},
				{1, TEXTURE, 0, &TNoButton}
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
		DSCircleButton.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TCircleButton}
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
		DSSelection4.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TSelection4}
			});
		DSTileSelText.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TTileSelText}
			});
		DSBoardSelText.init(this, &DSLPlain, {
				{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
				{1, TEXTURE, 0, &TBoardSelText}
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
		DSDoor.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TDoor}
			});
		DSBlackboardText.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr}, 
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr}, 
					{2, TEXTURE, 0, &TBlackboardText}
			});
		DSLion.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TLion}
			});
		DSBlackboardFrame.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TBlackboardFrame}
			});
		DSBlackboardBoard.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TBlackboardBoard}
			});
		DSPictureFrame1.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr}, 
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr}, 
					{2, TEXTURE, 0, &TPictureFrame}
			});
		DSPictureFrame2.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TPictureFrame}
			});
		DSKettle.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TKettle}
			});
		DSPictureFrameImage1.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TPictureFrameImage1}
			});
		DSPictureFrameImage2.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TPictureFrameImage2}
			});
		DSVase.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TVase}
			});
		DSChair.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TChair}
			});
		DSLamp.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(RoughSurfaceUniformBlock), nullptr},
					{2, TEXTURE, 0, &TLamp}
			});
		DSCandle.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr}, 
					{1, UNIFORM, sizeof(SmoothSurfaceUniformBlock), nullptr}, 
					{2, TEXTURE, 0, &TCandle} 
			});
		DSFlame.init(this, &DSLGeneric, {
					{0, UNIFORM, sizeof(CommonUniformBlock), nullptr},
					{1, UNIFORM, sizeof(PlainWithEmissionUniformBlock), nullptr},
					{2, TEXTURE, 0, &TFlame}
			});
		
	}

	//--------------------------------------------
	// DESTROY PIPELINES AND DESCRIPTOR SETS
	//--------------------------------------------
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PRoughSurfaces.cleanup();
		PSmoothSurfaces.cleanup();
		PTile.cleanup();
		PPlain.cleanup();
		PPlainWithEmission.cleanup();
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
		DSLion.cleanup();
		DSPictureFrame1.cleanup(); 
		DSPictureFrameImage1.cleanup(); 
		DSPictureFrame2.cleanup();
		DSPictureFrameImage2.cleanup();
		DSVase.cleanup();
		DSChair.cleanup();
		DSFlame.cleanup();
		DSLamp.cleanup();
		DSCandle.cleanup(); 
		DSKettle.cleanup();
		DSDoor.cleanup();
		DSBlackboardFrame.cleanup(); 
		DSBlackboardBoard.cleanup();
		DSBlackboardText.cleanup();
		DSGameOver.cleanup();
		DSYouWin.cleanup();
		DSBackToMenu.cleanup();
		DSYesButton.cleanup();
		DSNoButton.cleanup();
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
		DSCircleButton.cleanup();
		DSPlayButton.cleanup();
		DSSelection1.cleanup();
		DSSelection2.cleanup();
		DSSelection3.cleanup();
		DSSelection4.cleanup();
		DSTileSelText.cleanup();
		DSBoardSelText.cleanup();

	}

	//------------------------------------------------------------
	// DESTROY MODELS, TEXTURES AND DESCRIPTOR SET LAYOUTS
	//------------------------------------------------------------
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
		TBackToMenu.cleanup();
		TYesButton.cleanup();
		TNoButton.cleanup();
		TButton.cleanup();
		TArrowButtonLeft.cleanup();
		TArrowButtonRight.cleanup();
		TCircleButton.cleanup();
		TPlayButton.cleanup();
		TSelection1.cleanup();
		TSelection2.cleanup();
		TSelection3.cleanup();
		TSelection4.cleanup();
		TLion.cleanup();
		TVase.cleanup();
		TChair.cleanup();
		TPictureFrame.cleanup(); 
		TPictureFrameImage1.cleanup(); 
		TPictureFrameImage2.cleanup();
		TTileSelText.cleanup(); 
		TBoardSelText.cleanup();
		TFlame.cleanup();
		TCandle.cleanup();
		TLamp.cleanup();
		TKettle.cleanup();
		TDoor.cleanup();
		TBlackboardFrame.cleanup();
		TBlackboardBoard.cleanup();
		TBlackboardText.cleanup();

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
		MBackToMenu.cleanup();
		MYesButton.cleanup();
		MNoButton.cleanup();
		MPlainRectangle.cleanup();
		MArrowButton.cleanup();
		MCircleButton.cleanup();
		MLion.cleanup();
		MPictureFrame.cleanup();
		MVase.cleanup();
		MChair.cleanup();
		MFlame.cleanup();
		MCandle.cleanup();
		MLamp.cleanup();
		MKettle.cleanup();
		MDoor.cleanup();
		MBlackboardFrame.cleanup();
		MBlackboardBoard.cleanup();

		// Cleanup descriptor set layouts
		DSLTile.cleanup();
		DSLGeneric.cleanup();
		DSLGubo.cleanup();
		DSLTextureOnly.cleanup();
		DSLPlain.cleanup();

		// Destroy the pipelines
		PTile.destroy();
		PRoughSurfaces.destroy();
		PSmoothSurfaces.destroy();
		PPlain.destroy();
		PPlainWithEmission.destroy();
		PUI.destroy();
	}

	//-----------------------------------
	// CREATION OF THE COMMAND BUFFER
	//-----------------------------------
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {


		// PPlain
		
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
		//Rectangles
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
		DSSelection4.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSPlayButton.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSTileSelText.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);
		DSBoardSelText.bind(commandBuffer, PPlain, 0, currentImage);
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
		//Circle button
		MCircleButton.bind(commandBuffer);
		DSCircleButton.bind(commandBuffer, PPlain, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCircleButton.indices.size()), 1, 0, 0, 0);


		// PTile
		
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
		//Chair
		MChair.bind(commandBuffer);
		DSChair.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MChair.indices.size()), 1, 0, 0, 0);
		//Picture frame image
		MPlainRectangle.bind(commandBuffer); 
		DSPictureFrameImage1.bind(commandBuffer, PRoughSurfaces, 0, currentImage); 
		vkCmdDrawIndexed(commandBuffer, 
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0); 
		DSPictureFrameImage2.bind(commandBuffer, PRoughSurfaces, 0, currentImage); 
		vkCmdDrawIndexed(commandBuffer, 
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0); 
		//Lamp
		MLamp.bind(commandBuffer);
		DSLamp.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MLamp.indices.size()), 1, 0, 0, 0);
		//Door
		MDoor.bind(commandBuffer);
		DSDoor.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MDoor.indices.size()), 1, 0, 0, 0);


		// PSmoothSurfaces
		
		PSmoothSurfaces.bind(commandBuffer);
		DSGubo.bind(commandBuffer, PSmoothSurfaces, 1, currentImage);
		// Lion statue
		MLion.bind(commandBuffer);
		DSLion.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MLion.indices.size()), 1, 0, 0, 0);
		//Picture frame 1
		MPictureFrame.bind(commandBuffer);
		DSPictureFrame1.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPictureFrame.indices.size()), 1, 0, 0, 0);
		//Picture frame 2
		MPictureFrame.bind(commandBuffer);
		DSPictureFrame2.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPictureFrame.indices.size()), 1, 0, 0, 0);
		//Vase
		MVase.bind(commandBuffer);
		DSVase.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MVase.indices.size()), 1, 0, 0, 0);
		//Candle
		MCandle.bind(commandBuffer);
		DSCandle.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MCandle.indices.size()), 1, 0, 0, 0);
		//Kettle
		MKettle.bind(commandBuffer);
		DSKettle.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MKettle.indices.size()), 1, 0, 0, 0);
		//Blackboard
		MBlackboardFrame.bind(commandBuffer);
		DSBlackboardFrame.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MBlackboardFrame.indices.size()), 1, 0, 0, 0);
		MBlackboardBoard.bind(commandBuffer);
		DSBlackboardBoard.bind(commandBuffer, PSmoothSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MBlackboardBoard.indices.size()), 1, 0, 0, 0);


		// PRoughSurfaces
		
		PRoughSurfaces.bind(commandBuffer);
		DSGubo.bind(commandBuffer, PRoughSurfaces, 1, currentImage);
		//Blackboard commands text
		MPlainRectangle.bind(commandBuffer);
		DSBlackboardText.bind(commandBuffer, PRoughSurfaces, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MPlainRectangle.indices.size()), 1, 0, 0, 0);


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
		// Back to menu
		MBackToMenu.bind(commandBuffer);
		DSBackToMenu.bind(commandBuffer, PUI, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MBackToMenu.indices.size()), 1, 0, 0, 0);
		// Back to menu
		MYesButton.bind(commandBuffer);
		DSYesButton.bind(commandBuffer, PUI, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MYesButton.indices.size()), 1, 0, 0, 0);
		// Back to menu
		MNoButton.bind(commandBuffer);
		DSNoButton.bind(commandBuffer, PUI, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MNoButton.indices.size()), 1, 0, 0, 0);


		// PPlainWithEmission

		PPlainWithEmission.bind(commandBuffer);
		// Flame
		MFlame.bind(commandBuffer);
		DSFlame.bind(commandBuffer, PPlainWithEmission, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MFlame.indices.size()), 1, 0, 0, 0);
	}

	//---------------------
	// MAIN UPDATE CYCLE
	//---------------------
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
		bool mButton = false;
		getSixAxis(deltaT, m, r, fire, click, enter, mButton); 

		// To debounce the pressing of the fire button, and start the event when the key is released
		static bool wasFire = false;
		bool handleFire = (wasFire && (!fire));
		wasFire = fire;

		// To debounce the pressing of the mouse left click, and start the event when the key is released
		static bool wasClick = false; 
		bool handleClick = (wasClick && (!click)); 
		wasClick = click; 

		// To get the position of the cursor on screen
		double mousex, mousey;
		glfwGetCursorPos(window, &mousex, &mousey);
		int x = int(mousex);
		int y = int(mousey);


		// Access texture data for object selection
		void* data;
		vkMapMemory(device, entityImageMemory, 0, VK_WHOLE_SIZE, 0, &data); // Put entityImageMemory in &data
		vkGetImageSubresourceLayout(device, entityImage, &entitySubresource, &entityLayout); // Get info from entityImage
		
		// Access &data to read pointed pixel content
		int* pixels = reinterpret_cast<int*>(data);
		int index = y * entityLayout.rowPitch/4 + x;
		int hoverIndex = -1;
		if (y < windowHeight && x < windowWidth) {
			hoverIndex = pixels[index];
		}
		vkUnmapMemory(device, entityImageMemory);


		// Initialization of the game
		string structurePath = "./structure.json";
		static MahjongGame game = MahjongGame(structurePath);
		static bool reset = false;


		//--------------------------
		// STATE MACHINE FOR THE GAME
		// -------------------------
		bool enterPressedFirstTime = false;
		switch (gameState) {
			
			case -1: //menu	

				if (reset) {
					game = MahjongGame(structurePath);
					boardTextureIdx = 0;
					tileTextureIdx = 0;
					circleTextureIdx = 0;
					reset = false;
				}

				// Get clicks to change textures and shaders
				// Change tiles texture
				if (handleClick && hoverIndex==-42) {
					tileTextureIdx++;
					if (tileTextureIdx == 4) tileTextureIdx = 0;
					PlaySound(TEXT("sounds/button_click.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				if(handleClick && hoverIndex == -41) {
					tileTextureIdx--;
					if (tileTextureIdx == -1) tileTextureIdx = 3;
					PlaySound(TEXT("sounds/button_click.wav"), NULL, SND_FILENAME | SND_ASYNC); 
				}

				// Change board texture
				if (handleClick && hoverIndex == -44) {
					boardTextureIdx++;
					if (boardTextureIdx == 4) boardTextureIdx = 0;
					PlaySound(TEXT("sounds/button_click.wav"), NULL, SND_FILENAME | SND_ASYNC); 
				}
				if (handleClick && hoverIndex == -43) {
					boardTextureIdx--;
					if (boardTextureIdx == -1) boardTextureIdx = 3;
					PlaySound(TEXT("sounds/button_click.wav"), NULL, SND_FILENAME | SND_ASYNC); 
				}

				//Change day/Night
				if (handleClick && hoverIndex == -45) {
					circleTextureIdx++;
					if (circleTextureIdx == 2) circleTextureIdx = 0;
					PlaySound(TEXT("sounds/button_click.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}

				//Start the game
				if (handleClick && hoverIndex == -30) {
					gameState = 0;

					//random gen of the index to use to chose the picture for the picture frame
					int max = 3;
					int min = 0;
					std::mt19937 rng(time(NULL));
					std::uniform_int_distribution<int> gen(min, max);
					pictureFrameImageIdx1 = gen(rng);

					max = 4;
					std::mt19937 rng2(time(NULL));
					std::uniform_int_distribution<int> gen2(min, max);
					pictureFrameImageIdx2 = gen2(rng);

					PlaySound(TEXT("sounds/button_click.wav"), NULL, SND_FILENAME | SND_ASYNC);

					enterPressedFirstTime = true;
				}
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
					if (hoverIndex != firstTileIndex) {
						secondTileIndex = hoverIndex;
						gameState = 2;
					} 
					else {
						firstTileIndex = -1;
						gameState = 0;
					}
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
				//notify error
				PlaySound(TEXT("sounds/game_error_tone_1.wav"), NULL, SND_FILENAME | SND_ASYNC);
				//deselect tiles
				firstTileIndex = -1;
				secondTileIndex = -1;
				gameState = 0;
				break;
			case 4:
				//two pieces start to disappear
				DisappearingTileTransparency = DisappearingTileTransparency - 2.5f * deltaT;
				if (DisappearingTileTransparency <= 0) {
					DisappearingTileTransparency = 0;
					gameState = 5;
				}
				break;
			case 5:
				//remove the tile
				game.removeTiles(firstTileIndex, secondTileIndex);
				if (game.isWon() || game.isGameOver()) {
					gameState = 6;
				}
				else {
					gameState = 0;
				}
				break;
			case 6:
				// The game has ended
				if (game.isWon()) {
					youwinubo.visible = 1.0f;
					gameoverubo.visible = 0.0f;
					PlaySound(TEXT("sounds/clapping_people.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				else if (game.isGameOver()) {
					gameoverubo.visible = 1.0f;
					youwinubo.visible = 0.0f;
					PlaySound(TEXT("sounds/retro_error_long_tone.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				gameState = 7;
				break;
			case 7: 
				// PRESS ENTER TO GO BACK TO MENU
				if (enter) {
					// Go back to menu
					gameoverubo.visible = 0.0f;
					youwinubo.visible = 0.0f;
					boardTextureIdx = 0;
					tileTextureIdx = 0;
					circleTextureIdx = 0;
					gameState = -1;
					reset = true;
				}
				break;
			case 8:	// screen to go back to menu
				youwinubo.visible = 0.0f;
				gameoverubo.visible = 0.0f;
				backtomenuubo.visible = 1.0f;
				yesbuttonubo.visible = 1.0f;
				nobuttonubo.visible = 1.0f;
				if (handleClick && (hoverIndex == -2 || hoverIndex == -3)) {
					backtomenuubo.visible = 0.0f;
					yesbuttonubo.visible = 0.0f;
					nobuttonubo.visible = 0.0f;
					if (hoverIndex == -2) {	//yes
						gameState = -1;
						reset = true;
						boardTextureIdx = 0;
						tileTextureIdx = 0;
						circleTextureIdx = 0;
					}
					else firstTileIndex != -1 ? gameState = 1 : gameState = 0;
				}
		}
		if ((gameState==1 || gameState== 0) && mButton) {
			gameState = 8;
		}

		//---------------------------
		//CAMERA SETTINGS
		//---------------------------

		//Change position accoring to received commands
		CamH += m.z * movSpeed * deltaT;
		CamRadius -= m.x * movSpeed * deltaT;
		CamRadius = glm::clamp(CamRadius, 0.20f, 1.5f); //minumum and maximum zoom of the cam

		CamPitch -= r.x * rotSpeed * deltaT;
		CamPitch = glm::clamp(CamPitch, glm::radians(-10.0f), glm::radians(89.0f)); //constraints on degrees on elevation of the cam 

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
			CamRadius = 4.0f;
			CamPitch = 0.0f;
			CamYaw = 0.0f;
		}


		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;

		//a
		glm::vec3 camTarget = glm::vec3(0, 0.6f, -0.5);
		if (gameState == -1) {
			camTarget = homeMenuPosition + glm::vec3(0, 1.2f, 0);
		}

		//c
		glm::vec3 camPos = camTarget + CamRadius * glm::vec3(cos(CamPitch) * sin(CamYaw), sin(CamPitch), cos(CamPitch) * cos(CamYaw));


		glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));


		//--------------------------
		// BUFFER FILLING
		//--------------------------

		// Useful saved positions
		// Candle+Flame Position
		glm::vec3 candlePos = glm::vec3(0.35f, 0.6f, -0.7f);
		glm::vec3 candleLightPos = candlePos + glm::vec3(0.0f, 0.115f, 0.0f); 
		// Day lantern point light position
		glm::vec3 lanternLightPos = glm::vec3(0.0f, 2.4f, 0.0f);
		// Picture frame position
		glm::mat4 pictureFramePosition = glm::translate(glm::mat4(1), glm::vec3(1.96f, 1.75f, 0.3f));


		// Day/Night parameter setting + Gubo filling
		bool isNight = circleTextureIdx;
		if (isNight) {
			isCandleAlight = 1;
			generalSColor = glm::vec3(239.0f/255.0f, 192.0f/255.0f, 112.0f/255.0f); //Cold light color
			lampTextureIdx = 0;
			landscapeTextureIdx = 1;
			gubo.PlightPos = candleLightPos;
			gubo.PlightColor = glm::vec3(239.0f/255.0f, 192.0f/255.0f, 112.0f/255.0f); //Cold light color
			gubo.beta = 1.6f;
			gubo.g = 0.6f;
			gubo.AmbLightColor = glm::vec3(0.01f);
		}
		else {
			isCandleAlight = 0;
			generalSColor = glm::vec3(1.0f, 1.0f, 1.0f);
			lampTextureIdx = 1;
			landscapeTextureIdx = 0;
			gubo.PlightPos = lanternLightPos;
			gubo.PlightColor = glm::vec3(255.0f/255.0f, 252.0f/255.0f, 221.0f/255.0f); //Yellow-ish white
			gubo.beta = 1.0f;
			gubo.g = 1.3f;
			gubo.AmbLightColor = glm::vec3(0.01f);
		}
		gubo.eyePos = camPos;
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
		// Back to menu message
		backtomenuubo.transparency = 1.0f;
		backtomenuubo.objectIdx = -1;
		DSBackToMenu.map(currentImage, &backtomenuubo, sizeof(backtomenuubo), 0);
		// Back to menu message
		yesbuttonubo.transparency = 1.0f;
		yesbuttonubo.objectIdx = -2;
		DSYesButton.map(currentImage, &yesbuttonubo, sizeof(yesbuttonubo), 0);
		// Back to menu message
		nobuttonubo.transparency = 1.0f;
		nobuttonubo.objectIdx = -3;
		DSNoButton.map(currentImage, &nobuttonubo, sizeof(nobuttonubo), 0);

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
		// [24] - Lion statue
		// [25] - Picture frame 1
		// [26] - Picture frame image 1
		// [27] - Vase
		// [28] - Chair
		// [29] - Picture frame 2
		// [30] - Picture frame image 2
		// [31] - Flame
		// [32] - Candle
		// [33] - day/night time selection title
		// [34] - circular day/night button
		// [35] - Lamp
		// [36] - Kettle
		// [37] - Door
		// [38] - Blackboard Frame
		// [39] - Blackboard Board
		// [40] - Blackboard Text

		glm::mat4 translateUp = glm::translate(glm::mat4(2.0f), glm::vec3(0.0f, 1.5f, 0.0f));

		// Home screen background
		glm::mat4 WorldH = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.5f, 0.0f)) * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 4.0f);
		commonubo[9].mvpMat = Prj * View * WorldH;
		commonubo[9].mMat = WorldH;
		commonubo[9].nMat = glm::inverse(glm::transpose(WorldH));
		commonubo[9].transparency = 0.0f;
		commonubo[9].textureIdx = boardTextureIdx;
		DSHome.map(currentImage, &commonubo[9], sizeof(commonubo[9]), 0);

		// Button1
		glm::mat4 WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.15f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.3f);
		commonubo[11].mvpMat = Prj * View * WorldB;
		commonubo[11].mMat = WorldB;
		commonubo[11].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[11].transparency = 1.0f;
		commonubo[11].textureIdx = 0;
		DSButton1.map(currentImage, &commonubo[11], sizeof(commonubo[11]), 0);

		// Tile Selection Text
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.15f, 0.13f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.3f);
		tileSelTextubo.mvpMat = Prj * View * WorldB;
		tileSelTextubo.mMat = WorldB;
		tileSelTextubo.nMat = glm::inverse(glm::transpose(WorldB));
		tileSelTextubo.transparency = 1.0f;
		tileSelTextubo.textureIdx = tileTextureIdx;
		DSTileSelText.map(currentImage, &tileSelTextubo, sizeof(tileSelTextubo), 0);

		// Button2
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.8f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.3f);
		commonubo[12].mvpMat = Prj * View * WorldB;
		commonubo[12].mMat = WorldB;
		commonubo[12].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[12].transparency = 1.0f;
		commonubo[12].textureIdx = 0;
		DSButton2.map(currentImage, &commonubo[12], sizeof(commonubo[12]), 0);

		// Board Selection Text
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.8f, 0.13f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.3f);
		boardSelTextubo.mvpMat = Prj * View * WorldB;
		boardSelTextubo.mMat = WorldB;
		boardSelTextubo.nMat = glm::inverse(glm::transpose(WorldB));
		boardSelTextubo.transparency = 1.0f;
		boardSelTextubo.textureIdx = boardTextureIdx;
		DSBoardSelText.map(currentImage, &boardSelTextubo, sizeof(boardSelTextubo), 0);

		/*//Button3
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.0f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[13].mvpMat = Prj * View * WorldB;
		commonubo[13].mMat = WorldB;
		commonubo[13].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[13].transparency = 1.0f;
		commonubo[13].textureIdx = 0;
		DSButton3.map(currentImage, &commonubo[13], sizeof(commonubo[13]), 0);*/

		// Arrow button 1 Left
		glm::mat4 WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(0.35f, 0.0f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[14].mvpMat = Prj * View * WorldA_B;
		commonubo[14].mMat = WorldA_B;
		commonubo[14].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[14].transparency = 1.0f;
		commonubo[14].textureIdx = 0;
		commonubo[14].objectIdx = -41;
		DSArrowButton1_left.map(currentImage, &commonubo[14], sizeof(commonubo[14]), 0);

		// Arrow button 2 Left
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(0.35f, -1.65f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[15].mvpMat = Prj * View * WorldA_B;
		commonubo[15].mMat = WorldA_B;
		commonubo[15].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[15].transparency = 1.0f;
		commonubo[15].textureIdx = 0;
		commonubo[15].objectIdx = -43;
		DSArrowButton2_left.map(currentImage, &commonubo[15], sizeof(commonubo[15]), 0);

		// Day/night button
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(1.05f, -3.2f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[34].mvpMat = Prj * View * WorldA_B;
		commonubo[34].mMat = WorldA_B;
		commonubo[34].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[34].transparency = 1.0f;
		commonubo[34].textureIdx = circleTextureIdx;
		commonubo[34].objectIdx = -45;
		DSCircleButton.map(currentImage, &commonubo[34], sizeof(commonubo[34]), 0);

		// Arrow button 1 Right
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(3.7f, 0.0f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[17].mvpMat = Prj * View * WorldA_B;
		commonubo[17].mMat = WorldA_B;
		commonubo[17].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[17].transparency = 1.0f;
		commonubo[17].textureIdx = 0;
		commonubo[17].objectIdx = -42;
		DSArrowButton1_right.map(currentImage, &commonubo[17], sizeof(commonubo[17]), 0);

		// Arrow button 2 Right
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(3.7f, -1.65f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[18].mvpMat = Prj * View * WorldA_B;
		commonubo[18].mMat = WorldA_B;
		commonubo[18].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[18].transparency = 1.0f;
		commonubo[18].textureIdx = 0;
		commonubo[18].objectIdx = -44;
		DSArrowButton2_right.map(currentImage, &commonubo[18], sizeof(commonubo[18]), 0);

		/*//Arrow button 3 Right
		WorldA_B = glm::translate(glm::mat4(1.0f), glm::vec3(3.5f, -1.0f, 0.11f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.0f);
		commonubo[19].mvpMat = Prj * View * WorldA_B;
		commonubo[19].mMat = WorldA_B;
		commonubo[19].nMat = glm::inverse(glm::transpose(WorldA_B));
		commonubo[19].transparency = 1.0f;
		commonubo[19].textureIdx = 0;
		DSArrowButton3_right.map(currentImage, &commonubo[19], sizeof(commonubo[19]), 0);*/

		// Play button
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(-2.7f, -3.6f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.6f);
		commonubo[20].mvpMat = Prj * View * WorldB;
		commonubo[20].mMat = WorldB;
		commonubo[20].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[20].transparency = 1.0f;
		commonubo[20].textureIdx = 0;
		commonubo[20].objectIdx = -30;
		DSPlayButton.map(currentImage, &commonubo[20], sizeof(commonubo[20]), 0); 
		
		// Game settings title
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.7f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1.2f, 0.5f, 1.0f) * 1.4f);
		commonubo[21].mvpMat = Prj * View * WorldB;
		commonubo[21].mMat = WorldB;
		commonubo[21].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[21].transparency = 1.0f;
		commonubo[21].textureIdx = 0;
		DSSelection1.map(currentImage, &commonubo[21], sizeof(commonubo[21]), 0);

		// Tile selection title 
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 1.01f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1.2f, 0.5f, 1.0f) * 0.8f);
		commonubo[22].mvpMat = Prj * View * WorldB;
		commonubo[22].mMat = WorldB;
		commonubo[22].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[22].transparency = 1.0f;
		commonubo[22].textureIdx = 0;
		DSSelection2.map(currentImage, &commonubo[22], sizeof(commonubo[22]), 0);

		// Board selection title
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.6f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1.3f, 0.5f, 1.0f) * 0.8f);
		commonubo[23].mvpMat = Prj * View * WorldB;
		commonubo[23].mMat = WorldB;
		commonubo[23].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[23].transparency = 1.0f;
		commonubo[23].textureIdx = 0;
		DSSelection3.map(currentImage, &commonubo[23], sizeof(commonubo[23]), 0);

		// day/night time selection title
		WorldB = glm::translate(glm::mat4(1.0f), glm::vec3(2.65f, -3.1f, 0.12f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1.0f) * 0.9f);
		commonubo[33].mvpMat = Prj * View * WorldB;
		commonubo[33].mMat = WorldB;
		commonubo[33].nMat = glm::inverse(glm::transpose(WorldB));
		commonubo[33].transparency = 1.0f;
		commonubo[33].textureIdx = 0;
		DSSelection4.map(currentImage, &commonubo[33], sizeof(commonubo[33]), 0);
		 
		// Matrix setup for rotating tile
		static float ang = 0.0f;
		ang += homeTileRotSpeed * deltaT;
		tileHomeubo.transparency = 1.0f;
		glm::mat4 rotTileW = translateUp * homeMenuWorld *
			glm::translate(glm::mat4(1), glm::vec3(-2.4f, -0.3f, 0.5f)) *
			glm::rotate(glm::mat4(1), glm::radians(-80.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1), glm::sin(ang)+0.5f, glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(40.0f)) *
			glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::translate(glm::mat4(1), glm::vec3(0.0f, -0.00675f, 0.0f));
		tileHomeubo.amb = 10.0f;
		tileHomeubo.gamma = 300.0f;
		tileHomeubo.sColor = glm::vec3(0.1f);
		tileHomeubo.mvpMat = Prj * View * rotTileW;
		tileHomeubo.mMat = rotTileW;
		tileHomeubo.nMat = glm::inverse(glm::transpose(rotTileW));
		tileHomeubo.tileIdx = -2;
		tileHomeubo.suitIdx = 10;
		tileHomeubo.selectedIdx = -10;
		tileHomeubo.hoverIdx = -10;
		tileHomeubo.textureIdx = tileTextureIdx;
		tileHomeubo.isInMenu = 1;
		DSHTile.map(currentImage, &tileHomeubo, sizeof(tileHomeubo), 0);

		// Matrix setup for Game Title
		glm::mat4 WorldTitle = glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, -1.0f, 0.1f)) * translateUp * homeMenuWorld * glm::scale(glm::mat4(1), glm::vec3(1) * 1.6f);
			//* glm::scale(glm::mat4(1), glm::vec3(-1.0f, 1.0f, -1.0f));
		commonubo[10].mvpMat = Prj * View * WorldTitle;
		commonubo[10].mMat = WorldTitle;
		commonubo[10].nMat = glm::inverse(glm::transpose(WorldTitle));
		commonubo[10].transparency = 1.0f;
		commonubo[10].textureIdx = 0;
		DSGameTitle.map(currentImage, &commonubo[10], sizeof(commonubo[10]), 0);
		
		// Matrix setup for background
		glm::mat4 World = glm::mat4(1);
		glm::mat4 baseTranslation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -0.5f));
		World = baseTranslation * 
				glm::translate(glm::mat4(1), glm::vec3(0.04f, 0.0f, 0.0f)) * 
				glm::scale(glm::mat4(1), glm::vec3(3.55f, 1.0f, 1.4f));
		bgubo.amb = 1.2f; bgubo.sigma = 0.7f;
		commonubo[0].mvpMat = Prj * View * World;
		commonubo[0].mMat = World;
		commonubo[0].nMat = glm::inverse(glm::transpose(World));
		commonubo[0].transparency = 0.0f;
		commonubo[0].textureIdx = boardTextureIdx;
		DSBackground.map(currentImage, &commonubo[0], sizeof(commonubo[0]), 0);
		DSBackground.map(currentImage, &bgubo, sizeof(bgubo), 1);

		// Matrix setup for walls
		World = glm::mat4(1);
		wallubo.amb = 1.2f; wallubo.sigma = 0.7f;
		commonubo[1].mvpMat = Prj * View * World;
		commonubo[1].mMat = World;
		commonubo[1].nMat = glm::inverse(glm::transpose(World));
		commonubo[1].transparency = 0.0f;
		commonubo[1].textureIdx = 0;
		DSWall.map(currentImage, &commonubo[1], sizeof(commonubo[1]), 0);
		DSWall.map(currentImage, &wallubo, sizeof(wallubo), 1);

		// Matrix setup for floor
		World = glm::mat4(1);
		floorubo.amb = 1.2f; floorubo.sigma = 0.7f;
		commonubo[3].mvpMat = Prj * View * World;
		commonubo[3].mMat = World;
		commonubo[3].nMat = glm::inverse(glm::transpose(World));
		commonubo[3].transparency = 0.0f;
		commonubo[3].textureIdx = 0;
		DSFloor.map(currentImage, &commonubo[3], sizeof(commonubo[3]), 0);
		DSFloor.map(currentImage, &floorubo, sizeof(floorubo), 1);

		// Matrix setup for ceiling
		World = glm::mat4(1);
		ceilingubo.amb = 1.0f; ceilingubo.sigma = 0.7f;
		commonubo[2].mvpMat = Prj * View * World;
		commonubo[2].mMat = World;
		commonubo[2].nMat = glm::inverse(glm::transpose(World));
		commonubo[2].transparency = 0.0f;
		commonubo[2].textureIdx = 0;
		DSCeiling.map(currentImage, &commonubo[2], sizeof(commonubo[2]), 0);
		DSCeiling.map(currentImage, &ceilingubo, sizeof(ceilingubo), 1);

		// Matrix setup for table
		World = baseTranslation;
		tableubo.amb = 25.0f; tableubo.sigma = 0.7f;
		commonubo[4].mvpMat = Prj * View * World;
		commonubo[4].mMat = World;
		commonubo[4].nMat = glm::inverse(glm::transpose(World));
		commonubo[4].transparency = 0.0f;
		commonubo[4].textureIdx = 0;
		DSTable.map(currentImage, &commonubo[4], sizeof(commonubo[4]), 0);
		DSTable.map(currentImage, &tableubo, sizeof(tableubo), 1);

		// Matrix setup for windows
		// Window 1
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 1.5f, -2.0f));
		window1ubo.amb = 1.0f; window1ubo.sigma = 0.9f;
		commonubo[5].mvpMat = Prj * View * World;
		commonubo[5].mMat = World;
		commonubo[5].nMat = glm::inverse(glm::transpose(World));
		commonubo[5].transparency = 1.0f;
		commonubo[6].textureIdx = 0;
		DSWindow1.map(currentImage, &commonubo[5], sizeof(commonubo[5]), 0);
		DSWindow1.map(currentImage, &window1ubo, sizeof(window1ubo), 1);
		// Window 2
		World = glm::translate(glm::mat4(1), glm::vec3(-1.0f, 1.5f, -2.0f));
		window2ubo.amb = 1.0f; window2ubo.sigma = 0.9f;
		commonubo[6].mvpMat = Prj * View * World;
		commonubo[6].mMat = World;
		commonubo[6].nMat = glm::inverse(glm::transpose(World));
		commonubo[6].transparency = 1.0f;
		DSWindow2.map(currentImage, &commonubo[6], sizeof(commonubo[6]), 0);
		DSWindow2.map(currentImage, &window2ubo, sizeof(window2ubo), 1);
		// Window 3
		World = glm::translate(glm::mat4(1), glm::vec3(1.0f, 1.5f, -2.0f));
		window3ubo.amb = 1.0f; window3ubo.sigma = 0.9f;
		commonubo[7].mvpMat = Prj * View * World;
		commonubo[7].mMat = World;
		commonubo[7].nMat = glm::inverse(glm::transpose(World));
		commonubo[7].transparency = 1.0f;
		commonubo[7].textureIdx = 0;
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
		commonubo[8].textureIdx = landscapeTextureIdx;
		DSLandscape.map(currentImage, &commonubo[8], sizeof(commonubo[8]), 0);

		// Lion statue
		World = glm::translate(glm::mat4(1), glm::vec3(-1.4f, 0.0f, 1.2f)) *
				glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1), glm::vec3(0.7));
		commonubo[24].mvpMat = Prj * View * World;
		commonubo[24].mMat = World;
		commonubo[24].nMat = glm::inverse(glm::transpose(World));
		commonubo[24].transparency = 0.0f;
		commonubo[24].textureIdx = 0;
		lionubo.amb = 1.0f; lionubo.gamma = 200.0f; lionubo.sColor = glm::vec3(1.0f, 1.0f, 1.0f);
		if (isNight) {
			lionubo.amb = 0.01f; lionubo.gamma = 10000.0f;
		}
		else {
			lionubo.amb = 1.0f; lionubo.gamma = 200.0f;
		}
		lionubo.sColor = generalSColor;
		DSLion.map(currentImage, &commonubo[24], sizeof(commonubo[24]), 0);
		DSLion.map(currentImage, &lionubo, sizeof(lionubo), 1);

		// Picture frame 1
		World = pictureFramePosition *
			glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
			glm::scale(glm::mat4(1), glm::vec3(0.5)); 
		commonubo[25].mvpMat = Prj * View * World; 
		commonubo[25].mMat = World; 
		commonubo[25].nMat = glm::inverse(glm::transpose(World)); 
		commonubo[25].transparency = 0.0f; 
		commonubo[25].textureIdx = 0; 
		if (isNight) {
			pictureFrameubo1.amb = 0.01f; pictureFrameubo1.gamma = 10000.0f;
		}
		else {
			pictureFrameubo1.amb = 1.0f; pictureFrameubo1.gamma = 200.0f;
		}
		pictureFrameubo1.sColor = generalSColor;
		DSPictureFrame1.map(currentImage, &commonubo[25], sizeof(commonubo[25]), 0);
		DSPictureFrame1.map(currentImage, &pictureFrameubo1, sizeof(pictureFrameubo1), 1);

		// Picture frame Image 1
		World = pictureFramePosition * glm::translate(glm::mat4(1), glm::vec3(0.0f, -0.26f, -0.015f)) *
			glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.47f)) * glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.2f, 1.0f)) * 
			glm::scale(glm::mat4(1), glm::vec3(-1.0f, 1.0f, -1.0f));
		commonubo[26].mvpMat = Prj * View * World;
		commonubo[26].mMat = World;
		commonubo[26].nMat = glm::inverse(glm::transpose(World));
		commonubo[26].transparency = 0.0f;
		commonubo[26].textureIdx = pictureFrameImageIdx1;
		pictureFrameImageubo1.amb = 20.0f; pictureFrameImageubo1.sigma = 1.1f;
		DSPictureFrameImage1.map(currentImage, &commonubo[26], sizeof(commonubo[26]), 0);
		DSPictureFrameImage1.map(currentImage, &pictureFrameImageubo1, sizeof(pictureFrameImageubo1), 1);

		// Picture frame 2
		World = pictureFramePosition * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.3f, -1.5f)) *
			glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.5));
		commonubo[29].mvpMat = Prj * View * World;
		commonubo[29].mMat = World;
		commonubo[29].nMat = glm::inverse(glm::transpose(World));
		commonubo[29].transparency = 0.0f;
		commonubo[29].textureIdx = 0;
		if (isNight) {
			pictureFrameubo2.amb = 0.01f; pictureFrameubo2.gamma = 10000.0f;
		}
		else {
			pictureFrameubo2.amb = 1.0f; pictureFrameubo2.gamma = 200.0f;
		}
		pictureFrameubo2.sColor = generalSColor;
		DSPictureFrame2.map(currentImage, &commonubo[29], sizeof(commonubo[29]), 0);
		DSPictureFrame2.map(currentImage, &pictureFrameubo2, sizeof(pictureFrameubo2), 1);

		// Picture frame Image 2
		World = pictureFramePosition * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.3f, -1.5f)) *
			glm::translate(glm::mat4(1), glm::vec3(0.0f, -0.26f, -0.015f)) *
			glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.47f)) * glm::scale(glm::mat4(1), glm::vec3(1.0f, 1.2f, 1.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(-1.0f, 1.0f, -1.0f));
		commonubo[30].mvpMat = Prj * View * World;
		commonubo[30].mMat = World;
		commonubo[30].nMat = glm::inverse(glm::transpose(World));
		commonubo[30].transparency = 0.0f;
		commonubo[30].textureIdx = pictureFrameImageIdx2;
		pictureFrameImageubo2.amb = 20.0f; pictureFrameImageubo2.sigma = 1.1f;
		DSPictureFrameImage2.map(currentImage, &commonubo[30], sizeof(commonubo[30]), 0);
		DSPictureFrameImage2.map(currentImage, &pictureFrameImageubo2, sizeof(pictureFrameImageubo2), 1);

		// Vase
		World = glm::translate(glm::mat4(1), glm::vec3(1.5f, 0.0f, -1.8f)) *
			glm::rotate(glm::mat4(1), glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.016f));
		commonubo[27].mvpMat = Prj * View * World;
		commonubo[27].mMat = World;
		commonubo[27].nMat = glm::inverse(glm::transpose(World));
		commonubo[27].transparency = 0.0f;
		commonubo[27].textureIdx = 0;
		if (isNight) {
			vaseubo.amb = 0.0001f; vaseubo.gamma = 10000.0f;
		}
		else {
			vaseubo.amb = 1.0f; vaseubo.gamma = 200.0f;
		}
		vaseubo.sColor = generalSColor;
		DSVase.map(currentImage, &commonubo[27], sizeof(commonubo[27]), 0);
		DSVase.map(currentImage, &vaseubo, sizeof(vaseubo), 1);

		// Chair
		World = glm::translate(glm::mat4(1), glm::vec3(0.15f, -0.1f, 0.3f)) *
			glm::rotate(glm::mat4(1), glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.7f));
		commonubo[28].mvpMat = Prj * View * World;
		commonubo[28].mMat = World;
		commonubo[28].nMat = glm::inverse(glm::transpose(World));
		commonubo[28].transparency = 0.0f;
		commonubo[28].textureIdx = 0;
		chairubo.amb = 20.0f; chairubo.sigma = 0.5f;
		DSChair.map(currentImage, &commonubo[28], sizeof(commonubo[28]), 0);
		DSChair.map(currentImage, &chairubo, sizeof(chairubo), 1);

		// Door
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 2.0f)) *
			glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(1.2f, 1.0f, 1.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.8f));
		commonubo[37].mvpMat = Prj * View * World; 
		commonubo[37].mMat = World;
		commonubo[37].nMat = glm::inverse(glm::transpose(World));
		commonubo[37].transparency = 0.0f;
		commonubo[37].textureIdx = 0;
		doorubo.amb = 20.0f; doorubo.sigma = 0.5f;
		DSDoor.map(currentImage, &commonubo[37], sizeof(commonubo[37]), 0);
		DSDoor.map(currentImage, &doorubo, sizeof(doorubo), 1);


		// Flame
		std::mt19937 rngFlame(time(NULL));
		std::uniform_int_distribution<int> genScaleDiff(8, 12);
		float scaleDiff = genScaleDiff(rngFlame)/10.0f;
		std::uniform_int_distribution<int> genRotDiff(0, 180);
		float rotationDiff = genRotDiff(rngFlame);
		std::uniform_int_distribution<int> genEmissionPicker(0, 3);
		int emissionPicker = genEmissionPicker(rngFlame);
		glm::vec3 emissionColors[4] = {
			glm::vec3(235.0f/255.0f, 103.0f/255.0f, 52.0f/255.0f),		//Dark orange
			glm::vec3(245.0f/255.0f, 2.0f/255.0f, 2.0f/255.0f),			//Red
			glm::vec3(232.0f/255.0f, 65.0f/255.0f, 19.0f/255.0f),		//Lighter red
			glm::vec3(245.0f/255.0f, 136.0f/255.0f, 2.0f/255.0f),		// Light orange
		};
		glm::vec3 chosenEmissionColor = emissionColors[emissionPicker];
		std::uniform_int_distribution<int> genshearCoeff(-2, 2);
		float shearhx = genshearCoeff(rngFlame)/10.0f;
		float shearhz = genshearCoeff(rngFlame)/10.0f;
		World = glm::translate(glm::mat4(1), candleLightPos) * //glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.115f, 0.0f)) *
			glm::rotate(glm::mat4(1), glm::radians(rotationDiff), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.16f)) *
			glm::scale(glm::mat4(1), glm::vec3(float(isCandleAlight))) *
			glm::scale(glm::mat4(1), glm::vec3(1.0f, scaleDiff, 1.0f)) *
			glm::shearY3D(glm::mat4(1), shearhx, shearhz);
		commonubo[31].mvpMat = Prj * View * World;
		commonubo[31].mMat = World;
		commonubo[31].nMat = glm::inverse(glm::transpose(World));
		commonubo[31].transparency = 0.0f;
		commonubo[31].textureIdx = 0;
		flameEmissionubo.emission = chosenEmissionColor;
		DSFlame.map(currentImage, &commonubo[31], sizeof(commonubo[31]), 0);
		DSFlame.map(currentImage, &flameEmissionubo, sizeof(flameEmissionubo), 1);

		// Candle
		World = glm::translate(glm::mat4(1), candlePos) *
			glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.055f));
		commonubo[32].mvpMat = Prj * View * World;
		commonubo[32].mMat = World;
		commonubo[32].nMat = glm::inverse(glm::transpose(World));
		commonubo[32].transparency = 0.0f;
		commonubo[32].textureIdx = 0;
		if (isNight) {
			candleubo.amb = 1.5f; candleubo.gamma = 1000.0f;
		}
		else {
			candleubo.amb = 1.0f; candleubo.gamma = 200.0f;
		}
		
		if (isCandleAlight) candleubo.sColor = chosenEmissionColor;	//change specular color according to emitted color by the candle light
		else candleubo.sColor = glm::vec3(1.0f, 1.0f, 1.0f);
		DSCandle.map(currentImage, &commonubo[32], sizeof(commonubo[32]), 0);
		DSCandle.map(currentImage, &candleubo, sizeof(candleubo), 1);

		// Kettle
		World = glm::translate(glm::mat4(1), glm::vec3(-0.4f, 0.6f, -0.5f)) *
			glm::rotate(glm::mat4(1), glm::radians(235.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.4f));
		commonubo[36].mvpMat = Prj * View * World;
		commonubo[36].mMat = World;
		commonubo[36].nMat = glm::inverse(glm::transpose(World));
		commonubo[36].transparency = 0.0f;
		commonubo[36].textureIdx = 0;
		if (isNight) {
			kettleubo.amb = 0.0001f; kettleubo.gamma = 10000.0f;
		}
		else {
			kettleubo.amb = 1.0f; kettleubo.gamma = 200.0f;
		}
		kettleubo.sColor = generalSColor;
		DSKettle.map(currentImage, &commonubo[36], sizeof(commonubo[36]), 0);
		DSKettle.map(currentImage, &kettleubo, sizeof(kettleubo), 1);

		// Blackboard
		World = glm::translate(glm::mat4(1), glm::vec3(-2.0f, 1.3f, -0.5f)) *
			glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.008f));
		commonubo[38].mvpMat = Prj * View * World;
		commonubo[38].mMat = World;
		commonubo[38].nMat = glm::inverse(glm::transpose(World));
		commonubo[38].transparency = 0.0f;
		commonubo[38].textureIdx = 0;
		if (isNight) {
			blackboardFrameubo.amb = 0.0001f; blackboardFrameubo.gamma = 10000.0f;
		}
		else {
			blackboardFrameubo.amb = 1.0f; blackboardFrameubo.gamma = 200.0f;
		}
		blackboardFrameubo.sColor = 0.2f*generalSColor;
		DSBlackboardFrame.map(currentImage, &commonubo[38], sizeof(commonubo[38]), 0);
		DSBlackboardFrame.map(currentImage, &blackboardFrameubo, sizeof(blackboardFrameubo), 1);
		commonubo[39].mvpMat = Prj * View * World;
		commonubo[39].mMat = World;
		commonubo[39].nMat = glm::inverse(glm::transpose(World));
		commonubo[39].transparency = 0.0f;
		commonubo[39].textureIdx = 0;
		if (isNight) {
			blackboardBoardubo.amb = 0.0001f; blackboardBoardubo.gamma = 10000.0f;
		}
		else {
			blackboardBoardubo.amb = 1.0f; blackboardBoardubo.gamma = 200.0f;
		}
		blackboardBoardubo.sColor = generalSColor;
		DSBlackboardBoard.map(currentImage, &commonubo[39], sizeof(commonubo[39]), 0);
		DSBlackboardBoard.map(currentImage, &blackboardBoardubo, sizeof(blackboardBoardubo), 1);

		// Blackboard text
		World = glm::translate(glm::mat4(1), glm::vec3(-1.97f, 1.1f, -0.5f)) *
			glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(0.54f)) *
			glm::scale(glm::mat4(1), glm::vec3(1.6f, 1.0f, 1.0f));
		commonubo[40].mvpMat = Prj * View * World;
		commonubo[40].mMat = World;
		commonubo[40].nMat = glm::inverse(glm::transpose(World));
		commonubo[40].transparency = 1.0f;
		commonubo[40].textureIdx = 0;
		blackboardTextubo.amb = 20.0f; blackboardTextubo.sigma = 1.3f;
		DSBlackboardText.map(currentImage, &commonubo[40], sizeof(commonubo[40]), 0);
		DSBlackboardText.map(currentImage, &blackboardTextubo, sizeof(blackboardTextubo), 1);

		// Lamp
		World = glm::translate(glm::mat4(1), glm::vec3(0.0f, 3.01f, 0.0f)) *
			glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::scale(glm::mat4(1), glm::vec3(1.5f));
		commonubo[35].mvpMat = Prj * View * World;
		commonubo[35].mMat = World;
		commonubo[35].nMat = glm::inverse(glm::transpose(World));
		commonubo[35].transparency = 0.0f;
		commonubo[35].textureIdx = lampTextureIdx;
		if(isNight) lampubo.amb = 20.0f;
		else lampubo.amb = 1000.0f;
		lampubo.sigma = 0.3f;
		DSLamp.map(currentImage, &commonubo[35], sizeof(commonubo[35]), 0);
		DSLamp.map(currentImage, &lampubo, sizeof(lampubo), 1);

		// Matrix setup for tiles
		for (int i = 0; i < 144; i++) {
			float scaleFactor = game.tiles[i].isRemoved ? 0.0f : 1.0f;
			glm::mat4 Tbase = baseTranslation * glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.6f, 0.0f));
			glm::mat4 Tmat = glm::translate(glm::mat4(1), game.tiles[i].position * scaleFactor); // matrix for translation
			glm::mat4 Smat = glm::scale(glm::mat4(1), glm::vec3(scaleFactor));

			World = Tbase * Tmat * Smat; // translate tile in position

			tileubo[i].amb = 1.0f; 
			tileubo[i].gamma = 300.0f;
			if(isNight) tileubo[i].sColor = generalSColor;
			else tileubo[i].sColor =glm::vec3(0.5f);
			tileubo[i].tileIdx = game.tiles[i].tileIdx;
			tileubo[i].suitIdx = game.tiles[i].suitIdx;
			tileubo[i].transparency = 1.0f;
			tileubo[i].textureIdx = tileTextureIdx;
			tileubo[i].isInMenu = 0;

			// Highlight the piece on which the mouse is hoovering
			tileubo[i].hoverIdx = hoverIndex;

			// Highlight the first selected piece
			if (i==firstTileIndex) {
				tileubo[i].selectedIdx = firstTileIndex;
			}
			// Highlight the second selected piece
			else if (i == secondTileIndex) {
				tileubo[i].selectedIdx = secondTileIndex;
			}
			else {
				tileubo[i].selectedIdx = -1;
			}
			
			tileubo[i].mvpMat = Prj * View * World; 
			tileubo[i].mMat = World; 
			tileubo[i].nMat = glm::inverse(glm::transpose(World)); 
			if ((gameState == 4 || gameState == 5) && (i == firstTileIndex || i == secondTileIndex)) {
				// Set transparency to DisappearingTileTransparency;
				tileubo[i].transparency = DisappearingTileTransparency;	
			}
			DSTile[i].map(currentImage, &tileubo[i], sizeof(tileubo[i]), 0); 
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
