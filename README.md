L3-Compiler
===========

Проблемы:
	Конфликты:
	 1)
		func(2 + 2) 	вызов функции
	 		или же
		id1 (2 + 2)	умножение

Замечания:
-1) Не освобождается память из под имен временных переменных.
		const char* varName = GetTmpVarName(_blockLocalsCount);
		_scopeTmpVars.insert(std::pair<int, Variable>(_blockLocalsCount, var));
		AddScopeVar(varName, var);
1) Нет операции произведения - нужно добавить.

2) Bison выбирает наибольшее вхождение? ident или ident + ident.

3) Обработка параметров main(в каком виде будет аргумент)
 
Compiler for L3 language. With Flex/Bison/C++.
