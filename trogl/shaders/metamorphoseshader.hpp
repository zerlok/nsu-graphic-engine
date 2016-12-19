#ifndef __METAMORPHOSE_SHADER_HPP__
#define __METAMORPHOSE_SHADER_HPP__


#include <string>
#include "core/shader.hpp"


class MetamorphoseShader : public Shader
{
	public:
		MetamorphoseShader();
		~MetamorphoseShader();

		void passObject(const Object* obj) const;

		void _registerAttributes() override;
		void _passInternalAttributes() override;

		void passComponent(const Component* comp) const override;
		void passArrayOfComponents(const ComponentsPtrs& components) const override;

	private:
		static const std::string VS_FILE;

		GLuint _attrObjPosition;
		GLuint _attrCosSqAlpha;
};


#endif // __METAMORPHOSE_SHADER_HPP__
