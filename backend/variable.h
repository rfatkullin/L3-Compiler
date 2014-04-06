#ifndef _VARIABLE_H_
#define _VARIABLE_H_

#include "node.h"

class Variable
{
public :

	static bool VarCompare(const Variable& var1, const Variable& var2)
	{
		return var1._id < var2._id;
	}

    Variable()
		:   _id(-1),
			_type(NULL),
			_isArg(false)
    {}

	Variable(int id, TypeNode* type, bool isArg)
		:	_id(id),
			_type(type),
			_isArg(isArg)
    {}

	bool 		_isArg;
	int 		_id;
	TypeNode* 	_type;
};

#endif
