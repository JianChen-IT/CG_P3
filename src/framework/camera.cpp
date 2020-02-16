#include "camera.h"
#include <math.h>
Camera::Camera()
{
	this->fov = 60;
	this->aspect = 800/600.0;
	this->near_plane = 0.01;
	this->far_plane = 10000;

	eye = Vector3(0, 10, 20);
	center = Vector3(0, 10, 0);
	up = Vector3(0, 1, 0);


	//here I gave you two matrices so you can develop them independently
	//pay attention that they are column-major order [column][row]
	
	//example view:   eye(0,10,20)  center(0,10,0)  up(0,1,0)
	/*view_matrix.setIdentity();
	view_matrix.M[0][0] = 1.0; 	view_matrix.M[0][1] = 0.0; view_matrix.M[0][2] = 0.0;  view_matrix.M[0][3] = 0.0;
	view_matrix.M[1][0] = 0.0; 	view_matrix.M[1][1] = 1.0; view_matrix.M[1][2] = 0.0;  view_matrix.M[1][3] = 0.0;
	view_matrix.M[2][0] = 0.0; 	view_matrix.M[2][1] = 0.0; view_matrix.M[2][2] = 1.0;  view_matrix.M[2][3] = 0.0;
	view_matrix.M[3][0] = 0.0; 	view_matrix.M[3][1] = -10.0; view_matrix.M[3][2] = -20.0;  view_matrix.M[3][3] = 1.0;*/

	//example projection:   fov: 60deg, aspect: 1.33333337, near_plane: 0.1, far_plane: 10000
	/*projection_matrix.setIdentity();
	projection_matrix.M[0][0] = 1.29903817; projection_matrix.M[0][1] = 0.0; projection_matrix.M[0][2] = 0.0;  projection_matrix.M[0][3] = 0.0;
	projection_matrix.M[1][0] = 0.0; 	projection_matrix.M[1][1] = 1.73205090; projection_matrix.M[1][2] = 0.0;  projection_matrix.M[1][3] = 0.0;
	projection_matrix.M[2][0] = 0.0; 	projection_matrix.M[2][1] = 0.0; projection_matrix.M[2][2] = -1.00000191; projection_matrix.M[2][3] = -1;
	projection_matrix.M[3][0] = 0.0; 	projection_matrix.M[3][1] = 0.0; projection_matrix.M[3][2] = -0.0200000200;  projection_matrix.M[3][3] = 0.0;*/



}

void Camera::updateViewMatrix()
{
	//IMPLEMENT THIS using eye, center and up, store in this->view_matrix
	//Careful with the order of matrix multiplications, and be sure to use normalized vectors

	//YOUR CODE HERE
	/*INFO RETRIEVED FROM SLIDES 26 -> "Camera and projections" slides*/

	Vector3 front = (this->center - this->eye).normalize();
	Vector3 side = front.cross(this->up).normalize();
	Vector3 top = side.cross(front);

	view_matrix.M[0][0] = side.x; 	view_matrix.M[0][1] = top.x; view_matrix.M[0][2] = -front.x;  view_matrix.M[0][3] = 0.0;
	view_matrix.M[1][0] = side.y; 	view_matrix.M[1][1] = top.y; view_matrix.M[1][2] = -front.y;  view_matrix.M[1][3] = 0.0;
	view_matrix.M[2][0] = side.z; 	view_matrix.M[2][1] = top.z; view_matrix.M[2][2] = -front.z;  view_matrix.M[2][3] = 0.0;
	view_matrix.M[3][0] = -this->eye.x; 	view_matrix.M[3][1] = -this->eye.y; view_matrix.M[3][2] = -this->eye.z;  view_matrix.M[3][3] = 1.0;

	
	viewprojection_matrix = view_matrix * projection_matrix;
}
/*Radian converter*/
float rad(float angle) {
	return angle * M_PI /180;
}
void Camera::updateProjectionMatrix()
{
	//IMPLEMENT THIS using fov, aspect, near_plane and far_plane, store in this->projection_matrix
	//Careful with using degrees in trigonometric functions, must be radians, and use float types in divisions

	//YOUR CODE HERE
	float f = 1 / tan(rad(this->fov)/2);

	projection_matrix.M[0][0] = f / this->aspect; projection_matrix.M[0][1] = 0.0; projection_matrix.M[0][2] = 0.0;  projection_matrix.M[0][3] = 0.0;
	projection_matrix.M[1][0] = 0.0; 	projection_matrix.M[1][1] = f; projection_matrix.M[1][2] = 0.0;  projection_matrix.M[1][3] = 0.0;
	projection_matrix.M[2][0] = 0.0; 	projection_matrix.M[2][1] = 0.0; projection_matrix.M[2][2] = (this->far_plane + this->near_plane) / (this->near_plane - this->far_plane); projection_matrix.M[2][3] = -1;
	projection_matrix.M[3][0] = 0.0; 	projection_matrix.M[3][1] = 0.0; projection_matrix.M[3][2] = 2 * ((this->far_plane * this->near_plane) / (this->near_plane - this->far_plane));  projection_matrix.M[3][3] = 0.0;

	viewprojection_matrix = view_matrix * projection_matrix;
}

Vector3 Camera::projectVector( Vector3 pos )
{
	Vector4 pos4 = Vector4(pos.x, pos.y, pos.z, 1.0);
	Vector4 result = viewprojection_matrix * pos4;
	return result.getVector3() / result.w;
}

void Camera::lookAt( Vector3 eye, Vector3 center, Vector3 up )
{
	this->eye = eye;
	this->center = center;
	this->up = up;

	this->updateViewMatrix();
}

void Camera::perspective( float fov, float aspect, float near_plane, float far_plane )
{
	this->fov = fov;
	this->aspect = aspect;
	this->near_plane = near_plane;
	this->far_plane = far_plane;

	this->updateProjectionMatrix();
}

Matrix44 Camera::getViewProjectionMatrix()
{
	viewprojection_matrix = view_matrix * projection_matrix;
	return viewprojection_matrix ;
}

