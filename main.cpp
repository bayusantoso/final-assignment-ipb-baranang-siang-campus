#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "glm.h"
#include "lodepng.h"

int xRot = 0, yRot = 0;

float cameraAngle = 0.0;
float x = 0, y = 3, z = 20;
float lx = 0, ly = 0, lz = -1;
GLMmodel* model1 = NULL;
GLMmodel* model2 = NULL;

// A 3D point.  Used for the scene's geometry.
typedef struct {
  float x, y, z;
} Vertex3D;

// A 2D point.  Used for a scene's texture coordinates on the geometry.
typedef struct {
  float x, y;
} Vertex2D;

// While an Object file may contain multiple objects, each with a set of
// vertices and texture vertices, all vertex indecies and all texture vertex
// indecies are unique.  Therefore, we load them in single lists.
#define MAX_VERTICES 3000
Vertex3D vertices[MAX_VERTICES];
int vertex_count = 0;
Vertex2D texture_vertices[MAX_VERTICES];
int texture_vertex_count = 0;

// A face is a polygon that makes up part of an object.
#define MAX_FACE_VERTICES 4
typedef struct {
  int vertices[MAX_FACE_VERTICES];
  int texture_vertices[MAX_FACE_VERTICES];
  int vertex_count;
} Face;

// An object is a major part of the scene, such as hair, clothing, and a body.
typedef struct {
  #define MAX_NAME_LENGTH 50
  char name[MAX_NAME_LENGTH];

  #define MAX_FACES 1800
  Face faces[MAX_FACES];
  int face_count;

  GLuint texture;
} Object;

// Objects are parts of the scene, such as hair, body, and clothing.
#define MAX_OBJECTS 100
Object objects[MAX_OBJECTS];
int object_count = 0;

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

void loadObjectWithTexture(char scne_filename[100], char file_path[100]) {
    glEnable (GL_TEXTURE_2D);

        static const char *SCENE_FILENAME;
        SCENE_FILENAME = scne_filename;
      // Open the Object file that contains geometry and texture vertices.
      // This particular scene is by Peter McAlpine.
      static const char *READ_MODE = "r";
      FILE *f = fopen (SCENE_FILENAME, READ_MODE);

      Object *current_object;

      // Read each line of the Object file.
      static const int MAX_LINE_LENGTH = 100;
      char line[MAX_LINE_LENGTH];
      while (fgets (line, MAX_LINE_LENGTH, f) != NULL) {
        if (strncmp (line, "o ", 2) == 0) { // If starting a new object...
          current_object = &objects[object_count];

          // Read the object's name.
          sscanf (line, "o %s", current_object->name);
          current_object->face_count = 0;

          // Load the PNG as the object's texture.
          // The texture must be loaded into OpenGL after OpenGL is initialized.
          static const int MAX_FILENAME_LENGTH = 100;
          char filename[MAX_FILENAME_LENGTH];
          sprintf (filename, file_path, current_object->name);
          unsigned char *buffer, *image;
          size_t buffer_size, image_size;
          LodePNG_Decoder decoder;
          LodePNG_Decoder_init (&decoder);
          LodePNG_loadFile (&buffer, &buffer_size, filename);
          LodePNG_Decoder_decode (&decoder, &image, &image_size, buffer, buffer_size);
          assert (!decoder.error);
          glGenTextures (1, &current_object->texture);
          glBindTexture (GL_TEXTURE_2D, current_object->texture);
          glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, decoder.infoPng.width, decoder.infoPng.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
          glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          free (image);
          free (buffer);
          LodePNG_Decoder_cleanup (&decoder);

          object_count++;
        } else if (strncmp (line, "v ", 2) == 0) { // If describing a vertex...
          Vertex3D *v = &vertices[vertex_count++];
          sscanf (line, "v %f %f %f", &v->x, &v->y, &v->z);
        } else if (strncmp (line, "vt ", 3) == 0) { // If texture vertex...
          Vertex2D *vt = &texture_vertices[texture_vertex_count++];
          sscanf (line, "vt %f %f", &vt->x, &vt->y);
          vt->y = 1 - vt->y;
        } else if (strncmp (line, "f ", 2) == 0) { // If an object face...
          Face *face = &current_object->faces[current_object->face_count++];
          face->vertex_count = 0;
          char *subline = &line[2];
          while (subline != NULL) {
            int vertex_index, texture_vertex_index;
            sscanf (subline, "%i/%i", &vertex_index, &texture_vertex_index);
            int fv = face->vertex_count;
            face->vertices[fv] = vertex_index - 1;
            face->texture_vertices[fv] = texture_vertex_index - 1;
            face->vertex_count++;
            subline = strchr (subline, ' ');
            if (subline != NULL) {
              subline = &subline[1];
            }
          }
        } // if type of line
      } // for each line

      fclose (f);

      int o;

      // Display interesting stats about the loaded Object file.
      printf ("Vertices: %i\n", vertex_count);
      printf ("Texture vertices: %i\n", texture_vertex_count);
      printf ("Objects: %i\n", object_count);
      for (o = 0; o < object_count; ++o) {
        printf ("  %s faces: %i\n", objects[o].name, objects[o].face_count);
      }

      // Start the main program loop.
      glutMainLoop(); // never returns

      glDisable (GL_TEXTURE_2D);

      // If the main loop ever ended, we'd want to free the textures.
      for (o = 0; o < object_count; ++o) {
        glDeleteTextures (1, &objects[o].texture);
      }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(0.3, 0.3, 0.3);
    grid_floor(20, 20);
    // For each object...
  int o;
  for (o = 0; o < object_count; ++o) {
    Object *object = &objects[o];
    glBindTexture (GL_TEXTURE_2D, object->texture);

    // For each face of the current object...
    int f;
    for (f = 0; f < object->face_count; ++f) {
      Face *face = &object->faces[f];
      glPushMatrix();
          glTranslated(0,4,0);
          glScalef(0.5,0.5,0.5);
          glBegin (GL_POLYGON);
            int v;
            for (v = 0; v < face->vertex_count; ++v) {
              // Place a texture coordinate.
              int texture_vertex_index = face->texture_vertices[v];
              Vertex2D *texture_vertex = &texture_vertices[texture_vertex_index];
              glTexCoord2f (texture_vertex->x, texture_vertex->y);

              // Place a geometry vertex.
              int vertex_index = face->vertices[v];
              Vertex3D *vertex = &vertices[vertex_index];
              glVertex3f (vertex->x, vertex->y, vertex->z);
            }
          glEnd();
      glPopMatrix();
    }
  }
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

    loadObjectWithTexture("model/eumi.obj","model/eumi_body.png");

    glutMainLoop();

    return 0;
}
