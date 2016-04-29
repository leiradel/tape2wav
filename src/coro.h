#ifndef CORO_H
#define CORO_H

/* Use at the beginning of the coroutine, you must have declared a variable coro_t* coro */
#define CORO_ENTER() \
  { \
  scope_t* CORO_scope; \
  CORO_again: \
  CORO_scope = coro->stack + coro->sp; \
  switch ( coro->step ) { \
  case CORO_BEGIN:

/* Use to define labels which are targets to GOTO and GOSUB */
#define CORO_LABEL( x ) \
  case x:

/* Use at the end of the coroutine, x is the return value */
#define CORO_LEAVE( x ) \
  } \
  } \
  do { return ( x ); } while ( 0 )

/* Go to the x label */
#define CORO_GOTO( x ) \
  do { \
    coro->step = ( x ); \
    goto CORO_again; \
  } while ( 0 )

/* Go to a subroutine, execution continues until the subroutine returns via RET */
/* x is the subroutine label, y and z are the A and B arguments */
#define CORO_GOSUB( x, y, z ) \
  do { \
    coro->sp++; \
    coro->stack[ coro->sp ].ret = __LINE__; \
    coro->stack[ coro->sp ].a = ( y ); \
    coro->stack[ coro->sp ].b = ( z ); \
    coro->step = ( x ); \
    goto CORO_again; \
    case __LINE__: ; \
  } while ( 0 )

/* Returns from a subroutine */
#define CORO_RET() \
  do { \
    coro->step = coro->stack[ coro->sp-- ].ret; \
    goto CORO_again; \
  } while ( 0 )

/* Yields to the caller, execution continues from this point when the coroutine is resumed */
#define CORO_YIELD( x ) \
  do { \
    coro->step = __LINE__; \
    return ( x ); \
    case __LINE__: ; \
  } while ( 0 )

/* Local variables */
#define CORO_A ( CORO_scope->a )
#define CORO_B ( CORO_scope->b )
#define CORO_I ( CORO_scope->i )
#define CORO_J ( CORO_scope->j )
#define CORO_K ( CORO_scope->k )

/* The coroutine entry point, never use 0 as a label */
#define CORO_BEGIN 0

/* Sets up a coroutine, x is a pointer to coro_t */
#define CORO_SETUP( x ) \
  do { \
    ( x )->step = CORO_BEGIN; \
    ( x )->sp = 0; \
  } while ( 0 )

/* A coroutine CORO_scope */
typedef struct
{
  int ret, a, b, i, j, k;
}
scope_t;

/* A coroutine */
typedef struct
{
  int step, sp;
  scope_t stack[ 8 ];
}
coro_t;

#endif /* CORO_H */
