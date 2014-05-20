#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

int xRot = 0, yRot = 0;

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

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(0.3, 0.3, 0.3);
    grid_floor(20, 20);

    glutSwapBuffers();
    glFlush();
}

void pressKey(int key, int x, int y) {

    switch (key) {
        case GLUT_KEY_LEFT  : yRot -= 1;break;
        case GLUT_KEY_RIGHT : yRot += 1;break;
        case GLUT_KEY_UP    : xRot -= 1;break;
        case GLUT_KEY_DOWN  : xRot += 1;break;
    }

    glutPostRedisplay();
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45,1,1,100);
    gluLookAt(0, 5, 20, 0, 0, 0, 0.0f,1.0f,0.0f);

    glMatrixMode(GL_MODELVIEW);
}

void lighting(){
    // Fungsi mengaktifkan pencahayaan
    // Pembahasan di materi selanjutnya

    const GLfloat light_ambient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
    const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat light_position[] = { -2.0f, 3.0f, 5.0f, 1.0f };

    const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
    const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
    const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
    const GLfloat high_shininess[] = { 100.0f };

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
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
    glutCreateWindow("3D Objects");

    glutDisplayFunc(display);
    glutSpecialFunc(pressKey);
    init();

    lighting();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glutMainLoop();

    return 0;
}
