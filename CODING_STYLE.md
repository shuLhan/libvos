# libvos Coding Style

## Formatting

* Indentation must use tab character, without expanding into space.

* A single tab must be 8 characters.

* Text width must be 80 characters.


## Naming

* Static variable or method must be in uppercase.

* Class name must be using camel case style (e.g. UpperName).

* Variable and method name must be using underscore style (e.g. a_var, a_method,
  etc.).


## Class Declarations

The declaration of class's fields and methods must in the following orders,

```
class X : [public|protected|private] ParentClass
{
public:
	// static variables
	// static methods

	// public fields

	// constructor
	// destructor
	// public methods

protected:
	// protected fields
	// protected methods

private:
	X(const X&);
	void operator=(const X&);

	// private fields
	// private methods
};
```

## Variable Style

* Pointer must be placed right after variable name, no space between them. For
  example: `char* p`.

  **Rationale**: If a method return a pointer, writing `char *v()` will make
  them like pointer to function, not function that return a pointer.


## Comment Syntax

* Use `/**/` style for comment.

* Use `//` for comment inside code.

* Write comment in the `.cc` file, not in the header.

* Grammar for class: `Class X represent ...`

* Grammar for class field: `Field X contains ...`

* Grammar for variables:
  - `Variable V contains ...`
  - `Variable W is a ...`

* Grammar for method: `Method M will ...`
