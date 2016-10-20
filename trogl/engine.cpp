#include "engine.hpp"

#include <logger.hpp>
#include <sstream>
#include "common/utils.h"


// TODO: place default shaders into Material (default material shaders).
const ShaderPtr Engine::DEFAULT_VERTEX_SHADER = new Shader(
		"Default vertex shader",
		"attribute vec4 position;"
		"attribute vec4 color;"
		"uniform vec4 constColor;"
		"void main() {\n"
		"  gl_Position = gl_ModelViewProjectionMatrix * position;\n"
		"  gl_FrontColor = color * constColor;\n"
		"}\n");
const ShaderPtr Engine::DEFAULT_FRAGMENT_SHADER = new Shader(
		"Default fragment shader",
		"void main() {\n"
		"  gl_FragColor = gl_Color;\n"
		"}\n");
Engine* Engine::_current = nullptr;


Engine::Engine(bool displayFPS)
	: _scene(nullptr),
	  _gui(nullptr),
	  _guiFPS(),
	  _isValid(false),
	  _width(0),
	  _height(0),
	  _glVertexShader(),
	  _glFragmentShader(),
	  _glShaderProgram(),
	  _glVBO(),
	  _glCBO(),
	  _glIBO(),
	  _attrConstColor(),
	  _vertices(),
	  _colors(),
	  _indicies(),
	  _vertexShader(DEFAULT_VERTEX_SHADER),
	  _fragmentShader(DEFAULT_FRAGMENT_SHADER)
{
	logDebug << "Engine init started" << logEnd;

	setDisplayFPS(displayFPS);

	// TODO: move glut init into "initialize" function, create bool _isInited for checks.
	int argc = 1;
	char* argv = "engine";
	glutInit(&argc, &argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowPosition(2000, 100);

	logDebug << "Engine created" << logEnd;
}


Engine::~Engine()
{
	if (_isValid)
	{
		deinitShaders();
		deinitGeometry();
	}

	logDebug << "Engine removed" << logEnd;
}


void Engine::setVertextShader(const ShaderPtr& vs)
{
	_vertexShader = vs;
}


void Engine::setFragmentShader(const ShaderPtr& fs)
{
	_fragmentShader = fs;
}


void Engine::setDisplayFPS(bool displayFPS)
{
	if (displayFPS)
		_guiFPS = new GUIfps(0, 0, 1, 1);
	else
		_guiFPS = nullptr;
}


void Engine::setGUI(const GUIPtr& gui)
{
	_gui = gui;
}


void Engine::setActiveScene(const ScenePtr& scene)
{
	_scene = scene;
}


void Engine::showScene()
{
	if (!_scene)
	{
		logError << "Cannot show scene - it was not set yet!" << logEnd;
		return;
	}

	const Camera& cam = _scene->getCamera().get_reference();
	glutInitWindowSize(
				cam.getWidth(),
				cam.getHeight());
	int sceneWindow = glutCreateWindow(generateWindowName(*_scene).c_str());

	if (!runGlewTest())
	{
		logError << "Cannot show scene " << _scene->getName()
				 << " - engine couldn't initialize" << logEnd;
		return;
	}

	if (!cam.isValid())
	{
		logError << "Cannot show scene " << _scene->getName()
				 << " - camera settings are invalid" << logEnd;
		return;
	}

	// TODO: make Engine as singleton.
	_current = this;
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(cycle);

	// TODO: add light to scene.

	// TODO: add objects' materials (save shaders).
	for (const MeshPtr& m : _scene->getMeshes())
		assignGeometry(m);

	initGeometry();
	initShaders();

	logInfo << "Engine rendering started (scene: " << _scene->getName() << ")" << logEnd;
	glutMainLoop();
	logInfo << "Engine rendering finished." << logEnd;

	glutDestroyWindow(sceneWindow);
}


void Engine::assignGeometry(const MeshPtr& mesh)
{
	static const size_t vStep = 4;
	static const size_t iStep = 3;
	const size_t vOffset = _vertices.size();
	const size_t iOffset = _indicies.size();
	_vertices.resize(vOffset + (mesh->getVertices().size() * vStep), 0.0f);
	_colors.resize(_vertices.size(), 0.0f);
	_indicies.resize(iOffset + (mesh->getFaces().size() * iStep), 0.0f);

	// add vertecies and colors.
	size_t idx = 0;
	const glm::vec3& objPos = mesh->getPosition();
	for (size_t i = vOffset; i < _vertices.size(); i += vStep)
	{
		const Mesh::Vertex& v = mesh->getVertex(idx++);

		_vertices[i] = v.getPosition().x + objPos.x;
		_vertices[i+1] = v.getPosition().y + objPos.y;
		_vertices[i+2] = v.getPosition().z + objPos.z;
		_vertices[i+3] = 1.0f;

		_colors[i] = v.getColor().getRedF();
		_colors[i+1] = v.getColor().getGreenF();
		_colors[i+2] = v.getColor().getBlueF();
		_colors[i+3] = v.getColor().getAlphaF();
	}

	// add indicies from mesh faces.
	idx = 0;
	const size_t indexNumOffset = vOffset / vStep;
	for (size_t i = iOffset; i < _indicies.size(); i += iStep)
	{
		const Mesh::Face& f = mesh->getFace(idx++);

		_indicies[i] = indexNumOffset + f.getFirstIndex();
		_indicies[i+1] = indexNumOffset + f.getSecondIndex();
		_indicies[i+2] = indexNumOffset + f.getThirdIndex();
	}
}


void Engine::drawMatrix(const glm::mat4x4& mat)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&mat[0][0]);

	glUniform4f(_attrConstColor, 3, 3, 3, 1);
	_vertexShader->prepareForRender();
	_fragmentShader->prepareForRender();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, _glVBO);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, _glCBO);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glIBO);
	glDrawElements(GL_TRIANGLES, _indicies.size() * sizeof(GLuint), GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}


void Engine::drawGUI()
{
	// What does it do?
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); // save
	glLoadIdentity();// and clear
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST); // also disable the depth test so renders on top

	if (_guiFPS)
	{
		_guiFPS->tick();
		drawGUILabel(*_guiFPS);
	}

	if (_gui)
	{
		for (const GUIComponentPtr& comp : _gui->getComponents())
		{
			switch (comp->getGuiComponentType())
			{
				case GUIComponent::Type::LABEL:
					drawGUILabel((const GUILabel&)(*comp));
					break;
				case GUIComponent::Type::PLANE:
					drawGUIPlane((const GUIPlane&)(*comp));
					break;
			}
		}
	}

	glEnable(GL_DEPTH_TEST); // Turn depth testing back on

	// What does it do?
	glMatrixMode(GL_PROJECTION);
	glPopMatrix(); // revert back to the matrix I had before.
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void Engine::drawGUILabel(const GUILabel& glabel)
{
	glRasterPos2f(-0.9, +0.9);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	const char* p = glabel.getText().c_str();
	do
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
	while (*(++p));
}


void Engine::drawGUIPlane(const GUIPlane& gplane)
{
}


void Engine::renderFrame()
{
	const Color& bg = _scene->getBgColor();

	// TODO: add comment description: what does each line do with GL.
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(bg.getRedF(), bg.getGreenF(), bg.getBlueF(), bg.getAlphaF());
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the colour buffer (more buffers later on)
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glLoadIdentity(); // Load the Identity Matrix to reset our drawing locations
	glUseProgram(_glShaderProgram);

	// Matrix Projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const CameraPtr& cam = _scene->getCamera();
	_width = cam->getWidth();
	_height = cam->getHeight();
	gluPerspective(cam->getFOV(),
				   (GLdouble)_width/(GLdouble)_height,
				   cam->getLowDistance(),
				   cam->getHighDistance());

	// TODO: create camera lookAt method, which returns the position where does camera look at.
	const glm::mat4x4 matView  = glm::lookAt(cam->getPosition(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	// Drawing.
	drawMatrix(matView);
	drawGUI();

	// In the end.
	glUseProgram(0);
	glFlush();
}


void Engine::initGeometry()
{
	logInfo << "Initializing the geometry (vertices, colors, polygons): "
			<< _vertices.size() << " "
			<< _colors.size() << " "
			<< _indicies.size() << logEnd;

	glGenBuffers(1, &_glVBO);
	glBindBuffer(GL_ARRAY_BUFFER, _glVBO);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(GLfloat), _vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &_glIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indicies.size() * sizeof(GLfloat), _indicies.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &_glCBO);
	glBindBuffer(GL_ARRAY_BUFFER, _glCBO);
	glBufferData(GL_ARRAY_BUFFER, _colors.size() * sizeof(GLuint), _colors.data(), GL_STATIC_DRAW);
}


void Engine::deinitGeometry()
{
	glDeleteBuffers(sizeof(_glVBO), &_glVBO);
	glDeleteBuffers(sizeof(_glIBO), &_glIBO);
	glDeleteBuffers(sizeof(_glVBO), &_glCBO);

	_vertices.clear();
	_colors.clear();
	_indicies.clear();

	logDebug << "Geometry deinited." << logEnd;
}


void Engine::initShaders()
{
	// TODO: Take shaders from object materials
	/* HOWTO:
	 * Compile shader program for each shader from objects' materials. Save
	 * shader program into shaders vector.
	 * Place compilation process in different functions.
	 */
	const char* vsSrc = _vertexShader->getSrcPtr();
	const char* fsSrc = _fragmentShader->getSrcPtr();

	int success = 0;

	_glVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_glVertexShader, 1, &vsSrc, NULL);
	glCompileShader(_glVertexShader);
	glGetShaderiv(_glVertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		const int MAX_INFO_LOG_SIZE = 1024;
		GLchar infoLog[MAX_INFO_LOG_SIZE];
		glGetShaderInfoLog(_glVertexShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
		logError << "Error in " << _vertexShader->getName()
				 << "compilation:\n" << infoLog << logEnd;
	}
	else
		logDebug << _vertexShader->getName() << " compiled successfuly." << logEnd;

	_glFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_glFragmentShader, 1, &(fsSrc), NULL);
	glCompileShader(_glFragmentShader);
	glGetShaderiv(_glFragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		const int MAX_INFO_LOG_SIZE = 1024;
		GLchar infoLog[MAX_INFO_LOG_SIZE];
		glGetShaderInfoLog(_glFragmentShader, MAX_INFO_LOG_SIZE, NULL, infoLog);
		logError << "Error in " << _fragmentShader->getName()
				 << " compilation:\n" << infoLog << logEnd;
	}
	else
		logDebug << _fragmentShader->getName() << " compiled successfuly." << logEnd;

	_glShaderProgram = glCreateProgram();
	glAttachShader(_glShaderProgram, _glVertexShader);
	glAttachShader(_glShaderProgram, _glFragmentShader);

	glBindAttribLocation(_glShaderProgram, 0, "position");
	glBindAttribLocation(_glShaderProgram, 1, "color");

	glLinkProgram(_glShaderProgram);
	glGetProgramiv(_glShaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		const int MAX_INFO_LOG_SIZE = 1024;
		GLchar infoLog[MAX_INFO_LOG_SIZE];
		glGetProgramInfoLog(_glShaderProgram, MAX_INFO_LOG_SIZE, NULL, infoLog);
		logError << "Error in program linkage: " << infoLog << logEnd;
	}
	else
		logDebug << "Shader program linked successfuly." << logEnd;

	glValidateProgram(_glShaderProgram);
	glGetProgramiv(_glShaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success)
	{
		const int MAX_INFO_LOG_SIZE = 1024;
		GLchar infoLog[MAX_INFO_LOG_SIZE];
		glGetProgramInfoLog(_glShaderProgram, MAX_INFO_LOG_SIZE, NULL, infoLog);
		logError << "Error in program validation: " << infoLog << logEnd;
	}
	else
		logDebug << "Shader program is valid." << logEnd;

	_attrConstColor = glGetUniformLocation(_glShaderProgram, "constColor");

	_vertexShader->initUniformsLocations(_glShaderProgram);
	_fragmentShader->initUniformsLocations(_glShaderProgram);
}


void Engine::deinitShaders()
{
	// TODO: remove all shader programs (for each object) and delete shaders.
	glDetachShader(_glShaderProgram, _glVertexShader);
	glDetachShader(_glShaderProgram, _glFragmentShader);
	glDeleteProgram(_glShaderProgram);
	glDeleteShader(_glFragmentShader);
	glDeleteShader(_glVertexShader);

	logDebug << "Shaders deinited." << logEnd;
}


std::string Engine::generateWindowName(const Scene& scene)
{
	std::stringstream ss;
	ss << "TroGL Engine [" << scene.getName() << "]";
	return ss.str();
}


bool Engine::runGlewTest()
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		logFatal << "Error in glewInit, code: " << err
				 << ", message: " << glewGetErrorString(err) << logEnd;
		_isValid = false;
	}

	_isValid = true;
	return _isValid;
}


void Engine::display()
{
	_current->renderFrame();
}


void Engine::reshape(int w, int h)
{
	float& width = _current->_width;
	float& height = _current->_height;

	width = w;
	height = h;

	glViewport(0, 0, (GLsizei) width, (GLsizei) height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLdouble)width/(GLdouble)height, 0.01, 100.0);
}


void Engine::cycle()
{
	glutPostRedisplay();
}
