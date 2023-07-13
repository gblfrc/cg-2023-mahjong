// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include <glm/ext/vector_common.hpp>
#include <glm/ext/scalar_common.hpp>
#include "MahjongGame.hpp"

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)

struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
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
};

struct BackgroundUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 sColor;
	// adjust removing matrices
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBlock {
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 AmbLightColor;
	alignas(16) glm::vec3 eyePos;
};

// The vertices data structures
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

// MAIN ! 
class Mahjong : public BaseProject {
protected:

	// Current aspect ratio (used by the callback that resized the window)
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo;
	DescriptorSetLayout DSLTile;
	DescriptorSetLayout DSLBackground;

	// Vertex formats
	VertexDescriptor VMesh;

	// Pipelines [Shader couples]
	Pipeline PTile;
	Pipeline PBackground;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	Model<VertexMesh> MBackground;
	Model<VertexMesh> MTile;

	DescriptorSet DSGubo;
	DescriptorSet DSBackground;
	DescriptorSet DSTile[144];
	//DescriptorSet DSTile;

	//Texture TPoolCloth, TWhiteTiles;
	Texture TPoolCloth;
	Texture TWhiteTiles;

	// C++ storage for uniform variables
	TileUniformBlock tileubo[144];	//not necessary as an array, works also with only one TileUniformBlock
	//TileUniformBlock tileubo;
	BackgroundUniformBlock bgubo;
	GlobalUniformBlock gubo;

	// Other application parameters
	float CamH, CamRadius, CamPitch, CamYaw;
	const float initialCamRadius = 8.5f;
	const float initialPitch = glm::radians(90.0f);
	const float initialYaw = glm::radians(0.0f);
	int gameState;
	float DisappearingTileTransparency = 1.0f;
	int firstTileIndex = -1;			//initialize at -1
	int secondTileIndex = -1;
	const glm::mat4 removedTileWorld = glm::translate(glm::mat4(1.0), glm::vec3(10.0f, -20.0f, 0.0f)) * 
								glm::scale(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
	bool disappearedTiles[144] = {0};

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, title and initial background
		windowWidth = 1200;
		windowHeight = 900;
		windowTitle = "Mahjong";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.0f, 0.005f, 0.01f, 1.0f };

		// Descriptor pool sizes
		uniformBlocksInPool = 146;
		texturesInPool = 145;
		setsInPool = 146;

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
		// Descriptor Layouts [what will be passed to the shaders]
		DSLTile.init(this, {
			// this array contains the bindings:
			// first  element : the binding number
			// second element : the type of element (buffer or texture)
			//                  using the corresponding Vulkan constant
			// third  element : the pipeline stage where it will be used
			//                  using the corresponding Vulkan constant
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLBackground.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			});

		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});


		// Vertex descriptors
		VMesh.init(this, {
			// this array contains the bindings
			// first  element : the binding number
			// second element : the stride of this binging
			// third  element : whether this parameter change per vertex or per instance
			//                  using the corresponding Vulkan constant
			{0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				// this array contains the location
				// first  element : the binding number
				// second element : the location number
				// third  element : the offset of this element in the memory record
				// fourth element : the data type of the element
				//                  using the corresponding Vulkan constant
				// fifth  elmenet : the size in byte of the element
				// sixth  element : a constant defining the element usage
				//                   POSITION - a vec3 with the position
				//                   NORMAL   - a vec3 with the normal vector
				//                   UV       - a vec2 with a UV coordinate
				//                   COLOR    - a vec4 with a RGBA color
				//                   TANGENT  - a vec4 with the tangent vector
				//                   OTHER    - anything else
				//
				// ***************** DOUBLE CHECK ********************
				//    That the Vertex data structure you use in the "offsetoff" and
				//	in the "sizeof" in the previous array, refers to the correct one,
				//	if you have more than one vertex format!
				// ***************************************************
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm),
					   sizeof(glm::vec3), NORMAL},
				{0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV),
					   sizeof(glm::vec2), UV}
			});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		//PTile.init(this, &VMesh, "shaders/PhongVert.spv", "shaders/TileFrag.spv", {&DSLGubo, &DSLTile});
		//PBackground.init(this, &VMesh, /**/"shaders/PhongVert.spv"/*TO CHANGE */ , "shaders/LambertON.spv", {&DSLGubo, &DSLBackground});
		PBackground.init(this, &VMesh, "shaders/BackgroundVert.spv", "shaders/BackgroundFrag.spv", { &DSLGubo, &DSLBackground });
		PTile.init(this, &VMesh, "shaders/TileVert.spv", "shaders/TileFrag.spv", { &DSLGubo, &DSLTile });
		PTile.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, true); //default values except for last one that is transparency


		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		// Create Background model manually
		float side = 18.0f;
		MBackground.vertices = {
			{{-side, 0.0f, side},{0.0f, 1.0f, 0.0f},{0.0f, 0.0f}},
			{{side, 0.0f, side},{0.0f, 1.0f, 0.0f},{1.0f, 0.0f}},
			{{side, 0.0f, -side},{0.0f, 1.0f, 0.0f},{1.0f, 1.0f}},
			{{-side, 0.0f, -side},{0.0f, 1.0f, 0.0f},{0.0f, 1.0f}},
		};
		MBackground.indices = { 0, 1, 2,    0, 2, 3 };
		MBackground.initMesh(this, &VMesh);
		// Import Tile model
		MTile.init(this, &VMesh, "Models/Tile.obj", OBJ);

		// Create the textures
		// The second parameter is the file name
		TPoolCloth.init(this, "textures/background/poolcloth.png");
		TWhiteTiles.init(this, "textures/tiles/tiles_white.png");

		// Init local variables
		CamH = 1.0f;
		CamRadius = initialCamRadius; //was 4.5f initially
		CamPitch = initialPitch;
		CamYaw = initialYaw;
		gameState = 0;				//CHANGE TO 0
		firstTileIndex = 120;
		secondTileIndex = 120;
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PBackground.create();
		PTile.create();

		// Here you define the data set			//MADE A CYCLE FOR THE 144 DS
		for (int i = 0; i < 144; i++) {
			DSTile[i].init(this, &DSLTile, {
				// the second parameter, is a pointer to the Uniform Set Layout of this set
				// the last parameter is an array, with one element per binding of the set.
				// first  elmenet : the binding number
				// second element : UNIFORM or TEXTURE (an enum) depending on the type
				// third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
				// fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
							{0, UNIFORM, sizeof(TileUniformBlock), nullptr},
							{1, TEXTURE, 0, &TWhiteTiles}
				});
		}

		DSGubo.init(this, &DSLGubo, {
			{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
			});

		DSBackground.init(this, &DSLBackground, {
					{0, UNIFORM, sizeof(BackgroundUniformBlock), nullptr},
					{1, TEXTURE, 0, &TPoolCloth}
			});


	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PBackground.cleanup();
		PTile.cleanup();

		// Cleanup datasets
		DSGubo.cleanup();
		DSBackground.cleanup();
		for (int i = 0; i < 144; i++) {
			DSTile[i].cleanup();
		}

	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		TPoolCloth.cleanup();
		TWhiteTiles.cleanup();

		// Cleanup models
		MBackground.cleanup();
		MTile.cleanup();

		// Cleanup descriptor set layouts
		DSLTile.cleanup();
		DSLBackground.cleanup();
		DSLGubo.cleanup();

		// Destroys the pipelines
		PTile.destroy();
		PBackground.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// sets global uniforms (see below fro parameters explanation)

		/*
		DSGubo.bind(commandBuffer, PMesh, 0, currentImage);				//????

		// binds the pipeline
		PMesh.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter
		*/


		// binds the model
		//MTile.bind(commandBuffer);
		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter

		// binds the data set
		//for (int i = 0; i < 144; i++) {
		//	DSTile[i].bind(commandBuffer, PTile, 1, currentImage);
		//	// For a Dataset object, this command binds the corresponing dataset
		//	// to the command buffer and pipeline passed in its first and second parameters.
		//	// The third parameter is the number of the set being bound
		//	// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		//	// This is done automatically in file Starter.hpp, however the command here needs also the index
		//	// of the current image in the swap chain, passed in its last parameter

		//	// record the drawing command in the command buffer
		//	vkCmdDrawIndexed(commandBuffer,
		//		static_cast<uint32_t>(MTile.indices.size()), 1, 0, 0, 0);
		//	// the second parameter is the number of indexes to be drawn. For a Model object,
		//	// this can be retrieved with the .indices.size() method.
		//}
		//

		// Background pipeline binding
		PBackground.bind(commandBuffer);
		MBackground.bind(commandBuffer);
		DSGubo.bind(commandBuffer, PBackground, 0, currentImage);
		DSBackground.bind(commandBuffer, PBackground, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(MBackground.indices.size()), 1, 0, 0, 0);
		// Tile pipeline binding
		PTile.bind(commandBuffer);
		MTile.bind(commandBuffer);
		DSGubo.bind(commandBuffer, PTile, 0, currentImage);
		for (int i = 0; i < 144; i++) {
			DSTile[i].bind(commandBuffer, PTile, 1, currentImage);
			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MTile.indices.size()), 1, 0, 0, 0);
		}

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		
		// Integration with the timers and the controllers
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		bool click = false;
		getSixAxis(deltaT, m, r, fire, click);
		//std::cout << "\nclicked is :" << clicked<<"\n";
		
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
			//cout << x << ", " << y << " ---> " << pixels[index] << "\n";	//DEBUG PRINT
			hoverIndex = pixels[index];
		}
		vkUnmapMemory(device, entityImageMemory);

		if(handleClick) std::cout << "\nClick on tile: " << pixels[index]<<"\n";

		string structurePath = "./structure.json";
		static MahjongGame game = MahjongGame(structurePath);

		//std:cout << "\nGameState: " << gameState<<"\n";	//DEBUG PRINT
		switch (gameState) {		// main state machine implementation
			
			case -1: //menu	
				//show menu overlay
				// disable commands?
				//get clicks to change textures and shaders
				break;
			case 0:
				//no piece selected
				firstTileIndex = -1;
				secondTileIndex = -1;
				DisappearingTileTransparency = 1.0f; //not transparent
				if (handleClick & hoverIndex > -1) {
					firstTileIndex = hoverIndex;
					gameState = 1;
				}
				
				break;
			case 1:
				//1 piece selected and highlighted
				if (handleClick & hoverIndex>-1) {
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
				disappearedTiles[firstTileIndex] = true;
				disappearedTiles[secondTileIndex] = true;
				//game.removeTiles(firstTileIndex, secondTileIndex);
				//go back to initial state
				gameState = 0;
				break;

		}

		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(90.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;
		const float rotSpeed = glm::radians(90.0f);
		const float movSpeed = 10.0f;


		CamH += m.z * movSpeed * deltaT;
		CamRadius -= m.x * movSpeed * deltaT;
		CamRadius = glm::clamp(CamRadius, 7.8f, 20.0f); //minumum and maximum zoom of the cam


		CamPitch -= r.x * rotSpeed * deltaT;
		CamPitch = glm::clamp(CamPitch, glm::radians(30.0f), glm::radians(89.0f)); //constraints on degrees on elevation of the cam 

		CamYaw += r.y * rotSpeed * deltaT;


		/* da usare insieme a ubo perché servono le matrici
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

		//Game logic: overwrites coordinates if fire (space) is pressed and released
		//Bring to initial position
		if (handleFire) { //replace hanfleFire with "wasFire" to have event happen upon pressing and not release of fire key
			//glm::vec3 
			CamRadius = initialCamRadius;
			CamPitch = initialPitch;
			CamYaw = initialYaw;
		}

		//a
		//glm::vec3 camTarget = glm::vec3(0,CamH,0);
		glm::vec3 camTarget = glm::vec3(0, 0, 0);

		//c  da vedere
		glm::vec3 camPos = camTarget + CamRadius * glm::vec3(cos(CamPitch) * sin(CamYaw), sin(CamPitch), cos(CamPitch) * cos(CamYaw));


		glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));

		gubo.DlightDir = glm::normalize(glm::vec3(1, 2, 3));
		gubo.DlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.AmbLightColor = glm::vec3(0.1f);
		gubo.eyePos = camPos;

		// Writes value to the GPU
		DSGubo.map(currentImage, &gubo, sizeof(gubo), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

		// Matrix setup for background
		glm::mat4 World = glm::mat4(1);
		bgubo.amb = 1.0f; bgubo.gamma = 180.0f; bgubo.sColor = glm::vec3(1.0f);
		bgubo.mvpMat = Prj * View * World;
		bgubo.mMat = World;
		bgubo.nMat = glm::inverse(glm::transpose(World));
		DSBackground.map(currentImage, &bgubo, sizeof(bgubo), 0);

		// Matrix setup for tiles
		for (int i = 0; i < 144; i++) {
			double scaleFactor = game.tiles[i].isRemoved() ? 0.0f : 50.0f;
			glm::mat4 Tmat = glm::translate(glm::mat4(1), game.tiles[i].position * 50.0f); // matrix for translation
			glm::mat4 Smat = glm::scale(glm::mat4(1), glm::vec3(scaleFactor));

			World = Tmat * Smat; // translate tile in position
			//World = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-9.2 + i%10*2, 0, 9.2-i/10*2)), glm::vec3(50.0f));
			tileubo[i].amb = 1.0f; 
			tileubo[i].gamma = 200.0f; //CHANGE GAMMA HIGHER FOR POINT LIGHT
			tileubo[i].sColor = glm::vec3(1.0f);
			tileubo[i].tileIdx = game.tiles[i].tileIdx;
			tileubo[i].suitIdx = game.tiles[i].suitIdx;
			tileubo[i].transparency = 1.0f;

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