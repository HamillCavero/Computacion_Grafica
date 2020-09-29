#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

void framebuffer_tamanho_callback(GLFWwindow* ventana, int ancho, int alto) {
	glViewport(0, 0, ancho, alto);
}
void procesarEntrada(GLFWwindow* ventana) {
	if (glfwGetKey(ventana, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(ventana, true);
}

const unsigned int ANCHO = 600;
const unsigned int ALTO = 600;

class CProgramaShaders {
	GLuint idPrograma;
public:
	CProgramaShaders(string rutaShaderVertice, string rutaShaderFragmento) {
		//Variables para leer los archivos de código
		string strCodigoShaderVertice;
		string strCodigoShaderFragmento;
		ifstream pArchivoShaderVertice;
		ifstream pArchivoShaderFragmento;
		//Mostramos excepciones en caso haya
		pArchivoShaderVertice.exceptions(ifstream::failbit | ifstream::badbit);
		pArchivoShaderFragmento.exceptions(ifstream::failbit | ifstream::badbit);
		try {
			//Abriendo los archivos de código de los shader
			pArchivoShaderVertice.open(rutaShaderVertice);
			pArchivoShaderFragmento.open(rutaShaderFragmento);
			//Leyendo la información de los archivos
			stringstream lectorShaderVertice, lectorShaderFragmento;
			lectorShaderVertice << pArchivoShaderVertice.rdbuf();
			lectorShaderFragmento << pArchivoShaderFragmento.rdbuf();
			//Cerrando los archivos
			pArchivoShaderVertice.close();
			pArchivoShaderFragmento.close();
			//Pasando la información leida a string
			strCodigoShaderVertice = lectorShaderVertice.str();
			strCodigoShaderFragmento = lectorShaderFragmento.str();
		}
		catch (ifstream::failure) {
			cout << "ERROR: Los archivos no pudieron ser leidos correctamente.\n";
		}
		const char* codigoShaderVertice = strCodigoShaderVertice.c_str();
		const char* codigoShaderFragmento = strCodigoShaderFragmento.c_str();
		//Asociando y compilando los códigos de los shader
		GLuint idShaderVertice, idShaderFragmento;
		//Shader de Vértice
		idShaderVertice = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(idShaderVertice, 1, &codigoShaderVertice, NULL);
		glCompileShader(idShaderVertice);
		verificarErrores(idShaderVertice, "Vértice");
		//Shader de Fragmento
		idShaderFragmento = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(idShaderFragmento, 1, &codigoShaderFragmento, NULL);
		glCompileShader(idShaderFragmento);
		verificarErrores(idShaderVertice, "Fragmento");
		//Programa de Shaders
		this->idPrograma = glCreateProgram();
		glAttachShader(this->idPrograma, idShaderVertice);
		glAttachShader(this->idPrograma, idShaderFragmento);
		glLinkProgram(this->idPrograma);
		//Ahora ya podemos eliminar los programas de los shaders
		glDeleteShader(idShaderVertice);
		glDeleteShader(idShaderFragmento);
	}
	~CProgramaShaders() {
		glDeleteProgram(this->idPrograma);
	}
	void usar() const {
		glUseProgram(this->idPrograma);
	}
	void setVec3(const string& nombre, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(this->idPrograma, nombre.c_str()), x, y, z);
	}
	void setVec3(const string& nombre, const glm::vec3& valor) const {
		glUniform3fv(glGetUniformLocation(this->idPrograma, nombre.c_str()), 1, &valor[0]);
	}
private:
	void verificarErrores(GLuint identificador, string tipo) {
		GLint ok;
		GLchar log[1024];

		if (tipo == "Programa") {
			glGetProgramiv(this->idPrograma, GL_LINK_STATUS, &ok);
			if (!ok) {
				glGetProgramInfoLog(this->idPrograma, 1024, NULL, log);
				cout << "Error de enlace con el programa: " << log << "\n";
			}
		}
		else {
			glGetShaderiv(identificador, GL_COMPILE_STATUS, &ok);
			if (!ok) {
				glGetShaderInfoLog(identificador, 1024, nullptr, log);
				cout << "Error de compilación con el Shader de " << tipo << ": " << log << "\n";
			}
		}
	}
};

int main() {
	//Inicializar glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Creando la ventana
	GLFWwindow* ventana = glfwCreateWindow(ANCHO, ALTO, "Compu Grafica", NULL, NULL);
	if (ventana == NULL) {
		cout << "Problemas al crear la ventana\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(ventana);
	glfwSetFramebufferSizeCallback(ventana, framebuffer_tamanho_callback);

	//Cargar Glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Problemas al cargar GLAD\n";
		return -1;
	}

	CProgramaShaders programa_shaders = CProgramaShaders("GLSL/codigo.vs", "GLSL/codigo.fs");

	//Definiendo la geometría de la figura en función de vértices
	float vertices[] = {
		-0.5, -0.5, 0.0, 1.0, 0.0, 0.0,//inferior izquierdo
		 0.5, -0.5, 0.0, 0.0, 1.0, 0.0,//inferior derecho
		-0.5,  0.5, 0.0, 0.0, 0.0, 1.0,//superior izquierdo
		 0.5,  0.5, 0.0, 1.0, 0.0, 1.0 //superior derecho
	};

	unsigned int indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	//Enviando la geometría al GPU: Definiendo los buffers (Vertex Array Objects y Vertex Buffer Objects)
	unsigned int id_array_vertices, id_array_buffers, id_element_buffer;
	glGenVertexArrays(1, &id_array_vertices);
	glGenBuffers(1, &id_array_buffers);
	glGenBuffers(1, &id_element_buffer);

	//anexando los buffers para su uso en la tarjeta gráfica
	glBindVertexArray(id_array_vertices);

	//Anexando buffers y cargandolos con los datos
	glBindBuffer(GL_ARRAY_BUFFER, id_array_buffers);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_element_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//Indicando las especificaciones de los atributos
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	while (!glfwWindowShouldClose(ventana)) {
		procesarEntrada(ventana);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		programa_shaders.usar();
		programa_shaders.setVec3("traslacion", glm::vec3(0.5, 0.0, 0.0));
		glBindVertexArray(id_array_vertices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_element_buffer);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(ventana);
		glfwPollEvents();
	}
	//Liberando memoria
	glDeleteVertexArrays(1, &id_array_vertices);
	glDeleteBuffers(1, &id_array_buffers);
	glfwTerminate();
	return 0;
}
