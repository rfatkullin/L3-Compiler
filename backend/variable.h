#ifndef _VARIABLE_H_
#define _VARIABLE_H_

class Variable
{
public :
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
