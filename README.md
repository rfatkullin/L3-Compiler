L3-Compiler
===========

Проблемы:
 Конфликты: 
	func(2 + 2) 	вызов функции
 или же 
	id1 (2 + 2)	умножение

Замечания:
-1) Не освобождается память из под имен временных переменных.
		const char* varName = GetTmpVarName(_blockLocalsCount);
		_scopeTmpVars.insert(std::pair<int, Variable>(_blockLocalsCount, var));
		AddScopeVar(varName, var);
0) Локальные переменные внутри циклов
1) Нет операции произведения - нужно добавить.
2) Нет поддержки массивов. Например, не будет работать следующее for a[i] = 1 to 9 do.
4) Нет операций со строками.
7) Приведение типов.
9) Должен ли стек быть пустым при возврате из функции/процедуры.
10) Нужно ли очищать память. Обработал узел, удалил его.
11)	В вызове функций может быть лишняя запятая после параметров sum(2, 5,) - проверить
		params : expr COMMA params
			{
				IdentsNode* node = new IdentsNode( $1, $3 );
				$$ = node;
			}
	| expr
		{
		    std::list<ExprNode*> tmpList = new std::list<ExprNode*>();
		    tmpList.push_back($1);
		    $$ = tmpList;
		}
	|
		{ $$ = new std::list<ExprNode*>(); };


Работа с массивами:
Объявление :
	arr -> [[[int]]];
Определение :
	arr = [[[int]] 3];
	arr[0] = [[int] 3];
	arr[1] = [[int] 3];
	arr[2] = [[int] 3];
	arr[0][0] = [int 3];
Обращение к элементам массива:
	arr[0][0][0] = 1;
	arr[0][0][1] = 2;
	arr[0][0][2] = 3;



Compiler for L3 language. With Flex/Bison/C++.
