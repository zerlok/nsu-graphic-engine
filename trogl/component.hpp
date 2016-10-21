#ifndef __COMPONENT_HPP__
#define __COMPONENT_HPP__


#include <string>
#include <iostream>
#include <sharedpointer.h>


class Component
{
	public:
		// Enum.
		enum class Type
		{
			GUI,
			SCENE,
			OBJECT,
			MATERIAL,
			TEXTURE,
			SHADER,
		};

		// Constructors / Destructor.
		Component(const Type& t,
				  const std::string& name = std::string());
		Component(const Component& c);
		Component(Component&& c);
		virtual ~Component();

		// Operators.
		Component& operator=(const Component& c);
		Component& operator=(Component&& c);

		bool operator==(const Component& c) const;
		bool operator!=(const Component& c) const;

		// Methods.
		const Type& getType() const;
		const std::string& getName() const;

		void setName(const std::string& name);

		// Virtual methods.
		virtual Component copy() const;
		virtual std::string toString() const;

		// Friend functions.
		friend std::ostream& operator<<(std::ostream& out, const Component& c);

	private:
		// Static fields.
		static size_t _globID;

		// Static methods.
		static std::string _generateNameFromType(const Type& type, const size_t& id);
		static std::string _generateNameFromCopy(const Component& c);

		// Fields.
		Type _type;
		size_t _ID;

	protected:
		// Fields.
		std::string _name;
};

using ComponentPtr = SharedPointer<Component>;

std::ostream& operator<<(std::ostream& out, const Component& c);


#endif // __COMPONENT_HPP__
