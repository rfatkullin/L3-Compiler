L3-Compiler
===========

Замечания:
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


Compiler for L3 language. With Flex/Bison/C++. 
