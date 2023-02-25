
typedef union  {
	Node	*p;
	Cell	*cp;
	int	i;
	uuchar	*s;
} YYSTYPE;
extern YYSTYPE yylval;
# define FIRSTTOKEN 257
# define PROGRAM 258
# define PASTAT 259
# define PASTAT2 260
# define XBEGIN 261
# define XEND 262
# define NL 263
# define ARRAY 264
# define MATCH 265
# define NOTMATCH 266
# define MATCHOP 267
# define FINAL 268
# define DOT 269
# define ALL 270
# define CCL 271
# define NCCL 272
# define CHAR 273
# define OR 274
# define STAR 275
# define QUEST 276
# define PLUS 277
# define SHIFT_CHAR 278
# define AND 279
# define BOR 280
# define APPEND 281
# define EQ 282
# define GE 283
# define GT 284
# define LE 285
# define LT 286
# define NE 287
# define IN 288
# define ARG 289
# define BLTIN 290
# define BREAK 291
# define CLOSE 292
# define CONTINUE 293
# define DELETE 294
# define DO 295
# define EXIT 296
# define FOR 297
# define FUNC 298
# define SUB 299
# define GSUB 300
# define IF 301
# define INDEX 302
# define LSUBSTR 303
# define MATCHFCN 304
# define NEXT 305
# define TOUPPER 306
# define TOLOWER 307
# define ADD 308
# define MINUS 309
# define MULT 310
# define DIVIDE 311
# define MOD 312
# define ASSIGN 313
# define ASGNOP 314
# define ADDEQ 315
# define SUBEQ 316
# define MULTEQ 317
# define DIVEQ 318
# define MODEQ 319
# define POWEQ 320
# define PRINT 321
# define PRINTF 322
# define SPRINTF 323
# define ELSE 324
# define INTEST 325
# define CONDEXPR 326
# define POSTINCR 327
# define PREINCR 328
# define POSTDECR 329
# define PREDECR 330
# define VAR 331
# define IVAR 332
# define VARNF 333
# define CALL 334
# define NUMBER 335
# define STRING 336
# define FIELD 337
# define REGEXPR 338
# define GETLINE 339
# define RETURN 340
# define SPLIT 341
# define SUBSTR 342
# define WHILE 343
# define CAT 344
# define NOT 345
# define UMINUS 346
# define POWER 347
# define DECR 348
# define INCR 349
# define INDIRECT 350
# define LASTTOKEN 351
