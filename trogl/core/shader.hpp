#ifndef __SHADER_HPP__
#define __SHADER_HPP__


#include <string>
#include <unordered_map>
#include <sharedpointer.h>
#include "common/color.hpp"
#include "opengls.hpp"
#include "component.hpp"
#include "object.hpp"



class Attributes
{
	public:
		using Attr = GLuint;
		using AttrsMap = std::unordered_map<std::string, Attr>;

		Attributes();
		Attributes(const Attributes& attrs);
		Attributes(Attributes&& attrs);
		~Attributes();

		Attributes& operator=(const Attributes& attrs);
		Attributes& operator=(Attributes&& attrs);

		void clear();
		void setShaderProgram(const GLuint& glProg);
		bool registerate(const std::string& name);

		template<class T>
		bool pass(const std::string& name, const T& value) const
		{
			AttrsMap::const_iterator it = _attrsMap.find(name);
			if (it == _attrsMap.cend())
				return false;

			_pass((*it).second, value);
			return true;
		}

		bool pass(const std::string& name,
				  const float& val1,
				  const float& val2,
				  const float& val3,
				  const float& val4) const;

	private:
		AttrsMap _attrsMap;
		GLuint _glShaderProg;

		static void _pass(const GLuint& loc, const int& value);
		static void _pass(const GLuint& loc, const float& value);
		static void _pass(const GLuint& loc, const glm::vec2& value);
		static void _pass(const GLuint& loc, const glm::vec3& value);
		static void _pass(const GLuint& loc, const glm::vec4& value);
		static void _pass(const GLuint& loc, const Color& value);
};


class Shader : public Component
{
	public:
		// Inner classes.
		enum class Status
		{
			NOT_COMPILED = 0,
			COMPILATION_FAILED,
			COMPILATION_SUCCESSFUL,
			NOT_BINDED,
			BINDED,
		};

		using Attr = Attributes::Attr;

		// Static fields.
		static const std::string SRC_DIR;
		static const std::string DEFAULT_VS_FILE;
		static const std::string DEFAULT_FS_FILE;

		// Static methods.
		static std::string loadFile(const std::string& filename);

		// Constructors / Destructor.
		Shader(const std::string& name,
			   const std::string& vsFile = DEFAULT_VS_FILE,
			   const std::string& fsFile = DEFAULT_FS_FILE);
		Shader(const Shader& sh);
		Shader(Shader&& sh);
		virtual ~Shader();

		// Operators.
		Shader& operator=(const Shader& sh);
		Shader& operator=(Shader&& sh);

		// Methods.
		bool isCompiled() const;
		bool isCompiledSuccessfully() const;
		bool isBinded() const;
		const Status& getStatus() const;

		void compile();
		void bind();
		void unbind();

		template<class T>
		bool passAttribute(const std::string& name,
						   const T& value) const
		{
			return _externalAttributes.pass(name, value);
		}

		// Virtual methods.
		virtual void passObject(Object const*) = 0;

	protected:
		// Constructor.
		Shader(const std::string& name);

	private:
		// Fields.
		Status _status;

	protected:
		// Fields.
		std::string _vertexSrc;
		std::string _fragmentSrc;

		size_t _shadersCompileCount;
		GLuint _glVertexShader;		// Vertices rendering
		GLuint _glFragmentShader;	// Faces rendering.
		GLuint _glShaderProgram;	// Total shadering program.

		Attributes _internalAttributes;
		Attributes _externalAttributes;

		// Virtual methods.
		virtual void _registerAttributes() = 0;
		virtual void _passInternalAttributes() = 0;

	private:
		// Methods.
		bool _compileVertexShader();
		bool _compileFragmentShader();
		bool _compileShaderProgram();
};

using ShaderPtr = SharedPointer<Shader>;

std::ostream& operator<<(std::ostream& out, const Shader::Status& st);


#endif // __SHADER_HPP__
