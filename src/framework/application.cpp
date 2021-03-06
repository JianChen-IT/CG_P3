#include "application.h"
#include "utils.h"
#include "image.h"
#include "mesh.h"

Mesh* mesh = NULL;
Image* texture = NULL;
Camera* camera = NULL;
int mode=3;
Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);

	framebuffer.resize(w, h);
	Z_buffer.resize(w, h);
}

//Here we have already GL working, so we can create meshes and textures
//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	
	//here we create a global camera and set a position and projection properties
	camera = new Camera();
	camera->lookAt(Vector3(0,10,20),Vector3(0,10,0),Vector3(0,1,0)); //define eye,center,up
	camera->perspective(60, window_width / (float)window_height, 0.1, 10000); //define fov,aspect,near,far
	
	//load a mesh
	mesh = new Mesh();
	if( !mesh->loadOBJ("lee.obj") )
		std::cout << "FILE Lee.obj NOT FOUND" << std::endl;
	Z_buffer.fill(FLT_MAX);
	//load the texture
	texture = new Image();
	texture->loadTGA("color.tga");


}
/*Function used to pass from (-1 to +1) to (0, 1)*/
float dist(float coor) {
	return ((1 - coor) / 2) + coor;
}
//render one frame
void Application::render(Image& framebuffer)
{
	framebuffer.fill(Color(40, 45, 60 )); //clear


	for (int i = 0; i < mesh->vertices.size(); i += 3)
	{
		/*Getting the mesh vertices*/
		Vector3 vertex1 = mesh->vertices[i];
		Vector3 vertex2 = mesh->vertices[i + 1]; 
		Vector3 vertex3 = mesh->vertices[i + 2]; 
		/*Normalizing the texels of the texture directly*/
		Vector2 texcoord1 = Vector2(mesh->uvs[i].x * texture->width, mesh->uvs[i].y * texture->height);
		Vector2 texcoord2 = Vector2(mesh->uvs[i+1].x * texture->width, mesh->uvs[i+1].y * texture->height); 
		Vector2 texcoord3 = Vector2(mesh->uvs[i+2].x * texture->width, mesh->uvs[i+2].y * texture->height); 

		/*project every point in the mesh to normalized coordinates using the viewprojection_matrix inside camera*/
		Vector3 normalized_point1 = camera->projectVector(vertex1);
		Vector3 normalized_point2 = camera->projectVector(vertex2);
		Vector3 normalized_point3 = camera->projectVector(vertex3);

		
		//convert from normalized (-1 to +1) to framebuffer coordinates (0,W)
		int x1 =  (dist(normalized_point1.x))*framebuffer.width;
		int y1 =  (dist(normalized_point1.y))*framebuffer.height;
		float z1 = vertex1.z;
		int x2 =  (dist(normalized_point2.x))*framebuffer.width;
		int y2 =  (dist(normalized_point2.y))*framebuffer.height;
		float z2 = vertex2.z;
		int x3 =  (dist(normalized_point3.x))*framebuffer.width;
		int y3 =  (dist(normalized_point3.y))*framebuffer.height;
		float z3 = vertex3.z;

		//paint point in framebuffer (using setPixel or drawTriangle)
		framebuffer.drawTriangle(x1, y1, z1, x2, y2, z2, x3, y3, z3, mode, camera, Z_buffer, texture, texcoord1, texcoord2, texcoord3);

		
	}
	
}

//called after render
void Application::update(double seconds_elapsed)
{
	Z_buffer.fill(FLT_MAX);
	if (keystate[SDL_SCANCODE_SPACE])
	{
		//...
	}
	/*Setting interactions specified in the lab*/
	if (keystate[SDL_SCANCODE_S]) {
		camera->center.y -= 5 * seconds_elapsed;	
	}
	if (keystate[SDL_SCANCODE_W]) {
		camera->center.y += 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_A]) {
		camera->center.x -= 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_D]) {
		camera->center.x += 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_DOWN]) {
		camera->eye.y -= 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_UP]) {
		camera->eye.y += 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_LEFT]) {
		camera->eye.x += 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_RIGHT]) {
		camera->eye.x -= 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_F]) {
		camera->fov -= 5 * seconds_elapsed;
	}
	if (keystate[SDL_SCANCODE_G]) {
		camera->fov += 5 * seconds_elapsed;
	}

	//if we modify the camera fields, then update matrices
	camera->lookAt(camera->eye, camera->center, camera->up);
	camera->perspective(camera->fov, camera->aspect, camera->near_plane, camera->far_plane);
}	

//keyboard press event 
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: exit(0); break; //ESC key, kill the app
		/*Changing the coloring modes*/
		case SDLK_1: mode = 1; break;
		case SDLK_2: mode = 2; break;
		case SDLK_3: mode = 3; break;
	}
}

//keyboard released event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	}
}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{
	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
