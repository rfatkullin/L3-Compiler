%option reentrant noyywrap bison-bridge bison-locations
%option extra-type="struct Extra *"

%{

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "lexer.h"
#include "parser.tab.h"

#define YY_USER_ACTION									\
{														\
	int i;												\
	struct Extra* extra = yyextra;						\
														\
	if ( !extra->continued )							\
	{													\
		yylloc_param->first_line = extra->curr_line;			\
		yylloc_param->first_column = extra->curr_column;		\
	}													\
														\
	extra->continued = 0;								\
														\
	for ( i = 0; i < yyleng; ++i )						\
	{													\
		if ( yytext[ i ] == '\n' )						\
		{												\
			extra->curr_line++;							\
			extra->curr_column = 1;						\
		}												\
		else 											\
		{ 												\
			extra->curr_column++; 						\
		}												\
	}													\
														\
	yylloc_param->last_line = extra->curr_line;				\
	yylloc_param->last_column = extra->curr_column;			\
}														\

void yyerror( YYLTYPE* loc, yyscan_t scanner, SubsDefNode** main_node, char* msg )
{
	printf( "Error (%d %d): %s\n", loc->first_line, loc->first_column, msg );
}

void Error( const char* a_message )
{
	printf( "Error " );
	printf( ": %s\n", a_message );
}

int 		continued = 0;
int 		commentDet = 0;
std::string tmpStr;
int 		tmpCh;

%}


LETTER			[a-zA-Z_?$@.#~]
DIGIT			[0-9]
IDENT			{LETTER}({LETTER}|{DIGIT})*
HEX_SUFF		({DIGIT}|[a-fA-F])+
HEX_NUM 		0x{HEX_SUFF}
DEC_NUM 		{DIGIT}+


%x COMMENTS CHAR_1 CHAR_2 ESCAPE STRING STRING_ESCAPE

%%
	/*Пропускаем пробелы*/
[\n\t ]+

	/*Пропускаем однострочные комментарии*/
\*.*				{ }

	/*Пропускаем многострочные комментарии*/
\(\*				{
						BEGIN( COMMENTS );
						continued  = 1;
						commentDet = 1;
					}
<COMMENTS>\(\*		{
						continued  = 1;
						++commentDet;
					}
<COMMENTS>[^*]* 	continued = 1;
<COMMENTS>\*\)		{
						--commentDet;
						continued = 1;
						if ( commentDet <= 0 )
						{
							if ( commentDet < 0 )
								Error( "Bad multiline comment!" );
							else
								printf( "Multiline comment.\n" );


							BEGIN( 0 );

							continued = 0;
						}


					}

<COMMENTS>\*		continued = 1;
<COMMENTS><<EOF>>	{
						Error( "End of program found, '*)' expected!" );
						commentDet = 0;
					  	return 0;
					}

	/*Зарезервированные слова*/
bool		return BOOL_TYPE;
char  		return CHAR_TYPE;
check 		return CHECK;
do		return DO;
else 		return ELSE;
elseif  	return ELSEIF;
endfor 		return ENDFOR;
endfunc 	return ENDFUNC;
endif  		return ENDIF;
endproc 	return ENDPROC;
endwhile 	return ENDWHILE;
ff 		return FF;
for 		return FOR;
func 		return FUNC;
if 		return IF;
int 		return INT_TYPE;
nil 		return NIL;
proc 		return PROC;
repeat 		return REPEAT;
step 		return STEP;
then 		return THEN;
to 		return TO;
tt 		return TT;
until 		return UNTIL;
while 		return WHILE;
print		return PRINT;

{IDENT}		{
				yylval->m_ident = new char[ strlen( yytext ) + 1 ];
				strcpy( yylval->m_ident, yytext );
				return IDENT;
			}

\(		return LPAREN;
\)		return RPAREN;
\[		return RLPAREN;
\]  	return RRPAREN;
,		return COMMA;
;		return SEMICOLON;
->		return POINTER_METHOD;
=		return ASSIGN;
\+		return PLUS;
-		return MINUS;
\/		return DIVIDE;
%		return MOD;
\^		return CAP;
!		return EXCL;
== 		return EQ;
!=		return NOT_EQ;
\<=		return LSS_EQ;
\>=		return GTR_EQ;
\<		return LSS;
\>		return GTR;
&&		return LOG_AND;
\|\| 	return LOG_OR;
\^\^	return LOG_CAP;

{HEX_NUM}	{
				char* numStart = &yytext[ 2 ];

				while ( (*numStart) == '0' )
					++numStart;

				if ( strlen( numStart ) == 0 )
					--numStart;
				else if ( strlen( numStart ) > 8 )
				{
					Error( "Numeric literal too big!" );
					numStart[ 8 ] = 0;
				}

				sscanf( numStart, "%x", &yylval->m_number );

				return NUMBER;
			}


{DEC_NUM} 	{	bool tooBig = false;

				char* numStart = yytext;

				while ( (*numStart) == '0' )
					++numStart;

				if ( strlen( numStart ) == 0 )
					--numStart;
				else if ( strlen( numStart ) > 10 )
				{
					numStart[ 10 ] = 0;
					tooBig = true;
				}

				if ( ( strlen( numStart ) == 10 ) && ( strcmp( numStart, "2147483647" ) > 0 ) )
				{
					numStart[ 9 ] = 0;
					tooBig = true;
				}

				if ( tooBig )
					Error( "Numeric literal too big!" );

				yylval->m_number = atoi( numStart );

				return NUMBER;
			}

\' 						{
							BEGIN( CHAR_1 );
							continued = 0;
						}

<CHAR_1,CHAR_2>\n 		{
							Error( "New line in char constant!" );
							BEGIN( 0 );
							yylval->m_ch = 0;
							continued 	 = 0;
							return CHAR;
						}

<CHAR_1,CHAR_2><<EOF>>	{
							Error( "Expected constant char!" );
							BEGIN( 0 );
							yylval->m_ch = 0;
							continued 	 = 0;
							return CHAR;
						}


<CHAR_1>\'				{
							Error( "Empty character constant!" );
							BEGIN( 0 );
							yylval->m_ch = 0;
							continued 	 = 0;
							return CHAR;
						}

<CHAR_1>\$ 				{
							BEGIN( ESCAPE );
							continued = 1;
						}

<ESCAPE>['"]{1}\$		{
							yylval->m_ch = yytext[ 0 ];
							continued 	 = 1;
							BEGIN( CHAR_2 );
						}

<ESCAPE><\$				{
							yylval->m_ch = 8;
							continued 	 = 1;
							BEGIN( CHAR_2 );
						}

<ESCAPE>>\$				{
							yylval->m_ch = 36;
							continued 	 = 1;
							BEGIN( CHAR_2 );
						}


<ESCAPE>@\$				{
							yylval->m_ch = 7;
							continued 	 = 1;
							BEGIN( CHAR_2 );
						}


<ESCAPE>\^\$			{
							yylval->m_ch = '\n';
							continued 	 = 1;
							BEGIN( CHAR_2 );
						}

<ESCAPE><<\$				{
							yylval->m_ch = '\r';
							continued = 1;
							BEGIN( CHAR_2 );
						}

<ESCAPE>{HEX_SUFF}\$	{ 	char* numStart = yytext;

							while ( (*numStart) == '0' )
								++numStart;

							if ( strlen( numStart ) == 0 )
								--numStart;
							else if ( strlen( numStart ) > 9 )
								Error( "Numeric literal to char too big!" );

							numStart[ 9 ] = 0;                                                        
                                                        sscanf( yytext, "%x", &tmpCh);
                                                        yylval->m_ch = tmpCh;
							continued = 1;
							BEGIN( CHAR_2 );
						}

<ESCAPE>\$	 			{
							BEGIN( CHAR_2 );
							yylval->m_ch = 0;
							continued 	 = 1;
						}

<ESCAPE>\'				{
							BEGIN( 0 );
							yylval->m_ch = '$';
							continued 	 = 0;
							return CHAR;
						}

<CHAR_1>.				{
							BEGIN( CHAR_2 );
							yylval->m_ch = yytext[ 0 ];
							continued 	 = 1;
						}

<CHAR_2>\'				{
							BEGIN( 0 );
							continued = 0;
							return CHAR;
						}

<ESCAPE,CHAR_2>[^\n\']*	{
							Error( "Too many symbols in character constant!" );
							BEGIN( 0 );
							continued = 1;
							return CHAR;
						}

\"								{
									BEGIN( STRING );
									continued = 0;
									tmpStr = "";
								}

<STRING>[^\"\$]*\n				{
									Error( "New line in string constant!" );
									BEGIN( 0 );
									tmpStr += yytext;

									yylval->m_str = new char [ tmpStr.length() + 1 ];
									strcpy( yylval->m_str, tmpStr.c_str() );

									return STR;
								}

<STRING>\$						{
									BEGIN( STRING_ESCAPE );
									continued = 1;
								}

<STRING_ESCAPE>[ \t]*\n   		{
									BEGIN( STRING );
									continued = 1;
								}

<STRING_ESCAPE>[@<>'"]{1}\$		{
									tmpStr += yytext[ 0 ];
									continued 	 = 1;
									BEGIN( STRING );
								}

<STRING_ESCAPE>^\$				{
									tmpStr += '\n';
									continued 	 = 1;
									BEGIN( STRING );
								}

<STRING_ESCAPE><<\$				{
									tmpStr += '\r';
									continued 	 = 1;
									BEGIN( STRING );
								}

<STRING_ESCAPE>{HEX_SUFF}\$                     {
                                                    sscanf( yytext, "%x", &tmpCh );
                                                    tmpStr += (char)tmpCh;
                                                    continued = 1;
                                                    BEGIN( STRING );
                                                }

<STRING_ESCAPE>\$				{
									BEGIN( STRING );
									continued = 1;
								}

<STRING>[^\"\$\n]+				{
									continued = 1;
									tmpStr += yytext;
								}

<STRING>\"						{
									BEGIN( 0 );
									yylval->m_str = (char*)tmpStr.c_str();
									return STR;
								}

. 								{ printf( "%d = '%c'\n", yytext[ 0 ], yytext[ 0 ] ); Error( "Unexpected character!" );}

%%

void init_scanner( char* program, yyscan_t* scanner, struct Extra* extra )
{
	extra->continued = 0;
	extra->curr_line = 1;
	extra->curr_column = 1;
	yylex_init( scanner );
	yylex_init_extra( extra, scanner );
	yy_scan_string( program, *scanner );
}

void destroy_scanner( yyscan_t scanner )
{
	yylex_destroy( scanner );
}
