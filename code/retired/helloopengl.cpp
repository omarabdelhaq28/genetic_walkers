#include <GL/glut.h>

// draw a red square in a window

void draw(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
		glColor3f(1.0f, 0.0f, 0.0f);	// red
		glVertex2f(-0.5f, 	-0.5f);
		glVertex2f(	0.5f, 	-0.5f);
		glVertex2f(	0.5f,	 0.5f);
		glVertex2f(-0.5f,	 0.5f);
	glEnd();

	glFlush();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutCreateWindow("OpenGL Zero");
	glutInitWindowSize(320, 320);
	glutInitWindowPosition(50, 50);
	glutDisplayFunc(draw);
	glutMainLoop();

	return 0;
}