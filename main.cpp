#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "glm.h"

int xRot = 0, yRot = 0;

float cameraAngle = 0.0;
float x = 0, y = 3, z = 20;
float lx = 0, ly = 0, lz = -1;
GLMmodel* model1 = NULL;
GLMmodel* model2 = NULL;

void rotateCamera(float ang) {
    // rotate the camera (left / right)
	lx = sin(ang);
	lz = -cos(ang);
	glLoadIdentity();
	gluLookAt(x, y, z, x + lx, y + ly, z + lz, 0, 1, 0);
}

void moveCamera(int h_direction, int v_direction) {
    // move the camera (forward, backward, up, down)
	x = x + h_direction*(lx)*0.3;
	z = z + h_direction*(lz)*0.3;
	y = y + v_direction*0.1;
	glLoadIdentity();
	gluLookAt(x, y, z, x + lx, y + ly, z + lz, 0, 1, 0);
}

void grid_floor(int baris, int kolom) {
    // draw grid on floor (0,0,0)
    int i;

    int kiri  = -kolom / 2;
    int kanan = kolom / 2;
    int bawah = -baris / 2;
    int atas  = baris / 2;

    glBegin(GL_LINES);
    glLineWidth(1);

    for (i = 0; i < baris + 1; i++) {
        glVertex3f(kiri, 0, baris / 2 - i);
        glVertex3f(kanan, 0, baris / 2 - i);
    }

    for (i = 0; i < kolom + 1; i++) {
        glVertex3f(kolom / 2 - i, 0, bawah);
        glVertex3f(kolom / 2 - i, 0, atas);
    }

    glEnd();
}

void load_BMP_texture(char *filename) {

    FILE *file;
    short int bpp;
    short int planes;
    long size;
    unsigned int texture;

    long imwidth;
    long imheight;
    char *imdata;

    file = fopen(filename, "rb");
    fseek(file, 18, SEEK_CUR);

    fread(&imwidth, 4, 1, file);
    fread(&imheight, 4, 1, file);
    size = imwidth * imheight * 3;

    fread(&bpp, 2, 1, file);
    fread(&planes, 2, 1, file);

    fseek(file, 24, SEEK_CUR);
    imdata = (char *)malloc(size);

    fread(imdata, size, 1, file);

	char temp;
    for(long i = 0; i < size; i+=3){
        temp = imdata[i];
        imdata[i] = imdata[i+2];
        imdata[i+2] = temp;
    }

    fclose(file);

    glGenTextures(1, &texture); // then we need to tell OpenGL that we are generating a texture
    glBindTexture(GL_TEXTURE_2D, texture); // now we bind the texture that we are working with

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imwidth, imheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imdata);

    free(imdata); // free the texture
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(0.3, 0.3, 0.3);
    grid_floor(20, 20);

    // Your object here
    //glEnable(GL_TEXTURE_2D);
    glPushMatrix();
        glTranslated(0,1,0);
        glRotatef(90,0,1,0);
        glScalef(5,5,5);
        glmDraw(model1, GLM_SMOOTH | GLM_MATERIAL);
    glPopMatrix();
    //glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
    glFlush();
}

void pressKey(int key, int x, int y) {

    switch (key) {
	    case GLUT_KEY_LEFT :
            cameraAngle -= 0.02;
            rotateCamera(cameraAngle);
            break;
        case GLUT_KEY_RIGHT :
            cameraAngle += 0.02;
            rotateCamera(cameraAngle);
            break;
        case GLUT_KEY_UP :
            moveCamera(1, 0);
            break;
        case GLUT_KEY_DOWN :
            moveCamera(-1, 0);
            break;
        case GLUT_KEY_PAGE_UP :
            moveCamera(0, 1);
            break;
        case GLUT_KEY_PAGE_DOWN :
            moveCamera(0, -1);
            break;
    }

    glutPostRedisplay();
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45,1,1,100);

    model1 = glmReadOBJ("F:/Kuliah/Semester 2/Komputer Grafis/Tugas Akhir/Project/IPB Baranang Siang Campus Digital Map/final-assignment-ipb-baranang-siang-campus/bin/Debug/model/map1.obj");
    glmUnitize(model1);
    glmFacetNormals(model1);
    glmVertexNormals(model1, 90.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(x, y, z, x + lx, y + ly, z + lz, 0, 1, 0);

}

void lighting(){

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);

    float light_ambient[]  = { 1, 1, 1, 1.0 };
    float light_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
    float light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    float light_position[] = {0.0, 0.0, 50.0, 0.0 };

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    float light_position_1[] = {0.0, 10.0, 0.0, 0.0 };
    glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position_1);
    /*glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 2.0);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1.0);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.5);*/

    float mat_ambient[]    = { 1.0, 1.0, 1.0, 1.0 };
    float mat_diffuse[]    = { 1.0, 1.0, 1.0, 1.0 };
    float mat_specular[]   = { 1.0, 1.0, 1.0, 1.0 };
    float high_shininess[] = { 25.0 };

    glEnable(GL_COLOR_MATERIAL);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

}

int main(int argc, char * * argv) {
    glutInit( & argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Texture Mapping");

    glutDisplayFunc(display);
    glutSpecialFunc(pressKey);

    init();
    lighting();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glutMainLoop();

    return 0;
}
